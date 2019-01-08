/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_comments.c        Envoi des comments aux clients                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                      sam 11 avr 2009 10:50:24 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique_comments.c
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
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                         */
/* Entr�e: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_comment_atelier ( struct CLIENT *client, struct CMD_TYPE_COMMENT *rezo_comment )
  { gboolean retour;
    retour = Retirer_commentDB( rezo_comment );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_COMMENT_OK,
                     (gchar *)rezo_comment, sizeof(struct CMD_TYPE_COMMENT) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete comment %s", rezo_comment->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                               */
/* Entr�e: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_comment_atelier ( struct CLIENT *client, struct CMD_TYPE_COMMENT *rezo_comment )
  { struct CMD_TYPE_COMMENT *result;
    gint id;

    id = Ajouter_commentDB ( rezo_comment );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add comment %s", rezo_comment->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_commentDB( id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate comment %s", rezo_comment->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_COMMENT_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_COMMENT) );
              g_free(result);
            }
         }
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entr�e: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_comment_atelier ( struct CLIENT *client, struct CMD_TYPE_COMMENT *rezo_comment )
  { gboolean retour;
    retour = Modifier_commentDB ( rezo_comment );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save comment %s", rezo_comment->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Envoyer_comment_tag: Envoi des commentaires synoptiques au client en parametre                                             */
/* Entr�e: Le client et les tags reseaux                                                                                      */
/* Sortie: N�ant                                                                                                              */
/******************************************************************************************************************************/
 void Envoyer_comment_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_COMMENT *comment;
    struct DB *db;

    if ( ! Recuperer_commentDB( &db, client->syn_to_send->id ) )
     { return;                                                             /* Si pas de commentaires (??) */
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d comments", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    while ( (comment = Recuperer_commentDB_suite( &db ) ) )
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoyer_comment_atelier: envoi comment %d (%s) to client %s",
                 comment->id, comment->libelle, client->machine );
       Envoi_client ( client, tag, sstag,
                      (gchar *)comment, sizeof(struct CMD_TYPE_COMMENT) );
       g_free(comment);
     }
    Envoi_client ( client, tag, sstag_fin, NULL, 0 );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
