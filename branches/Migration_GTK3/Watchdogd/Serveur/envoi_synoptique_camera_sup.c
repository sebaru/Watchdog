/******************************************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_camera_sup.c     Envoi des camera_sups aux clients                                      */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          dim 22 mai 2005 17:35:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * envoi_synoptique_camera_sup.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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

/*************************************************** Prototypes de fonctions **************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/******************************************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                                             */
/* Entrée: le client demandeur et le syn en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_effacer_camera_sup_atelier ( struct CLIENT *client, struct CMD_TYPE_CAMERASUP *rezo_camera_sup )
  { gboolean retour;
    retour = Retirer_camera_supDB( rezo_camera_sup->id );
    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_CAMERA_SUP_OK,
                     (gchar *)rezo_camera_sup, sizeof(struct CMD_TYPE_CAMERASUP) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete camera_sup %s", rezo_camera_sup->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                                                   */
/* Entrée: le client demandeur et le syn en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_ajouter_camera_sup_atelier ( struct CLIENT *client, struct CMD_TYPE_CAMERASUP *rezo_camera_sup )
  { struct CMD_TYPE_CAMERASUP *result;
    gint id;

    id = Ajouter_camera_supDB ( rezo_camera_sup );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add camera_sup %s", rezo_camera_sup->libelle );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_camera_supDB( id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate camera_sup %s", rezo_camera_sup->libelle );
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_CAMERA_SUP_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_CAMERASUP) );
              g_free(result);
            }
         }
  }
/******************************************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                                           */
/* Entrée: le client demandeur et le syn en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_valider_editer_camera_sup_atelier ( struct CLIENT *client,
                                                struct CMD_TYPE_CAMERASUP *rezo_camera_sup )
  { if ( Modifier_camera_supDB ( rezo_camera_sup ) ==-1 )
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save camera_sup %d", rezo_camera_sup->camera_src_id);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                                      */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Envoyer_camera_sup_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_CAMERASUP *camera_sup;
    struct DB *db;

    if ( ! Recuperer_camera_supDB( &db, client->syn_to_send->id ) ) return;

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d camera_sups", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    while ( (camera_sup = Recuperer_camera_supDB_suite( &db )) != NULL )
     { Envoi_client ( client, tag, sstag, (gchar *)camera_sup, sizeof(struct CMD_TYPE_CAMERASUP) );
       g_free(camera_sup);
     }
    Envoi_client ( client, tag, sstag_fin, NULL, 0 );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
