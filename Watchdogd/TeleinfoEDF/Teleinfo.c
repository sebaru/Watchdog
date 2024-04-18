/******************************************************************************************************************************/
/* Watchdogd/Teleinfo/Teleinfo.c  Gestion des capteurs TELEINFO Watchdog 2.0                                                  */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         dim. 27 mai 2012 12:52:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Teleinfo.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
/* Init_teleinfo: Initialisation de la ligne TELEINFO                                                                         */
/* Sortie: l'identifiant de la connexion                                                                                      */
/******************************************************************************************************************************/
 static int Init_teleinfo ( struct THREAD *module )
  { struct termios oldtio;
    int fd;

    gchar *port    = Json_get_string ( module->config, "port" );
    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    fd = open( port, O_RDONLY | O_NOCTTY | O_NONBLOCK | O_CLOEXEC );
    if (fd<0)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
               "%s: Impossible d'ouvrir le port teleinfo '%s', erreur %d", thread_tech_id, port, fd );
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
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "Ouverture port teleinfo okay %s", port );

    Thread_send_comm_to_master ( module, TRUE );
    return(fd);
  }
/******************************************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                                       */
/* Entrée: la trame a recue                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Processer_trame( struct THREAD *module )
  { struct TELEINFO_VARS *vars = module->vars;

    if (Json_get_bool ( module->config, "standard" ) == FALSE )
     {      if ( ! strncmp ( vars->buffer, "ADCO", 4 ) )  { MQTT_Send_AI ( module, vars->Adco,  atof(vars->buffer + 5), TRUE ); }
       else if ( ! strncmp ( vars->buffer, "ISOUS", 5 ) ) { MQTT_Send_AI ( module, vars->Isous, atof(vars->buffer + 6), TRUE ); }
       else if ( ! strncmp ( vars->buffer, "BASE", 4 ) )  { MQTT_Send_AI ( module, vars->Base,  atof(vars->buffer + 5), TRUE ); }
       else if ( ! strncmp ( vars->buffer, "HCHC", 4 ) )  { MQTT_Send_AI ( module, vars->Hchc,  atof(vars->buffer + 5), TRUE ); }
       else if ( ! strncmp ( vars->buffer, "HCHP", 4 ) )  { MQTT_Send_AI ( module, vars->Hchp,  atof(vars->buffer + 5), TRUE ); }
       else if ( ! strncmp ( vars->buffer, "IINST", 5 ) ) { MQTT_Send_AI ( module, vars->Iinst, atof(vars->buffer + 6), TRUE ); }
       else if ( ! strncmp ( vars->buffer, "IMAX", 4 ) )  { MQTT_Send_AI ( module, vars->Imax,  atof(vars->buffer + 5), TRUE ); }
       else if ( ! strncmp ( vars->buffer, "PAPP", 4 ) )  { MQTT_Send_AI ( module, vars->Papp,  atof(vars->buffer + 5), TRUE ); }
/* Other buffer : HHPHC, MOTDETAT, PTEC, OPTARIF */
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_INFO, "Recu %s", vars->buffer );
            if ( ! strncmp ( vars->buffer, "ADCO", 4 ) )  { MQTT_Send_AI ( module, vars->Adco,  atof(vars->buffer + 5), TRUE ); }
     }

    vars->last_view = Partage->top;
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct TELEINFO_VARS) );
    struct TELEINFO_VARS *vars = module->vars;
    gint retval, nbr_octet_lu;
    struct timeval tv;
    fd_set fdselect;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    vars->Adco  = Mnemo_create_thread_AI ( module, "ADCO",  "N° d’identification du compteur", "numéro", ARCHIVE_1_JOUR );
    vars->Isous = Mnemo_create_thread_AI ( module, "ISOUS", "Intensité EDF souscrite ", "A", ARCHIVE_1_MIN );
    vars->Base  = Mnemo_create_thread_AI ( module, "BASE",  "Index option BASE", "Wh", ARCHIVE_1_MIN );
    vars->Hchc  = Mnemo_create_thread_AI ( module, "HCHC",  "Index heures creuses", "Wh", ARCHIVE_1_MIN );
    vars->Hchp  = Mnemo_create_thread_AI ( module, "HCHP",  "Index heures pleines", "Wh", ARCHIVE_1_MIN );
    vars->Iinst = Mnemo_create_thread_AI ( module, "IINST", "Intensité EDF instantanée", "A", ARCHIVE_1_MIN );
    vars->Imax  = Mnemo_create_thread_AI ( module, "IMAX",  "Intensité EDF maximale", "A", ARCHIVE_1_MIN );
    vars->Papp  = Mnemo_create_thread_AI ( module, "PAPP",  "Puissance apparente EDF consommée", "VA", ARCHIVE_1_MIN );

    nbr_octet_lu = 0;                                                               /* Initialisation des compteurs et buffer */
    memset (&vars->buffer, 0, TAILLE_BUFFER_TELEINFO );
    vars->mode = TINFO_RETRING;
    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/****************************************************** Ecoute du master ******************************************************/
       while ( module->MQTT_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *message = module->MQTT_messages->data;
          module->MQTT_messages = g_slist_remove ( module->MQTT_messages, message );
          pthread_mutex_unlock ( &module->synchro );
          gchar *tag = Json_get_string ( message, "tag" );
          Info_new( __func__, module->Thread_debug, LOG_DEBUG, "%s: tag '%s' not for this thread", thread_tech_id, tag );
          Json_node_unref(message);
        }
