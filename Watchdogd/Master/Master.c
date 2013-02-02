/**********************************************************************************************************/
/* Watchdogd/master.c        Gestion des MASTER de Watchdog v2.0                                                */
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
 #include "Master.h"
 #define PREMASTER   "CDE:"

/**********************************************************************************************************/
/* Sms_Lire_config : Lit la config Watchdog et rempli la structure mémoire                               */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Sms_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Sms_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_master.lib->Thread_debug = g_key_file_get_boolean ( gkf, "MASTER", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */
    chaine = g_key_file_get_string ( gkf, "MASTER", "masterbox_username", NULL );
    if (!chaine)
     { Info_new ( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                  "Sms_Lire_config: masterbox_username is missing. Using default." );
       g_snprintf( Cfg_master.masterbox_username, sizeof(Cfg_master.masterbox_username), DEFAUT_MASTERBOX_USERNAME );
     }
    else
     { g_snprintf( Cfg_master.masterbox_username, sizeof(Cfg_master.masterbox_username), "%s", chaine );
       g_free(chaine);
     }

    chaine = g_key_file_get_string ( gkf, "MASTER", "masterbox_password", NULL );
    if (!chaine)
     { Info_new ( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                  "Sms_Lire_config: masterbox_password is missing. Using default." );
       g_snprintf( Cfg_master.masterbox_password, sizeof(Cfg_master.masterbox_password), DEFAUT_MASTERBOX_PASSWORD );
     }
    else
     { g_snprintf( Cfg_master.masterbox_password, sizeof(Cfg_master.masterbox_password), "%s", chaine );
       g_free(chaine);
     }

    Cfg_master.recipients = g_key_file_get_string_list ( gkf, "MASTER", "recipients", NULL, NULL );
    if (!Cfg_master.recipients)
     { Info_new ( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                  "Sms_Lire_config: recipients are missing." );
     }

    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Sms_Liberer_config : Libere la mémoire allouer précédemment pour lire la config master                  */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Sms_Liberer_config ( void )
  { g_strfreev ( Cfg_master.recipients );
  }
/**********************************************************************************************************/
/* Envoyer_master: Envoi un master                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Envoyer_master ( struct CMD_TYPE_MESSAGE *msg )
  { struct CMD_TYPE_MESSAGE *copie;
    gint nbr;

    pthread_mutex_lock( &Cfg_master.lib->synchro );      /* On recupere le nombre de master en attente */
    nbr = g_slist_length(Cfg_master.Liste_master);
    pthread_mutex_unlock( &Cfg_master.lib->synchro );

    if (nbr > 50)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Envoyer_master: liste d'attente pleine" );
       return;
     }

    copie = (struct CMD_TYPE_MESSAGE *) g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!copie) { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                 "Envoyer_master: pas assez de mémoire pour copie" );
                  return;
                }
    memcpy ( copie, msg, sizeof(struct CMD_TYPE_MESSAGE) );

    pthread_mutex_lock( &Cfg_master.lib->synchro );
    Cfg_master.Liste_master = g_slist_append ( Cfg_master.Liste_master, copie );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );
  }
/**********************************************************************************************************/
/* Envoyer_master: Envoi un master                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Envoyer_master_masterbox_text ( gchar *texte )
  { struct CMD_TYPE_MESSAGE *msg;

    msg = (struct CMD_TYPE_MESSAGE *) g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!msg) { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                         "Envoyer_master_masterbox_text: pas assez de mémoire pour copie" );
                return;
              }

    g_snprintf(msg->libelle_master, sizeof(msg->libelle_master), "%s", texte );
    msg->id  = 0;
    msg->num = 0;
    msg->enable = TRUE;
    msg->master = MSG_MASTER_MASTERBOX;

    Envoyer_master ( msg );
  }
/**********************************************************************************************************/
/* Envoyer_master: Envoi un master                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Envoyer_master_gsm_text ( gchar *texte )
  { struct CMD_TYPE_MESSAGE *msg;

    msg = (struct CMD_TYPE_MESSAGE *) g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!msg) { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                         "Envoyer_master_gsm_text: pas assez de mémoire pour copie" );
                return;
              }

    g_snprintf(msg->libelle_master, sizeof(msg->libelle_master), "%s", texte );
    msg->id  = 0;
    msg->num = 0;
    msg->enable = TRUE;
    msg->master = MSG_MASTER_GSM;

    Envoyer_master ( msg );
  }
/**********************************************************************************************************/
/* Sms_Gerer_message: Fonction d'abonné appellé lorsqu'un message est disponible.                         */
/* Entrée: une structure CMD_TYPE_HISTO                                                                   */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Sms_Gerer_message ( struct CMD_TYPE_MESSAGE *msg )
  { gint taille;

    if ( ! msg->master ) { g_free(msg); return; }                           /* Si flag = 0; on return direct */

    pthread_mutex_lock( &Cfg_master.lib->synchro );                         /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_master.Liste_master );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Sms_Gerer_message: DROP message %D (length = %d > 150)", msg->num, taille);
       g_free(msg);
       return;
     }

    pthread_mutex_lock ( &Cfg_master.lib->synchro );
    Cfg_master.Liste_master = g_slist_append ( Cfg_master.Liste_master, msg );                    /* Ajout a la liste */
    pthread_mutex_unlock ( &Cfg_master.lib->synchro );
  }
