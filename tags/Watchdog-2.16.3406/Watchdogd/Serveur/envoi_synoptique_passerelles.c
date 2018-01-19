/******************************************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_passerelles.c        Envoi des passerelles aux clients                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          dim 22 mai 2005 17:45:31 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * envoi_synoptique_passerelles.c
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
/* Envoyer_passerelle_tag: Envoi des passerelles au client en parametre                                                       */
/* Entrée: Le client destinataire et les tags reseaux                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Envoyer_passerelle_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_TYPE_PASSERELLE *pass;
    struct CMD_ENREG nbr;
    GSList *liste_bit_init = NULL;
    struct DB *db;

    if ( ! Recuperer_passerelleDB( &db, client->syn_to_send->id ) )                                   /* Si pas de passerelle */
     { return; }
     
    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d gateways", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                       (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    while ( (pass = Recuperer_passerelleDB_suite( &db )) )
     { if (tag == TAG_SUPERVISION)
        { if ( ! g_slist_find( liste_bit_init, GINT_TO_POINTER(pass->vignette_activite) ) )
           { liste_bit_init = g_slist_prepend( liste_bit_init, GINT_TO_POINTER(pass->vignette_activite) );
             Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                      "liste des bit_init_syn pass %d", pass->vignette_activite );
           }

          if ( ! g_slist_find( liste_bit_init, GINT_TO_POINTER(pass->vignette_secu_bien) ) )
           { liste_bit_init = g_slist_prepend( liste_bit_init, GINT_TO_POINTER(pass->vignette_secu_bien) );
             Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                      "liste des bit_init_syn pass %d", pass->vignette_secu_bien );
           }

          if ( ! g_slist_find( liste_bit_init, GINT_TO_POINTER(pass->vignette_secu_personne) ) )
           { liste_bit_init = g_slist_prepend( liste_bit_init, GINT_TO_POINTER(pass->vignette_secu_personne) );
             Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                      "liste des bit_init_syn pass %d", pass->vignette_secu_personne );
           }
         }
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoyer_passerelle_tag: pass %d (%s) to client %s",
                 pass->id, pass->libelle, client->machine );
       Envoi_client ( client, tag, sstag,
                      (gchar *)pass, sizeof(struct CMD_TYPE_PASSERELLE) );
       g_free(pass);
     }
    Envoi_client ( client, tag, sstag_fin, NULL, 0 );
    Envoyer_bit_init_motif ( client, liste_bit_init );                                     /* Envoi des bits d'initialisation */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
