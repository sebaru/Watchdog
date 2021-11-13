/**********************************************************************************************************/
/* Client/protocole_synoptique.c    Gestion du protocole_synoptique pour Watchdog                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 11:55:51 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_synoptique.c
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
 void Gerer_protocole_synoptique ( struct CONNEXION *connexion )
  { static GList *Arrivee_synoptique = NULL;
           
    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_SYNOPTIQUE_OK:
             { if (!Tester_page_notebook(TYPE_PAGE_SYNOPTIQUE)) { Creer_page_synoptique(); }
             }
            break;
       case SSTAG_SERVEUR_ADD_SYNOPTIQUE_OK:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;
               Proto_afficher_un_synoptique( syn );
             }
            break;
       case SSTAG_SERVEUR_DEL_SYNOPTIQUE_OK:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;
               Proto_cacher_un_synoptique( syn );
             }
            break;
       case SSTAG_SERVEUR_EDIT_SYNOPTIQUE_OK:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;
               Menu_ajouter_editer_synoptique( syn );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_SYNOPTIQUE_OK:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;
               Proto_rafraichir_un_synoptique( syn );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               Set_progress_plus(1);

               syn = (struct CMD_TYPE_SYNOPTIQUE *)g_try_malloc0( sizeof( struct CMD_TYPE_SYNOPTIQUE ) );
               if (!syn) return; 
               memcpy( syn, connexion->donnees, sizeof(struct CMD_TYPE_SYNOPTIQUE ) );
               Arrivee_synoptique = g_list_append( Arrivee_synoptique, syn );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FIN:
             { g_list_foreach( Arrivee_synoptique, (GFunc)Proto_afficher_un_synoptique, NULL );
               g_list_foreach( Arrivee_synoptique, (GFunc)g_free, NULL );
               g_list_free( Arrivee_synoptique );
               Arrivee_synoptique = NULL;
               Chercher_page_notebook( TYPE_PAGE_SYNOPTIQUE, 0, TRUE );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