/**********************************************************************************************************/
/* Sms_recipient_authorized : Renvoi TRUE si Watchdog peut recevoir/emettre au destinataire en parametre  */
/* Entrée: le nom du destinataire                                                                         */
/* Sortie : booléen, TRUE/FALSE                                                                           */
/**********************************************************************************************************/
 static gboolean Sms_recipient_authorized ( const gchar *tel )
  { gchar **liste;
    gint cpt = 0;
    if (!Cfg_master.recipients) return(FALSE);                        /* Si aucun destinataire, retour FALSE */
    liste = Cfg_master.recipients;
    while (liste[cpt])
     { if ( ! strncmp ( tel, liste[cpt], strlen(liste[cpt]) ) ) return(TRUE);
       cpt++;
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Traiter_commande_master: Fonction appeler pour traiter la commande master recu par le telephone              */
/* Entrée: le message text à traiter                                                                      */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Traiter_commande_master ( gchar *from, gchar *texte )
  { struct DB *db;

    if ( Sms_recipient_authorized ( from ) == FALSE )
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
                "Traiter_commande_master : unknown sender %s. Dropping message %s...", from, texte );
       return;
     }

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                 "Traiter_commande_master : Connexion DB failed. master not handled" );
       return;
     }
    if ( ! Recuperer_mnemoDB_by_command_text ( Config.log, db, (gchar *)texte ) )
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                 "Traiter_commande_master : Error searching Database" );
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
                  Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
                             "Traiter_commande_master: Mise a un du bit M%03d = 1", result_mnemo->num );
                  Envoyer_commande_dls(result_mnemo->num); 
                  break;
             case MNEMO_ENTREE:
                  break;
             case MNEMO_ENTREE_ANA:
                  break;
             default: Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
                               "Traiter_commande_master: Cannot handle commande type %d (num=%03d)",
                                result_mnemo->type, result_mnemo->num );
                      break;
           }
          g_free(result_mnemo);
        }
     }
    Libere_DB_SQL( Config.log, &db );
  }
/**********************************************************************************************************/
/* Lire_master_gsm: Lecture de tous les MASTER du GSM                                                           */
/* Entrée: Rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Lire_master_gsm ( void )
  { struct gn_statemachine *state;
    gn_master_folder folder;
    gn_master_folder_list folderlist;
    gn_error error;
    gn_data data;
    gn_master master;
    gint master_index;

    if ((error=gn_lib_phoneprofile_load("", &state)) != GN_ERR_NONE)               /* Read config file */
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Lire_master_gsm: Read Phone profile NOK (%s)", gn_error_print(error) );
       return;
     }

    if ((error=gn_lib_phone_open(state)) != GN_ERR_NONE)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Lire_master_gsm: Open Phone NOK (%s)", gn_error_print(error) );
       gn_lib_phone_close(state);
       gn_lib_phoneprofile_free(&state);
       gn_lib_library_free();
       return;
     }

    gn_data_clear(&data);

    folder.folder_id = 0;
    data.master_folder_list = &folderlist;
    data.master_folder = &folder;

    memset ( &master, 0, sizeof(gn_master) );
    master.memory_type = gn_str2memory_type("ME");       /* On recupere les MASTER du Mobile equipment (pas SM) */
    data.master = &master;

    for (master_index=1; ;master_index++)
     { master.number = master_index;

       if ((error = gn_master_get (&data, state)) == GN_ERR_NONE)                      /* On recupere le MASTER */
        { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
                   "Lire_master_gsm: Recu MASTER %s de %s", (gchar *)master.user_data[0].u.text, master.remote.number );
          Traiter_commande_master ( master.remote.number, (gchar *)master.user_data[0].u.text );
          gn_master_delete (&data, state);                               /* On l'a traité, on peut l'effacer */
        }
       else if (error == GN_ERR_INVALIDLOCATION) break;       /* On regarde toutes les places de stockage */
       else  { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                        "Lire_master_gsm: error %s from %s (master_index=%d)",
                        gn_error_print(error), master.remote.number, master_index );
               break;
             }
     }

    gn_lib_phone_close(state);
    gn_lib_phoneprofile_free(&state);
    gn_lib_library_free();
  }
