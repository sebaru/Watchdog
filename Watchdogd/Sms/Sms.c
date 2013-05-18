/**********************************************************************************************************/
/* Watchdogd/sms.c        Gestion des SMS de Watchdog v2.0                                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                   ven. 02 avril 2010 20:37:40 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Sms.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <string.h>
 #include <unistd.h>
 #include <gnokii.h>
 #include <curl/curl.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sms.h"
 #define PRESMS   "CDE:"

/**********************************************************************************************************/
/* Sms_Lire_config : Lit la config Watchdog et rempli la structure mémoire                               */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Sms_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;
    GError *error;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, &error) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Sms_Lire_config : unable to load config file %s: %s", Config.config_file, error->message );
       g_error_free(error);       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_sms.lib->Thread_debug = g_key_file_get_boolean ( gkf, "SMS", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */
    chaine = g_key_file_get_string ( gkf, "SMS", "smsbox_username", NULL );
    if (!chaine)
     { Info_new ( Config.log, Cfg_sms.lib->Thread_debug, LOG_ERR,
                  "Sms_Lire_config: smsbox_username is missing. Using default." );
       g_snprintf( Cfg_sms.smsbox_username, sizeof(Cfg_sms.smsbox_username), DEFAUT_SMSBOX_USERNAME );
     }
    else
     { g_snprintf( Cfg_sms.smsbox_username, sizeof(Cfg_sms.smsbox_username), "%s", chaine );
       g_free(chaine);
     }

    chaine = g_key_file_get_string ( gkf, "SMS", "smsbox_password", NULL );
    if (!chaine)
     { Info_new ( Config.log, Cfg_sms.lib->Thread_debug, LOG_ERR,
                  "Sms_Lire_config: smsbox_password is missing. Using default." );
       g_snprintf( Cfg_sms.smsbox_password, sizeof(Cfg_sms.smsbox_password), DEFAUT_SMSBOX_PASSWORD );
     }
    else
     { g_snprintf( Cfg_sms.smsbox_password, sizeof(Cfg_sms.smsbox_password), "%s", chaine );
       g_free(chaine);
     }

    Cfg_sms.recipients = g_key_file_get_string_list ( gkf, "SMS", "recipients", NULL, NULL );
    if (!Cfg_sms.recipients)
     { Info_new ( Config.log, Cfg_sms.lib->Thread_debug, LOG_ERR,
                  "Sms_Lire_config: recipients are missing." );
     }

    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Sms_Liberer_config : Libere la mémoire allouer précédemment pour lire la config sms                  */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Sms_Liberer_config ( void )
  { g_strfreev ( Cfg_sms.recipients );
  }
/**********************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Envoyer_sms ( struct CMD_TYPE_MESSAGE *msg )
  { struct CMD_TYPE_MESSAGE *copie;
    gint nbr;

    pthread_mutex_lock( &Cfg_sms.lib->synchro );      /* On recupere le nombre de sms en attente */
    nbr = g_slist_length(Cfg_sms.Liste_sms);
    pthread_mutex_unlock( &Cfg_sms.lib->synchro );

    if (nbr > 50)
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                "Envoyer_sms: liste d'attente pleine" );
       return;
     }

    copie = (struct CMD_TYPE_MESSAGE *) g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!copie) { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_ERR,
                 "Envoyer_sms: pas assez de mémoire pour copie" );
                  return;
                }
    memcpy ( copie, msg, sizeof(struct CMD_TYPE_MESSAGE) );

    pthread_mutex_lock( &Cfg_sms.lib->synchro );
    Cfg_sms.Liste_sms = g_slist_append ( Cfg_sms.Liste_sms, copie );
    pthread_mutex_unlock( &Cfg_sms.lib->synchro );
  }
