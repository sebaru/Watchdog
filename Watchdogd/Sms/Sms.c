/**********************************************************************************************************/
/* Watchdogd/sms.c        Gestion des SMS de Watchdog v2.0                                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                   ven. 02 avril 2010 20:37:40 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Sms.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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
 #include <bonobo/bonobo-i18n.h>
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <string.h>
 #include <unistd.h>
 #include <gnokii.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "Reseaux.h"
 #include "watchdogd.h"
 #define PRESMS   "CDE:"
/**********************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Envoyer_sms ( gchar *libelle )
  { gchar *copie;
    gint nbr;

    pthread_mutex_lock( &Partage->com_sms.synchro );      /* On recupere le nombre de sms en attente */
    nbr = g_list_length(Partage->com_sms.liste_sms);
    pthread_mutex_unlock( &Partage->com_sms.synchro );

    if (nbr > 50)
     { Info( Config.log, DEBUG_INFO, "Envoyer_sms: liste d'attente pleine" ); return; }

    copie = g_strdup ( libelle );
    if (!copie) { Info( Config.log, DEBUG_MEM, "Envoyer_sms: pas assez de mémoire" ); return; }

    pthread_mutex_lock( &Partage->com_sms.synchro );
    Partage->com_sms.liste_sms = g_list_append ( Partage->com_sms.liste_sms, copie );
    pthread_mutex_unlock( &Partage->com_sms.synchro );
  }
/**********************************************************************************************************/
/* Envoyer_sms: Envoi un sms                                                                              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Run_sms ( void )
  { static struct gn_statemachine *state;
    static gn_sms_folder folder;
    static gn_sms_folder_list folderlist;
    static GList *liste_sms;
    static gn_error error;
    static gn_data data;
    static gn_sms sms;
    static gint sms_index;

    prctl(PR_SET_NAME, "W-SMS", 0, 0, 0 );
    Info ( Config.log, DEBUG_INFO, "SMS: Run_sms: Demarrage" );

    if ((error=gn_lib_phoneprofile_load("", &state)) != GN_ERR_NONE)                  /* Read config file */
     { Info_c ( Config.log, DEBUG_INFO, "SMS: Run_sms: Read Phone profile NOK", gn_error_print(error) );
       Info_n( Config.log, DEBUG_FORK, "SMS: Run_sms: Down", pthread_self() );
       pthread_exit(GINT_TO_POINTER(0));
     }

    if ((error=gn_lib_phone_open(state)) != GN_ERR_NONE)
     { Info_c( Config.log, DEBUG_INFO, "SMS: Run_sms: Open Phone NOK", gn_error_print(error) );
       Info_n( Config.log, DEBUG_FORK, "SMS: Run_sms: Down", pthread_self() );
       pthread_exit(GINT_TO_POINTER(0));
     }
    
#ifdef bouh

    if ((error = gn_gsm_initialise(&state)) != GN_ERR_NONE)                           /* Connect to phone */
     { Info_c ( Config.log, DEBUG_INFO, "SMS: Run_sms: INIT NOK", gn_error_print(error) );
       Info_n( Config.log, DEBUG_FORK, "SMS: Run_sms: Down", pthread_self() );
       pthread_exit(GINT_TO_POINTER(0));
     }

