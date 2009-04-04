/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_message.c    Gestion du protocole_message pour Watchdog                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 avr 2009 11:12:54 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_message.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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
 #include "Erreur.h"
 #include "Reseaux.h"
 #include "Client.h"
 #include "Message_DB.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_message( gint Id_serveur, struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util->id, client->util->gids, GID_MESSAGE) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_MESSAGE:
             { Client_mode( client, ENVOI_MESSAGE );
             }
            break;
       case SSTAG_CLIENT_EDIT_MESSAGE:
             { struct CMD_ID_MESSAGE *msg;
               msg = (struct CMD_ID_MESSAGE *)connexion->donnees;
               Proto_editer_message( client, msg );
             }
            break;
       case SSTAG_CLIENT_ADD_MESSAGE:
             { struct CMD_ADD_MESSAGE *msg;
               msg = (struct CMD_ADD_MESSAGE *)connexion->donnees;
               Proto_ajouter_message( client, msg );
             }
            break;
       case SSTAG_CLIENT_DEL_MESSAGE:
             { struct CMD_ID_MESSAGE *msg;
               msg = (struct CMD_ID_MESSAGE *)connexion->donnees;
               Proto_effacer_message( client, msg );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_MESSAGE:
             { struct CMD_EDIT_MESSAGE *msg;
               msg = (struct CMD_EDIT_MESSAGE *)connexion->donnees;
               Proto_valider_editer_message( client, msg );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