/**********************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Envoyer_sms_smsbox_text ( gchar *texte )
  { struct CMD_TYPE_MESSAGE *msg;

    msg = (struct CMD_TYPE_MESSAGE *) g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!msg) { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_ERR,
                         "Envoyer_sms_smsbox_text: pas assez de mémoire pour copie" );
                return;
              }

    g_snprintf(msg->libelle_sms, sizeof(msg->libelle_sms), "%s", texte );
    msg->id  = 0;
    msg->num = 0;
    msg->enable = TRUE;
    msg->sms = MSG_SMS_SMSBOX;

    Envoyer_sms ( msg );
  }
/**********************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Envoyer_sms_gsm_text ( gchar *texte )
  { struct CMD_TYPE_MESSAGE *msg;

    msg = (struct CMD_TYPE_MESSAGE *) g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!msg) { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_ERR,
                         "Envoyer_sms_gsm_text: pas assez de mémoire pour copie" );
                return;
              }

    g_snprintf(msg->libelle_sms, sizeof(msg->libelle_sms), "%s", texte );
    msg->id  = 0;
    msg->num = 0;
    msg->enable = TRUE;
    msg->sms = MSG_SMS_GSM;

    Envoyer_sms ( msg );
  }
/**********************************************************************************************************/
/* Sms_Gerer_message: Fonction d'abonné appellé lorsqu'un message est disponible.                         */
/* Entrée: une structure CMD_TYPE_HISTO                                                                   */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Sms_Gerer_message ( struct CMD_TYPE_MESSAGE *msg )
  { gint taille;

    if ( ! msg->sms ) { g_free(msg); return; }                           /* Si flag = 0; on return direct */

    pthread_mutex_lock( &Cfg_sms.lib->synchro );                         /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_sms.Liste_sms );
    pthread_mutex_unlock( &Cfg_sms.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                "Sms_Gerer_message: DROP message %D (length = %d > 150)", msg->num, taille);
       g_free(msg);
       return;
     }

    pthread_mutex_lock ( &Cfg_sms.lib->synchro );
    Cfg_sms.Liste_sms = g_slist_append ( Cfg_sms.Liste_sms, msg );                    /* Ajout a la liste */
    pthread_mutex_unlock ( &Cfg_sms.lib->synchro );
  }
/**********************************************************************************************************/
/* Sms_recipient_authorized : Renvoi TRUE si Watchdog peut recevoir/emettre au destinataire en parametre  */
/* Entrée: le nom du destinataire                                                                         */
/* Sortie : booléen, TRUE/FALSE                                                                           */
/**********************************************************************************************************/
 static gboolean Sms_recipient_authorized ( const gchar *tel )
  { gchar **liste;
    gint cpt = 0;
    if (!Cfg_sms.recipients) return(FALSE);                        /* Si aucun destinataire, retour FALSE */
    liste = Cfg_sms.recipients;
    while (liste[cpt])
     { if ( ! strncmp ( tel, liste[cpt], strlen(liste[cpt]) ) ) return(TRUE);
       cpt++;
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Traiter_commande_sms: Fonction appeler pour traiter la commande sms recu par le telephone              */
/* Entrée: le message text à traiter                                                                      */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Traiter_commande_sms ( gchar *from, gchar *texte )
  { struct DB *db;

    if ( Sms_recipient_authorized ( from ) == FALSE )
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_NOTICE,
                "Traiter_commande_sms : unknown sender %s. Dropping message %s...", from, texte );
       return;
     }

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                 "Traiter_commande_sms : Connexion DB failed. sms not handled" );
       return;
     }
    if ( ! Recuperer_mnemoDB_by_command_text ( Config.log, db, (gchar *)texte ) )
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                 "Traiter_commande_sms : Error searching Database" );
     }
    else 
     { struct CMD_TYPE_MNEMONIQUE *mnemo, *result_mnemo;
          
       for ( result_mnemo = NULL ; ; )
        { mnemo = Recuperer_mnemoDB_suite( Config.log, db );
          if (!mnemo) break;
          if (db->nbr_result!=1) g_free(mnemo);
                            else result_mnemo = mnemo;
        }
       if (result_mnemo)
        { switch ( result_mnemo->type )
           { case MNEMO_MONOSTABLE:                                      /* Positionnement du bit interne */
                  Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_NOTICE,
                             "Traiter_commande_sms: Mise a un du bit M%03d = 1", result_mnemo->num );
                  Envoyer_commande_dls(result_mnemo->num); 
                  break;
             case MNEMO_ENTREE:
                  break;
             case MNEMO_ENTREE_ANA:
                  break;
             default: Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_NOTICE,
                               "Traiter_commande_sms: Cannot handle commande type %d (num=%03d)",
                                result_mnemo->type, result_mnemo->num );
                      break;
           }
          g_free(result_mnemo);
        }
     }
    Libere_DB_SQL( Config.log, &db );
  }
