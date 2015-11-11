/**********************************************************************************************************/
/* Client/protocole_courbe.c    Gestion du protocole_courbe pour la connexion au serveur Watchdog         */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_courbe.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
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

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_courbe ( struct CONNEXION *connexion )
  { static GList *Arrivee_mnemo = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_ADD_COURBE_OK:
             { struct CMD_TYPE_COURBE *courbe;
               courbe = (struct CMD_TYPE_COURBE *)connexion->donnees;
               printf("proto ajouter courbe !!\n");
               Proto_ajouter_courbe( courbe );
             }
            break;
       case SSTAG_SERVEUR_START_COURBE:
             { struct CMD_START_COURBE *courbe;
               courbe = (struct CMD_START_COURBE *)connexion->donnees;
               Proto_start_courbe( courbe );
             }
            break;
       case SSTAG_SERVEUR_APPEND_COURBE:
             { struct CMD_APPEND_COURBE *courbe;
               courbe = (struct CMD_APPEND_COURBE *)connexion->donnees;
               Proto_append_courbe( courbe );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_COURBE:
             { struct CMD_TYPE_MNEMO_FULL *mnemo;
               Set_progress_plus(1);

               mnemo = (struct CMD_TYPE_MNEMO_FULL *)g_try_malloc0( sizeof( struct CMD_TYPE_MNEMO_FULL ) );
               if (!mnemo) return; 
               memcpy( mnemo, connexion->donnees, sizeof(struct CMD_TYPE_MNEMO_FULL ) );
               Arrivee_mnemo = g_list_append( Arrivee_mnemo, mnemo );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_COURBE_FIN:
             { g_list_foreach( Arrivee_mnemo, (GFunc)Proto_afficher_une_source_for_courbe, NULL );
               g_list_foreach( Arrivee_mnemo, (GFunc)g_free, NULL );
               g_list_free( Arrivee_mnemo );
               Arrivee_mnemo = NULL;
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
