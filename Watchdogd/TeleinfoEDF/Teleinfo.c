/******************************************************************************************************************************/
/* Watchdogd/Teleinfo/Teleinfo.c  Gestion des capteurs TELEINFO Watchdog 2.0                                                  */
/* Projet Abls-Habitat version 4.0       Gestion d'habitat                                     dim. 27 mai 2012 12:52:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Teleinfo.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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
    return(fd);
  }
/******************************************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                                       */
/* Entrée: la trame a recue                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Processer_trame( gboolean mode_standard, struct THREAD *module )
  { struct TELEINFO_VARS *vars = module->vars;

    if (mode_standard == FALSE)
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
     { gint taille = strlen(vars->buffer) + 1;
            if (g_str_has_prefix ( vars->buffer, "IRMS1"   )) { MQTT_Send_AI ( module, vars->IRMS1,  atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "IRMS2"   )) { MQTT_Send_AI ( module, vars->IRMS2,  atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "IRMS3"   )) { MQTT_Send_AI ( module, vars->IRMS3,  atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "URMS1"   )) { MQTT_Send_AI ( module, vars->URMS1,  atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "URMS2"   )) { MQTT_Send_AI ( module, vars->URMS2,  atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "URMS3"   )) { MQTT_Send_AI ( module, vars->URMS3,  atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "PREF"    )) { MQTT_Send_AI ( module, vars->PREF,   atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "PCOUP"   )) { MQTT_Send_AI ( module, vars->PCOUP,  atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "SINSTS"  )) { MQTT_Send_AI ( module, vars->SINSTS, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "SINSTS1" )) { MQTT_Send_AI ( module, vars->SINSTS, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "SINSTS2" )) { MQTT_Send_AI ( module, vars->SINSTS, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "SINSTS3" )) { MQTT_Send_AI ( module, vars->SINSTS, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "SMAXSN"  )) { MQTT_Send_AI ( module, vars->SMAXSN, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "SMAXSN1" )) { MQTT_Send_AI ( module, vars->SMAXSN, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "SMAXSN2" )) { MQTT_Send_AI ( module, vars->SMAXSN, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "SMAXSN3" )) { MQTT_Send_AI ( module, vars->SMAXSN, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "UMOY1"   )) { MQTT_Send_AI ( module, vars->UMOY1,  atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "UMOY2"   )) { MQTT_Send_AI ( module, vars->UMOY2,  atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "UMOY3"   )) { MQTT_Send_AI ( module, vars->UMOY3,  atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "NTARF"   )) { MQTT_Send_AI ( module, vars->NTARF,  atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "ADSC"    )) { MQTT_Send_AI ( module, vars->ADSC,   atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EAST"    )) { MQTT_Send_AI ( module, vars->EAST,   atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASF01"  )) { MQTT_Send_AI ( module, vars->EASF01, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASF02"  )) { MQTT_Send_AI ( module, vars->EASF02, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASF03"  )) { MQTT_Send_AI ( module, vars->EASF03, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASF04"  )) { MQTT_Send_AI ( module, vars->EASF04, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASF05"  )) { MQTT_Send_AI ( module, vars->EASF05, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASF06"  )) { MQTT_Send_AI ( module, vars->EASF06, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASF07"  )) { MQTT_Send_AI ( module, vars->EASF07, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASF08"  )) { MQTT_Send_AI ( module, vars->EASF08, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASF09"  )) { MQTT_Send_AI ( module, vars->EASF09, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASF10"  )) { MQTT_Send_AI ( module, vars->EASF10, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASD01"  )) { MQTT_Send_AI ( module, vars->EASD01, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASD02"  )) { MQTT_Send_AI ( module, vars->EASD02, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASD03"  )) { MQTT_Send_AI ( module, vars->EASD03, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "EASD04"  )) { MQTT_Send_AI ( module, vars->EASD04, atof(vars->buffer + taille), TRUE ); }
       else if (g_str_has_prefix ( vars->buffer, "PRM"     )) { MQTT_Send_AI ( module, vars->PRM,    atof(vars->buffer + taille), TRUE ); }
     }
    vars->last_view = Partage->top;
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct TELEINFO_VARS) );
    struct TELEINFO_VARS *vars = module->vars;
    gint retval, nbr_octet_lu;
    struct timeval tv;
    fd_set fdselect;

    gchar *thread_tech_id  = Json_get_string ( module->config, "thread_tech_id" );
    gboolean mode_standard = Json_get_bool ( module->config, "standard" );

    if (mode_standard == FALSE)
     { vars->Adco  = Mnemo_create_thread_AI ( module, "ADCO",  "N° d’identification du compteur", "numéro", ARCHIVE_1_JOUR );
       vars->Isous = Mnemo_create_thread_AI ( module, "ISOUS", "Intensité EDF souscrite ", "A", ARCHIVE_1_MIN );
       vars->Base  = Mnemo_create_thread_AI ( module, "BASE",  "Index option BASE", "Wh", ARCHIVE_1_MIN );
       vars->Hchc  = Mnemo_create_thread_AI ( module, "HCHC",  "Index heures creuses", "Wh", ARCHIVE_1_MIN );
       vars->Hchp  = Mnemo_create_thread_AI ( module, "HCHP",  "Index heures pleines", "Wh", ARCHIVE_1_MIN );
       vars->Iinst = Mnemo_create_thread_AI ( module, "IINST", "Intensité EDF instantanée", "A", ARCHIVE_1_MIN );
       vars->Imax  = Mnemo_create_thread_AI ( module, "IMAX",  "Intensité EDF maximale", "A", ARCHIVE_1_MIN );
       vars->Papp  = Mnemo_create_thread_AI ( module, "PAPP",  "Puissance apparente EDF consommée", "VA", ARCHIVE_1_MIN );
     }
    else
     { vars->IRMS1    = Mnemo_create_thread_AI ( module, "IRMS1",   "Courant efficace, phase 1", "A", ARCHIVE_1_MIN );
       vars->IRMS2    = Mnemo_create_thread_AI ( module, "IRMS2",   "Courant efficace, phase 2", "A", ARCHIVE_1_MIN );
       vars->IRMS3    = Mnemo_create_thread_AI ( module, "IRMS3",   "Courant efficace, phase 3", "A", ARCHIVE_1_MIN );
       vars->URMS1    = Mnemo_create_thread_AI ( module, "URMS1",   "Tension efficace, phase 1", "V", ARCHIVE_1_MIN );
       vars->URMS2    = Mnemo_create_thread_AI ( module, "URMS2",   "Tension efficace, phase 2", "V", ARCHIVE_1_MIN );
       vars->URMS3    = Mnemo_create_thread_AI ( module, "URMS3",   "Tension efficace, phase 3", "V", ARCHIVE_1_MIN );
       vars->PREF     = Mnemo_create_thread_AI ( module, "PREF",    "Puissance app. de référence (souscrite)", "kVA", ARCHIVE_1_JOUR );
       vars->PCOUP    = Mnemo_create_thread_AI ( module, "PCOUP",   "Puissance app. de coupure", "kVA", ARCHIVE_1_JOUR );
       vars->SINSTS   = Mnemo_create_thread_AI ( module, "SINSTS",  "Puissance app. Instantanée soutirée", "VA", ARCHIVE_1_MIN );
       vars->SINSTS1  = Mnemo_create_thread_AI ( module, "SINSTS1", "Puissance app. Instantanée soutirée phase 1", "VA", ARCHIVE_1_MIN );
       vars->SINSTS2  = Mnemo_create_thread_AI ( module, "SINSTS2", "Puissance app. Instantanée soutirée phase 2", "VA", ARCHIVE_1_MIN );
       vars->SINSTS3  = Mnemo_create_thread_AI ( module, "SINSTS3", "Puissance app. Instantanée soutirée phase 3", "VA", ARCHIVE_1_MIN );
       vars->SMAXSN   = Mnemo_create_thread_AI ( module, "SMAXSN",  "Puissance app. max. soutirée n", "VA", ARCHIVE_1_HEURE );
       vars->SMAXSN1  = Mnemo_create_thread_AI ( module, "SMAXSN1", "Puissance app. max. soutirée n phase 1", "VA", ARCHIVE_1_HEURE );
       vars->SMAXSN2  = Mnemo_create_thread_AI ( module, "SMAXSN2", "Puissance app. max. soutirée n phase 2", "VA", ARCHIVE_1_HEURE );
       vars->SMAXSN3  = Mnemo_create_thread_AI ( module, "SMAXSN3", "Puissance app. max. soutirée n phase 3", "VA", ARCHIVE_1_HEURE );
       vars->UMOY1    = Mnemo_create_thread_AI ( module, "UMOY1",   "Tension moyenne phase 1", "V", ARCHIVE_1_HEURE );
       vars->UMOY2    = Mnemo_create_thread_AI ( module, "UMOY2",   "Tension moyenne phase 2", "V", ARCHIVE_1_HEURE );
       vars->UMOY3    = Mnemo_create_thread_AI ( module, "UMOY3",   "Tension moyenne phase 3", "V", ARCHIVE_1_HEURE );;
       vars->NTARF    = Mnemo_create_thread_AI ( module, "NTARF",   "Numéro de l’index tarifaire en cours", "", ARCHIVE_1_HEURE );
       vars->ADSC     = Mnemo_create_thread_AI ( module, "ADSC",    "Adresse Secondaire du Compteur (n° de série)", "", ARCHIVE_1_JOUR );
       vars->EAST     = Mnemo_create_thread_AI ( module, "EAST",    "Energie active soutirée totale", "Wh", ARCHIVE_1_MIN );
       vars->EASF01   = Mnemo_create_thread_AI ( module, "EASF01",  "Energie active soutirée Fournisseur, index 01", "Wh", ARCHIVE_5_MIN );
       vars->EASF02   = Mnemo_create_thread_AI ( module, "EASF02",  "Energie active soutirée Fournisseur, index 02", "Wh", ARCHIVE_5_MIN );
       vars->EASF03   = Mnemo_create_thread_AI ( module, "EASF03",  "Energie active soutirée Fournisseur, index 03", "Wh", ARCHIVE_5_MIN );
       vars->EASF04   = Mnemo_create_thread_AI ( module, "EASF04",  "Energie active soutirée Fournisseur, index 04", "Wh", ARCHIVE_5_MIN );
       vars->EASF05   = Mnemo_create_thread_AI ( module, "EASF05",  "Energie active soutirée Fournisseur, index 05", "Wh", ARCHIVE_5_MIN );
       vars->EASF06   = Mnemo_create_thread_AI ( module, "EASF06",  "Energie active soutirée Fournisseur, index 06", "Wh", ARCHIVE_5_MIN );
       vars->EASF07   = Mnemo_create_thread_AI ( module, "EASF07",  "Energie active soutirée Fournisseur, index 07", "Wh", ARCHIVE_5_MIN );
       vars->EASF08   = Mnemo_create_thread_AI ( module, "EASF08",  "Energie active soutirée Fournisseur, index 08", "Wh", ARCHIVE_5_MIN );
       vars->EASF09   = Mnemo_create_thread_AI ( module, "EASF09",  "Energie active soutirée Fournisseur, index 09", "Wh", ARCHIVE_5_MIN );
       vars->EASF10   = Mnemo_create_thread_AI ( module, "EASF10",  "Energie active soutirée Fournisseur, index 10", "Wh", ARCHIVE_5_MIN );
       vars->EASD01   = Mnemo_create_thread_AI ( module, "EASD01",  "Energie active soutirée Distributeur, index 01", "Wh", ARCHIVE_5_MIN );
       vars->EASD02   = Mnemo_create_thread_AI ( module, "EASD02",  "Energie active soutirée Distributeur, index 02", "Wh", ARCHIVE_5_MIN );
       vars->EASD03   = Mnemo_create_thread_AI ( module, "EASD03",  "Energie active soutirée Distributeur, index 03", "Wh", ARCHIVE_5_MIN );
       vars->EASD04   = Mnemo_create_thread_AI ( module, "EASD04",  "Energie active soutirée Distributeur, index 04", "Wh", ARCHIVE_5_MIN );
       vars->PRM      = Mnemo_create_thread_AI ( module, "PRM",     "N° de compteur (ex-PDL)", "", ARCHIVE_1_JOUR );
       /*vars->STGE     = Mnemo_create_thread_AI ( module, "STGE",    "Puissance apparente EDF consommée", "VA", ARCHIVE_1_MIN );
       vars->SMAXSN-1 = Mnemo_create_thread_AI ( module, "SMAXSN-1",  "Puissance apparente EDF consommée", "VA", ARCHIVE_1_MIN );
       vars->RELAIS   = Mnemo_create_thread_AI ( module, "RELAIS",  "Etat Relais Heure Creuse", "bool", ARCHIVE_5_MIN );
       vars->NJOURF   = Mnemo_create_thread_AI ( module, "NJOURF",  "Numéro du jour en cours calendrier fournisseur", "", ARCHIVE_1_HEURE );
       vars->VTIC     = Mnemo_create_thread_AI ( module, "VTIC",    "Version de la TIC", "VA", ARCHIVE_1_MIN );
       vars->SMAXSN1-1 = Mnemo_create_thread_AI ( module, "SMAXSN1-1",  "Puissance apparente EDF consommée", "VA", ARCHIVE_1_MIN );
       vars->SMAXSN2-1 = Mnemo_create_thread_AI ( module, "SMAXSN2-1",  "Puissance apparente EDF consommée", "VA", ARCHIVE_1_MIN );
       vars->SMAXSN3-1 = Mnemo_create_thread_AI ( module, "SMAXSN3-1",  "Puissance apparente EDF consommée", "VA", ARCHIVE_1_MIN );
       vars->MSG1
       vars->NJOURF+1 = Mnemo_create_thread_AI ( module, "NJOURF+1",  "Puissance apparente EDF consommée", "VA", ARCHIVE_1_MIN );
       vars->PJOURF+1
       vars->DATE
       vars->NGTF
       vars->LTARF  HEURES PLEINES */
     }

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
                Processer_trame( mode_standard, module );
                nbr_octet_lu = 0;
                memset (&vars->buffer, 0, TAILLE_BUFFER_TELEINFO );
                Thread_send_comm_to_master ( module, TRUE );
              }
             else if (nbr_octet_lu + cpt < TAILLE_BUFFER_TELEINFO)                        /* Encore en dessous de la limite ? */
              { if (mode_standard)
                 {      if (vars->buffer[nbr_octet_lu] == '\t') vars->buffer[nbr_octet_lu] = 0;        /* Change \t into null */
                   else if (vars->buffer[nbr_octet_lu] == '\r') vars->buffer[nbr_octet_lu] = 0;        /* Change \r into null */
                 }
                nbr_octet_lu += cpt;                                                     /* Preparation du prochain caractere */
              }
             else { nbr_octet_lu = 0;                                                              /* Depassement de tampon ! */
                    memset (&vars->buffer, 0, TAILLE_BUFFER_TELEINFO );
                    Info_new( __func__, module->Thread_debug, LOG_ERR,
                             "BufferOverflow, dropping trame (nbr_octet_lu=%d, cpt=%d, taille buffer=%d)",
                              nbr_octet_lu, cpt, TAILLE_BUFFER_TELEINFO );
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
                      "USB device disappeared. Closing connexion and re-trying in %ds", TINFO_RETRY_DELAI/10 );
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
