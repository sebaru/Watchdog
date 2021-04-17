/******************************************************************************************************************************/
/* Watchdogd/Teleinfo/Teleinfo.c  Gestion des capteurs TELEINFO Watchdog 2.0                                                  */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         dim. 27 mai 2012 12:52:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Teleinfo.c
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

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Teleinfo.h"
 struct TELEINFO_CONFIG Cfg_teleinfo;
/******************************************************************************************************************************/
/* Teleinfo_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                               */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Teleinfo_Lire_config ( void )
  { gchar *result, requete[256];
    struct DB *db;

    Creer_configDB ( NOM_THREAD, "debug", "false" );
    result = Recuperer_configDB_by_nom ( NOM_THREAD, "debug" );
    Cfg_teleinfo.lib->Thread_debug = !g_ascii_strcasecmp(result, "true");
    g_free(result);

    SQL_Write_new ( "INSERT IGNORE %s SET tech_id='EDF01', description='DEFAULT', port='/dev/null', "
                    "instance='%s'", NOM_THREAD, g_get_host_name() );

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),
               "SELECT tech_id, description, port "
               " FROM %s WHERE instance='%s' LIMIT 1", NOM_THREAD, g_get_host_name());
    if (!Lancer_requete_SQL ( db, requete ))
     { Libere_DB_SQL ( &db );
       Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR, "%s: DB Requete failed", __func__ );
       return(FALSE);
     }

    if (Recuperer_ligne_SQL ( db ))
     { g_snprintf( Cfg_teleinfo.tech_id,     sizeof(Cfg_teleinfo.tech_id),     "%s", db->row[0] );
       g_snprintf( Cfg_teleinfo.description, sizeof(Cfg_teleinfo.description), "%s", db->row[1] );
       g_snprintf( Cfg_teleinfo.port,        sizeof(Cfg_teleinfo.port),        "%s", db->row[2] );
     }
    else Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR, "%s: DB Get Result failed", __func__ );
    Libere_DB_SQL ( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Modbus_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                 */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Teleinfo_Creer_DB ( void )
  { gint database_version;

    gchar *database_version_string = Recuperer_configDB_by_nom( NOM_THREAD, "database_version" );
    if (database_version_string)
     { database_version = atoi( database_version_string );
       g_free(database_version_string);
     } else database_version=0;

    Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'. Thread_Version '%s'.", __func__, database_version, WTD_VERSION );

    if (database_version==0)
     { SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s` ("
                       "`id` int(11) NOT NULL AUTO_INCREMENT,"
                       "`instance` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT 'localhost',"
                       "`date_create` datetime NOT NULL DEFAULT NOW(),"
                       "`tech_id` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                       "`description` VARCHAR(80) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                       "`port` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                       "PRIMARY KEY (`id`)"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", NOM_THREAD );
       goto end;
     }

end:
    database_version = 1;
    Modifier_configDB_int ( NOM_THREAD, "database_version", database_version );
  }
/******************************************************************************************************************************/
/* Init_teleinfo: Initialisation de la ligne TELEINFO                                                                         */
/* Sortie: l'identifiant de la connexion                                                                                      */
/******************************************************************************************************************************/
 static int Init_teleinfo ( void )
  { struct termios oldtio;
    int fd;

    fd = open( Cfg_teleinfo.port, O_RDONLY | O_NOCTTY | O_NONBLOCK | O_CLOEXEC );
    if (fd<0)
     { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR,
               "%s: Impossible d'ouvrir le port teleinfo '%s', erreur %d", __func__, Cfg_teleinfo.port, fd );
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
              "%s: Ouverture port teleinfo okay %s", __func__, Cfg_teleinfo.port );

    if (!Cfg_teleinfo.nbr_connexion)
     { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO,
                "%s: %s: Initialise le DLS et charge les AI ", __func__, Cfg_teleinfo.tech_id );
       if (Dls_auto_create_plugin( Cfg_teleinfo.tech_id, "Gestion du compteur EDF" ) == FALSE)
        { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", __func__, Cfg_teleinfo.tech_id ); }

       Mnemo_auto_create_WATCHDOG ( FALSE, Cfg_teleinfo.tech_id, "IO_COMM", "Statut de la communication avec le compteur EDF" );

       Mnemo_auto_create_AI ( FALSE, Cfg_teleinfo.tech_id, "ADCO",  "N° d’identification du compteur", "numéro" );
       Mnemo_auto_create_AI ( FALSE, Cfg_teleinfo.tech_id, "ISOUS", "Intensité EDF souscrite ", "A" );
       Mnemo_auto_create_AI ( FALSE, Cfg_teleinfo.tech_id, "BASE",  "Index option BASE", "Wh" );
       Mnemo_auto_create_AI ( FALSE, Cfg_teleinfo.tech_id, "HCHC",  "Index heures creuses", "Wh" );
       Mnemo_auto_create_AI ( FALSE, Cfg_teleinfo.tech_id, "HCHP",  "Index heures pleines", "Wh" );
       Mnemo_auto_create_AI ( FALSE, Cfg_teleinfo.tech_id, "IINST", "Intensité EDF instantanée", "A" );
       Mnemo_auto_create_AI ( FALSE, Cfg_teleinfo.tech_id, "IMAX",  "Intensité EDF maximale", "A" );
       Mnemo_auto_create_AI ( FALSE, Cfg_teleinfo.tech_id, "PAPP",  "Puissance apparente EDF consommée", "VA" );
     }
    Cfg_teleinfo.comm_status = TRUE;
    Cfg_teleinfo.nbr_connexion++;
    return(fd);
  }
/******************************************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                                       */
/* Entrée: la trame a recue                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Send_AI_to_master ( gchar *name, gchar *chaine )                                      /* Envoi au master via ZMQ */
  { Zmq_Send_AI_to_master ( Cfg_teleinfo.zmq_to_master, NOM_THREAD, Cfg_teleinfo.tech_id, name, atof( chaine ), TRUE );
  }
/******************************************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                                       */
/* Entrée: la trame a recue                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Processer_trame( void )
  {      if ( ! strncmp ( Cfg_teleinfo.buffer, "ADCO", 4 ) )  { Send_AI_to_master ( "ADCO", Cfg_teleinfo.buffer + 5 ); }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "ISOUS", 5 ) ) { Send_AI_to_master ( "ISOUS", Cfg_teleinfo.buffer + 6 ); }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "BASE", 4 ) )  { Send_AI_to_master ( "BASE", Cfg_teleinfo.buffer + 5 ); }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "HCHC", 4 ) )  { Send_AI_to_master ( "HCHC", Cfg_teleinfo.buffer + 5 ); }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "HCHP", 4 ) )  { Send_AI_to_master ( "HCHP", Cfg_teleinfo.buffer + 5 ); }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "IINST", 5 ) ) { Send_AI_to_master ( "IINST", Cfg_teleinfo.buffer + 6 ); }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "IMAX", 4 ) )  { Send_AI_to_master ( "IMAX", Cfg_teleinfo.buffer + 5 ); }
    else if ( ! strncmp ( Cfg_teleinfo.buffer, "PAPP", 4 ) )  { Send_AI_to_master ( "PAPP", Cfg_teleinfo.buffer + 5 ); }
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

reload:
    memset( &Cfg_teleinfo, 0, sizeof(Cfg_teleinfo) );                               /* Mise a zero de la structure de travail */
    Cfg_teleinfo.lib = lib;                                        /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "W-TINFOEDF", "I/O", lib, WTD_VERSION, "Manage TELEINFOEDF Sensors" );
    Teleinfo_Creer_DB ();                                                                   /* Création de la base de données */
    Teleinfo_Lire_config ();                                                /* Lecture de la configuration logiciel du thread */

    Cfg_teleinfo.zmq_to_master = Zmq_Connect ( ZMQ_PUB, "pub-to-master",  "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

    nbr_octet_lu = 0;                                                               /* Initialisation des compteurs et buffer */
    memset (&Cfg_teleinfo.buffer, 0, TAILLE_BUFFER_TELEINFO );
    Cfg_teleinfo.mode = TINFO_RETRING;
    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (Cfg_teleinfo.mode == TINFO_WAIT_BEFORE_RETRY)
        { if ( Cfg_teleinfo.date_next_retry <= Partage->top )
           { Cfg_teleinfo.mode = TINFO_RETRING;
             Cfg_teleinfo.date_next_retry = 0;
             Cfg_teleinfo.nbr_connexion = 0;
             Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_NOTICE, "%s: Retrying Connexion.", __func__ );

           }
          else continue;
        }

       if (Cfg_teleinfo.mode == TINFO_RETRING)
        { Cfg_teleinfo.fd = Init_teleinfo();
          if (Cfg_teleinfo.fd<0)                                                               /* On valide l'acces aux ports */
           { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR,
                       "%s: Init TELEINFO failed. Re-trying in %ds", __func__, TINFO_RETRY_DELAI/10 );
             Cfg_teleinfo.mode = TINFO_WAIT_BEFORE_RETRY;
             Cfg_teleinfo.date_next_retry = Partage->top + TINFO_RETRY_DELAI;
           }
          else
           { Cfg_teleinfo.mode = TINFO_CONNECTED;
             Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_INFO, "%s: Acces TELEINFO FD=%d", __func__, Cfg_teleinfo.fd );
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
                if ( Cfg_teleinfo.lib->comm_next_update < Partage->top )
                 { Zmq_Send_WATCHDOG_to_master ( Cfg_teleinfo.zmq_to_master, NOM_THREAD, Cfg_teleinfo.tech_id, "IO_COMM", 400 );
                   Cfg_teleinfo.lib->comm_next_update = Partage->top + 300;
                 }
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
                             "%s: BufferOverflow, dropping trame (nbr_octet_lu=%d, cpt=%d, taille buffer=%d)",
                              __func__, nbr_octet_lu, cpt, TAILLE_BUFFER_TELEINFO  );
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
                      "%s: Fstat Error (%s), closing connexion and re-trying in %ds", __func__,
                       strerror(errno), TINFO_RETRY_DELAI/10 );
             closing = TRUE;
           }
          else if ( buf.st_nlink < 1 )
           { Info_new( Config.log, Cfg_teleinfo.lib->Thread_debug, LOG_ERR,
                      "%s: USB device disappeared. Closing connexion and re-trying in %ds", __func__, TINFO_RETRY_DELAI/10 );
             closing = TRUE;
           }
          if (closing == TRUE)
           { close(Cfg_teleinfo.fd);
             Cfg_teleinfo.mode = TINFO_WAIT_BEFORE_RETRY;
             Cfg_teleinfo.date_next_retry = Partage->top + TINFO_RETRY_DELAI;
             Zmq_Send_WATCHDOG_to_master ( Cfg_teleinfo.zmq_to_master, NOM_THREAD, Cfg_teleinfo.tech_id, "IO_COMM", 0 );
             Cfg_teleinfo.comm_status = FALSE;
           }
        }
     }
    close(Cfg_teleinfo.fd);                                                                   /* Fermeture de la connexion FD */
    Zmq_Close ( Cfg_teleinfo.zmq_to_master );

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
