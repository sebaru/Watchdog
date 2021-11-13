/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_histo_courbe.c    Gestion du protocole_courbe pour Watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 nov 2007 13:29:36 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_histo_courbe.c
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
 void Gerer_protocole_histo_courbe( struct CLIENT *client )
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
     { case SSTAG_CLIENT_WANT_PAGE_SOURCE_FOR_HISTO_COURBE:
             { /*Ref_client( client );                             /* Indique que la structure est utilisée */
               /*pthread_create( &tid, NULL, (void *)Envoyer_entreeANA_for_histo_courbe_thread, client );
               pthread_detach( tid );*/
             }
            break;
       case SSTAG_CLIENT_ADD_HISTO_COURBE:
             { while (client->courbe.num != -1) { printf("attends\n"); sched_yield(); }
               memcpy( &client->courbe, connexion->donnees, sizeof(struct CMD_TYPE_COURBE) );
               Ref_client( client, "Ajouter histo courbe" );
               pthread_create( &tid, NULL, (void *)Proto_ajouter_histo_courbe_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_SET_DATE:
             { struct CMD_HISTO_COURBE *courbe;
               courbe = (struct CMD_HISTO_COURBE *)connexion->donnees;
               client->histo_courbe.date_first = courbe->date_first;
               client->histo_courbe.date_last = courbe->date_last;
               Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                        "Gerer_protocole_histo_courbe: Set Date to %d/%d",
                        client->histo_courbe.date_first, client->histo_courbe.date_last
                       );
    
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
