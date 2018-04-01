/******************************************************************************************************************************/
/* Watchdogd/Serveur/envoi_camera.c        Configuration des cameras de Watchdog v2.0                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                       dim. 13 sept. 2009 11:24:00 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

/*************************************************** Prototypes de fonctions **************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/******************************************************************************************************************************/
/* Proto_editer_camera: Le client desire editer un camera                                                                     */
/* Entrée: le client demandeur et le camera en question                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_editer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera )
  { struct CMD_TYPE_CAMERA *camera;

    camera = Rechercher_cameraDB( rezo_camera->id );

    if (camera)
     { Envoi_client( client, TAG_LOWLEVEL, SSTAG_SERVEUR_EDIT_CAMERA_OK,
                     (gchar *)camera, sizeof(struct CMD_TYPE_CAMERA) );
       g_free(camera);                                                                                  /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate camera %s", rezo_camera->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Proto_valider_editer_camera: Le client valide l'edition d'un camera                                                        */
/* Entrée: le client demandeur et le camera en question                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_valider_editer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera )
  { struct CMD_TYPE_CAMERA *camera;
    gint retour;

    retour = Modifier_cameraDB ( rezo_camera );
    if (retour==-1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit camera %s", rezo_camera->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { camera = Rechercher_cameraDB( rezo_camera->id );
           if (camera) 
            { Envoi_client( client, TAG_LOWLEVEL, SSTAG_SERVEUR_VALIDE_EDIT_CAMERA_OK,
                            (gchar *)camera, sizeof(struct CMD_TYPE_CAMERA) );
              g_free(camera);
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
/******************************************************************************************************************************/
/* Proto_effacer_camera: Retrait du camera en parametre                                                                       */
/* Entrée: le client demandeur et le camera en question                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_effacer_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera )
  { gboolean retour;

    retour = Retirer_cameraDB( rezo_camera->id );

    if (retour)
     { Envoi_client( client, TAG_LOWLEVEL, SSTAG_SERVEUR_DEL_CAMERA_OK,
                     (gchar *)rezo_camera, sizeof(struct CMD_TYPE_CAMERA) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete camera %s", rezo_camera->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Proto_ajouter_camera: Un client nous demande d'ajouter un camera Watchdog                                                  */
/* Entrée: le camera à créer                                                                                                  */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_ajouter_camera ( struct CLIENT *client, struct CMD_TYPE_CAMERA *rezo_camera )
  { struct CMD_TYPE_CAMERA *camera;
    struct DB *Db_watchdog;
    gint id;

    id = Ajouter_cameraDB ( rezo_camera );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add camera %s", rezo_camera->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { camera = Rechercher_cameraDB( id );
           if (!camera) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate camera %s", rezo_camera->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_LOWLEVEL, SSTAG_SERVEUR_ADD_CAMERA_OK,
                            (gchar *)camera, sizeof(struct CMD_TYPE_CAMERA) );
              g_free(camera);
            }
         }
  }
/******************************************************************************************************************************/
/* Envoyer_cameras: Envoi des cameras au client GID_CAMERA                                                                    */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void *Envoyer_cameras_thread_tag ( struct CLIENT *client, guint tag, guint sstag, guint sstag_fin )
  { struct CMD_TYPE_CAMERA *camera;
    struct CMD_ENREG nbr;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiCAMERA", 0, 0, 0 );

    if ( ! Recuperer_cameraDB( &db ) )
     { Unref_client( client );                                                            /* Déréférence la structure cliente */
       Libere_DB_SQL( &db );
       pthread_exit( NULL );
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d cameras", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    while ( (camera = Recuperer_cameraDB_suite( &db ) ) != NULL )
     { Envoi_client ( client, tag, sstag, (gchar *)camera, sizeof(struct CMD_TYPE_CAMERA) );
       g_free(camera);
     }

    Envoi_client ( client, tag, sstag_fin, NULL, 0 );
    Libere_DB_SQL( &db );
    Unref_client( client );                                                               /* Déréférence la structure cliente */
    pthread_exit ( NULL );
  }
/******************************************************************************************************************************/
/* Envoyer_cameras: Envoi des cameras au client GID_CAMERA                                                                    */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void *Envoyer_cameras_thread ( struct CLIENT *client )
  { Envoyer_cameras_thread_tag( client, TAG_LOWLEVEL, SSTAG_SERVEUR_ADDPROGRESS_CAMERA,
                                                    SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FIN );
    return(NULL);
  }
/******************************************************************************************************************************/
/* Envoyer_cameras: Envoi des cameras au client GID_CAMERA                                                                   */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void *Envoyer_cameras_for_atelier_thread ( struct CLIENT *client )
  { Envoyer_cameras_thread_tag( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FOR_ATELIER,
                                                     SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FOR_ATELIER_FIN );
    return(NULL);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
