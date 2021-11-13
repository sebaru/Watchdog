/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_camera.c        Configuration des cameras de Watchdog v2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 11:24:00 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_camera.c
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
 #include <sys/prctl.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>
 #include <pthread.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Proto_editer_camera: Le client desire editer un camera                                                 */
/* Entrée: le client demandeur et le camera en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera )
  { struct CMD_TYPE_CAMERA *camera;
    struct DB *Db_watchdog;
#ifdef bouh
    Db_watchdog = client->Db_watchdog;

    camera = Rechercher_cameraDB( Config.log, Db_watchdog, rezo_camera->id );

    if (camera)
     { Envoi_client( client, TAG_CAMERA, SSTAG_SERVEUR_EDIT_CAMERA_OK,
                     (gchar *)camera, sizeof(struct CMD_TYPE_CAMERA) );
       g_free(camera);                                                              /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate camera %s", rezo_camera->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
#endif
  }
/**********************************************************************************************************/
/* Proto_valider_editer_camera: Le client valide l'edition d'un camera                                    */
/* Entrée: le client demandeur et le camera en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera )
  { struct CMD_TYPE_CAMERA *result;
    gboolean retour;
    struct DB *Db_watchdog;
#ifdef bouh
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
         { if (!result)
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate camera %s", rezo_camera->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else { Envoi_client( client, TAG_CAMERA, SSTAG_SERVEUR_VALIDE_EDIT_CAMERA_OK,
                                (gchar *)result, sizeof(struct CMD_TYPE_CAMERA) );
                  Partage->com_msrv.reset_motion_detect = TRUE;    /* Modification -> Reset motion_detect */
                  g_free(result);
                }
            }
         }
#endif
  }
/**********************************************************************************************************/
/* Proto_effacer_camera: Retrait du camera en parametre                                                   */
/* Entrée: le client demandeur et le camera en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera )
  { gboolean retour;
    struct DB *Db_watchdog;
#ifdef bouh
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_cameraDB( Config.log, Db_watchdog, rezo_camera );

    if (retour)
     { Envoi_client( client, TAG_CAMERA, SSTAG_SERVEUR_DEL_CAMERA_OK,
                     (gchar *)rezo_camera, sizeof(struct CMD_TYPE_CAMERA) );
       Partage->com_msrv.reset_motion_detect = TRUE;               /* Modification -> Reset motion_detect */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete camera %s", rezo_camera->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
#endif
  }
/**********************************************************************************************************/
/* Proto_ajouter_camera: Un client nous demande d'ajouter un camera Watchdog                              */
/* Entrée: le camera à créer                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera )
  { struct CMD_TYPE_CAMERA *camera;
    struct DB *Db_watchdog;
    gint id;
#ifdef bouh
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_cameraDB ( Config.log, Db_watchdog, rezo_camera );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add camera %s", rezo_camera->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { camera = Rechercher_cameraDB( Config.log, Db_watchdog, id );
           if (!camera) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate camera %s", rezo_camera->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_CAMERA, SSTAG_SERVEUR_ADD_CAMERA_OK,
                            (gchar *)camera, sizeof(struct CMD_TYPE_CAMERA) );
              Partage->com_msrv.reset_motion_detect = TRUE;        /* Modification -> Reset motion_detect */
              g_free(camera);
            }
         }
#endif
  }
/**********************************************************************************************************/
/* Envoyer_cameras: Envoi des cameras au client GID_CAMERA                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void *Envoyer_cameras_thread_tag ( struct CLIENT *client, guint tag, guint sstag, guint sstag_fin )
  { struct CMD_TYPE_CAMERA *camera;
    struct CMD_ENREG nbr;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiCAMERA", 0, 0, 0 );

Unref_client( client );                                        /* Déréférence la structure cliente */
pthread_exit( NULL );

#ifdef bouh
    db = Init_DB_SQL();       
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_cameraDB( Config.log, db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Libere_DB_SQL( &db );
       pthread_exit( NULL );
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d cameras", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { camera = Recuperer_cameraDB_suite( Config.log, db );
       if (!camera)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Libere_DB_SQL( &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       Envoi_client ( client, tag, sstag, (gchar *)camera, sizeof(struct CMD_TYPE_CAMERA) );
       g_free(camera);
     }
#endif
  }
/**********************************************************************************************************/
/* Envoyer_cameras: Envoi des cameras au client GID_CAMERA                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_cameras_thread ( struct CLIENT *client )
  { Envoyer_cameras_thread_tag( client, TAG_CAMERA, SSTAG_SERVEUR_ADDPROGRESS_CAMERA,
                                                    SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FIN );
    return(NULL);
  }
/**********************************************************************************************************/
/* Envoyer_cameras: Envoi des cameras au client GID_CAMERA                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_cameras_for_atelier_thread ( struct CLIENT *client )
  { Envoyer_cameras_thread_tag( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FOR_ATELIER,
                                                     SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FOR_ATELIER_FIN );
    return(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