/**********************************************************************************************************/
/* Lire_sms_gsm: Lecture de tous les SMS du GSM                                                           */
/* Entrée: Rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Lire_sms_gsm ( void )
  { struct gn_statemachine *state;
    gn_sms_folder folder;
    gn_sms_folder_list folderlist;
    gn_error error;
    gn_data data;
    gn_sms sms;
    gint sms_index;

    if ((error=gn_lib_phoneprofile_load("", &state)) != GN_ERR_NONE)               /* Read config file */
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                "Lire_sms_gsm: Read Phone profile NOK (%s)", gn_error_print(error) );
       return;
     }

    if ((error=gn_lib_phone_open(state)) != GN_ERR_NONE)
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                "Lire_sms_gsm: Open Phone NOK (%s)", gn_error_print(error) );
       gn_lib_phone_close(state);
       gn_lib_phoneprofile_free(&state);
       gn_lib_library_free();
       return;
     }

    gn_data_clear(&data);

    folder.folder_id = 0;
    data.sms_folder_list = &folderlist;
    data.sms_folder = &folder;

    memset ( &sms, 0, sizeof(gn_sms) );
    sms.memory_type = gn_str2memory_type("ME");       /* On recupere les SMS du Mobile equipment (pas SM) */
    data.sms = &sms;

    for (sms_index=1; ;sms_index++)
     { sms.number = sms_index;

       if ((error = gn_sms_get (&data, state)) == GN_ERR_NONE)                      /* On recupere le SMS */
        { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_NOTICE,
                   "Lire_sms_gsm: Recu SMS %s de %s", (gchar *)sms.user_data[0].u.text, sms.remote.number );
          Traiter_commande_sms ( sms.remote.number, (gchar *)sms.user_data[0].u.text );
          gn_sms_delete (&data, state);                               /* On l'a traité, on peut l'effacer */
        }
       else if (error == GN_ERR_INVALIDLOCATION) break;       /* On regarde toutes les places de stockage */
       else  { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                        "Lire_sms_gsm: error %s from %s (sms_index=%d)",
                        gn_error_print(error), sms.remote.number, sms_index );
               break;
             }
     }

    gn_lib_phone_close(state);
    gn_lib_phoneprofile_free(&state);
    gn_lib_library_free();
  }
/**********************************************************************************************************/
/* Envoi_sms_gsm: Envoi un sms par le gsm                                                                 */
/* Entrée: le message à envoyer sateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Envoi_sms_gsm ( struct CMD_TYPE_MESSAGE *msg, gchar *telephone )
  { struct gn_statemachine *state;
    gn_error error;
    gn_data data;
    gn_sms sms;

    if ((error=gn_lib_phoneprofile_load("", &state)) != GN_ERR_NONE)               /* Read config file */
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                "Envoi_sms_gsm: Read Phone profile NOK (%s)", gn_error_print(error) );
       return;
     }

    if ((error=gn_lib_phone_open(state)) != GN_ERR_NONE)
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                "Envoi_sms_gsm: Open Phone NOK (%s)", gn_error_print(error) );
       return;
     }

    gn_data_clear(&data);

    gn_sms_default_submit(&sms);                                             /* The memory is zeroed here */

    memset(&sms.remote.number, 0, sizeof(sms.remote.number));
    strncpy(sms.remote.number, telephone, sizeof(sms.remote.number) - 1);               /* Number a m'man */
    if (sms.remote.number[0] == '+') 
         { sms.remote.type = GN_GSM_NUMBER_International; }
    else { sms.remote.type = GN_GSM_NUMBER_Unknown; }

    if (!sms.smsc.number[0])                                                      /* Récupération du SMSC */
     { data.message_center = g_malloc0(sizeof(gn_sms_message_center));
       data.message_center->id = 1;
       if (gn_sm_functions(GN_OP_GetSMSCenter, &data, state) == GN_ERR_NONE)
        { strcpy(sms.smsc.number, data.message_center->smsc.number);
          sms.smsc.type = data.message_center->smsc.type;
        }
       else
        { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING, "Envoi_sms_gsm: Pb avec le SMSC" ); }
       g_free(data.message_center);
     }

    if (!sms.smsc.type) sms.smsc.type = GN_GSM_NUMBER_Unknown;

    sms.user_data[0].length = g_snprintf( (gchar *)sms.user_data[0].u.text, sizeof (sms.user_data[0].u.text),
                                          "%s", msg->libelle_sms );
        
    sms.user_data[0].type = GN_SMS_DATA_Text;
    if (!gn_char_def_alphabet(sms.user_data[0].u.text))
     { sms.dcs.u.general.alphabet = GN_SMS_DCS_8bit; }
                                                  /* 18/08/12 Test de passage en '8bit' au lieu de 'UCS2' */

    sms.user_data[1].type = GN_SMS_DATA_None;
