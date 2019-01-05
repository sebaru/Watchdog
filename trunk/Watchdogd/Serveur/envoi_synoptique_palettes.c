/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_palettes.c     Envoi des palettes aux clients                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                      dim 22 mai 2005 17:35:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique_palettes.c
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
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_palette_atelier ( struct CLIENT *client, struct CMD_TYPE_PALETTE *rezo_palette )
  { gboolean retour;
    retour = Retirer_paletteDB( rezo_palette );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_PALETTE_OK,
                     (gchar *)rezo_palette, sizeof(struct CMD_TYPE_PALETTE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete palette %s", rezo_palette->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                               */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_palette_atelier ( struct CLIENT *client, struct CMD_TYPE_PALETTE *rezo_palette )
  { struct CMD_TYPE_PALETTE *result;
    gint id;

    id = Ajouter_paletteDB ( rezo_palette );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add palette %s", rezo_palette->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_paletteDB( id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate palette %s", rezo_palette->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_PALETTE_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_PALETTE) );
              g_free(result);
            }
         }
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_palette_atelier ( struct CLIENT *client, struct CMD_TYPE_PALETTE *rezo_palette )
  { gboolean retour;
    retour = Modifier_paletteDB ( rezo_palette );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save palette %s", rezo_palette->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Envoyer_palette_tag: Envoi les palettes au client en parametre                                                             */
/* Entrée: Le client et les tags associés                                                                                     */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Envoyer_palette_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_TYPE_PALETTE *palette;
    struct CMD_ENREG nbr;
    struct DB *db;

    if ( ! Recuperer_paletteDB( &db, client->syn_to_send->id ) )
     { return;
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d palettes", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }
    for( ; ; )
     { palette = Recuperer_paletteDB_suite( &db );
       if (!palette)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          return;
        }

       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoyer_palette_thread_tag: envoi pass %d (%s) to client %d",
                palette->id, palette->libelle, client->machine );
       Envoi_client ( client, tag, sstag,
                      (gchar *)palette, sizeof(struct CMD_TYPE_PALETTE) );
       g_free(palette);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
