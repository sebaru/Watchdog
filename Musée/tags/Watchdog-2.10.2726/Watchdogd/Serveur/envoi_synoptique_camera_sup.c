/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_camera_sup.c     Envoi des camera_sups aux clients                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 22 mai 2005 17:35:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique_camera_sup.c
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

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                         */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_camera_sup_atelier ( struct CLIENT *client, struct CMD_TYPE_CAMERA_SUP *rezo_camera_sup )
  { gboolean retour;
#ifdef bouh
    retour = Retirer_camera_supDB( Config.log, Db_watchdog, rezo_camera_sup );
    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_CAMERA_SUP_OK,
                     (gchar *)rezo_camera_sup, sizeof(struct CMD_TYPE_CAMERA_SUP) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete camera_sup %s", rezo_camera_sup->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
#endif
  }
/**********************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                               */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_camera_sup_atelier ( struct CLIENT *client, struct CMD_TYPE_CAMERA_SUP *rezo_camera_sup )
  { struct CMD_TYPE_CAMERA_SUP *result;
    gint id;

#ifdef bouh
    id = Ajouter_camera_supDB ( Config.log, Db_watchdog, rezo_camera_sup );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add camera_sup %s", rezo_camera_sup->libelle );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_camera_supDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate camera_sup %s", rezo_camera_sup->libelle );
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_CAMERA_SUP_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_CAMERA_SUP) );
              g_free(result);
            }
         }
#endif
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_camera_sup_atelier ( struct CLIENT *client,
                                                struct CMD_TYPE_CAMERA_SUP *rezo_camera_sup )
  { gboolean retour;
#ifdef bouh
    retour = Modifier_camera_supDB ( Config.log, Db_watchdog, rezo_camera_sup );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save camera_sup %d", rezo_camera_sup->camera_src_id);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
#endif
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_camera_sup_atelier_thread ( struct CLIENT *client )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_CAMERA_SUP *camera_sup;
    struct DB *db;
    gchar titre[20];
    g_snprintf( titre, sizeof(titre), "W-CAMS-%06d", client->ssrv_id );
    prctl(PR_SET_NAME, titre, 0, 0, 0 );
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit( NULL );

#ifdef bouh

    db = Init_DB_SQL();       
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_camera_supDB( Config.log, db, client->syn.id ) )
     { Libere_DB_SQL( &db );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d camera_sups", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { camera_sup = Recuperer_camera_supDB_suite( Config.log, db );
       if (!camera_sup)
        { Libere_DB_SQL( &db );
          /* Test 19/09/2009 Client_mode( client, ENVOI_GROUPE_FOR_PROPRIETE_SYNOPTIQUE );*/
          Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAMERA_SUP_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }
       Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAMERA_SUP,
                      (gchar *)camera_sup, sizeof(struct CMD_TYPE_CAMERA_SUP) );
       g_free(camera_sup);
     }
#endif
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_camera_sup_supervision_thread ( struct CLIENT *client )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_CAMERA_SUP *camera_sup;
    struct DB *db;

    gchar titre[20];
    g_snprintf( titre, sizeof(titre), "W-CAMS-%06d", client->ssrv_id );
    prctl(PR_SET_NAME, titre, 0, 0, 0 );
    Client_mode( client, ENVOI_IXXX_SUPERVISION );
    Unref_client( client );                                     /* Déréférence la structure cliente */
    pthread_exit( NULL );
#ifdef bouh

    db = Init_DB_SQL();       
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_camera_supDB( Config.log, db, client->num_supervision ) )
     { Client_mode( client, ENVOI_IXXX_SUPERVISION );
       Libere_DB_SQL( &db );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d camera_sups", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { camera_sup = Recuperer_camera_supDB_suite( Config.log, db );
       if (!camera_sup)                                                                     /* Terminé ?? */
        { Client_mode( client, ENVOI_IXXX_SUPERVISION );
          Libere_DB_SQL( &db );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAMERA_SUP_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit( NULL );
        }

       Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAMERA_SUP,
                      (gchar *)camera_sup, sizeof(struct CMD_TYPE_CAMERA_SUP) );
       g_free(camera_sup);
     }
#endif
  }
/*--------------------------------------------------------------------------------------------------------*/
