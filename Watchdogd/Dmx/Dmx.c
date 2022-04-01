/******************************************************************************************************************************/
/* Watchdogd/Dmx/Dmx.c  Gestion des modules MODBUS Watchdog 2.0                                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    22.10.2019 23:42:08 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Dmx.c
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

 #include <stdio.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <errno.h>
 #include <sys/ioctl.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <semaphore.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Dmx.h"

/******************************************************************************************************************************/
/* Dmx_do_mapping : mappe les entrees/sorties Wago avec la zone de mémoire interne dynamique                                  */
/* Entrée : la structure referencant le module                                                                                */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Dmx_do_mapping ( struct SUBPROCESS *module )
  { struct DMX_VARS *vars = module->vars;
    gchar critere[80];
    struct DB *db;

    gchar *tech_id = Json_get_string ( module->config, "tech_id" );
/******************************* Recherche des event text EA a raccrocher aux bits internes ***********************************/
    gint cpt = 0;
    g_snprintf( critere, sizeof(critere),"%s:AO%%", tech_id );
    if ( ! Recuperer_mnemos_AO_by_text ( &db, "dmx", critere ) )
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: Error searching Database for '%s'", __func__, critere ); }
    else while ( Recuperer_mnemos_AO_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *map_text = db->row[2], *libelle = db->row[3];
       gchar *min = db->row[4], *max = db->row[5], *type=db->row[6], *valeur = db->row[7];
       gchar debut[80];
       gint num;
       Info_new( Config.log, module->Thread_debug, LOG_DEBUG, "%s: %s: Match found '%s' '%s:%s' - %s", __func__, tech_id,
                 map_text, tech_id, acro, libelle );
       if ( sscanf ( map_text, "%[^:]:AO%d", debut, &num ) == 2 )                            /* Découpage de la ligne ev_text */
        { if (1<=num && num<=DMX_CHANNEL)
           { g_snprintf( vars->Canal[num-1].tech_id,  sizeof(vars->Canal[num-1].tech_id), "%s", tech_id );
             g_snprintf( vars->Canal[num-1].acronyme, sizeof(vars->Canal[num-1].acronyme), "%s", acro );
             vars->Canal[num-1].min    = atof(min);
             vars->Canal[num-1].max    = atof(max);
             vars->Canal[num-1].type   = atoi(type);
             vars->Canal[num-1].valeur = 0.0;                        /*atof(valeur); a l'init, on considère le canal à zero */
             Info_new( Config.log, module->Thread_debug, LOG_INFO,
                       "%s: AO Canal %d : '%s:%s'=%s ('%s') loaded", __func__, num, tech_id, acro, valeur, libelle );
             cpt++;
           }
          else Info_new( Config.log, module->Thread_debug, LOG_WARNING, "%s: map '%s': num %d out of range '%d'", __func__,
                         map_text, num, DMX_CHANNEL );
        }
       else Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: event '%s': Sscanf Error", __func__, map_text );
     }
    Info_new( Config.log, module->Thread_debug, LOG_INFO, "%s: %d AO loaded", __func__, cpt );

    Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: DMX '%s' : mapping done", __func__, tech_id );
  }
/******************************************************************************************************************************/
/* Dmx_init: Initialisation de la ligne DMX                                                                                   */
/* Entrée: Néant.                                                                                                             */
/* Sortie: -1 si erreur, sinon, le FileDescriptor associé                                                                     */
/******************************************************************************************************************************/
 static gboolean Dmx_init ( struct SUBPROCESS *module )
  { struct DMX_VARS *vars = module->vars;
    gchar *tech_id = Json_get_string ( module->config, "tech_id" );
    gchar *device  = Json_get_string ( module->config, "device" );

    vars->fd = open( device, O_RDWR | O_NOCTTY /*| O_NONBLOCK*/ );
    if (vars->fd<0)
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: %s: Impossible d'ouvrir le device '%s', retour=%d (%s)", __func__,
                 tech_id, device, vars->fd, strerror(errno) );
       return(FALSE);
     }

    vars->taille_trame_dmx = sizeof(struct TRAME_DMX);
    memset ( &vars->Trame_dmx, 0, sizeof(struct TRAME_DMX) );
    Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: %s: Ouverture port dmx okay %s",
              __func__, tech_id, device );
    SubProcess_send_comm_to_master ( module, TRUE );
    Dmx_do_mapping( module );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Dmx_close: Fermeture de la ligne DMX                                                                                       */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dmx_close ( struct SUBPROCESS *module )
  { struct DMX_VARS *vars = module->vars;
    gchar *tech_id = Json_get_string ( module->config, "tech_id" );
    if ( vars->fd != -1 )
     { close(vars->fd);
       Info_new( Config.log, module->Thread_debug, LOG_NOTICE,
		         "%s: %s: Fermeture device '%s' dmx okay", __func__, tech_id, Json_get_string ( module->config, "device" ) );
	   vars->fd = -1;
     }
    SubProcess_send_comm_to_master ( module, FALSE );
  }
