/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_histo_courbe.c    Gestion du protocole_courbe pour Watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 nov 2007 13:29:36 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_histo_courbe.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2007 - Sébastien Lefevre
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

/********************************* Définitions des prototypes programme ***********************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_histo_courbe( gint Id_serveur, struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util->id, client->util->gids, GID_HISTO) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_SOURCE_FOR_HISTO_COURBE:
             { Client_mode( client, ENVOI_ENTREEANA_FOR_HISTO_COURBE );
             }
            break;
       case SSTAG_CLIENT_ADD_HISTO_COURBE:
             { while (client->courbe.id != -1) { printf("attends\n"); sched_yield(); }
               memcpy( &client->courbe, connexion->donnees, sizeof(struct CMD_ID_COURBE) );
               pthread_create( &tid, NULL, (void *)Proto_ajouter_histo_courbe_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_DEL_HISTO_COURBE:
             { struct CMD_ID_COURBE *courbe;
               courbe = (struct CMD_ID_COURBE *)connexion->donnees;
               Proto_effacer_histo_courbe( client, courbe );
             }
            break;
       case SSTAG_CLIENT_SET_DATE:
             { struct CMD_HISTO_COURBE *courbe;
               courbe = (struct CMD_HISTO_COURBE *)connexion->donnees;
               client->histo_courbe.date_first = courbe->date_first;
               client->histo_courbe.date_last = courbe->date_last;
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
