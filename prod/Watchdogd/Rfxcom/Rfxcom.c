/**********************************************************************************************************/
/* Watchdogd/Rfxcom/Rfxcom.c  Gestion des capteurs RFXCOM Watchdog 2.0                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 27 mai 2012 12:52:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Rfxcom.c
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
 #include <termios.h>
 #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
 #include "Rfxcom.h"

/**********************************************************************************************************/
/* Rfxcom_Lire_config : Lit la config Watchdog et rempli la structure mémoire                             */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 gboolean Rfxcom_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_rfxcom.lib->Thread_debug = FALSE;                                  /* Settings default parameters */
    Cfg_rfxcom.enable            = FALSE; 
    g_snprintf( Cfg_rfxcom.port, sizeof(Cfg_rfxcom.port), "%s", DEFAUT_PORT_RFXCOM );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                      /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_WARNING,
                "Rfxcom_Lire_config: Database connexion failed. Using Default Parameters" );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )       /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,                      /* Print Config */
                "Rfxcom_Lire_config: '%s' = %s", nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "port" ) )
        { g_snprintf( Cfg_rfxcom.port, sizeof(Cfg_rfxcom.port), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_rfxcom.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_rfxcom.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_NOTICE,
                   "Rfxcom_Lire_config: Unknown Parameter '%s'(='%s') in Database", nom, valeur );
        }
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Init_rfxcom: Initialisation de la ligne RFXCOM                                                         */
/* Sortie: l'identifiant de la connexion                                                                  */
/**********************************************************************************************************/
 static int Init_rfxcom ( void )
  { gchar trame_reset[] = { 0x0D, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    gchar trame_get_status[] = { 0x0D, 00, 00, 01, 02, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    gchar trame_set_proto[] = { 0x0D, 00, 00, 02, 03, 0x53, 00, 0x80, 0x00, 0x26, 00, 00, 00, 00 };
                                                                         /* 0x20 Oregon */
                                                                         /* 0x08 HomEasy */
                                                                         /* 0x04 AC */
                                                                         /* 0x02 ARC */
                                                                         /* 0x01 X10 */
                                                                   /* 0x08 Lacrosse Frame */
                                                             /* 0x80 Undecoded Frame */
    struct termios oldtio;
    int fd;

    fd = open( Cfg_rfxcom.port, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd<0)
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_ERR,
               "Init_rfxcom: Impossible d'ouvrir le port rfxcom %s, erreur %d:%s",
                Cfg_rfxcom.port, fd, strerror(errno) );
       return(-1);
     }
    else
     { memset(&oldtio, 0, sizeof(oldtio) );
       oldtio.c_cflag = B38400 | CS8 | CREAD | CLOCAL;
       oldtio.c_oflag = 0;
       oldtio.c_iflag = 0;
       oldtio.c_lflag = 0;
       oldtio.c_cc[VTIME]    = 0;
       oldtio.c_cc[VMIN]     = 0;
       tcsetattr(fd, TCSANOW, &oldtio);
       tcflush(fd, TCIOFLUSH);
       Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_NOTICE,
                 "Init_rfxcom: Ouverture port rfxcom okay %s", Cfg_rfxcom.port );
     }
    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_DEBUG, "Init_rfxcom: Sending INIT" );
    if (write (fd, &trame_reset, sizeof(trame_reset) ) == -1)
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_WARNING, "Init_rfxcom: Sending INIT failed " ); }
    sleep(2);
    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_DEBUG, "Init_rfxcom: Sending SET PROTO" );
    if (write (fd, &trame_set_proto, sizeof(trame_set_proto) ) == -1)
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_WARNING, "Init_rfxcom: Sending SET PROTO failed " ); }
    sleep(2);
    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_DEBUG, "Init_rfxcom: Sending GET STATUS" );
    if (write (fd, &trame_get_status, sizeof(trame_get_status) ) == -1)
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_WARNING, "Init_rfxcom: Sending GET STATUS failed " ); }
    return(fd);
  }
