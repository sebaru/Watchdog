/**********************************************************************************************************/
/* Client/protocole_dls.c    Gestion du protocole_dls pour la connexion au serveur Watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam. 31 déc. 2011 17:34:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_dls.c
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
 void Gerer_protocole_dls ( struct CONNEXION *connexion )
  { static GList *Arrivee_dls = NULL;
    static GList *Arrivee_syn     = NULL;
               
    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_DLS_OK:
             { if (!Tester_page_notebook(TYPE_PAGE_PLUGIN_DLS)) { Creer_page_plugin_dls(); }
             }
            break;
       case SSTAG_SERVEUR_ADD_PLUGIN_DLS_OK:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Proto_afficher_un_plugin_dls( dls );
             }
            break;
       case SSTAG_SERVEUR_DEL_PLUGIN_DLS_OK:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Proto_cacher_un_plugin_dls( dls );
             }
            break;
       case SSTAG_SERVEUR_EDIT_PLUGIN_DLS_OK:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Menu_ajouter_editer_plugin_dls( dls );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_PLUGIN_DLS_OK:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Proto_rafraichir_un_plugin_dls( dls );
             }
            break;
       case SSTAG_SERVEUR_SOURCE_DLS_START:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Creer_page_source_dls( dls );
             }
            break;
       case SSTAG_SERVEUR_SOURCE_DLS:
             { struct CMD_TYPE_SOURCE_DLS *dls;
               gchar *buffer;
               dls = (struct CMD_TYPE_SOURCE_DLS *)connexion->donnees;
               buffer = (gchar *)dls + sizeof(struct CMD_TYPE_SOURCE_DLS);
               Proto_append_source_dls( dls, buffer );
             }
            break;
       case SSTAG_SERVEUR_SOURCE_DLS_END:
             { /*struct CMD_TYPE_SOURCE_DLS *dls;
               dls = (struct CMD_TYPE_SOURCE_DLS *)connexion->donnees;*/
             }
            break;
       case SSTAG_SERVEUR_DLS_COMPIL_STATUS:
             { struct CMD_GTK_MESSAGE *erreur;
               erreur = (struct CMD_GTK_MESSAGE *)connexion->donnees;
               Dls_set_compil_status ( erreur->message );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_PLUGIN_DLS:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               Set_progress_plus(1);

               dls = (struct CMD_TYPE_PLUGIN_DLS *)g_try_malloc0( sizeof( struct CMD_TYPE_PLUGIN_DLS ) );
               if (!dls) return; 
               memcpy( dls, connexion->donnees, sizeof(struct CMD_TYPE_PLUGIN_DLS ) );
               printf("One plugin receive %s %d\n", dls->nom, dls->compil_date );
               Arrivee_dls = g_list_append( Arrivee_dls, dls );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_PLUGIN_DLS_FIN:
             { g_list_foreach( Arrivee_dls, (GFunc)Proto_afficher_un_plugin_dls, NULL );
               g_list_foreach( Arrivee_dls, (GFunc)g_free, NULL );
               g_list_free( Arrivee_dls );
               Arrivee_dls = NULL;
               Chercher_page_notebook( TYPE_PAGE_PLUGIN_DLS, 0, TRUE );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SYN_FOR_PLUGIN_DLS:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               Set_progress_plus(1);
               syn = (struct CMD_TYPE_SYNOPTIQUE *)g_try_malloc0( sizeof( struct CMD_TYPE_SYNOPTIQUE ) );
               if (!syn) return; 
printf("recu un syn\n");
               memcpy( syn, connexion->donnees, sizeof(struct CMD_TYPE_SYNOPTIQUE ) );
               Arrivee_syn = g_list_append( Arrivee_syn, syn );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_SYN_FOR_PLUGIN_DLS_FIN:
             { g_list_foreach( Arrivee_syn, (GFunc)Proto_afficher_un_syn_for_plugin_dls, NULL );
               g_list_foreach( Arrivee_syn, (GFunc)g_free, NULL );
               g_list_free( Arrivee_syn );
               Arrivee_syn = NULL;
             }
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO:
             { struct CMD_TYPE_MNEMO_BASE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMO_BASE *)connexion->donnees;
               Proto_afficher_mnemo_dls( mnemo );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
