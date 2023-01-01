/**********************************************************************************************************/
/* Watchdogd/Enocean/Enocean.c  Gestion des capteurs ENOCEAN Watchdog 2.0                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                     dim. 28 déc. 2014 15:46:44 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Enocean.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
 #include <termios.h>
 #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
 #include "Enocean.h"
 struct ENOCEAN_CONFIG Cfg_enocean;
 unsigned char ENOCEAN_CRC8TABLE[256] =
  { 0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
    0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
    0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5,
    0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85,
    0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
    0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2,
    0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
    0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
    0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c,
    0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
    0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
    0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c,
    0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b,
    0x76, 0x71, 0x78, 0x7f, 0x6A, 0x6d, 0x64, 0x63,
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
    0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb,
    0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8D, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb,
    0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3
  };

/**********************************************************************************************************/
/* Enocean_Lire_config : Lit la config Watchdog et rempli la structure mémoire                             */
/* Entrée: le pointeur sur la PROCESS                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 gboolean Enocean_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_enocean.lib->Thread_debug = FALSE;                                  /* Settings default parameters */
    Cfg_enocean.enable            = FALSE;
    g_snprintf( Cfg_enocean.port, sizeof(Cfg_enocean.port), "%s", DEFAUT_PORT_ENOCEAN );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                      /* Connexion a la base de données */
     { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                "Enocean_Lire_config: Database connexion failed. Using Default Parameters" );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )       /* Récupération d'une config dans la DB */
     { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_INFO,                     /* Print Config */
                "Enocean_Lire_config: '%s' = %s", nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "port" ) )
        { g_snprintf( Cfg_enocean.port, sizeof(Cfg_enocean.port), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_enocean.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_enocean.lib->Thread_debug = TRUE;  }
       else
        { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_NOTICE,
                   "Enocean_Lire_config: Unknown Parameter '%s'(='%s') in Database", nom, valeur );
        }
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Init_enocean: Initialisation de la ligne TTY ENOCEAN                                                   */
/* Sortie: l'identifiant de la connexion                                                                  */
/**********************************************************************************************************/
 static int Init_enocean ( void )
  { struct termios oldtio;
    int fd;

    fd = open( Cfg_enocean.port, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd<0)
     { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_ERR,
               "Init_enocean: Impossible d'ouvrir le port enocean %s, erreur %d (%s)",
                Cfg_enocean.port, fd, strerror(errno) );
       return(-1);
     }
    else
     { memset(&oldtio, 0, sizeof(oldtio) );
       oldtio.c_cflag = B57600 | CS8 | CREAD | CLOCAL;
       oldtio.c_oflag = 0;
       oldtio.c_iflag = 0;
       oldtio.c_lflag = 0;
       oldtio.c_cc[VTIME]    = 0;
       oldtio.c_cc[VMIN]     = 0;
       tcsetattr(fd, TCSANOW, &oldtio);
       tcflush(fd, TCIOFLUSH);
       Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_NOTICE,
                 "Init_enocean: Ouverture port enocean okay %s", Cfg_enocean.port );
     }
    return(fd);
  }
