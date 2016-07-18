/**********************************************************************************************************/
/* Client/protocole_mnemonique.c    Gestion du protocole_mnemonique pour la connexion au serveur Watchdog */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 15 déc. 2010 11:32:01 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_mnemonique.c
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
 void Gerer_protocole_mnemonique ( struct CONNEXION *connexion )
  { static GList *Arrivee_mnemonique = NULL;
    static GList *Arrivee_dls     = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_MNEMO_OK:
             { if (!Tester_page_notebook(TYPE_PAGE_MNEMONIQUE)) { Creer_page_mnemonique(); }
             }
            break;
       case SSTAG_SERVEUR_ADD_MNEMONIQUE_OK:
             { struct CMD_TYPE_MNEMO_BASE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMO_BASE *)connexion->donnees;
               Proto_afficher_un_mnemonique( mnemo );
             }
            break;
       case SSTAG_SERVEUR_DEL_MNEMONIQUE_OK:
             { struct CMD_TYPE_MNEMO_BASE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMO_BASE *)connexion->donnees;
               Proto_cacher_un_mnemonique( mnemo );
             }
            break;
       case SSTAG_SERVEUR_EDIT_MNEMONIQUE_OK:
             { struct CMD_TYPE_MNEMO_FULL *mnemo;
               mnemo = (struct CMD_TYPE_MNEMO_FULL *)connexion->donnees;
               Menu_ajouter_editer_mnemonique( mnemo );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_MNEMONIQUE_OK:
             { struct CMD_TYPE_MNEMO_BASE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMO_BASE *)connexion->donnees;
               Proto_rafraichir_un_mnemonique( mnemo );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE:
             { struct CMD_TYPE_MNEMONIQUES *mnemos;
               gint i;
               mnemos = (struct CMD_TYPE_MNEMONIQUES *)connexion->donnees;
               Set_progress_plus( mnemos->nbr_mnemos );
               for (i=0; i<mnemos->nbr_mnemos; i++)
                { struct CMD_TYPE_MNEMO_BASE *mnemo;
                  mnemo = (struct CMD_TYPE_MNEMO_BASE *)g_try_malloc0( sizeof( struct CMD_TYPE_MNEMO_BASE ) );
                  if (!mnemo) break;
                  memcpy( mnemo, &mnemos->mnemo[i], sizeof(struct CMD_TYPE_MNEMO_BASE ) );
                  Arrivee_mnemonique = g_list_append( Arrivee_mnemonique, mnemo );
                }
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE_FIN:
             { g_list_foreach( Arrivee_mnemonique, (GFunc)Proto_afficher_un_mnemonique, NULL );
               g_list_foreach( Arrivee_mnemonique, (GFunc)g_free, NULL );
               g_list_free( Arrivee_mnemonique );
               Arrivee_mnemonique = NULL;
               Chercher_page_notebook( TYPE_PAGE_MNEMONIQUE, 0, TRUE );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_DLS_FOR_MNEMO:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               Set_progress_plus(1);
               dls = (struct CMD_TYPE_PLUGIN_DLS *)g_try_malloc0( sizeof( struct CMD_TYPE_PLUGIN_DLS ) );
               if (!dls) return; 

               memcpy( dls, connexion->donnees, sizeof(struct CMD_TYPE_PLUGIN_DLS ) );
               Arrivee_dls = g_list_append( Arrivee_dls, dls );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_DLS_FOR_MNEMO_FIN:
             { g_list_foreach( Arrivee_dls, (GFunc)Proto_afficher_un_dls_for_mnemonique, NULL );
               g_list_foreach( Arrivee_dls, (GFunc)g_free, NULL );
               g_list_free( Arrivee_dls );
               Arrivee_dls = NULL;
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