/******************************************************************************************************************************/
/* Rfxcom_Envoyer_event : Process un evenement en entrée                                                                      */
/* Entrée: l'evenement a processer                                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Rfxcom_Envoyer_event ( struct CMD_TYPE_MSRV_EVENT *event )
  { gchar trame_send_AC[] = { 0x0B, 0x11, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    gint type,sstype,id1,id2,id3,id4,housecode,unitcode,val, cpt;
    gchar instance[24], thread[24];
    
    if ( strcmp ( event->thread, "MSRV" ) ) return;                                               /* On ecoute que le MSRV ?? */

    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_DEBUG,
             "Rfxcom_envoyer_event: Processing event %s (from instance %s, thread %s)",
              event->objet, event->instance, event->thread );

    if ( sscanf ( event->objet, "%[^:]:%[^:]:%d:%d:%d:%d:%d:%d:%d:%d:%d",
                  instance, thread, &type, &sstype, &id1, &id2, &id3, &id4, &housecode, &unitcode, &val ) != 11 )
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_WARNING,
                "Rfxcom_envoyer_event: Event %s Syntax Error (from instance %s, thread %s)",
                 event->objet, event->instance, event->thread );
       return;
     }

    if ( strcmp ( instance, Config.instance_id ) ) return;                                      /* Are we the right thread ?? */
    if ( strcmp ( thread, NOM_THREAD ) ) return;
    
    if ( type == 0x11 && sstype == 0x00 )                                                             /* Envoi de lighting ?? */
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_DEBUG,
           "Rfxcom_envoyer_sortie: Envoi de %s au module ids=%02d %02d %02d %02d unit %02d",
            (val ? "ON " : "OFF"), id1, id2, id3, id4, unitcode );
       trame_send_AC[0]  = 0x0B; /* Taille */
       trame_send_AC[1]  = 0x11; /* lightning 2 */
       trame_send_AC[2]  = 0x00; /* AC */
       trame_send_AC[3]  = 0x01; /* Seqnbr */
       trame_send_AC[4]  = id1 << 6;
       trame_send_AC[5]  = id2;
       trame_send_AC[6]  = id3;
       trame_send_AC[7]  = id4;
       trame_send_AC[8]  = unitcode;
       trame_send_AC[9]  = (val ? 1 : 0);
       trame_send_AC[10] = 0x0; /* level */
       trame_send_AC[11] = 0x0; /* rssi */
     }
    else { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_WARNING,
                    "Rfxcom_envoyer_event: Event %s not supported (from instance %s, thread %s)",
                     event->objet, event->instance, event->thread );
           return;
         }

    for ( cpt = 0; cpt < 3 ; cpt++)                                                                       /* Send Three times */
     { gint retour;
       retour = write ( Cfg_rfxcom.fd, &trame_send_AC, trame_send_AC[0] + 1 );
       if (retour == -1)
        { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_WARNING,
                   "Rfxcom_envoyer_sortie: Write Error for event %s (%s)", event->objet, strerror(errno) );
        }
     }
  }