/*	sms.delivery_report = true; */
    data.sms = &sms;                                                                      /* Envoi du SMS */

    error = gn_sms_send(&data, state);
    if (error == GN_ERR_NONE)
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_INFO, "Envoi_sms_gsm: Envoi SMS Ok %s", msg->libelle_sms ); }
    else
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                "Envoi_sms_gsm: Envoi SMS Nok (%s)", gn_error_print(error)); }

    gn_lib_phone_close(state);
    gn_lib_phoneprofile_free(&state);
    gn_lib_library_free();
    sleep(5);                                         /* Attente de 5 secondes pour ne pas saturer le GSM */
  }
/**********************************************************************************************************/
/* Envoi_sms_smsbox: Envoi un sms par SMSBOX                                                              */
/* Entrée: le message à envoyer sateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Envoi_sms_smsbox ( struct CMD_TYPE_MESSAGE *msg, gchar *telephone )
  { gchar erreur[CURL_ERROR_SIZE+1];
    struct curl_httppost *formpost;
    struct curl_httppost *lastptr;
    CURLcode res;
    CURL *curl;
    
    formpost = lastptr = NULL;
    curl_formadd( &formpost, &lastptr,
                  CURLFORM_COPYNAME,     "login",
                  CURLFORM_COPYCONTENTS, Cfg_sms.smsbox_username,
                  CURLFORM_END); 
    curl_formadd( &formpost, &lastptr,
                  CURLFORM_COPYNAME,     "pass",
                  CURLFORM_COPYCONTENTS, Cfg_sms.smsbox_password,
                  CURLFORM_END); 
    curl_formadd( &formpost, &lastptr,
                  CURLFORM_COPYNAME,     "msg",
                  CURLFORM_COPYCONTENTS, msg->libelle_sms,
                  CURLFORM_END); 
    curl_formadd( &formpost, &lastptr,
                  CURLFORM_COPYNAME,     "dest",
                  CURLFORM_COPYCONTENTS, telephone,
                  CURLFORM_END); 
    if (Partage->top < TOP_MIN_ENVOI_SMS)
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_INFO,
                "Envoi_sms_smsbox: Envoi trop tot !! (%s)", msg->libelle_sms );
       curl_formadd( &formpost, &lastptr,       /* Pas de SMS les 2 premières minutes de vie du processus */
                     CURLFORM_COPYNAME,     "origine",          /* 'debugvar' pour lancer en mode semonce */
                     CURLFORM_COPYCONTENTS, "debugvar",
                     CURLFORM_END);
     }
    else
     { curl_formadd( &formpost, &lastptr,       /* Pas de SMS les 2 premières minutes de vie du processus */
                     CURLFORM_COPYNAME,     "origine",          /* 'debugvar' pour lancer en mode semonce */
                     CURLFORM_COPYCONTENTS, VERSION,
                     CURLFORM_END);
     }


    curl_formadd( &formpost, &lastptr,
                  CURLFORM_COPYNAME,     "mode",
                  CURLFORM_COPYCONTENTS, "Economique",
                  CURLFORM_END);

    curl = curl_easy_init();
    if (curl)
     { curl_easy_setopt(curl, CURLOPT_URL, "https://api.smsbox.fr/api.php" );
       curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
       curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, erreur );
       curl_easy_setopt(curl, CURLOPT_VERBOSE, 1 );
       res = curl_easy_perform(curl);
       if (!res)
        { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_INFO,
                   "Envoi_sms_smsbox: Envoi SMS %s to %s", msg->libelle_sms, telephone );
        }
       else
        { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                   "Envoi_sms_smsbox: Envoi SMS Nok - Pb cURL (%s)", erreur);
        }
       curl_easy_cleanup(curl);
     }
    else
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                "Envoi_sms_smsbox: Envoi SMS Nok - Pb cURL Init");
     }
    curl_formfree(formpost);
  }
