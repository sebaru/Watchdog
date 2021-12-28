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

/******************************************************************************************************************************/
/* Teleinfo_Creer_DB : Creation de la database du process                                                                     */
/* Entrée: le pointeur sur la structure PROCESS                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Teleinfo_Creer_DB ( struct PROCESS *lib )
  {
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'.", __func__, lib->database_version );

    SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s` ("
                    "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                    "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                    "`uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
                    "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                    "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "`port` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "FOREIGN KEY (`uuid`) REFERENCES `processes` (`uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                    ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", lib->name );

    Process_set_database_version ( lib, 1 );
  }
/******************************************************************************************************************************/
/* Init_teleinfo: Initialisation de la ligne TELEINFO                                                                         */
/* Sortie: l'identifiant de la connexion                                                                                      */
/******************************************************************************************************************************/
 static int Init_teleinfo ( struct SUBPROCESS *module )
  { struct TELEINFO_VARS *vars = module->vars;
    struct termios oldtio;
    int fd;

    gchar *port    = Json_get_string ( module->config, "port" );
    gchar *tech_id = Json_get_string ( module->config, "tech_id" );

    fd = open( port, O_RDONLY | O_NOCTTY | O_NONBLOCK | O_CLOEXEC );
    if (fd<0)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
               "%s: %s: Impossible d'ouvrir le port teleinfo '%s', erreur %d", __func__, tech_id, port, fd );
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
    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: Ouverture port teleinfo okay %s", __func__, port );

    if (!vars->nbr_connexion)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
                "%s: %s: Charge les AI ", __func__, tech_id );

       Mnemo_auto_create_AI ( FALSE, tech_id, "ADCO",  "N° d’identification du compteur", "numéro" );
       Mnemo_auto_create_AI ( FALSE, tech_id, "ISOUS", "Intensité EDF souscrite ", "A" );
       Mnemo_auto_create_AI ( FALSE, tech_id, "BASE",  "Index option BASE", "Wh" );
       Mnemo_auto_create_AI ( FALSE, tech_id, "HCHC",  "Index heures creuses", "Wh" );
       Mnemo_auto_create_AI ( FALSE, tech_id, "HCHP",  "Index heures pleines", "Wh" );
       Mnemo_auto_create_AI ( FALSE, tech_id, "IINST", "Intensité EDF instantanée", "A" );
       Mnemo_auto_create_AI ( FALSE, tech_id, "IMAX",  "Intensité EDF maximale", "A" );
       Mnemo_auto_create_AI ( FALSE, tech_id, "PAPP",  "Puissance apparente EDF consommée", "VA" );
     }
    SubProcess_send_comm_to_master_new ( module, TRUE );
    vars->nbr_connexion++;
    return(fd);
  }
/******************************************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                                       */
/* Entrée: la trame a recue                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Processer_trame( struct SUBPROCESS *module )
  { struct TELEINFO_VARS *vars = module->vars;
    gchar *tech_id = Json_get_string ( module->config, "tech_id" );

         if ( ! strncmp ( vars->buffer, "ADCO", 4 ) )  { Zmq_Send_AI_to_master_new ( module, tech_id, "ADCO",  atof(vars->buffer + 5), TRUE ); }
    else if ( ! strncmp ( vars->buffer, "ISOUS", 5 ) ) { Zmq_Send_AI_to_master_new ( module, tech_id, "ISOUS", atof(vars->buffer + 6), TRUE ); }
    else if ( ! strncmp ( vars->buffer, "BASE", 4 ) )  { Zmq_Send_AI_to_master_new ( module, tech_id, "BASE",  atof(vars->buffer + 5), TRUE ); }
    else if ( ! strncmp ( vars->buffer, "HCHC", 4 ) )  { Zmq_Send_AI_to_master_new ( module, tech_id, "HCHC",  atof(vars->buffer + 5), TRUE ); }
    else if ( ! strncmp ( vars->buffer, "HCHP", 4 ) )  { Zmq_Send_AI_to_master_new ( module, tech_id, "HCHP",  atof(vars->buffer + 5), TRUE ); }
    else if ( ! strncmp ( vars->buffer, "IINST", 5 ) ) { Zmq_Send_AI_to_master_new ( module, tech_id, "IINST", atof(vars->buffer + 6), TRUE ); }
    else if ( ! strncmp ( vars->buffer, "IMAX", 4 ) )  { Zmq_Send_AI_to_master_new ( module, tech_id, "IMAX",  atof(vars->buffer + 5), TRUE ); }
    else if ( ! strncmp ( vars->buffer, "PAPP", 4 ) )  { Zmq_Send_AI_to_master_new ( module, tech_id, "PAPP",  atof(vars->buffer + 5), TRUE ); }
/* Other buffer : HHPHC, MOTDETAT, PTEC, OPTARIF */
    vars->last_view = Partage->top;
  }
/******************************************************************************************************************************/
/* Run_subprocess: Prend en charge un des sous process du thread                                                              */
/* Entrée: la structure SUBPROCESS associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_subprocess ( struct SUBPROCESS *module )
  { SubProcess_init ( module, sizeof(struct TELEINFO_VARS) );
    struct TELEINFO_VARS *vars = module->vars;
    gint retval, nbr_octet_lu;
    struct timeval tv;
    fd_set fdselect;

    gchar *tech_id = Json_get_string ( module->config, "tech_id" );

    nbr_octet_lu = 0;                                                               /* Initialisation des compteurs et buffer */
    memset (&vars->buffer, 0, TAILLE_BUFFER_TELEINFO );
    vars->mode = TINFO_RETRING;
    while(module->lib->Thread_run == TRUE && module->lib->Thread_reload == FALSE)            /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       SubProcess_send_comm_to_master_new ( module, module->comm_status );         /* Périodiquement envoie la comm au master */