/**********************************************************************************************************/
/* Chercher_enocean: Retrouve un module/capteur dans la liste gérée en fonction des paramètres             */
/* Entrée: les paramètres de critères de recherche                                                        */
/* Sortie: le module, ou NULL si erreur                                                                   */
/**********************************************************************************************************/
 static void Enocean_Envoyer_sortie ( gint num_a )
  {
#ifdef bouh
 gchar trame_send_AC[] = { 0x0B, 0x11, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    struct MODULE_ENOCEAN *module;
    GSList *liste_modules;

    module = NULL;
    pthread_mutex_lock ( &Cfg_enocean.lib->synchro );
    liste_modules = Cfg_enocean.Modules_ENOCEAN;
    while ( liste_modules )
     { module = (struct MODULE_ENOCEAN *)liste_modules->data;

       if (module->enocean.type == 0x11 && module->enocean.sous_type == 0x00 &&
           module->enocean.a_min == num_a
          )
        { gint cpt;
          Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
              "Enocean_envoyer_sortie: Envoi de A(%03d)=%d au module ids=%02d %02d %02d %02d unit %02d",
               num_a, A(num_a), module->enocean.id1, module->enocean.id2,
               module->enocean.id3, module->enocean.id4, module->enocean.unitcode );
          trame_send_AC[0]  = 0x0B; /* Taille */
          trame_send_AC[1]  = 0x11; /* lightning 2 */
          trame_send_AC[2]  = 0x00; /* AC */
          trame_send_AC[3]  = 0x01; /* Seqnbr */
          trame_send_AC[4]  = module->enocean.id1 << 6;
          trame_send_AC[5]  = module->enocean.id2;
          trame_send_AC[6]  = module->enocean.id3;
          trame_send_AC[7]  = module->enocean.id4;
          trame_send_AC[8]  = module->enocean.unitcode;
          trame_send_AC[9]  = (A(num_a) ? 1 : 0);
          trame_send_AC[10] = 0x0; /* level */
          trame_send_AC[11] = 0x0; /* rssi */
          for ( cpt = 0; cpt < 3 ; cpt++)
           { gint retour;
             retour = write ( Cfg_enocean.fd, &trame_send_AC, trame_send_AC[0] + 1 );
             if (retour == -1)
              { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                         "Enocean_envoyer_sortie: Write Error for A(%03d) : %s", num_a, strerror(errno) );
              }
           }
        }
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_enocean.lib->synchro );
#endif
  }
/**********************************************************************************************************/
/* Enocean_crc_header: Calcul le Header CRC de la trame en parametre                                      */
/* Entrée: la trame recue                                                                                 */
/* Sortie: le CRC sur 8 bits !                                                                            */
/**********************************************************************************************************/
 static unsigned char Enocean_crc_header( struct TRAME_ENOCEAN *trame )
  { unsigned char resultCRC = 0;
    unsigned char *ptr;
    gint i;
    ptr = (unsigned char *)trame;
    for (i = 1; i < ENOCEAN_HEADER_LENGTH - 1; i++)                                    /* Last Byte = CRC */
     { resultCRC = ENOCEAN_CRC8TABLE[ resultCRC ^ ptr[i] ]; }
    return( resultCRC );
  }
/**********************************************************************************************************/
/* Enocean_crc_data: Calcul le Data CRC de la trame en parametre                                          */
/* Entrée: la trame recue                                                                                 */
/* Sortie: le CRC sur 8 bits !                                                                            */
/**********************************************************************************************************/
 static unsigned char Enocean_crc_data( struct TRAME_ENOCEAN *trame )
  { unsigned char resultCRC = 0;
    unsigned char *ptr;
    gint i;
    ptr = (unsigned char *)trame;
    for (i = ENOCEAN_HEADER_LENGTH; i < Cfg_enocean.index_bute - 1; i++)               /* Last byte = CRC */
     { resultCRC = ENOCEAN_CRC8TABLE[ resultCRC ^ ptr[i] ]; }
    return( resultCRC );
  }
