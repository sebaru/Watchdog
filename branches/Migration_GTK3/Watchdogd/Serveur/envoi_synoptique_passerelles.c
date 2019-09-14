/******************************************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_passerelles.c        Envoi des passerelles aux clients                                  */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          dim 22 mai 2005 17:45:31 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * envoi_synoptique_passerelles.c
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

/*************************************************** Prototypes de fonctions **************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/******************************************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                                             */
/* Entrée: le client demandeur et le syn en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_effacer_passerelle_atelier ( struct CLIENT *client, struct CMD_TYPE_PASSERELLE *rezo_pass )
  { gboolean retour;
    retour = Retirer_passerelleDB( rezo_pass );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_PASS_OK,
                     (gchar *)rezo_pass, sizeof(struct CMD_TYPE_PASSERELLE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete pass %s", rezo_pass->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                                                   */
/* Entrée: le client demandeur et le syn en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_ajouter_passerelle_atelier ( struct CLIENT *client, struct CMD_TYPE_PASSERELLE *rezo_pass )
  { struct CMD_TYPE_PASSERELLE *result;
    gint id;
    id = Ajouter_passerelleDB ( rezo_pass );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add pass %s", rezo_pass->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_passerelleDB( id );
           if (!result)
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate pass %s", rezo_pass->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_PASS_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_PASSERELLE) );
              g_free(result);
            }
         }
  }
/******************************************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                                           */
/* Entrée: le client demandeur et le syn en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_valider_editer_passerelle_atelier ( struct CLIENT *client, struct CMD_TYPE_PASSERELLE *rezo_pass )
  { gboolean retour;
    retour = Modifier_passerelleDB ( rezo_pass );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save pass %s", rezo_pass->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Envoyer_bit_init_pass: Envoi des passerelles à l'initialisation                                                            */
/* Entrée: Le client destinataire et les tags reseaux                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Envoyer_bit_init_pass ( void *user_data, struct DLS_TREE *dls_tree )
  { struct CLIENT *client = user_data;

    if( g_slist_find( client->Liste_pass, GINT_TO_POINTER(dls_tree->syn_vars.syn_id) ) )
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                   "%s: envoi des parametres du synoptique %d a %s", __func__, dls_tree->syn_vars.syn_id, client->machine );
       Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_SET_SYN_VARS,
                      (gchar *)&dls_tree->syn_vars, sizeof(struct CMD_TYPE_SYN_VARS) );
     }
  }
/******************************************************************************************************************************/
/* Envoyer_passerelle_tag: Envoi des passerelles au client en parametre                                                       */
/* Entrée: Le client destinataire et les tags reseaux                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Envoyer_passerelle_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_TYPE_PASSERELLE *pass;
    struct CMD_ENREG nbr;
    struct DB *db;

    if ( ! Recuperer_passerelleDB( &db, client->syn_to_send->id ) )                                   /* Si pas de passerelle */
     { return; }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d gateways", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                       (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    client->Liste_pass = g_slist_prepend( client->Liste_pass, GINT_TO_POINTER(client->syn_to_send->id) );/* Pour update de la page source */
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
              "%s: add syn_cible_id '%d'", __func__, client->syn_to_send->id );

    while ( (pass = Recuperer_passerelleDB_suite( &db )) )                                      /* Et toutes les pages filles */
     { if (tag == TAG_SUPERVISION)
        { if ( ! g_slist_find( client->Liste_pass, GINT_TO_POINTER(pass->syn_cible_id) ) )
           { client->Liste_pass = g_slist_prepend( client->Liste_pass, GINT_TO_POINTER(pass->syn_cible_id) );
             Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                      "%s: add syn_cible_id '%d' length=%d", __func__, pass->syn_cible_id, g_slist_length(client->Liste_pass) );
           }
         }
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG, "%s: pass %d (%s) to client %s", __func__,
                 pass->id, pass->libelle, client->machine );
       Envoi_client ( client, tag, sstag, (gchar *)pass, sizeof(struct CMD_TYPE_PASSERELLE) );
       g_free(pass);
     }
    Envoi_client ( client, tag, sstag_fin, NULL, 0 );
    Dls_foreach ( client, NULL, Envoyer_bit_init_pass );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
