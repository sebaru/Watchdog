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
 
 #include <glib.h>
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <string.h>
 #include <unistd.h>
 #include <gnokii.h>
 #include <curl/curl.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #define PRESMS   "CDE:"
/**********************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Envoyer_sms ( struct CMD_TYPE_MESSAGE *msg )
  { struct CMD_TYPE_MESSAGE *copie;
    gint nbr;

    pthread_mutex_lock( &Partage->com_sms.synchro );      /* On recupere le nombre de sms en attente */
    nbr = g_list_length(Partage->com_sms.liste_sms);
    pthread_mutex_unlock( &Partage->com_sms.synchro );

    if (nbr > 50)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Envoyer_sms: liste d'attente pleine" ); return; }

    copie = (struct CMD_TYPE_MESSAGE *) g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!copie) { Info_new( Config.log, Config.log_all, LOG_ERR, "Envoyer_sms: pas assez de mémoire pour copie" ); return; }
    memcpy ( copie, msg, sizeof(struct CMD_TYPE_MESSAGE) );

    pthread_mutex_lock( &Partage->com_sms.synchro );
    Partage->com_sms.liste_sms = g_list_append ( Partage->com_sms.liste_sms, copie );
    pthread_mutex_unlock( &Partage->com_sms.synchro );
  }
