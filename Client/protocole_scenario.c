/**********************************************************************************************************/
/* Client/protocole_scenario.c    Gestion du protocole_message pour la connexion au serveur Watchdog       */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 10 aoû 2008 12:42:49 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_scenario.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - Sébastien Lefevre
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
 void Gerer_protocole_scenario ( struct CONNEXION *connexion )
  { static GList *Arrivee_scenario = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_SCENARIO_OK:
             { Creer_page_scenario();
             }
       case SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE:
             { struct CMD_TYPE_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               Proto_afficher_mnemo_scenario( mnemo );
             }
            break;
       case SSTAG_SERVEUR_ADD_SCENARIO_OK:
             { struct CMD_SHOW_SCENARIO *sce;
               sce = (struct CMD_SHOW_SCENARIO *)connexion->donnees;
               Proto_afficher_un_scenario( sce );
             }
            break;
       case SSTAG_SERVEUR_DEL_SCENARIO_OK:
             { struct CMD_ID_SCENARIO *sce;
               sce = (struct CMD_ID_SCENARIO *)connexion->donnees;
               Proto_cacher_un_scenario( sce );
             }
            break;
       case SSTAG_SERVEUR_EDIT_SCENARIO_OK:
             { struct CMD_EDIT_SCENARIO *sce;
               sce = (struct CMD_EDIT_SCENARIO *)connexion->donnees;
               Menu_ajouter_editer_scenario( sce );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_SCENARIO_OK:
             { struct CMD_SHOW_SCENARIO *sce;
               sce = (struct CMD_SHOW_SCENARIO *)connexion->donnees;
               Proto_rafraichir_un_scenario( sce );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SCENARIO:
             { struct CMD_SHOW_SCENARIO *sce;
               Set_progress_plusun();
printf("Addprogress scenario\n");
               sce = (struct CMD_SHOW_SCENARIO *)g_malloc0( sizeof( struct CMD_SHOW_SCENARIO ) );
               if (!sce) return; 
               memcpy( sce, connexion->donnees, sizeof(struct CMD_SHOW_SCENARIO ) );
               Arrivee_scenario = g_list_append( Arrivee_scenario, sce );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SCENARIO_FIN:
             { g_list_foreach( Arrivee_scenario, (GFunc)Proto_afficher_un_scenario, NULL );
               g_list_foreach( Arrivee_scenario, (GFunc)g_free, NULL );
               g_list_free( Arrivee_scenario );
               Arrivee_scenario = NULL;
               Chercher_page_notebook( TYPE_PAGE_SCENARIO, 0, TRUE );
printf("Addprogress scenario fin\n");
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