#endif
    gn_data_clear(&data);
    sms_index = 1;                          /* On commence par regarder la premiere place de stockage SMS */
    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     {

       if (Partage->com_sms.sigusr1)                                 /* A-t'on recu un signal USR1 ? */
        { int nbr;

          Partage->com_sms.sigusr1 = FALSE;
          Info( Config.log, DEBUG_INFO, "SMS: Run_sms: SIGUSR1" );
          pthread_mutex_lock( &Partage->com_sms.synchro );/* On recupere le nombre de sms en attente */
          nbr = g_list_length(Partage->com_sms.liste_sms);
          pthread_mutex_unlock( &Partage->com_sms.synchro );
          Info_n( Config.log, DEBUG_INFO, "SMS: Nbr SMS a envoyer", nbr );
        }

/********************************************** Lecture de SMS ********************************************/
       folder.folder_id = 0;
       data.sms_folder_list = &folderlist;
       data.sms_folder = &folder;

       memset ( &sms, 0, sizeof(gn_sms) );
       sms.memory_type = gn_str2memory_type("ME");    /* On recupere les SMS du Mobile equipment (pas SM) */
       sms.number = sms_index;
       data.sms = &sms;

       if ((error = gn_sms_get (&data, state)) == GN_ERR_NONE)                      /* On recupere le SMS */
        { 
          if ( ! strncmp( (gchar *)sms.user_data[0].u.text, PRESMS, strlen(PRESMS) ) )/* Commence par CDE ? */
           { guint num_bit;
             num_bit = atoi( (gchar *)sms.user_data[0].u.text + strlen(PRESMS));
             Info_c ( Config.log, DEBUG_INFO, "SMS: Run_sms: Recu SMS", (gchar *)sms.user_data[0].u.text );
             Info_c ( Config.log, DEBUG_INFO, "SMS: Run_sms:       de", sms.remote.number );
             Envoyer_commande_dls ( num_bit );                                /* Activation du monostable */
             gn_sms_delete (&data, state);                           /* On l'a traité, on peut l'effacer */
           }
          else
           { Info_c ( Config.log, DEBUG_INFO, "SMS: Run_sms: Wrong CDE", (gchar *)sms.user_data[0].u.text );
             Info_c ( Config.log, DEBUG_INFO, "SMS: Run_sms:        de", sms.remote.number );
           }
        }
       else if (error == GN_ERR_INVALIDLOCATION) sms_index=1;      /* On regarde toutes les places de stockage */
       else { Info_c ( Config.log, DEBUG_INFO, "SMS: Run_sms: erreor", gn_error_print(error) );
            }

       sms_index++;                                        /* Le prochain coup, on regarde la zone sms +1 */
/************************************************ Envoi de SMS ********************************************/
       if ( !Partage->com_sms.liste_sms )                               /* Attente de demande d'envoi SMS */
        { sleep(5);
          sched_yield();
          continue;
        }

       Info( Config.log, DEBUG_DLS, "SMS: Run_sms send: debut" );
       pthread_mutex_lock( &Partage->com_sms.synchro );
       liste_sms = Partage->com_sms.liste_sms;                         /* Sauvegarde du ptr sms a envoyer */
       pthread_mutex_unlock( &Partage->com_sms.synchro );

       gn_sms_default_submit(&sms);                                          /* The memory is zeroed here */

       memset(&sms.remote.number, 0, sizeof(sms.remote.number));
       strncpy(sms.remote.number, "0683426100", sizeof(sms.remote.number) - 1);         /* Number a m'man */
       if (sms.remote.number[0] == '+') 
            { sms.remote.type = GN_GSM_NUMBER_International; }
       else { sms.remote.type = GN_GSM_NUMBER_Unknown; }

       if (!sms.smsc.number[0])                                                   /* Récupération du SMSC */
        { data.message_center = calloc(1, sizeof(gn_sms_message_center));
          data.message_center->id = 1;
          if (gn_sm_functions(GN_OP_GetSMSCenter, &data, state) == GN_ERR_NONE)
           { strcpy(sms.smsc.number, data.message_center->smsc.number);
             sms.smsc.type = data.message_center->smsc.type;
           }
          else
           { Info ( Config.log, DEBUG_INFO, "SMS: Run_sms: Pb avec le SMSC" ); }
          free(data.message_center);
        }

       if (!sms.smsc.type) sms.smsc.type = GN_GSM_NUMBER_Unknown;

       sms.user_data[0].length = g_snprintf( sms.user_data[0].u.text, sizeof (sms.user_data[0].u.text),
                                             "%s", (gchar *)liste_sms->data );
        
       sms.user_data[0].type = GN_SMS_DATA_Text;
       if (!gn_char_def_alphabet(sms.user_data[0].u.text))
        { sms.dcs.u.general.alphabet = GN_SMS_DCS_UCS2; }

       sms.user_data[1].type = GN_SMS_DATA_None;
/*	sms.delivery_report = true; */
       data.sms = &sms;                                                                   /* Envoi du SMS */
       error = gn_sms_send(&data, state);
       if (error == GN_ERR_NONE)
        { Info_c ( Config.log, DEBUG_INFO, "SMS: Run_sms: Envoi SMS Ok", 
                   liste_sms->data ); }
       else
        { Info_c ( Config.log, DEBUG_INFO, "SMS: Run_sms: Envoi SMS Nok", gn_error_print(error)); }

       pthread_mutex_lock( &Partage->com_sms.synchro );
       g_free( liste_sms->data );
       Partage->com_sms.liste_sms = g_list_remove ( Partage->com_sms.liste_sms, liste_sms->data );
       Info_n ( Config.log, DEBUG_INFO, "SMS: Run_sms: Reste a envoyer",
                g_list_length(Partage->com_sms.liste_sms) );
       pthread_mutex_unlock( &Partage->com_sms.synchro );

       Info( Config.log, DEBUG_DLS, "SMS: Run_sms send: fin" );
     }

    gn_lib_phone_close(state);
    gn_lib_phoneprofile_free(&state);
    gn_lib_library_free();

    Info_n( Config.log, DEBUG_FORK, "SMS: Run_sms: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