/***************************************************** Ecoute du master *******************************************************/
       JsonNode *request;
       while ( (request = SubProcess_Listen_to_master_new ( module ) ) != NULL)
        { /*gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );*/
          json_node_unref(request);
        }

/************************************************* Traitement opérationnel ****************************************************/
       if (vars->mode == TINFO_WAIT_BEFORE_RETRY)
        { if ( vars->date_next_retry <= Partage->top )
           { vars->mode = TINFO_RETRING;
             vars->date_next_retry = 0;
             vars->nbr_connexion = 0;
             Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: Retrying Connexion.", __func__ );
           }
        }
       else if (vars->mode == TINFO_RETRING)
        { vars->fd = Init_teleinfo( module );
          if (vars->fd<0)                                                               /* On valide l'acces aux ports */
           { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                       "%s: %s: Init TELEINFO failed. Re-trying in %ds", __func__, tech_id, TINFO_RETRY_DELAI/10 );
             vars->mode = TINFO_WAIT_BEFORE_RETRY;
             vars->date_next_retry = Partage->top + TINFO_RETRY_DELAI;
           }
          else
           { vars->mode = TINFO_CONNECTED;
             Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: %s: Acces TELEINFO FD=%d", __func__, tech_id, vars->fd );
           }
        }
       if (vars->mode != TINFO_CONNECTED) continue;
/************************************************ Gestion trame TELEINFO ******************************************************/
       FD_ZERO(&fdselect);                                                           /* Reception sur la ligne serie TELEINFO */
       FD_SET(vars->fd, &fdselect );
       tv.tv_sec = 1;
       tv.tv_usec= 0;
       retval = select(vars->fd+1, &fdselect, NULL, NULL, &tv );                             /* Attente d'un caractere */
       if (retval>=0 && FD_ISSET(vars->fd, &fdselect) )
        { int cpt;

          cpt = read( vars->fd, (unsigned char *)&vars->buffer + nbr_octet_lu, 1 );
          if (cpt>0)
           { if (vars->buffer[nbr_octet_lu] == '\n')                                          /* Process de la trame ? */
              { vars->buffer[nbr_octet_lu] = 0x0;                                            /* Caractère fin de trame */
                Processer_trame( module );
                nbr_octet_lu = 0;
                memset (&vars->buffer, 0, TAILLE_BUFFER_TELEINFO );
                SubProcess_send_comm_to_master_new ( module, TRUE );
              }
             else if (nbr_octet_lu + cpt < TAILLE_BUFFER_TELEINFO)                        /* Encore en dessous de la limite ? */
              { /* Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
                         "Run_process: Get one char : %d, %c (pos %d)",
                          vars->buffer[nbr_octet_lu], vars->buffer[nbr_octet_lu], nbr_octet_lu );*/
                nbr_octet_lu += cpt;                                                     /* Preparation du prochain caractere */
              }
             else { nbr_octet_lu = 0;                                                              /* Depassement de tampon ! */
                    memset (&vars->buffer, 0, TAILLE_BUFFER_TELEINFO );
                    Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                             "%s: %s: BufferOverflow, dropping trame (nbr_octet_lu=%d, cpt=%d, taille buffer=%d)",
                              __func__, tech_id, nbr_octet_lu, cpt, TAILLE_BUFFER_TELEINFO  );
                  }
           }
        }
       if (!(Partage->top % 50))                                                                /* Test toutes les 5 secondes */
        { gboolean closing = FALSE;
          struct stat buf;
          gint retour;
          retour = fstat( vars->fd, &buf );
          if (retour == -1)
           { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                      "%s: %s: Fstat Error (%s), closing connexion and re-trying in %ds", __func__, tech_id,
                       strerror(errno), TINFO_RETRY_DELAI/10 );
             closing = TRUE;
           }
          else if ( buf.st_nlink < 1 )
           { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                      "%s: %s: USB device disappeared. Closing connexion and re-trying in %ds", __func__, tech_id, TINFO_RETRY_DELAI/10 );
             closing = TRUE;
           }
          if (closing == TRUE)
           { close(vars->fd);
             vars->mode = TINFO_WAIT_BEFORE_RETRY;
             vars->date_next_retry = Partage->top + TINFO_RETRY_DELAI;
             SubProcess_send_comm_to_master_new ( module, FALSE );
           }
        }
     }
    close(vars->fd);                                                                          /* Fermeture de la connexion FD */

    SubProcess_end(module);
  }
/******************************************************************************************************************************/
/* Run_process: Run du Process                                                                                                */
/* Entrée: la structure PROCESS associée                                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_process ( struct PROCESS *lib )
  {
reload:
    Teleinfo_Creer_DB ( lib );                                                                 /* Création de la DB du thread */
    Thread_init ( "teleinfoedf", "I/O", lib, WTD_VERSION, "Manage TELEINFOEDF Sensors" );

    lib->config = Json_node_create();
    if(lib->config) SQL_Select_to_json_node ( lib->config, "subprocess", "SELECT * FROM %s WHERE uuid='%s'", lib->name, lib->uuid );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: %d subprocess to load", __func__, Json_get_int ( lib->config, "nbr_subprocess" ) );

    Json_node_foreach_array_element ( lib->config, "subprocess", Process_Load_one_subprocess, lib );/* Chargement des modules */
    while( lib->Thread_run == TRUE && lib->Thread_reload == FALSE) sleep(1);                 /* On tourne tant que necessaire */
    Process_Unload_all_subprocess ( lib );

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }

    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