/**********************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Envoyer_sms_smsbox_text ( gchar *texte )
  { struct CMD_TYPE_MESSAGE *msg;

    msg = (struct CMD_TYPE_MESSAGE *) g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!msg) { Info_new( Config.log, Config.log_all, LOG_ERR,
                         "Envoyer_sms_smsbox_text: pas assez de mémoire pour copie" );
                return;
              }

    g_snprintf(msg->libelle_sms, sizeof(msg->libelle_sms), "%s", texte );
    msg->id  = 0;
    msg->num = 0;
    msg->enable = TRUE;
    msg->sms = MSG_SMS_SMSBOX;

    pthread_mutex_lock( &Partage->com_sms.synchro );
    Partage->com_sms.liste_sms = g_list_append ( Partage->com_sms.liste_sms, msg );
    pthread_mutex_unlock( &Partage->com_sms.synchro );
  }
/**********************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Envoyer_sms_gsm_text ( gchar *texte )
  { struct CMD_TYPE_MESSAGE *msg;

    msg = (struct CMD_TYPE_MESSAGE *) g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!msg) { Info_new( Config.log, Config.log_all, LOG_ERR,
                         "Envoyer_sms_gsm_text: pas assez de mémoire pour copie" );
                return;
              }

    g_snprintf(msg->libelle_sms, sizeof(msg->libelle_sms), "%s", texte );
    msg->id  = 0;
    msg->num = 0;
    msg->enable = TRUE;
    msg->sms = MSG_SMS_GSM;

    pthread_mutex_lock( &Partage->com_sms.synchro );
    Partage->com_sms.liste_sms = g_list_append ( Partage->com_sms.liste_sms, msg );
    pthread_mutex_unlock( &Partage->com_sms.synchro );
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
     { Info_new( Config.log, Config.log_all, LOG_WARNING,
                "Lire_sms_gsm: Read Phone profile NOK (%s)", gn_error_print(error) );
       return;
     }

    if ((error=gn_lib_phone_open(state)) != GN_ERR_NONE)
     { Info_new( Config.log, Config.log_all, LOG_WARNING,
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
        {                                                                           /* Commence par CDE ? */
          if ( ! strncmp( (gchar *)sms.user_data[0].u.text, PRESMS, strlen(PRESMS) ) )
           { guint num_bit;
             num_bit = atoi( (gchar *)sms.user_data[0].u.text + strlen(PRESMS));
             Info_new( Config.log, Config.log_all, LOG_INFO,
                      "Lire_sms_gsm: Recu SMS %s de %s", (gchar *)sms.user_data[0].u.text, sms.remote.number );
             if ( Config.sms_m_min <= num_bit && num_bit <= Config.sms_m_max)
              { Envoyer_commande_dls ( num_bit ); }                           /* Activation du monostable */ 
             else Info_new( Config.log, Config.log_all, LOG_INFO,
                           "Lire_sms_gsm: permission denied M number %d", num_bit );

           }
          else
           { Info_new( Config.log, Config.log_all, LOG_INFO,
                      "Lire_sms_gsm: Wrong CDE %s de %s", (gchar *)sms.user_data[0].u.text );
           }
          gn_sms_delete (&data, state);                               /* On l'a traité, on peut l'effacer */
        }
       else if (error == GN_ERR_INVALIDLOCATION) break;       /* On regarde toutes les places de stockage */
       else  { Info_new( Config.log, Config.log_all, LOG_WARNING,
                        "Lire_sms_gsm: error %s from %s", gn_error_print(error), sms.number );
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
     { Info_new( Config.log, Config.log_all, LOG_WARNING,
                "Envoi_sms_gsm: Read Phone profile NOK (%s)", gn_error_print(error) );
       return;
     }

    if ((error=gn_lib_phone_open(state)) != GN_ERR_NONE)
     { Info_new( Config.log, Config.log_all, LOG_WARNING,
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
     { data.message_center = calloc(1, sizeof(gn_sms_message_center));
       data.message_center->id = 1;
       if (gn_sm_functions(GN_OP_GetSMSCenter, &data, state) == GN_ERR_NONE)
        { strcpy(sms.smsc.number, data.message_center->smsc.number);
          sms.smsc.type = data.message_center->smsc.type;
        }
       else
        { Info_new( Config.log, Config.log_all, LOG_WARNING, "Envoi_sms_gsm: Pb avec le SMSC" ); }
       free(data.message_center);
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
     { Info_new( Config.log, Config.log_all, LOG_INFO, "Envoi_sms_gsm: Envoi SMS Ok %s", msg->libelle_sms ); }
    else
     { Info_new( Config.log, Config.log_all, LOG_WARNING,
                "Envoi_sms_gsm: Envoi SMS Nok (%s)", gn_error_print(error)); }

    gn_lib_phone_close(state);
    gn_lib_phoneprofile_free(&state);
    gn_lib_library_free();
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
                  CURLFORM_COPYCONTENTS, Config.smsbox_username,
                  CURLFORM_END); 
    curl_formadd( &formpost, &lastptr,
                  CURLFORM_COPYNAME,     "pass",
                  CURLFORM_COPYCONTENTS, Config.smsbox_password,
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
     { Info_new( Config.log, Config.log_all, LOG_INFO,
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
        { Info_new( Config.log, Config.log_all, LOG_INFO,
                   "Envoi_sms_smsbox: Envoi SMS %s to %s", msg->libelle_sms, telephone );
        }
       else
        { Info_new( Config.log, Config.log_all, LOG_WARNING,
                   "Envoi_sms_smsbox: Envoi SMS Nok - Pb cURL (%s)", erreur); }
     }
    else
     { Info_new( Config.log, Config.log_all, LOG_WARNING,
                "Envoi_sms_smsbox: Envoi SMS Nok - Pb cURL Init");
     }
    curl_easy_cleanup(curl);
    curl_formfree(formpost);
  }
/**********************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Run_sms ( void )
  { struct CMD_TYPE_MESSAGE *msg;
    GList *liste_sms;
    
    prctl(PR_SET_NAME, "W-SMS", 0, 0, 0 );
    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_sms: Starting" );
                                                                /* Initialisation des variables du thread */
    Partage->com_sms.Thread_run    = TRUE;                                          /* Le thread tourne ! */

    while(Partage->com_sms.Thread_run == TRUE)                           /* On tourne tant que necessaire */
     {

       if (Partage->com_sms.Thread_reload)                              /* A-t'on recu un signal RELOAD ? */
        { Info_new( Config.log, Config.log_all, LOG_INFO, "Run_sms: RELOAD" );
          Partage->com_sms.Thread_reload = FALSE;
        }

       if (Partage->com_sms.Thread_sigusr1)                               /* A-t'on recu un signal USR1 ? */
        { int nbr;

          Info_new( Config.log, Config.log_all, LOG_INFO, "Run_sms: SIGUSR1" );
          pthread_mutex_lock( &Partage->com_sms.synchro );     /* On recupere le nombre de sms en attente */
          nbr = g_list_length(Partage->com_sms.liste_sms);
          pthread_mutex_unlock( &Partage->com_sms.synchro );
          Info_new( Config.log, Config.log_all, LOG_INFO, "Run_sms: Nbr SMS a envoyer = %d", nbr );
          Partage->com_sms.Thread_sigusr1 = FALSE;
        }

/********************************************** Lecture de SMS ********************************************/
       Lire_sms_gsm();

/************************************************ Envoi de SMS ********************************************/
       if ( !Partage->com_sms.liste_sms )                               /* Attente de demande d'envoi SMS */
        { sleep(10);
          sched_yield();
          continue;
        }

       Info_new( Config.log, Config.log_all, LOG_DEBUG, "Run_sms send: debut" );
       pthread_mutex_lock( &Partage->com_sms.synchro );
       liste_sms = Partage->com_sms.liste_sms;                         /* Sauvegarde du ptr sms a envoyer */
       pthread_mutex_unlock( &Partage->com_sms.synchro );

       msg = liste_sms->data;
/**************************************** Envoi en mode GSM ***********************************************/

       if (Partage->top < TOP_MIN_ENVOI_SMS)
        { Info_new( Config.log, Config.log_all, LOG_INFO,
                   "Envoi_sms_gsm: Envoi trop tot !! (%s)", msg->libelle_sms ); }
       else if (msg->sms == MSG_SMS_GSM)
        { if ( strcmp(Config.sms_telephone1, DEFAUT_SMS_TELEPHONE) ) Envoi_sms_gsm ( msg, Config.sms_telephone1 );
          if ( strcmp(Config.sms_telephone2, DEFAUT_SMS_TELEPHONE) ) Envoi_sms_gsm ( msg, Config.sms_telephone2 );
        }
/**************************************** Envoi en mode SMSBOX ********************************************/
       else if (msg->sms == MSG_SMS_SMSBOX)
        { if ( strcmp(Config.sms_telephone1, DEFAUT_SMS_TELEPHONE) ) Envoi_sms_smsbox ( msg, Config.sms_telephone1 );
          if ( strcmp(Config.sms_telephone2, DEFAUT_SMS_TELEPHONE) ) Envoi_sms_smsbox ( msg, Config.sms_telephone2 );
        }

       pthread_mutex_lock( &Partage->com_sms.synchro );
       Partage->com_sms.liste_sms = g_list_remove ( Partage->com_sms.liste_sms, msg );
       Info_new( Config.log, Config.log_all, LOG_INFO, "Run_sms: Reste %d a envoyer",
                 g_list_length(Partage->com_sms.liste_sms) );
       pthread_mutex_unlock( &Partage->com_sms.synchro );
       g_free( msg );

       Info_new( Config.log, Config.log_all, LOG_DEBUG, "Run_sms send: fin" );
     }

    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Run_sms: Down (%s)", pthread_self() );
    Partage->com_sms.TID = 0;                             /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
