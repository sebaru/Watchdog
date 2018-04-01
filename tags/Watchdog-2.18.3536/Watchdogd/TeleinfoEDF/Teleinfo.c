/******************************************************************************************************************************/
/* Watchdogd/Teleinfo/Teleinfo.c  Gestion des capteurs TELEINFO Watchdog 2.0                                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         dim. 27 mai 2012 12:52:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Teleinfo.h"

/******************************************************************************************************************************/
/* Teleinfo_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                               */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Teleinfo_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_teleinfo.lib->Thread_debug = FALSE;                                                    /* Settings default parameters */
    Cfg_teleinfo.enable            = FALSE; 
    g_snprintf( Cfg_teleinfo.port, sizeof(Cfg_teleinfo.port),
               "%s", DEFAUT_PORT_TELEINFO );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_WARNING,
                "Sms_Lire_config: Database connexion failed. Using Default Parameters" );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,                                        /* Print Config */
                "Teleinfo_Lire_config: '%s' = %s", nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "port" ) )
        { g_snprintf( Cfg_teleinfo.port, sizeof(Cfg_teleinfo.port), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_teleinfo.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_teleinfo.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE,
                   "Sms_Lire_config: Unknown Parameter '%s'(='%s') in Database", nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Init_teleinfo: Initialisation de la ligne TELEINFO                                                                         */
/* Sortie: l'identifiant de la connexion                                                                                      */
/******************************************************************************************************************************/
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
/******************************************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                                       */
/* Entrée: la trame a recue                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Processer_trame( void )
  { 
    if ( (! strncmp ( Cfg_teleinfo.buffer, "ADCO", 4 )) && Cfg_teleinfo.last_view_adco + 300 <= Partage->top )
     { Send_Event ( g_get_host_name(), NOM_THREAD, EVENT_INPUT, "ADCO", atof( Cfg_teleinfo.buffer + 5) );
       Cfg_teleinfo.last_view_adco = Partage->top;
     }
    else if ( (! strncmp ( Cfg_teleinfo.buffer, "ISOUS", 5 )) && Cfg_teleinfo.last_view_isous + 300 <= Partage->top )
     { Send_Event ( g_get_host_name(), NOM_THREAD, EVENT_INPUT, "ISOUS", atof( Cfg_teleinfo.buffer + 6) );
       Cfg_teleinfo.last_view_isous = Partage->top;
     }
    else if ( (! strncmp ( Cfg_teleinfo.buffer, "HCHC", 4 )) && Cfg_teleinfo.last_view_hchc + 300 <= Partage->top )
     { Send_Event ( g_get_host_name(), NOM_THREAD, EVENT_INPUT, "HCHC", atof( Cfg_teleinfo.buffer + 5) );
	      Cfg_teleinfo.last_view_hchc = Partage->top;
     }
    else if ( (! strncmp ( Cfg_teleinfo.buffer, "HCHP", 4 )) && Cfg_teleinfo.last_view_hchp + 300 <= Partage->top )
     { Send_Event ( g_get_host_name(), NOM_THREAD, EVENT_INPUT, "HCHP", atof( Cfg_teleinfo.buffer + 5) );
	      Cfg_teleinfo.last_view_hchp = Partage->top;
     }
    else if ( (! strncmp ( Cfg_teleinfo.buffer, "IINST", 5 )) && Cfg_teleinfo.last_view_iinst + 300 <= Partage->top )
     { Send_Event ( g_get_host_name(), NOM_THREAD, EVENT_INPUT, "IINST", atof( Cfg_teleinfo.buffer + 6) );
	      Cfg_teleinfo.last_view_iinst = Partage->top;
     }
    else if ( (! strncmp ( Cfg_teleinfo.buffer, "IMAX", 4 )) && Cfg_teleinfo.last_view_imax + 300 <= Partage->top )
     { Send_Event ( g_get_host_name(), NOM_THREAD, EVENT_INPUT, "IMAX", atof( Cfg_teleinfo.buffer + 5) );
	      Cfg_teleinfo.last_view_imax = Partage->top;
     }
    else if ( (! strncmp ( Cfg_teleinfo.buffer, "PAPP", 4 )) && Cfg_teleinfo.last_view_papp + 300 <= Partage->top )
     { Send_Event ( g_get_host_name(), NOM_THREAD, EVENT_INPUT, "PAPP", atof( Cfg_teleinfo.buffer + 5) );
	      Cfg_teleinfo.last_view_papp = Partage->top;
     }
    else if ( (! strncmp ( Cfg_teleinfo.buffer, "PTEC", 4 )) && Cfg_teleinfo.last_view_ptec + 300 <= Partage->top )
     { Send_Event ( g_get_host_name(), NOM_THREAD, EVENT_INPUT, "PTEC", atof( Cfg_teleinfo.buffer + 5) );
	      Cfg_teleinfo.last_view_ptec = Partage->top;
     }
    else if ( (! strncmp ( Cfg_teleinfo.buffer, "HHPHC", 5 )) && Cfg_teleinfo.last_view_hhchp + 300 <= Partage->top )
     { Send_Event ( g_get_host_name(), NOM_THREAD, EVENT_INPUT, "HHPHC", atof( Cfg_teleinfo.buffer + 6) );
	      Cfg_teleinfo.last_view_hhchp = Partage->top;
     }
    else if ( (! strncmp ( Cfg_teleinfo.buffer, "OPTARIF", 7 )) && Cfg_teleinfo.last_view_optarif + 300 <= Partage->top )
     { Send_Event ( g_get_host_name(), NOM_THREAD, EVENT_INPUT, "OPTARIF", atof( Cfg_teleinfo.buffer + 8) );
	      Cfg_teleinfo.last_view_optarif = Partage->top;
     }
    else { return; }
/* Other buffer : HHPHC, MOTDETAT, PTEC, OPTARIF */
    Cfg_teleinfo.last_view = Partage->top;
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du thread Teleinfo                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { gint retval, nbr_octet_lu;
    struct timeval tv;
    fd_set fdselect;

    prctl(PR_SET_NAME, "W-TINFOEDF", 0, 0, 0 );
    memset( &Cfg_teleinfo, 0, sizeof(Cfg_teleinfo) );                               /* Mise a zero de la structure de travail */
    Cfg_teleinfo.lib = lib;                                        /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_teleinfo.lib->TID = pthread_self();                                                 /* Sauvegarde du TID pour le pere */
    Teleinfo_Lire_config ();                                                /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );
    Cfg_teleinfo.lib->Thread_run = TRUE;                                                                /* Le thread tourne ! */

    g_snprintf( lib->admin_prompt, sizeof(lib->admin_prompt), NOM_THREAD );
    g_snprintf( lib->admin_help,   sizeof(lib->admin_help),   "Manage TELEINFOEDF sensors" );

    if (!Cfg_teleinfo.enable)
     { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread is not enabled in config. Shutting Down %p",
                 pthread_self() );
       goto end;
     }

    nbr_octet_lu = 0;                                                               /* Initialisation des compteurs et buffer */
    memset (&Cfg_teleinfo.buffer, 0, TAILLE_BUFFER_TELEINFO );
    Cfg_teleinfo.mode = TINFO_RETRING;
    while( lib->Thread_run == TRUE)                                                          /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE, "Run_thread: recu signal SIGUSR1" );
          lib->Thread_sigusr1 = FALSE;
        }

       if (Cfg_teleinfo.reload == TRUE)
        { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE, "Run_thread: Reloading in progress" );
          close(Cfg_teleinfo.fd);                                                             /* Fermeture de la connexion FD */
          Cfg_teleinfo.fd = Init_teleinfo();
          if (Cfg_teleinfo.fd<0)                                                               /* On valide l'acces aux ports */
           { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_CRIT,
                      "Run_thread: Reloading with port %s failed", Cfg_teleinfo.port );
           }
          Cfg_teleinfo.reload = FALSE;
        }

       if (Cfg_teleinfo.mode == TINFO_WAIT_BEFORE_RETRY)
        { if ( Cfg_teleinfo.date_next_retry <= Partage->top )
		   { Cfg_teleinfo.mode = TINFO_RETRING;
			 Cfg_teleinfo.date_next_retry = 0;
		   }
		  else continue;
		}

       if (Cfg_teleinfo.mode == TINFO_RETRING)
        { Cfg_teleinfo.fd = Init_teleinfo();
          if (Cfg_teleinfo.fd<0)                                                               /* On valide l'acces aux ports */
           { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR,
                       "Run_thread: Init TELEINFO failed. Re-trying in %ds", TINFO_RETRY_DELAI/10 );
             Cfg_teleinfo.mode = TINFO_WAIT_BEFORE_RETRY;
             Cfg_teleinfo.date_next_retry = Partage->top + TINFO_RETRY_DELAI;
           }
          else
           { Cfg_teleinfo.mode = TINFO_CONNECTED;
			 Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,"Acces TELEINFO FD=%d", Cfg_teleinfo.fd );
		   }
        }

