/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_camera.c        Configuration des cameras de Watchdog v2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 11:24:00 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_camera.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - 
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
 #include <sys/prctl.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>
 #include <pthread.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "Reseaux.h"
 #include "watchdogd.h"
/**********************************************************************************************************/
/* Preparer_envoi_camera: convertit une structure CAMERA en structure CMD_TYPE_CAMERA                     */
/* Entr�e: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_TYPE_CAMERA *Preparer_envoi_camera ( struct CAMERADB *camera )
  { struct CMD_TYPE_CAMERA *rezo_camera;

    rezo_camera = (struct CMD_TYPE_CAMERA *)g_malloc0( sizeof(struct CMD_TYPE_CAMERA) );
    if (!rezo_camera) { return(NULL); }

    rezo_camera->id         = camera->id;
    rezo_camera->type       = camera->type;
    memcpy( &rezo_camera->libelle,  camera->libelle,  sizeof(rezo_camera->libelle) );
    memcpy( &rezo_camera->location, camera->location, sizeof(rezo_camera->location) );
    return( rezo_camera );
  }
/**********************************************************************************************************/
/* Proto_editer_camera: Le client desire editer un camera                                                 */
/* Entr�e: le client demandeur et le camera en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera )
  { struct CMD_TYPE_CAMERA edit_camera;
    struct CAMERADB *camera;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    camera = Rechercher_cameraDB( Config.log, Db_watchdog, rezo_camera->id );

    if (camera)
     { edit_camera.id         = camera->id;                                 /* Recopie des info editables */
       edit_camera.type       = camera->type;
       memcpy( &edit_camera.libelle,  camera->libelle,  sizeof(edit_camera.libelle) );
       memcpy( &edit_camera.location, camera->location, sizeof(edit_camera.location) );

       Envoi_client( client, TAG_CAMERA, SSTAG_SERVEUR_EDIT_CAMERA_OK,
                     (gchar *)&edit_camera, sizeof(struct CMD_TYPE_CAMERA) );
       g_free(camera);                                                              /* liberation m�moire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate camera %s", rezo_camera->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_camera: Le client valide l'edition d'un camera                                    */
/* Entr�e: le client demandeur et le camera en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera )
  { struct CAMERADB *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_cameraDB ( Config.log, Db_watchdog, rezo_camera );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit camera %s", rezo_camera->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_cameraDB( Config.log, Db_watchdog, rezo_camera->id );
           if (result) 
            { struct CMD_TYPE_CAMERA *camera;
              camera = Preparer_envoi_camera ( result );
              g_free(result);
              if (!camera)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_CAMERA, SSTAG_SERVEUR_VALIDE_EDIT_CAMERA_OK,
                                   (gchar *)camera, sizeof(struct CMD_TYPE_CAMERA) );
                     g_free(camera);
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate camera %s", rezo_camera->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_cameras: Envoi des cameras au client GID_CAMERA                                                */
/* Entr�e: N�ant                                                                                          */
/* Sortie: N�ant                                                                                          */
/**********************************************************************************************************/
 static void *Envoyer_cameras_thread_tag ( struct CLIENT *client, guint tag, guint sstag, guint sstag_fin )
  { struct CMD_TYPE_CAMERA *rezo_camera;
    struct CMD_ENREG nbr;
    struct CAMERADB *camera;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiCAMERA", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* D�r�f�rence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_cameraDB( Config.log, db ) )
     { Unref_client( client );                                        /* D�r�f�rence la structure cliente */
       Libere_DB_SQL( Config.log, &db );
       pthread_exit( NULL );
     }

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d cameras", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { camera = Recuperer_cameraDB_suite( Config.log, db );
       if (!camera)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* D�r�f�rence la structure cliente */
          pthread_exit ( NULL );
        }
       rezo_camera = Preparer_envoi_camera( camera );
       g_free(camera);
       if (rezo_camera)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilit� d'envoyer sur le reseau */
          Envoi_client ( client, tag, sstag,
                         (gchar *)rezo_camera, sizeof(struct CMD_TYPE_CAMERA) );
          g_free(rezo_camera);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_cameras: Envoi des cameras au client GID_CAMERA                                                */
/* Entr�e: N�ant                                                                                          */
/* Sortie: N�ant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_cameras_thread ( struct CLIENT *client )
  { Envoyer_cameras_thread_tag( client, TAG_CAMERA, SSTAG_SERVEUR_ADDPROGRESS_CAMERA,
                                                    SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FIN );
  }
/**********************************************************************************************************/
/* Envoyer_cameras: Envoi des cameras au client GID_CAMERA                                                */
/* Entr�e: N�ant                                                                                          */
/* Sortie: N�ant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_cameras_for_atelier_thread ( struct CLIENT *client )
  { Envoyer_cameras_thread_tag( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FOR_ATELIER,
                                                     SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FOR_ATELIER_FIN );
  }
/*--------------------------------------------------------------------------------------------------------*/