/**********************************************************************************************************/
/* Processer_trame_ERP1: traitement de la trame ERP1 recue par le controleur EnOcean                      */
/* Entrée: la trame a recue                                                                               */
/* Sortie: TRUE si processed                                                                              */
/**********************************************************************************************************/
 static gboolean Processer_trame_ERP1( struct TRAME_ENOCEAN *trame )
  { gchar *action, *button = "unknown", chaine[128];

    if (trame->data[0] == 0xD2)
     { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
                "Processer_trame_ERP1: Received VLD" );
     }
    else if (trame->data[0] == 0xF6)                                                      /* RPS Telegram */
     { if ( trame->data[1] & 0x10 ) action = "Pressed";
                               else action = "Released";
       if ( (trame->data[6] & 0x30) == 0x30 )                                     /* Status : T21 et NU ? */
        { switch( (trame->data[1] & 0xE0)>>5 )
           { case 0: button = "Button-AI"; break;
             case 1: button = "Button-AO"; break;
             case 2: button = "Button-BI"; break;
             case 3: button = "Button-BO"; break;
           }
        }
       else if ( (trame->data[6] & 0x30) == 0x20 )                                /* Status : Juste T21 ? */
        { switch( (trame->data[1] & 0xE0)>>5 )
           { case 0: button = "No-Button"; break;
             case 1: button = "3/4-Buttons"; break;
           }
        }

       g_snprintf( chaine, sizeof(chaine), "%02X%02X%02X%02X:%s:%s",
                   trame->data[2], trame->data[3], trame->data[4], trame->data[5],
                   button, action );
/*       Send_Event( g_get_host_name(), NOM_THREAD, EVENT_INPUT, chaine, 0 );*/

       Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                 "Processer_trame_ERP1: New_Event : %s", chaine );
       return(TRUE);
     }
    else if (trame->data[0] == 0xA5)
     { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
                "Processer_trame_ERP1: Received RADIO_ERP1-4BS" );
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                   */
/* Entrée: la trame a recue                                                                               */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Processer_trame( struct TRAME_ENOCEAN *trame )
  { gchar chaine[32];
    gint cpt;
    memset( chaine, 0, sizeof(chaine) );
    for (cpt=0; cpt<trame->data_length_lsb+trame->optional_data_length; cpt++)
     { g_snprintf( &chaine[2*cpt], 3, "%02X", trame->data[cpt] ); }/* Mise en forme au format HEX */
    Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
             "Processer_trame: Received RADIO_ERP1-%s", chaine );

    if (trame->packet_type == 1 && Processer_trame_ERP1 ( trame )) return;                  /* RADIO_ERP1 */

    Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
             "Processer_trame: Unmanaged telegram: packet type %0X - %02X-%02X-%02X",
              trame->packet_type, trame->data[0], trame->data[1], trame->data[2] );
  }
/**********************************************************************************************************/
/* Enocean_Gerer_sortie: Ajoute une demande d'envoi RF dans la liste des envois ENOCEAN                   */
/* Entrées: le numéro de la sortie                                                                        */
/**********************************************************************************************************/
 void Enocean_Gerer_sortie( gint num_a )                                   /* Num_a est l'id de la sortie */
  {
#ifdef bouh
    pthread_mutex_lock( &Cfg_enocean.lib->synchro );             /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_enocean.Liste_sortie );
    pthread_mutex_unlock( &Cfg_enocean.lib->synchro );

    if (taille > 150)
     { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                "Enocean_Gerer_sortie: DROP (taille>150)  id=%d", num_a );
       return;
     }

    pthread_mutex_lock( &Cfg_enocean.lib->synchro );       /* Ajout dans la liste de tell a traiter */
    Cfg_enocean.Liste_sortie = g_slist_prepend( Cfg_enocean.Liste_sortie, GINT_TO_POINTER(num_a) );
    pthread_mutex_unlock( &Cfg_enocean.lib->synchro );
#endif
  }
/**********************************************************************************************************/
/* Enocean_select: Permet d'estimer la disponibilité d'une information reçue à traiter                    */
/* Entrée : Néant                                                                                         */
/* Sortie : 0 - pas d'info, 1 presence d'info, -1, erreur                                                 */
/**********************************************************************************************************/
 static gint Enocean_select ( void )
  { struct timeval tv;
    fd_set fdselect;
    gint retval;
    tv.tv_sec = 0;
    tv.tv_usec= 100000;
    FD_ZERO(&fdselect);                                  /* Reception sur la ligne serie ENOCEAN */
    FD_SET(Cfg_enocean.fd, &fdselect );
    retval = select(Cfg_enocean.fd+1, &fdselect, NULL, NULL, &tv );    /* Attente d'un caractere */
    if (retval==0) return(0);
    if (retval==1 && FD_ISSET(Cfg_enocean.fd, &fdselect) )
     { Cfg_enocean.date_last_view = Partage->top;
       return(1);
     }
    Cfg_enocean.comm_status = ENOCEAN_DISCONNECT;                                /* Disconnect sur erreur */
    Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_ERR,
             "Enocean_select: Error %d (%s)", errno, strerror(errno) );
    return(-1);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Enocean                                                            */