/**********************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                   */
/* Entrée: la trame a recue                                                                               */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static int Processer_trame( struct TRAME_RFXCOM *trame )
  { 
    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_DEBUG,
              "Processer_trame taille=%d, type=%02d(0x%02x), sous_type=%02d(0x%02X), seqno=%03d",
               trame->taille, trame->type, trame->type, trame->sous_type, trame->sous_type, trame->seqno );

    if (trame->type == 0x01 && trame->sous_type == 0x00)
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                 "Processer_trame get_status Cmd= %d (0x%2X)", trame->data[0], trame->data[0] );
       if (trame->data[1] == 0x52) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                             "Processer_trame get_status 433MHz receiver only" );   
       if (trame->data[1] == 0x53) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                             "Processer_trame get_status 433MHz transceiver" );   
       Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                 "Processer_trame get_status firmware %d (0x%2X)", trame->data[2], trame->data[2] );
       if (trame->data[3] & 0x80) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Unencoded Frame" );   
       if (trame->data[3] & 0x40) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU6" );   
       if (trame->data[3] & 0x20) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU5" );   
       if (trame->data[3] & 0x10) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU4" );   
       if (trame->data[3] & 0x08) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU3" );   
       if (trame->data[3] & 0x04) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto FineOffset/Viking" );   
       if (trame->data[3] & 0x02) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Rubicson" );   
       if (trame->data[3] & 0x01) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto AE" );   
       if (trame->data[4] & 0x80) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto BlindsT1" );   
       if (trame->data[4] & 0x40) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto BlindsT0" );   
       if (trame->data[4] & 0x20) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto ProGuard" );   
       if (trame->data[4] & 0x10) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto FS20" );   
       if (trame->data[4] & 0x08) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto LaCrosse" );   
       if (trame->data[4] & 0x04) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Hideki" );   
       if (trame->data[4] & 0x02) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto LightwaveRF" );   
       if (trame->data[4] & 0x01) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Mertik" );   
       if (trame->data[5] & 0x80) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Visonic" );   
       if (trame->data[5] & 0x40) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto ATI" );   
       if (trame->data[5] & 0x20) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto OregonScientific" );   
       if (trame->data[5] & 0x10) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto MeianTech" );   
       if (trame->data[5] & 0x08) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto HomeEasy/EU" );   
       if (trame->data[5] & 0x04) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto AC" );   
       if (trame->data[5] & 0x02) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto ARC" );   
       if (trame->data[5] & 0x01) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto X10" );   
     }
    else if (trame->type == 0x02)
     { switch (trame->sous_type)
        { case 0x00: Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                              "Processer_trame : Transceiver message : Error, receiver did not lock" );
                     break;
          case 0x01: switch (trame->data[0])
                      { case 0x00: Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : ACK, transmit OK" );
                                   break;
                        case 0x01: Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : ACK, "
                                            "but transmit started after 3 seconds delay anyway with RF receive data" );
                                   break;
                        case 0x02: Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : NAK, transmitter "
                                            "did not lock on the requested transmit frequency" );
                                   break;
                        case 0x03: Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : NAK, "
                                            "AC address zero in id1-id4 not allowed" );
                                   break;
                        default  : Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : Unknown message..." );
                                   break;
                      }
                     break;
          default :  Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                              "Processer_trame : Transceiver message : unknown packet ssous_type %d", trame->sous_type);
        }
     } 
    else if (trame->type == 0x52 && trame->sous_type == 0x01)                                                       /* Oregon */
     { gchar chaine[128];

       g_snprintf ( chaine, sizeof(chaine), "%02X:%02X:%03d:%03d:%03d:%03d:%03d:%03d:TEMP",
                    trame->type, trame->sous_type, trame->data[0], trame->data[1], 0, 0, 0, 0 );
       Send_Event ( Config.instance_id, NOM_THREAD, EVENT_INPUT, chaine,
                     (trame->data[2] & 0x80 ? -1.0 : 1.0)* ( ((trame->data[2] & 0x7F)<<8) + trame->data[3]) / 10.0 );
                  
       g_snprintf ( chaine, sizeof(chaine), "%02X:%02X:%03d:%03d:%03d:%03d:%03d:%03d:HUMIDITY",
                    trame->type, trame->sous_type, trame->data[0], trame->data[1], 0, 0, 0, 0 );
       Send_Event ( Config.instance_id, NOM_THREAD, EVENT_INPUT, chaine, 1.0 * trame->data[4] );

       g_snprintf ( chaine, sizeof(chaine), "%02X:%02X:%03d:%03d:%03d:%03d:%03d:%03d:BATTERY",
                    trame->type, trame->sous_type, trame->data[0], trame->data[1], 0, 0, 0, 0 );
       Send_Event ( Config.instance_id, NOM_THREAD, EVENT_INPUT, chaine, 1.0 * (trame->data[6] >> 4) );
       
       g_snprintf ( chaine, sizeof(chaine), "%02X:%02X:%03d:%03d:%03d:%03d:%03d:%03d:RSSI",
                    trame->type, trame->sous_type, trame->data[0], trame->data[1], 0, 0, 0, 0 );
       Send_Event ( Config.instance_id, NOM_THREAD, EVENT_INPUT, chaine, 1.0 * (trame->data[6] & 0x0F) );
       
       Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                 "Processer_trame : get status type=%03d(0x%02X), sous_type=%03d(0x%02X), id1=%03d, id2=%03d, high=%03d, "
                 "signe=%02d, low=%03d, hum=%02d, humstatus=%02d, battery=%02d, rssi=%02d",
                 trame->type, trame->type, trame->sous_type, trame->sous_type, trame->data[0], trame->data[1],
                 trame->data[2] & 0x7F, trame->data[2] & 0x80, trame->data[3], trame->data[4], trame->data[5],
                 trame->data[6] >> 4, trame->data[6] & 0x0F
               );   
     }
    else if (trame->type == 0x11 && trame->sous_type == 0x00)                                                /* Lighting 2 AC */
     { gchar chaine[128];

       g_snprintf ( chaine, sizeof(chaine), "%02X:%02X:%03d:%03d:%03d:%03d:%03d:CMD:%03d",
                    trame->type, trame->sous_type, trame->data[0] & 0x03, trame->data[1],
                    trame->data[2], trame->data[3], trame->data[4], trame->data[5] );

       Send_Event ( Config.instance_id, NOM_THREAD, EVENT_INPUT, chaine, 1.0 * trame->data[6] );

       Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                 "Processer_trame : get lighting ! type=%03d(0x%02X), sous_type=%03d(0x%02X), id1=%03d, id2=%03d, "
                 "id3=%03d, id4=%03d, unitcode=%03d, cmnd=%03d, level=%03d rssi=%02d",
                 trame->type, trame->type, trame->sous_type, trame->sous_type, trame->data[0] & 0x03, trame->data[1],
                 trame->data[2], trame->data[3], trame->data[4], trame->data[5],
                 trame->data[6], trame->data[7] & 0x0F
               );   
     }
    else Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                   "Processer_trame unknown packet type %02d(0x%02X), sous_type=%02d(0x%02X)",
                   trame->type, trame->type, trame->sous_type, trame->sous_type );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Rfxcom_Pick_event: Récpère un evenement de la liste des events                                                             */