/**********************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct CMD_TYPE_MESSAGE *msg;
    
    prctl(PR_SET_NAME, "W-SMS", 0, 0, 0 );
    memset( &Cfg_sms, 0, sizeof(Cfg_sms) );                     /* Mise a zero de la structure de travail */
    Cfg_sms.lib = lib;                         /* Sauvegarde de la structure pointant sur cette librairie */
    Sms_Lire_config ();                                 /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );
    Cfg_sms.lib->Thread_run = TRUE;                                                 /* Le thread tourne ! */

    g_snprintf( Cfg_sms.lib->admin_prompt, sizeof(Cfg_sms.lib->admin_prompt), "sms" );
    g_snprintf( Cfg_sms.lib->admin_help,   sizeof(Cfg_sms.lib->admin_help),   "Manage SMS system" );

    Abonner_distribution_message ( Sms_Gerer_message );               /* Abonnement à la diffusion des messages */

    while(Cfg_sms.lib->Thread_run == TRUE)                               /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_sms.lib->Thread_sigusr1)                                   /* A-t'on recu un signal USR1 ? */
        { int nbr;

          Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_INFO, "Run_thread: SIGUSR1" );
          pthread_mutex_lock( &Cfg_sms.lib->synchro );         /* On recupere le nombre de sms en attente */
          nbr = g_slist_length(Cfg_sms.Liste_sms);
          pthread_mutex_unlock( &Cfg_sms.lib->synchro );
          Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_INFO, "Run_thread: Nbr SMS a envoyer = %d", nbr );
          Cfg_sms.lib->Thread_sigusr1 = FALSE;
        }

/********************************************** Lecture de SMS ********************************************/
       Lire_sms_gsm();

/************************************************ Envoi de SMS ********************************************/
       if ( !Cfg_sms.Liste_sms )                                        /* Attente de demande d'envoi SMS */
        { sleep(5);
          sched_yield();
          continue;
        }

       pthread_mutex_lock( &Cfg_sms.lib->synchro );
       msg = Cfg_sms.Liste_sms->data;
       Cfg_sms.Liste_sms = g_slist_remove ( Cfg_sms.Liste_sms, msg );
       Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_INFO, "Run_thread: Reste %d a envoyer apres le msg %d",
                 g_slist_length(Cfg_sms.Liste_sms), msg->num );
       pthread_mutex_unlock( &Cfg_sms.lib->synchro );
       if ( Partage->g[msg->num].etat )                                 /* On n'envoie que si MSGnum == 1 */
        { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_INFO,
                   "Run_thread : Sending msg %d (%s)", msg->num, msg->libelle_sms );
      
/**************************************** Envoi en mode GSM ***********************************************/

          if (Partage->top < TOP_MIN_ENVOI_SMS)
           { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_INFO,
                      "Envoi_sms_gsm: Envoi trop tot !! (%s)", msg->libelle_sms ); }
          else 
           { gchar **liste;
             gint cpt = 0;
             liste = Cfg_sms.recipients;
             while (liste[cpt])
              { if (msg->sms == MSG_SMS_GSM)    Envoi_sms_gsm   ( msg, liste[cpt] );
                if (msg->sms == MSG_SMS_SMSBOX) Envoi_sms_smsbox( msg, liste[cpt] );
                cpt++;
              }
           }
        }
       g_free( msg );
     }

    Desabonner_distribution_message ( Sms_Gerer_message );  /* Desabonnement de la diffusion des messages */
    Sms_Liberer_config();                                     /* Liberation de la configuration du thread */

    Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_sms.lib->TID = 0;                                 /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