/**********************************************************************************************************/
/* Envoi_master_gsm: Envoi un master par le gsm                                                                 */
/* Entrée: le message à envoyer sateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Envoi_master_gsm ( struct CMD_TYPE_MESSAGE *msg, gchar *telephone )
  { struct gn_statemachine *state;
    gn_error error;
    gn_data data;
    gn_master master;

    if ((error=gn_lib_phoneprofile_load("", &state)) != GN_ERR_NONE)               /* Read config file */
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Envoi_master_gsm: Read Phone profile NOK (%s)", gn_error_print(error) );
       return;
     }

    if ((error=gn_lib_phone_open(state)) != GN_ERR_NONE)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Envoi_master_gsm: Open Phone NOK (%s)", gn_error_print(error) );
       return;
     }

    gn_data_clear(&data);

    gn_master_default_submit(&master);                                             /* The memory is zeroed here */

    memset(&master.remote.number, 0, sizeof(master.remote.number));
    strncpy(master.remote.number, telephone, sizeof(master.remote.number) - 1);               /* Number a m'man */
    if (master.remote.number[0] == '+') 
         { master.remote.type = GN_GSM_NUMBER_International; }
    else { master.remote.type = GN_GSM_NUMBER_Unknown; }

    if (!master.masterc.number[0])                                                      /* Récupération du MASTERC */
     { data.message_center = g_malloc0(sizeof(gn_master_message_center));
       data.message_center->id = 1;
       if (gn_sm_functions(GN_OP_GetMASTERCenter, &data, state) == GN_ERR_NONE)
        { strcpy(master.masterc.number, data.message_center->masterc.number);
          master.masterc.type = data.message_center->masterc.type;
        }
       else
        { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING, "Envoi_master_gsm: Pb avec le MASTERC" ); }
       g_free(data.message_center);
     }

    if (!master.masterc.type) master.masterc.type = GN_GSM_NUMBER_Unknown;

    master.user_data[0].length = g_snprintf( (gchar *)master.user_data[0].u.text, sizeof (master.user_data[0].u.text),
                                          "%s", msg->libelle_master );
        
    master.user_data[0].type = GN_MASTER_DATA_Text;
    if (!gn_char_def_alphabet(master.user_data[0].u.text))
     { master.dcs.u.general.alphabet = GN_MASTER_DCS_8bit; }
                                                  /* 18/08/12 Test de passage en '8bit' au lieu de 'UCS2' */

    master.user_data[1].type = GN_MASTER_DATA_None;
/*	master.delivery_report = true; */
    data.master = &master;                                                                      /* Envoi du MASTER */

    error = gn_master_send(&data, state);
    if (error == GN_ERR_NONE)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO, "Envoi_master_gsm: Envoi MASTER Ok %s", msg->libelle_master ); }
    else
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Envoi_master_gsm: Envoi MASTER Nok (%s)", gn_error_print(error)); }

    gn_lib_phone_close(state);
    gn_lib_phoneprofile_free(&state);
    gn_lib_library_free();
    sleep(5);                                         /* Attente de 5 secondes pour ne pas saturer le GSM */
  }
/**********************************************************************************************************/
/* Envoi_master_masterbox: Envoi un master par MASTERBOX                                                              */
/* Entrée: le message à envoyer sateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Envoi_master_masterbox ( struct CMD_TYPE_MESSAGE *msg, gchar *telephone )
  { gchar erreur[CURL_ERROR_SIZE+1];
    struct curl_httppost *formpost;
    struct curl_httppost *lastptr;
    CURLcode res;
    CURL *curl;
    
    formpost = lastptr = NULL;
    curl_formadd( &formpost, &lastptr,
                  CURLFORM_COPYNAME,     "login",
                  CURLFORM_COPYCONTENTS, Cfg_master.masterbox_username,
                  CURLFORM_END); 
    curl_formadd( &formpost, &lastptr,
                  CURLFORM_COPYNAME,     "pass",
                  CURLFORM_COPYCONTENTS, Cfg_master.masterbox_password,
                  CURLFORM_END); 
    curl_formadd( &formpost, &lastptr,
                  CURLFORM_COPYNAME,     "msg",
                  CURLFORM_COPYCONTENTS, msg->libelle_master,
                  CURLFORM_END); 
    curl_formadd( &formpost, &lastptr,
                  CURLFORM_COPYNAME,     "dest",
                  CURLFORM_COPYCONTENTS, telephone,
                  CURLFORM_END); 
    if (Partage->top < TOP_MIN_ENVOI_MASTER)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
                "Envoi_master_masterbox: Envoi trop tot !! (%s)", msg->libelle_master );
       curl_formadd( &formpost, &lastptr,       /* Pas de MASTER les 2 premières minutes de vie du processus */
                     CURLFORM_COPYNAME,     "origine",          /* 'debugvar' pour lancer en mode semonce */
                     CURLFORM_COPYCONTENTS, "debugvar",
                     CURLFORM_END);
     }
    else
     { curl_formadd( &formpost, &lastptr,       /* Pas de MASTER les 2 premières minutes de vie du processus */
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
     { curl_easy_setopt(curl, CURLOPT_URL, "https://api.masterbox.fr/api.php" );
       curl_easy_setopt(curl, CURLOPT_HTTPPOST, formpost);
       curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, erreur );
       curl_easy_setopt(curl, CURLOPT_VERBOSE, 1 );
       res = curl_easy_perform(curl);
       if (!res)
        { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
                   "Envoi_master_masterbox: Envoi MASTER %s to %s", msg->libelle_master, telephone );
        }
       else
        { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                   "Envoi_master_masterbox: Envoi MASTER Nok - Pb cURL (%s)", erreur); }
     }
    else
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Envoi_master_masterbox: Envoi MASTER Nok - Pb cURL Init");
     }
    curl_easy_cleanup(curl);
    curl_formfree(formpost);
  }