/* Entrées: néant                                                                                                             */
/* Sortie: l'evenement, à freer a la fin                                                                                      */
/******************************************************************************************************************************/
 static struct CMD_TYPE_MSRV_EVENT *Rfxcom_Pick_event ( void )
  { struct CMD_TYPE_MSRV_EVENT *event;
    if (!Cfg_rfxcom.Liste_events) return(NULL);
    pthread_mutex_lock( &Cfg_rfxcom.lib->synchro );                                                          /* lockage futex */
    event = (struct CMD_TYPE_MSRV_EVENT *)Cfg_rfxcom.Liste_events->data;                            /* Recuperation du numero */
    Cfg_rfxcom.Liste_events = g_slist_remove ( Cfg_rfxcom.Liste_events, event );
    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
             "Rfxcom_Pick_event: Reste a traiter %03d events",
              g_slist_length(Cfg_rfxcom.Liste_events) );
    pthread_mutex_unlock( &Cfg_rfxcom.lib->synchro );
    return(event);
  }
/******************************************************************************************************************************/
/* Rfxcom_Gerer_sortie: Ajoute une demande d'envoi RF dans la liste des envois RFXCOM                                         */
/* Entrées: le numéro de la sortie                                                                                            */
/******************************************************************************************************************************/
 void Rfxcom_Gerer_event( struct CMD_TYPE_MSRV_EVENT *event )                                  /* Num_a est l'id de la sortie */
  { gint taille;

    pthread_mutex_lock( &Cfg_rfxcom.lib->synchro );                                  /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_rfxcom.Liste_events );
    pthread_mutex_unlock( &Cfg_rfxcom.lib->synchro );

    if (taille > MAX_ENREG_QUEUE)
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_WARNING,
                "Rfxcom_Gerer_event: DROP (taille>MAX_ENREG_QUEUE(%d))", MAX_ENREG_QUEUE);
       return;
     }

    pthread_mutex_lock( &Cfg_rfxcom.lib->synchro );                                  /* Ajout dans la liste de tell a traiter */
    Cfg_rfxcom.Liste_events = g_slist_prepend( Cfg_rfxcom.Liste_events, event );
    pthread_mutex_unlock( &Cfg_rfxcom.lib->synchro );
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du thread Rfxcom                                                                                 */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct TRAME_RFXCOM Trame;
    gint retval, nbr_oct_lu;
    struct timeval tv;
    fd_set fdselect;

    prctl(PR_SET_NAME, "W-RFXCOM", 0, 0, 0 );
    memset( &Cfg_rfxcom, 0, sizeof(Cfg_rfxcom) );                                   /* Mise a zero de la structure de travail */
    Cfg_rfxcom.lib = lib;                                          /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_rfxcom.lib->TID = pthread_self();                                                   /* Sauvegarde du TID pour le pere */
    Rfxcom_Lire_config ();                                                  /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );
    Cfg_rfxcom.lib->Thread_run = TRUE;                                                                  /* Le thread tourne ! */

    g_snprintf( lib->admin_prompt, sizeof(lib->admin_prompt), "rfxcom" );
    g_snprintf( lib->admin_help,   sizeof(lib->admin_help),   "Manage RFXCOM sensors" );

    if (!Cfg_rfxcom.enable)
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread is not enabled in config. Shutting Down %p",
                 pthread_self() );
       goto end;
     }

    Abonner_distribution_events ( Rfxcom_Gerer_event, NOM_THREAD );              /* Desabonnement de la diffusion des sorties */
    nbr_oct_lu = 0;
    Cfg_rfxcom.mode = RFXCOM_RETRING;
    while( lib->Thread_run == TRUE)                                      /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_NOTICE, "Run_thread: recu signal SIGUSR1" );
          lib->Thread_sigusr1 = FALSE;
        }

       if (Cfg_rfxcom.reload == TRUE)
        { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_NOTICE, "Run_thread: Reloading in progress" );
          Cfg_rfxcom.reload = FALSE;
        }

       if (Cfg_rfxcom.mode == RFXCOM_WAIT_BEFORE_RETRY)
        { if ( Cfg_rfxcom.date_next_retry <= Partage->top )
		         { Cfg_rfxcom.mode = RFXCOM_RETRING;
			          Cfg_rfxcom.date_next_retry = 0;
		         }
		      }

       if (Cfg_rfxcom.mode == RFXCOM_RETRING)
        { Cfg_rfxcom.fd = Init_rfxcom();
          if (Cfg_rfxcom.fd<0)                                                                 /* On valide l'acces aux ports */
           { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_CRIT,
                      "Run_thread: Init RFXCOM failed. Re-trying in %ds", RFXCOM_RETRY_DELAI/10 );
             Cfg_rfxcom.mode = RFXCOM_WAIT_BEFORE_RETRY;
             Cfg_rfxcom.date_next_retry = Partage->top + RFXCOM_RETRY_DELAI;
           }
          else
           { Cfg_rfxcom.mode = RFXCOM_CONNECTED;
			          Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,"Acces RFXCOM FD=%d", Cfg_rfxcom.fd );
		         }
        }
