/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_courbe.c    Gestion du protocole_courbe pour Watchdog                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 avr 2009 11:09:45 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_courbe.c
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
 void Gerer_protocole_courbe( struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util, GID_HISTO) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_SOURCE_FOR_COURBE:
             { Ref_client( client, "Send mnemonique for courbe" );
               pthread_create( &tid, NULL, (void *)Envoyer_mnemoniques_for_courbe_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_ADD_COURBE:
             { while (client->courbe.num != -1) { printf("attends\n"); sched_yield(); }
               memcpy( &client->courbe, connexion->donnees, sizeof(struct CMD_TYPE_COURBE) );
               Ref_client( client, "Send Ajouter courbe" );
               pthread_create( &tid, NULL, (void *)Proto_ajouter_courbe_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_DEL_COURBE:
             { struct CMD_TYPE_COURBE *courbe;
               courbe = (struct CMD_TYPE_COURBE *)connexion->donnees;
               Proto_effacer_courbe( client, courbe );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
