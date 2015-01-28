/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_satellite.c    Gestion du protocole_satellite pour Watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                    sam. 04 oct. 2014 17:34:56 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_satellite.c
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
/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_satellite( struct CLIENT *client )
  { struct CONNEXION *connexion;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util, GID_SATELLITE) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
                "Gerer_protocole_satellite: %s not allowed to send INTERNAL infos (not in group GID_SATELLITE=%d).",
                 client->util->nom, GID_SATELLITE );
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_SET_INTERNAL:
             { struct CMD_TYPE_SATELLITE *sat;
               struct CMD_TYPE_MSRV_EVENT *dup_event;
               sat = (struct CMD_TYPE_SATELLITE *)connexion->donnees;
               switch (sat->type)
                { case -1: dup_event = (struct CMD_TYPE_MSRV_EVENT *)g_malloc( sizeof(struct CMD_TYPE_MSRV_EVENT ) );
                           if (dup_event)
                            { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                                         "Gerer_protocole_satellite: Receiving EVENT from satellite %s",
                                          client->util->nom );
                              Envoyer_Event_msrv( dup_event );
                            }
                           else Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                                         "Gerer_protocole_satellite: Memory Alloc Error (%s) for satellite %s",
                                          strerrno(errno), client->util->nom );
                           break;
                  case MNEMO_ENTREE_ANA: SEA ( sat->num, sat->val_float );

                                         Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                                        "Gerer_protocole_satellite: Setting type SEA(%03d)=%8.2f from satellite %s",
                                         sat->num, sat->val_float, client->util->nom );
                                         break;
                  case MNEMO_ENTREE    : Envoyer_entree_dls ( sat->num, sat->val_int );
                                         Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                                        "Gerer_protocole_satellite: Setting type SE(%03d)=%03d from satellite %s",
                                         sat->num, sat->val_int, client->util->nom );
                                         break;
                }
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
