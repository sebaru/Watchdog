/******************************************************************************************************************************/
/* Watchdogd/Serveur/protocole_camera.c    Gestion du protocole_camera pour Watchdog                                          */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       dim. 13 sept. 2009 11:59:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * protocole_camera.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
 extern struct SSRV_CONFIG Cfg_ssrv;
/******************************************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                                                 */
/* Entrée: la connexion avec le serveur                                                                                       */
/* Sortie: Kedal                                                                                                              */
/******************************************************************************************************************************/
 void Gerer_protocole_lowlevel( struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_level_util( client->util, ACCESS_LEVEL_ATELIER ) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_CAMERA:
             { Envoi_client( client, TAG_LOWLEVEL, SSTAG_SERVEUR_CREATE_PAGE_CAMERA_OK, NULL, 0 );
               Ref_client( client, "Send Cameras" );                                 /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_cameras_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_EDIT_CAMERA:
             { struct CMD_TYPE_CAMERA *camera;
               camera = (struct CMD_TYPE_CAMERA *)connexion->donnees;
               Proto_editer_camera( client, camera );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_CAMERA:
             { struct CMD_TYPE_CAMERA *camera;
               camera = (struct CMD_TYPE_CAMERA *)connexion->donnees;
               Proto_valider_editer_camera( client, camera );
             }
            break;
       case SSTAG_CLIENT_ADD_CAMERA:
             { struct CMD_TYPE_CAMERA *camera;
               camera = (struct CMD_TYPE_CAMERA *)connexion->donnees;
               Proto_ajouter_camera( client, camera );
             }
            break;
       case SSTAG_CLIENT_DEL_CAMERA:
             { struct CMD_TYPE_CAMERA *camera;
               camera = (struct CMD_TYPE_CAMERA *)connexion->donnees;
               Proto_effacer_camera( client, camera );
             }
            break;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
