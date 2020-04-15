/******************************************************************************************************************************/
/* Watchdogd/Serveur/protocole_admin.c    Gestion du protocole_admin pour Watchdog                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 04 avr 2009 11:13:22 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * protocole_admin.c
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
/****************************************************** Prototypes de fonctions ***********************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
 extern struct SSRV_CONFIG Cfg_ssrv;
/******************************************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                                                 */
/* Entrée: la connexion avec le serveur                                                                                       */
/* Sortie: Kedal                                                                                                              */
/******************************************************************************************************************************/
 void Gerer_protocole_admin( struct CLIENT *client )
  { struct CONNEXION *connexion;
    connexion = client->connexion;

    if ( ! Tester_level_util( client->util, ACCESS_LEVEL_CLI) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_ADMIN:
             { Envoi_client( client, TAG_ADMIN, SSTAG_SERVEUR_CREATE_PAGE_ADMIN_OK, NULL, 0 );
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
                        "Gerer_protocole_admin: Acces to Watchdog CLI granted to %s@%s",
                         client->util->username, client->machine );
             }
            break;
       case SSTAG_CLIENT_REQUEST:
             { struct CMD_TYPE_ADMIN *admin;
               gchar *response;
               admin = (struct CMD_TYPE_ADMIN *)connexion->donnees;
               Envoyer_reseau ( connexion, TAG_ADMIN, SSTAG_SERVEUR_RESPONSE_START, NULL, 0 );         /* Debut de la reponse */
               response = Processer_commande_admin ( client->util->username, client->machine, admin->buffer );
               Envoyer_reseau ( connexion, TAG_ADMIN, SSTAG_SERVEUR_RESPONSE_BUFFER, response, strlen(response)+1 );
               g_free(response);
               Envoyer_reseau ( connexion, TAG_ADMIN, SSTAG_SERVEUR_RESPONSE_STOP, NULL, 0 );            /* Fin de la reponse */
              }
            break;

     }
  }
/*--------------------------------------------------------------------------------------------------------*/