/*************************************************** Reception trame RFXCOM ***************************************************/
       if (Cfg_rfxcom.mode != RFXCOM_CONNECTED) continue;
       FD_ZERO(&fdselect);                                                             /* Reception sur la ligne serie RFXCOM */
       FD_SET(Cfg_rfxcom.fd, &fdselect );
       tv.tv_sec = 0;
       tv.tv_usec= 100000;
       retval = select(Cfg_rfxcom.fd+1, &fdselect, NULL, NULL, &tv );                               /* Attente d'un caractere */
       if (retval>0 && FD_ISSET(Cfg_rfxcom.fd, &fdselect) )
        { int bute, cpt;

          if (nbr_oct_lu<TAILLE_ENTETE_RFXCOM)
           { bute = TAILLE_ENTETE_RFXCOM; } else { bute = sizeof(Trame); }

          cpt = read( Cfg_rfxcom.fd, (unsigned char *)&Trame + nbr_oct_lu, bute-nbr_oct_lu );
          if (cpt>0)
           { nbr_oct_lu = nbr_oct_lu + cpt;

             if (nbr_oct_lu >= TAILLE_ENTETE_RFXCOM + Trame.taille)                                       /* traitement trame */
              { nbr_oct_lu = 0;
                if (Trame.taille > 0) Processer_trame( &Trame );
                memset (&Trame, 0, sizeof(struct TRAME_RFXCOM) );
              }
           }
        }
       else if ( retval < 0 )                                     /* Si erreur, on ferme la connexion et on retente plus tard */
        { close(Cfg_rfxcom.fd);
	         Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_ERR,
                   "Run_thread: Select Error (%s), closing connexion and re-trying in %ds",
                    strerror(errno), RFXCOM_RETRY_DELAI/10 );
          Cfg_rfxcom.mode = RFXCOM_WAIT_BEFORE_RETRY;
          Cfg_rfxcom.date_next_retry = Partage->top + RFXCOM_RETRY_DELAI;
        }

       if (!(Partage->top % 50))                                                                /* Test toutes les 5 secondes */
        { gboolean closing = FALSE;
          struct stat buf;
	         gint retour;
		        retour = fstat( Cfg_rfxcom.fd, &buf );
		        if (retour == -1)
           { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_ERR,
                      "Run_thread: Fstat Error (%s), closing connexion and re-trying in %ds",
                       strerror(errno), RFXCOM_RETRY_DELAI/10 );
             closing = TRUE;
           }
          else if ( buf.st_nlink < 1 )
           { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_ERR,
                      "Run_thread: USB device disappeared. Closing connexion and re-trying in %ds", RFXCOM_RETRY_DELAI/10 );
             closing = TRUE;
           }
          if (closing == TRUE)
           { close(Cfg_rfxcom.fd);
             Cfg_rfxcom.mode = RFXCOM_WAIT_BEFORE_RETRY;
             Cfg_rfxcom.date_next_retry = Partage->top + RFXCOM_RETRY_DELAI;
           }
        }
/************************************************** Transmission des trames aux sorties ***************************************/
       if (Cfg_rfxcom.Liste_events)                                                           /* Si pas de message, on tourne */
        { struct CMD_TYPE_MSRV_EVENT *event;
          event = Rfxcom_Pick_event();                                                             /* Recuperation d'un event */
          Rfxcom_Envoyer_event ( event );
          g_free(event);
        }
     }                                                                                         /* Fin du while partage->arret */

    close(Cfg_rfxcom.fd);                                                                     /* Fermeture de la connexion FD */
end:
    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_rfxcom.lib->Thread_run = FALSE;                                                         /* Le thread ne tourne plus ! */
    Cfg_rfxcom.lib->TID = 0;                                                  /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
