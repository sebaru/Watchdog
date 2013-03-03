/**********************************************************************************************************/
/* Watchdogd/Teleinfo/Teleinfo.c  Gestion des capteurs TELEINFO Watchdog 2.0                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 27 mai 2012 12:52:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Teleinfo.c
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
 #include "Teleinfo.h"

/**********************************************************************************************************/
/* Teleinfo_Lire_config : Lit la config Watchdog et rempli la structure mémoire                             */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Teleinfo_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Teleinfo_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_teleinfo.lib->Thread_debug = g_key_file_get_boolean ( gkf, "TELEINFO", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */

    chaine = g_key_file_get_string ( gkf, "TELEINFO", "port", NULL );
    if (!chaine)
     { Info_new ( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR,
                  "Teleinfo_Lire_config: port is missing. Using default." );
       g_snprintf( Cfg_teleinfo.port, sizeof(Cfg_teleinfo.port), DEFAUT_PORT_TELEINFO );
     }
    else
     { g_snprintf( Cfg_teleinfo.port, sizeof(Cfg_teleinfo.port), "%s", chaine );
       g_free(chaine);
     }

    Cfg_teleinfo.min_ea = g_key_file_get_integer ( gkf, "TELEINFO", "min_ea", NULL );
    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Teleinfo_Liberer_config : Libere la mémoire allouer précédemment pour lire la config teleinfo          */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Teleinfo_Liberer_config ( void )
  { 
  }
/**********************************************************************************************************/
/* Init_teleinfo: Initialisation de la ligne TELEINFO                                                     */
/* Sortie: l'identifiant de la connexion                                                                  */
/**********************************************************************************************************/
 static int Init_teleinfo ( void )
  { struct termios oldtio;
    int fd;

    fd = open( Cfg_teleinfo.port, O_RDONLY | O_NOCTTY | O_NONBLOCK );
    if (fd<0)
     { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR,
               "Init_teleinfo: Impossible d'ouvrir le port teleinfo %s, erreur %d", Cfg_teleinfo.port, fd );
       return(-1);
     }
    memset(&oldtio, 0, sizeof(oldtio) );
    oldtio.c_cflag = B9600 | CS7 | CREAD | CLOCAL | PARENB;
    oldtio.c_oflag = 0;
    oldtio.c_iflag = 0;
    oldtio.c_lflag = 0;
    oldtio.c_cc[VTIME]    = 0;
    oldtio.c_cc[VMIN]     = 0;
    tcsetattr(fd, TCSANOW, &oldtio);
    tcflush(fd, TCIOFLUSH);
    Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE,
              "Init_teleinfo: Ouverture port teleinfo okay %s", Cfg_teleinfo.port );
    return(fd);
  }
/**********************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                   */
/* Entrée: la trame a recue                                                                               */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static int Processer_trame( void )
  { 

    Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_DEBUG,
              "Processer_trame %s", Cfg_teleinfo.buffer );
#ifdef bouh

    if (trame->type == 0x01 && trame->sous_type == 0x00)
     { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                 "Processer_trame get_status Cmd= %d (0x%2X)", trame->data[0], trame->data[0] );
       if (trame->data[1] == 0x52) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                             "Processer_trame get_status 433MHz receiver only" );   
       if (trame->data[1] == 0x53) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                             "Processer_trame get_status 433MHz transceiver" );   
       Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                 "Processer_trame get_status firmware %d (0x%2X)", trame->data[2], trame->data[2] );
       if (trame->data[3] & 0x80) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Unencoded Frame" );   
       if (trame->data[3] & 0x40) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU6" );   
       if (trame->data[3] & 0x20) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU5" );   
       if (trame->data[3] & 0x10) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU4" );   
       if (trame->data[3] & 0x08) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU3" );   
       if (trame->data[3] & 0x04) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto FineOffset/Viking" );   
       if (trame->data[3] & 0x02) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Rubicson" );   
       if (trame->data[3] & 0x01) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto AE" );   
       if (trame->data[4] & 0x80) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto BlindsT1" );   
       if (trame->data[4] & 0x40) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto BlindsT0" );   
       if (trame->data[4] & 0x20) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto ProGuard" );   
       if (trame->data[4] & 0x10) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto FS20" );   
       if (trame->data[4] & 0x08) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto LaCrosse" );   
       if (trame->data[4] & 0x04) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Hideki" );   
       if (trame->data[4] & 0x02) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto LightwaveRF" );   
       if (trame->data[4] & 0x01) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Mertik" );   
       if (trame->data[5] & 0x80) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Visonic" );   
       if (trame->data[5] & 0x40) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto ATI" );   
       if (trame->data[5] & 0x20) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto OregonScientific" );   
       if (trame->data[5] & 0x10) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto MeianTech" );   
       if (trame->data[5] & 0x08) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto HomeEasy/EU" );   
       if (trame->data[5] & 0x04) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto AC" );   
       if (trame->data[5] & 0x02) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto ARC" );   
       if (trame->data[5] & 0x01) Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto X10" );   
     }
    else if (trame->type == 0x02)
     { switch (trame->sous_type)
        { case 0x00: Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                              "Processer_trame : Transceiver message : Error, receiver did not lock" );
                     break;
          case 0x01: switch (trame->data[0])
                      { case 0x00: Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : ACK, transmit OK" );
                                   break;
                        case 0x01: Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : ACK, "
                                            "but transmit started after 3 seconds delay anyway with RF receive data" );
                                   break;
                        case 0x02: Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : NAK, transmitter "
                                            "did not lock on the requested transmit frequency" );
                                   break;
                        case 0x03: Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : NAK, "
                                            "AC address zero in id1-id4 not allowed" );
                                   break;
                        default  : Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : Unknown message..." );
                                   break;
                      }
                     break;
          default :  Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                              "Processer_trame : Transceiver message : unknown packet ssous_type %d", trame->sous_type);
        }
     } 
    else if (trame->type == 0x52 && trame->sous_type == 0x01)
     { struct MODULE_TELEINFO *module;
       Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                 "Processer_trame : get status type=%03d(0x%02X), sous_type=%03d(0x%02X), id1=%03d, id2=%03d, high=%03d, "
                 "signe=%02d, low=%03d, hum=%02d, humstatus=%02d, battery=%02d, rssi=%02d",
                 trame->type, trame->type, trame->sous_type, trame->sous_type, trame->data[0], trame->data[1],
                 trame->data[2] & 0x7F, trame->data[2] & 0x80, trame->data[3], trame->data[4], trame->data[5],
                 trame->data[6] >> 4, trame->data[6] & 0x0F
               );   
       module = Chercher_teleinfo( trame->type, trame->sous_type, TRUE, trame->data[0], TRUE, trame->data[1],
                                 FALSE, 0, FALSE, 0, FALSE, 0, FALSE, 0 );
       if (module)
        { SEA( module->teleinfo.ea_min,     (trame->data[2] & 0x80 ? -1.0 : 1.0)* ( (trame->data[2] & 0x7F) + trame->data[3])
                                           / 10.0 );                                              /* Temp */
          SEA( module->teleinfo.ea_min + 1,  trame->data[4] );                                  /* Humidity */
          SEA( module->teleinfo.ea_min + 2,  trame->data[6] >> 4);                               /* Battery */
          SEA( module->teleinfo.ea_min + 3,  trame->data[6] & 0x0F );                               /* RSSI */

          module->date_last_view = Partage->top;
        }
       else Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                      "Processer_trame: No module found for packet received type=%02d(0x%02X), sous_type=%02d(0x%02X)",
                      trame->type, trame->type, trame->sous_type, trame->sous_type );
     }
    else Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                   "Processer_trame unknown packet type %02d(0x%02X), sous_type=%02d(0x%02X)",
                   trame->type, trame->type, trame->sous_type, trame->sous_type );