/**********************************************************************************************************/
 void Run_process ( struct PROCESS *lib )
  { struct TRAME_ENOCEAN Trame;

    prctl(PR_SET_NAME, "W-ENOCEAN", 0, 0, 0 );
    memset( &Cfg_enocean, 0, sizeof(Cfg_enocean) );               /* Mise a zero de la structure de travail */
    Cfg_enocean.lib = lib;                      /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_enocean.lib->TID = pthread_self();                               /* Sauvegarde du TID pour le pere */
    Enocean_Lire_config ();                              /* Lecture de la configuration logiciel du thread */

    Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_NOTICE,
              "Run_process: Demarrage . . . TID = %p", pthread_self() );
    Cfg_enocean.lib->Thread_run = TRUE;                                              /* Le thread tourne ! */

    g_snprintf( lib->admin_prompt, sizeof(lib->admin_prompt), "enocean" );
    g_snprintf( lib->admin_help,   sizeof(lib->admin_help),   "Manage ENOCEAN sensors" );

    if (!Cfg_enocean.enable)
     { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_NOTICE,
                "Run_process: Thread is not enabled in config. Shutting Down %p",
                 pthread_self() );
       goto end;
     }

/*  Abonner_distribution_sortie ( Enocean_Gerer_sortie );     /* Desabonnement de la diffusion des sorties */
    Cfg_enocean.nbr_oct_lu = 0;
    Cfg_enocean.comm_status = ENOCEAN_CONNECT;
    while( lib->Thread_run == TRUE )                                     /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (lib->Thread_reload == TRUE)
        { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_NOTICE, "Run_process: recu signal SIGUSR1" );
          lib->Thread_reload = FALSE;
        }

       if (Cfg_enocean.reload == TRUE)
        { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_NOTICE, "Run_process: Reloading in progress" );
          Cfg_enocean.reload = FALSE;
        }

       switch (Cfg_enocean.comm_status)
        { case ENOCEAN_CONNECT:
           { Cfg_enocean.fd = Init_enocean();
             if (Cfg_enocean.fd<0)                                         /* On valide l'acces aux ports */
              { Cfg_enocean.comm_status = ENOCEAN_DISCONNECT; }
             else { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                             "Run_process: ENOCEAN FileDescriptor = %d opened", Cfg_enocean.fd );
                    Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;
                  }
             break;
           }
          case ENOCEAN_WAIT_FOR_SYNC:
           { guchar sync;
             gint cpt;
             if (Enocean_select()<=0) break;
             cpt = read( Cfg_enocean.fd, &sync, 1 );
             if (cpt>0)
              { if (sync==0x55) Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_HEADER;
                else Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
                              "Run_process: Wrong SYNC Byte (%02X). Dropping Frame", sync );
                Cfg_enocean.nbr_oct_lu = 0;
              }
             else Cfg_enocean.comm_status = ENOCEAN_DISCONNECT;                              /* Si erreur */
             break;
           }
          case ENOCEAN_WAIT_FOR_HEADER:
           { gint cpt;
             if (Cfg_enocean.date_last_view + ENOCEAN_TRAME_TIMEOUT <= Partage->top)
              { Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;
                Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                         "Run_process: Timeout wating for HEADER. Dropping Frame" );
                break;
              }
             if (Enocean_select()<=0) break;
             cpt = read( Cfg_enocean.fd, (unsigned char *)&Trame + Cfg_enocean.nbr_oct_lu,
                         ENOCEAN_HEADER_LENGTH - Cfg_enocean.nbr_oct_lu );
             if (cpt>0)
              { Cfg_enocean.nbr_oct_lu = Cfg_enocean.nbr_oct_lu + cpt;

                if (Cfg_enocean.nbr_oct_lu == ENOCEAN_HEADER_LENGTH)
                 { if (Trame.crc_header != Enocean_crc_header( &Trame ))    /* Vérification du CRC Header */
                    { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                               "Run_process: Wrong CRC HEADER. Dropping Frame" );
                      Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;
                    }
                   else
                    { Cfg_enocean.index_bute = ENOCEAN_HEADER_LENGTH + (Trame.data_length_msb << 8)
                                             + Trame.data_length_lsb
                                             + Trame.optional_data_length+1; /* On compte le CRC de fin ! */
                      if (Cfg_enocean.index_bute > sizeof(Trame))
                       { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_ERR,
                                  "Run_process: Trame too long (%d / %d max), can't handle, dropping",
                                   Cfg_enocean.index_bute, sizeof(Trame) );
                         Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;
                       }
                      else Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_DATA;
                    }
                 }
              }
             else Cfg_enocean.comm_status = ENOCEAN_DISCONNECT;                              /* Si erreur */
             break;
           }
          case ENOCEAN_WAIT_FOR_DATA:
           { gint cpt;
             if (Cfg_enocean.date_last_view + ENOCEAN_TRAME_TIMEOUT <= Partage->top)
              { Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;
                Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                         "Run_process: Timeout wating for DATA. Dropping Frame" );
                break;
              }
             if (Enocean_select()<=0) break;
             cpt = read( Cfg_enocean.fd, (unsigned char *)&Trame + Cfg_enocean.nbr_oct_lu,
                         Cfg_enocean.index_bute - Cfg_enocean.nbr_oct_lu );
             if (cpt>0)
              { Cfg_enocean.nbr_oct_lu = Cfg_enocean.nbr_oct_lu + cpt;

                if (Cfg_enocean.nbr_oct_lu == Cfg_enocean.index_bute)         /* Vérification du CRC Data */
                 { if ( ((unsigned char *)&Trame)[Cfg_enocean.index_bute-1] != Enocean_crc_data( &Trame ) )
                    { Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                               "Run_process: Wrong CRC DATA. Dropping Frame" );
                      Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;
                    }
                   else
                    { Processer_trame ( &Trame );                            /* Precessing received trame */
                      Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;            /* and wait for another */
                    }
                 }
              }
             else Cfg_enocean.comm_status = ENOCEAN_DISCONNECT;                              /* Si erreur */
             break;
           }
          case ENOCEAN_DISCONNECT:
           { if (Cfg_enocean.fd)
              { close(Cfg_enocean.fd);
                Cfg_enocean.fd = 0;
              }
             Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_ERR,
                      "Run_process: ENOCEAN Disconnected. Re-Trying in %d sec...",
                       ENOCEAN_RECONNECT_DELAY/10 );
             Cfg_enocean.date_retry_connect = Partage->top + ENOCEAN_RECONNECT_DELAY;
             Cfg_enocean.comm_status = ENOCEAN_WAIT_BEFORE_RECONNECT;
             break;
           }
          case ENOCEAN_WAIT_BEFORE_RECONNECT:
           { if (Cfg_enocean.date_retry_connect <= Partage->top)
              { Cfg_enocean.comm_status = ENOCEAN_CONNECT;
                Cfg_enocean.date_retry_connect = 0;
              }
             break;
           }
          default: Cfg_enocean.comm_status = ENOCEAN_CONNECT;
        }