/**********************************************************************************************************/
/* Envoyer_master: Envoi un master                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct CMD_TYPE_MESSAGE *msg;
    GSList *Liste_master;
    
    prctl(PR_SET_NAME, "W-MASTER", 0, 0, 0 );
    memset( &Cfg_master, 0, sizeof(Cfg_master) );                     /* Mise a zero de la structure de travail */
    Cfg_master.lib = lib;                         /* Sauvegarde de la structure pointant sur cette librairie */
    Sms_Lire_config ();                                 /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );
    Cfg_master.lib->Thread_run = TRUE;                                                 /* Le thread tourne ! */

    g_snprintf( Cfg_master.lib->admin_prompt, sizeof(Cfg_master.lib->admin_prompt), "master" );
    g_snprintf( Cfg_master.lib->admin_help,   sizeof(Cfg_master.lib->admin_help),   "Manage MASTER system" );

    Abonner_distribution_message ( Sms_Gerer_message );               /* Abonnement à la diffusion des messages */

    while(Cfg_master.lib->Thread_run == TRUE)                               /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_master.lib->Thread_sigusr1)                                   /* A-t'on recu un signal USR1 ? */
        { int nbr;

          Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO, "Run_thread: SIGUSR1" );
          pthread_mutex_lock( &Cfg_master.lib->synchro );         /* On recupere le nombre de master en attente */
          nbr = g_slist_length(Cfg_master.Liste_master);
          pthread_mutex_unlock( &Cfg_master.lib->synchro );
          Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO, "Run_thread: Nbr MASTER a envoyer = %d", nbr );
          Cfg_master.lib->Thread_sigusr1 = FALSE;
        }

/********************************************** Lecture de MASTER ********************************************/
       Lire_master_gsm();

/************************************************ Envoi de MASTER ********************************************/
       if ( !Cfg_master.Liste_master )                                        /* Attente de demande d'envoi MASTER */
        { sleep(5);
          sched_yield();
          continue;
        }

       pthread_mutex_lock( &Cfg_master.lib->synchro );
       Liste_master = Cfg_master.Liste_master;                         /* Sauvegarde du ptr master a envoyer */
       msg = Liste_master->data;
       pthread_mutex_unlock( &Cfg_master.lib->synchro );
       Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
                "Run_thread : Sending msg %d (%s)", msg->num, msg->libelle_master );
      
/**************************************** Envoi en mode GSM ***********************************************/

       if (Partage->top < TOP_MIN_ENVOI_MASTER)
        { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
                   "Envoi_master_gsm: Envoi trop tot !! (%s)", msg->libelle_master ); }
       else 
        { gchar **liste;
          gint cpt = 0;
          liste = Cfg_master.recipients;
          while (liste[cpt])
           { if (msg->master == MSG_MASTER_GSM)    Envoi_master_gsm   ( msg, liste[cpt] );
             if (msg->master == MSG_MASTER_MASTERBOX) Envoi_master_masterbox( msg, liste[cpt] );
             cpt++;
           }
        }

       pthread_mutex_lock( &Cfg_master.lib->synchro );
       Cfg_master.Liste_master = g_slist_remove ( Cfg_master.Liste_master, msg );
       Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO, "Run_thread: Reste %d a envoyer",
                 g_slist_length(Cfg_master.Liste_master) );
       pthread_mutex_unlock( &Cfg_master.lib->synchro );
       g_free( msg );
     }

    Desabonner_distribution_message ( Sms_Gerer_message );  /* Desabonnement de la diffusion des messages */
    Sms_Liberer_config();                                     /* Liberation de la configuration du thread */

    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_master.lib->TID = 0;                                 /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
