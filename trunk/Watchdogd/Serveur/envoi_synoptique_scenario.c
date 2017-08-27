/******************************************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_scenario.c     Envoi des scenarios aux clients                                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    17.07.2017 20:17:43 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * envoi_synoptique_scenario.c
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

/*************************************************** Prototypes de fonctions **************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/******************************************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                                             */
/* Entrée: le client demandeur et le syn en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_effacer_scenario_atelier ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_scenario )
  { gboolean retour;
    retour = Retirer_scenarioDB( rezo_scenario->id );
    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_SCENARIO_OK,
                     (gchar *)rezo_scenario, sizeof(struct CMD_TYPE_SCENARIO) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete scenario %d", rezo_scenario->id);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                                                   */
/* Entrée: le client demandeur et le syn en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_ajouter_scenario_atelier ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_scenario )
  { struct CMD_TYPE_SCENARIO *result;
    gint id;

    id = Ajouter_scenarioDB ( rezo_scenario );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add scenario %d", rezo_scenario->num );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_scenarioDB( id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate scenario %d", rezo_scenario->num );
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_SCENARIO_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_SCENARIO) );
              g_free(result);
            }
         }
  }
/******************************************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                                           */
/* Entrée: le client demandeur et le syn en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_valider_editer_scenario_atelier ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_scenario )
  { if ( Modifier_scenarioDB ( rezo_scenario ) ==-1 )
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save scenario %d", rezo_scenario->num);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                                      */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Envoyer_scenario_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_SCENARIO *scenario;
    struct DB *db;

    if ( ! Recuperer_scenarioDB( &db, client->syn_to_send->id ) ) return;

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d scenarios", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    while ( (scenario = Recuperer_scenarioDB_suite( &db )) != NULL )
     { Envoi_client ( client, tag, sstag, (gchar *)scenario, sizeof(struct CMD_TYPE_SCENARIO) );
       g_free(scenario);
     }
    Envoi_client ( client, tag, sstag_fin, NULL, 0 );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