#endif
    return(TRUE);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Teleinfo                                                             */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { gint retval, nbr_octet_lu;
    struct timeval tv;
    fd_set fdselect;

    prctl(PR_SET_NAME, "W-TINFOEDF", 0, 0, 0 );
    memset( &Cfg_teleinfo, 0, sizeof(Cfg_teleinfo) );               /* Mise a zero de la structure de travail */
    Cfg_teleinfo.lib = lib;                      /* Sauvegarde de la structure pointant sur cette librairie */
    Teleinfo_Lire_config ();                              /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );
    Cfg_teleinfo.lib->Thread_run = TRUE;                                              /* Le thread tourne ! */

    g_snprintf( lib->admin_prompt, sizeof(lib->admin_prompt), "teleinfo" );
    g_snprintf( lib->admin_help,   sizeof(lib->admin_help),   "Manage TELEINFO sensor" );

    Cfg_teleinfo.fd = Init_teleinfo();
    if (Cfg_teleinfo.fd<0)                                                   /* On valide l'acces aux ports */
     { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_CRIT,
                 "Run_thread: Init TELEINFO failed. exiting..." );
       Teleinfo_Liberer_config ();
       lib->Thread_run = FALSE;                                             /* Le thread ne tourne plus ! */
       lib->TID = 0;                                      /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     }
    else { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,"Acces TELEINFO FD=%d", Cfg_teleinfo.fd ); }

    nbr_octet_lu = 0;
    while( lib->Thread_run == TRUE)                                      /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE, "Run_thread: recu signal SIGUSR1" );
          lib->Thread_sigusr1 = FALSE;
        }

       if (Cfg_teleinfo.reload == TRUE)
        { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE, "Run_thread: Reloading in progress" );
          Cfg_teleinfo.reload = FALSE;
        }

/***************************************** Reception trame TELEINFO ***************************************/
       FD_ZERO(&fdselect);                                       /* Reception sur la ligne serie TELEINFO */
       FD_SET(Cfg_teleinfo.fd, &fdselect );
       tv.tv_sec = 1;
       tv.tv_usec= 0;
       retval = select(Cfg_teleinfo.fd+1, &fdselect, NULL, NULL, &tv );         /* Attente d'un caractere */
       if (retval>=0 && FD_ISSET(Cfg_teleinfo.fd, &fdselect) )
        { int cpt;

          cpt = read( Cfg_teleinfo.fd, (unsigned char *)&Cfg_teleinfo.buffer + nbr_octet_lu, 1 );
          if (cpt>0)
           { printf("Octet recu = %d, %c\n", Cfg_teleinfo.buffer[nbr_octet_lu], Cfg_teleinfo.buffer[nbr_octet_lu] );
             if (Cfg_teleinfo.buffer[nbr_octet_lu] == '\n')
              { Processer_trame();
                nbr_octet_lu = 0;
                memset (&Cfg_teleinfo.buffer, 0, TAILLE_BUFFER_TELEINFO );
              } else if (nbr_octet_lu<TAILLE_BUFFER_TELEINFO-cpt) nbr_octet_lu += cpt;
                else nbr_octet_lu = 0;
           }
        }
     }
    close(Cfg_teleinfo.fd);                                                 /* Fermeture de la connexion FD */

    Teleinfo_Liberer_config();                                  /* Liberation de la configuration du thread */
    Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_teleinfo.lib->TID = 0;                              /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
