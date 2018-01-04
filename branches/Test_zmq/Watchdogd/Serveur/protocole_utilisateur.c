/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_utilisateur.c    Gestion du protocole_utilisateur pour Watchdog            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 avr 2009 11:10:58 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_utilisateur.c
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
 void Gerer_protocole_utilisateur( struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_level_util( client->util, ACCESS_LEVEL_USER ) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_UTIL:
             { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_CREATE_PAGE_UTIL_OK, NULL, 0 );
               Ref_client( client, "Send Util" );
               pthread_create( &tid, NULL, (void *)Envoyer_utilisateurs_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_EDIT_UTIL:
             { struct CMD_TYPE_UTILISATEUR *util;
               util = (struct CMD_TYPE_UTILISATEUR *)connexion->donnees;
               Proto_editer_utilisateur( client, util );
             }
            break;
       case SSTAG_CLIENT_ADD_UTIL:
             { struct CMD_TYPE_UTILISATEUR *util;
               util = (struct CMD_TYPE_UTILISATEUR *)connexion->donnees;
               Proto_ajouter_utilisateur( client, util );
             }
            break;
       case SSTAG_CLIENT_DEL_UTIL:
             { struct CMD_TYPE_UTILISATEUR *util;
               util = (struct CMD_TYPE_UTILISATEUR *)connexion->donnees;
               Proto_effacer_utilisateur( client, util );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_UTIL:
             { struct CMD_TYPE_UTILISATEUR *util;
               util = (struct CMD_TYPE_UTILISATEUR *)connexion->donnees;
               Proto_valider_editer_utilisateur( client, util );
             }
            break;
       case SSTAG_CLIENT_CHANGE_PASSWORD:
             { struct CMD_TYPE_UTILISATEUR *util;
               util = (struct CMD_TYPE_UTILISATEUR *)connexion->donnees;
               Proto_set_password( client, util );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
