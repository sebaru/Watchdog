/**********************************************************************************************************/
/* Client/protocole_utilisateur.c   Gestion du protocole_utilisateur pour la connexion au serveur Watchdog*/
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 21:31:24 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_utilisateur.c
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
 void Gerer_protocole_utilisateur ( struct CONNEXION *connexion )
  { static GList *Arrivee_util = NULL;
    static GList *Arrivee_groupe = NULL;
    static GList *Arrivee_groupe_for_util = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_UTIL_OK:
             { if (!Tester_page_notebook(TYPE_PAGE_UTIL)) { Creer_page_utilisateur(); }
             }
            break;
       case SSTAG_SERVEUR_ADD_UTIL_OK:
             { struct CMD_TYPE_UTILISATEUR *util;
               util = (struct CMD_TYPE_UTILISATEUR *)connexion->donnees;
               Proto_afficher_un_utilisateur( util );
             }
            break;
       case SSTAG_SERVEUR_DEL_UTIL_OK:
             { struct CMD_TYPE_UTILISATEUR *util;
               util = (struct CMD_TYPE_UTILISATEUR *)connexion->donnees;
               Proto_cacher_un_utilisateur( util );
             }
            break;
       case SSTAG_SERVEUR_EDIT_UTIL_OK:
             { struct CMD_TYPE_UTILISATEUR *util;
               util = (struct CMD_TYPE_UTILISATEUR *)connexion->donnees;
               Menu_ajouter_editer_utilisateur( util );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_UTIL_OK:
             { struct CMD_TYPE_UTILISATEUR *util;
               util = (struct CMD_TYPE_UTILISATEUR *)connexion->donnees;
               Proto_rafraichir_un_utilisateur( util );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_UTIL:
             { struct CMD_TYPE_UTILISATEUR *util;
               Set_progress_plus(1);

               util = (struct CMD_TYPE_UTILISATEUR *)g_try_malloc0( sizeof(struct CMD_TYPE_UTILISATEUR) );
               if (!util) return; 
               memcpy( util, connexion->donnees, sizeof(struct CMD_TYPE_UTILISATEUR) );
               Arrivee_util = g_list_append( Arrivee_util, util );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_UTIL_FIN:
             { g_list_foreach( Arrivee_util, (GFunc)Proto_afficher_un_utilisateur, NULL );
               g_list_foreach( Arrivee_util, (GFunc)g_free, NULL );
               g_list_free( Arrivee_util );
               Arrivee_util = NULL;
               Chercher_page_notebook( TYPE_PAGE_UTIL, 0, TRUE );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_UTIL:
             { struct CMD_TYPE_GROUPE *groupe;
               Set_progress_plus(1);

               groupe = (struct CMD_TYPE_GROUPE *)g_try_malloc0( sizeof(struct CMD_TYPE_GROUPE) );
               if (!groupe) return; 
               memcpy( groupe, connexion->donnees, sizeof(struct CMD_TYPE_GROUPE) );
               Arrivee_groupe_for_util = g_list_append( Arrivee_groupe_for_util, groupe );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_UTIL_FIN:
             { g_list_foreach( Arrivee_groupe_for_util, (GFunc)Proto_afficher_un_groupe_existant, NULL );
               g_list_foreach( Arrivee_groupe_for_util, (GFunc)g_free, NULL );
               g_list_free( Arrivee_groupe_for_util );
               Arrivee_groupe_for_util = NULL;
               Proto_fin_affichage_groupes_existants();
             }
            break;

       case SSTAG_SERVEUR_ADD_GROUPE_OK:
             { struct CMD_TYPE_GROUPE *groupe;
               groupe = (struct CMD_TYPE_GROUPE *)connexion->donnees;
               Proto_afficher_un_groupe( groupe );
             }
            break;
       case SSTAG_SERVEUR_DEL_GROUPE_OK:
             { struct CMD_TYPE_GROUPE *groupe;
               groupe = (struct CMD_TYPE_GROUPE *)connexion->donnees;
               Proto_cacher_un_groupe( groupe );
             }
            break;
       case SSTAG_SERVEUR_EDIT_GROUPE_OK:
             { struct CMD_TYPE_GROUPE *groupe;
               groupe = (struct CMD_TYPE_GROUPE *)connexion->donnees;
               Menu_ajouter_editer_groupe( groupe );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_GROUPE_OK:
             { struct CMD_TYPE_GROUPE *groupe;
               groupe = (struct CMD_TYPE_GROUPE *)connexion->donnees;
               Proto_rafraichir_un_groupe( groupe );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_GROUPE:
             { struct CMD_TYPE_GROUPE *groupe;
               Set_progress_plus(1);

               groupe = (struct CMD_TYPE_GROUPE *)g_try_malloc0( sizeof( struct CMD_TYPE_GROUPE ) );
               if (!groupe) return; 
               memcpy( groupe, connexion->donnees, sizeof(struct CMD_TYPE_GROUPE ) );
               Arrivee_groupe = g_list_append( Arrivee_groupe, groupe );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FIN:
             { g_list_foreach( Arrivee_groupe, (GFunc)Proto_afficher_un_groupe, NULL );
               g_list_foreach( Arrivee_groupe, (GFunc)g_free, NULL );
               g_list_free( Arrivee_groupe );
               Arrivee_groupe = NULL;
               Chercher_page_notebook( TYPE_PAGE_GROUPE, 0, TRUE );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
