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
 static void Processer_trame( void )
  { 
    if ( ! strncmp ( Cfg_teleinfo.buffer, "ADCO", 4 ) )
     { SEA( Cfg_teleinfo.min_ea, atoi( Cfg_teleinfo.buffer + 5 ) );
     }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "ISOUS", 5 ) )
     { SEA( Cfg_teleinfo.min_ea + 1, atoi( Cfg_teleinfo.buffer + 6 ) );
     }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "HCHC", 4 ) )
     { SEA( Cfg_teleinfo.min_ea + 2, atoi( Cfg_teleinfo.buffer + 5 ) );
     }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "HCHP", 4 ) )
     { SEA( Cfg_teleinfo.min_ea + 3, atoi( Cfg_teleinfo.buffer + 5 ) );
     }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "IINST", 5 ) )
     { SEA( Cfg_teleinfo.min_ea + 4, atoi( Cfg_teleinfo.buffer + 6 ) );
     }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "IMAX", 4 ) )
     { SEA( Cfg_teleinfo.min_ea + 5, atoi( Cfg_teleinfo.buffer + 5 ) );
     }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "PAPP", 4 ) )
     { SEA( Cfg_teleinfo.min_ea + 6, atoi( Cfg_teleinfo.buffer + 5 ) );
     }
    else Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_DEBUG,
                   "Processer_trame unknown trame = %s", Cfg_teleinfo.buffer );
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
           { if (Cfg_teleinfo.buffer[nbr_octet_lu] == '\n')
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
