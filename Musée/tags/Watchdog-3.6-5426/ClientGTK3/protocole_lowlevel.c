/**********************************************************************************************************/
/* Client/protocole_camera.c    Gestion du protocole_camera pour Watchdog                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                   dim. 13 sept. 2009 11:55:51 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_camera.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sébastien Lefevre
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
 void Gerer_protocole_lowlevel ( struct CONNEXION *connexion )
  { static GList *Arrivee_camera = NULL;
           
    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_CAMERA_OK:
             { if (!Tester_page_notebook(TYPE_PAGE_CAMERA)) { Creer_page_camera(); }
             }
            break;
       case SSTAG_SERVEUR_ADD_CAMERA_OK:
             { struct CMD_TYPE_CAMERA *camera;
               camera = (struct CMD_TYPE_CAMERA *)connexion->donnees;
               Proto_afficher_un_camera( camera );
             }
            break;
       case SSTAG_SERVEUR_DEL_CAMERA_OK:
             { struct CMD_TYPE_CAMERA *camera;
               camera = (struct CMD_TYPE_CAMERA *)connexion->donnees;
               Proto_cacher_un_camera( camera );
             }
            break;
       case SSTAG_SERVEUR_EDIT_CAMERA_OK:
             { struct CMD_TYPE_CAMERA *camera;
               camera = (struct CMD_TYPE_CAMERA *)connexion->donnees;
               Menu_ajouter_editer_camera( camera );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_CAMERA_OK:
             { struct CMD_TYPE_CAMERA *camera;
               camera = (struct CMD_TYPE_CAMERA *)connexion->donnees;
               Proto_rafraichir_un_camera( camera );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_CAMERA:
             { struct CMD_TYPE_CAMERA *camera;
               Set_progress_plus(1);
               camera = (struct CMD_TYPE_CAMERA *)g_try_malloc0( sizeof( struct CMD_TYPE_CAMERA ) );
               if (!camera) return; 
               memcpy( camera, connexion->donnees, sizeof(struct CMD_TYPE_CAMERA ) );
               Arrivee_camera = g_list_append( Arrivee_camera, camera );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FIN:
             { g_list_foreach( Arrivee_camera, (GFunc)Proto_afficher_un_camera, NULL );
               g_list_foreach( Arrivee_camera, (GFunc)g_free, NULL );
               g_list_free( Arrivee_camera );
               Arrivee_camera = NULL;
               Chercher_page_notebook( TYPE_PAGE_CAMERA, 0, TRUE );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