/************************************************* Traitement opérationnel ****************************************************/
       if (vars->mode == TINFO_WAIT_BEFORE_RETRY)
        { if ( vars->date_next_retry <= Partage->top )
           { vars->mode = TINFO_RETRING;
             vars->date_next_retry = 0;
             Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Retrying Connexion.", thread_tech_id );
           }
        }
       else if (vars->mode == TINFO_RETRING)
        { vars->fd = Init_teleinfo( module );
          if (vars->fd<0)                                                               /* On valide l'acces aux ports */
           { Info_new( __func__, module->Thread_debug, LOG_ERR,
                       "%s: Init TELEINFO failed. Re-trying in %ds", thread_tech_id, TINFO_RETRY_DELAI/10 );
             vars->mode = TINFO_WAIT_BEFORE_RETRY;
             vars->date_next_retry = Partage->top + TINFO_RETRY_DELAI;
           }
          else
           { vars->mode = TINFO_CONNECTED;
             Info_new( __func__, module->Thread_debug, LOG_INFO, "%s: Acces TELEINFO FD=%d", thread_tech_id, vars->fd );
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
                Thread_send_comm_to_master ( module, TRUE );
              }
             else if (nbr_octet_lu + cpt < TAILLE_BUFFER_TELEINFO)                        /* Encore en dessous de la limite ? */
              { nbr_octet_lu += cpt; }                                                   /* Preparation du prochain caractere */
             else { nbr_octet_lu = 0;                                                              /* Depassement de tampon ! */
                    memset (&vars->buffer, 0, TAILLE_BUFFER_TELEINFO );
                    Info_new( __func__, module->Thread_debug, LOG_ERR,
                             "%s: %s: BufferOverflow, dropping trame (nbr_octet_lu=%d, cpt=%d, taille buffer=%d)",
                              __func__, thread_tech_id, nbr_octet_lu, cpt, TAILLE_BUFFER_TELEINFO );
                  }
           }
        }
       if (!(Partage->top % 50))                                                                /* Test toutes les 5 secondes */
        { gboolean closing = FALSE;
          struct stat buf;
          gint retour;
          retour = fstat( vars->fd, &buf );
          if (retour == -1)
           { Info_new( __func__, module->Thread_debug, LOG_ERR,
                      "%s: Fstat Error (%s), closing connexion and re-trying in %ds", thread_tech_id,
                       strerror(errno), TINFO_RETRY_DELAI/10 );
             closing = TRUE;
           }
          else if ( buf.st_nlink < 1 )
           { Info_new( __func__, module->Thread_debug, LOG_ERR,
                      "%s: USB device disappeared. Closing connexion and re-trying in %ds", thread_tech_id, TINFO_RETRY_DELAI/10 );
             closing = TRUE;
           }
          if (closing == TRUE)
           { close(vars->fd);
             vars->mode = TINFO_WAIT_BEFORE_RETRY;
             vars->date_next_retry = Partage->top + TINFO_RETRY_DELAI;
             Thread_send_comm_to_master ( module, FALSE );
           }
        }
     }
    close(vars->fd);                                                                          /* Fermeture de la connexion FD */

    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