/********************************************** Transmission des trames aux sorties ***********************/
#ifdef bouh
       if (Cfg_enocean.Liste_sortie)                            /* Si pas de message, on tourne */
        { gint num_a;
          pthread_mutex_lock( &Cfg_enocean.lib->synchro );                                /* lockage futex */
          num_a = GPOINTER_TO_INT(Cfg_enocean.Liste_sortie->data);               /* Recuperation du numero */
          Cfg_enocean.Liste_sortie = g_slist_remove ( Cfg_enocean.Liste_sortie, GINT_TO_POINTER(num_a) );
          Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                   "Run_enocean: Reste a traiter %d",
                    g_slist_length(Cfg_enocean.Liste_sortie) );
          pthread_mutex_unlock( &Cfg_enocean.lib->synchro );

          Enocean_Envoyer_sortie ( num_a );
        }
#endif
     }                                                                     /* Fin du while partage->arret */

/*  Desabonner_distribution_sortie ( Enocean_Gerer_sortie );  /* Desabonnement de la diffusion des sorties */
/*    Decharger_tous_enocean ();*/
    close(Cfg_enocean.fd);                                                 /* Fermeture de la connexion FD */
end:
    Info_new( __func__, Cfg_enocean.lib->Thread_debug, LOG_NOTICE,
              "Run_process: Down . . . TID = %p", pthread_self() );
    Cfg_enocean.lib->Thread_run = FALSE;                                     /* Le thread ne tourne plus ! */
    Cfg_enocean.lib->TID = 0;                              /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