/******************************************************************************************************************************/
/* Envoyer_trame_dmx_request: envoie une trame DMX au token USB                                                               */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static gboolean Envoyer_trame_dmx_request( struct SUBPROCESS *module )
  { struct DMX_VARS *vars = module->vars;
    if (module->comm_status == FALSE) return(FALSE);

    gchar *tech_id = Json_get_string ( module->config, "tech_id" );
    vars->Trame_dmx.start_delimiter = 0x7E;
    vars->Trame_dmx.label      = DMX_Output_Only_Send_DMX_Packet_Request;
    vars->Trame_dmx.length_lsb =  (sizeof(vars->Trame_dmx.channel)+1) & 0xFF;
    vars->Trame_dmx.length_msb =  ((sizeof(vars->Trame_dmx.channel)+1) >> 8) & 0xFF;
    vars->Trame_dmx.start_code = 0; /* Start Code DMX = 0 */
    for (gint cpt=0; cpt<DMX_CHANNEL; cpt++) { vars->Trame_dmx.channel[cpt] = (guchar)vars->Canal[cpt].valeur; }
    vars->Trame_dmx.end_delimiter = 0xE7; /* End delimiter */
    if ( write( vars->fd, &vars->Trame_dmx, sizeof(struct TRAME_DMX) ) != sizeof(struct TRAME_DMX) )/* Ecriture de la trame */
     { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: %s: Write Trame Error '%s'", __func__, tech_id, strerror(errno) );
       Dmx_close(module);
       return(FALSE);
     }
    vars->nbr_request++;
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Run_subprocess: Prend en charge un des sous process du thread                                                              */
/* Entrée: la structure SUBPROCESS associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_subprocess ( struct SUBPROCESS *module )
  { SubProcess_init ( module, sizeof(struct DMX_VARS) );
    struct DMX_VARS *vars = module->vars;

    gchar *tech_id = Json_get_string ( module->config, "tech_id" );

    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { usleep(1000);
       sched_yield();

       SubProcess_send_comm_to_master ( module, module->comm_status );         /* Périodiquement envoie la comm au master */
/****************************************************** Ecoute du master ******************************************************/
       while ( module->Master_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->Master_messages->data;
          module->Master_messages = g_slist_remove ( module->Master_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          gchar *bus_tag = Json_get_string ( request, "bus_tag" );
          if ( !strcasecmp( bus_tag, "SET_AO" ) &&
               Json_get_bool ( request, "alive" ) == TRUE &&
               Json_get_int  ( request, "type_sms" ) != MESSAGE_SMS_NONE )
           { gchar *tech_id  = Json_get_string ( request, "tech_id" );
             gchar *acronyme = Json_get_string ( request, "acronyme" );
             gint   valeur   = Json_get_int    ( request, "valeur" );
             if (!tech_id)
              { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: requete mal formée manque tech_id", __func__ ); }
             else if (!acronyme)
              { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: requete mal formée manque acronyme", __func__ ); }
             else if (!valeur)
              { Info_new( Config.log, module->Thread_debug, LOG_ERR, "%s: requete mal formée manque valeur", __func__ ); }
             else
              { for (gint num=0; num<DMX_CHANNEL; num++)
                 { if (!strcasecmp( vars->Canal[num].tech_id, tech_id) &&
                       !strcasecmp( vars->Canal[num].acronyme, acronyme))
                    { Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: Setting %s:%s=%f (Canal %d)", __func__,
                                tech_id, acronyme, valeur, num );
                      vars->Canal[num].valeur = valeur;
                      break;
                    }

                 }
                Envoyer_trame_dmx_request(module);
              }
           }
          Json_node_unref(request);
        }

/************************************************* Traitement opérationnel ****************************************************/
       if (module->comm_status == FALSE && vars->date_next_retry <= Partage->top )
        { vars->date_next_retry = 0;
          Info_new( Config.log, module->Thread_debug, LOG_NOTICE, "%s: %s: Retrying Connexion.", __func__, tech_id );
          if ( Dmx_init(module) == FALSE )
           { vars->date_next_retry = Partage->top + DMX_RETRY_DELAI; }
          else
           { Envoyer_trame_dmx_request(module); }                 /* Envoie d'une trame à l'init sans attendre la comm master */
        }

       if (!(Partage->top % 50))                                                                /* Test toutes les 5 secondes */
        { gboolean closing = FALSE;
          struct stat buf;
          gint retour;
          retour = fstat( vars->fd, &buf );
          if (retour == -1)
           { Info_new( Config.log, module->Thread_debug, LOG_ERR,
                      "%s: %s: Fstat Error (%s), closing and re-trying in %ds", __func__, tech_id,
                       strerror(errno), DMX_RETRY_DELAI/10 );
             closing = TRUE;
           }
          else if ( buf.st_nlink < 1 )
           { Info_new( Config.log, module->Thread_debug, LOG_ERR,
                      "%s: %s: USB device disappeared. Closing and re-trying in %ds", __func__, tech_id, DMX_RETRY_DELAI/10 );
             closing = TRUE;
           }
          if (closing == TRUE)
           { Dmx_close ( module );
			 vars->date_next_retry = Partage->top + DMX_RETRY_DELAI;
           }
        }
     }                                                                                         /* Fin du while partage->arret */
    Dmx_close(module);

    SubProcess_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