/************************************************ Reception trame TELEINFO ****************************************************/
       if (Cfg_teleinfo.mode != TINFO_CONNECTED) continue;
       FD_ZERO(&fdselect);                                                           /* Reception sur la ligne serie TELEINFO */
       FD_SET(Cfg_teleinfo.fd, &fdselect );
       tv.tv_sec = 1;
       tv.tv_usec= 0;
       retval = select(Cfg_teleinfo.fd+1, &fdselect, NULL, NULL, &tv );                             /* Attente d'un caractere */
       if (retval>=0 && FD_ISSET(Cfg_teleinfo.fd, &fdselect) )
        { int cpt;

          cpt = read( Cfg_teleinfo.fd, (unsigned char *)&Cfg_teleinfo.buffer + nbr_octet_lu, 1 );
          if (cpt>0)
           { if (Cfg_teleinfo.buffer[nbr_octet_lu] == '\n')                                          /* Process de la trame ? */
              { Cfg_teleinfo.buffer[nbr_octet_lu] = 0x0;                                            /* Caractère fin de trame */
                Processer_trame();
                nbr_octet_lu = 0;
                memset (&Cfg_teleinfo.buffer, 0, TAILLE_BUFFER_TELEINFO );
              }
             else if (nbr_octet_lu + cpt < TAILLE_BUFFER_TELEINFO)                        /* Encore en dessous de la limite ? */
              { /* Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_DEBUG,
                         "Run_thread: Get one char : %d, %c (pos %d)",
                          Cfg_teleinfo.buffer[nbr_octet_lu], Cfg_teleinfo.buffer[nbr_octet_lu], nbr_octet_lu );*/
                nbr_octet_lu += cpt;                                                     /* Preparation du prochain caractere */
              }
             else { nbr_octet_lu = 0;                                                              /* Depassement de tampon ! */
                    memset (&Cfg_teleinfo.buffer, 0, TAILLE_BUFFER_TELEINFO );
                    Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_DEBUG,
                             "Run_thread: BufferOverflow, dropping trame" );
                  }
             
           }
        }
       if (!(Partage->top % 50))                                                                /* Test toutes les 5 secondes */
        { gboolean closing = FALSE;
          struct stat buf;
	      gint retour;
		  retour = fstat( Cfg_teleinfo.fd, &buf );
		  if (retour == -1)
           { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR,
                      "Run_thread: Fstat Error (%s), closing connexion and re-trying in %ds",
                       strerror(errno), TINFO_RETRY_DELAI/10 );
             closing = TRUE;
           }
          else if ( buf.st_nlink < 1 )
           { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR,
                      "Run_thread: USB device disappeared. Closing connexion and re-trying in %ds", TINFO_RETRY_DELAI/10 );
             closing = TRUE;
           }
          if (closing == TRUE)
           { close(Cfg_teleinfo.fd);
             Cfg_teleinfo.mode = TINFO_WAIT_BEFORE_RETRY;
             Cfg_teleinfo.date_next_retry = Partage->top + TINFO_RETRY_DELAI;
           }
        }
     }
    close(Cfg_teleinfo.fd);                                                                   /* Fermeture de la connexion FD */

end:
    Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_teleinfo.lib->Thread_run = FALSE;                                                       /* Le thread ne tourne plus ! */
    Cfg_teleinfo.lib->TID = 0;                                                /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
