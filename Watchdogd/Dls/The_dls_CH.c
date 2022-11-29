/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_CH.c      Déclaration des fonctions pour la gestion des cpt_h                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           mar 14 fév 2006 15:03:51 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_CH.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "watchdogd.h"
 #include "Erreur.h"

/******************************************************************************************************************************/
/* Dls_data_CH_create_by_array : Création d'un CH pour le plugin                                                              */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_CH_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_CH *bit = g_try_malloc0 ( sizeof(struct DLS_CH) );
    if (!bit)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, tech_id, acronyme );
       return;
     }
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    bit->valeur = Json_get_double ( element, "valeur" );
    bit->etat   = Json_get_bool   ( element, "etat" );
    if (!strcasecmp ( tech_id, "SYS" ) ) bit->archivage = 2;            /* Si CH du plugin SYS, on archive toutes les minutes */
    plugin->Dls_data_CH = g_slist_prepend ( plugin->Dls_data_CH, bit );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_CH: Recherche un CH dans les plugins DLS                                                                   */
/* Entrée: le tech_id, l'acronyme                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_CH *Dls_data_lookup_CH ( gchar *tech_id, gchar *acronyme )
  { GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       GSList *liste = plugin->Dls_data_CH;
       while (liste)
        { struct DLS_CH *bit = liste->data;
          if ( !strcasecmp ( bit->acronyme, acronyme ) && !strcasecmp( bit->tech_id, tech_id ) ) return(bit);
          liste = g_slist_next(liste);
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_get_CH : Recupere la valeur du compteur en parametre                                                              */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gint Dls_data_get_CH ( struct DLS_CH *cpt_h )
  { if (cpt_h) return( cpt_h->valeur );
    return(0);
  }
/******************************************************************************************************************************/
/* Dls_data_set_CH: Positionne un CH dans la mémoire DLS                                                                      */
/* Entrée: le tech_id, l'acronyme, le pointeur d'accélération et la valeur entière                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_CH ( struct DLS_TO_PLUGIN *vars, struct DLS_CH *cpt_h, gboolean etat, gint reset )
  { if (!cpt_h) return;

    if (reset)
     { if (etat)
        { cpt_h->valeur = 0;
          cpt_h->etat = FALSE;
        }
     }
    else if (etat)
     { if ( ! cpt_h->etat )
        { cpt_h->etat = TRUE;
          cpt_h->old_top = Partage->top;
        }
       else
        { int new_top, delta;
          new_top = Partage->top;
          delta = new_top - cpt_h->old_top;
          if (delta >= 10)                                                              /* On compte +1 toutes les secondes ! */
           { cpt_h->valeur++;
             cpt_h->old_top = new_top;
             Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                       "%s: ligne %04d: Changing DLS_CH '%s:%s'=%d", __func__,
                       (vars ? vars->num_ligne : -1), cpt_h->tech_id, cpt_h->acronyme, cpt_h->valeur );
             Partage->audit_bit_interne_per_sec++;
           }
          if (cpt_h->last_arch + 600 < Partage->top)
           { Ajouter_arch( cpt_h->tech_id, cpt_h->acronyme, 1.0*cpt_h->valeur );
             cpt_h->last_arch = Partage->top;
           }
        }
     }
    else
     { cpt_h->etat = FALSE; }
  }
/******************************************************************************************************************************/
/* Dls_CH_to_json : Formate un CH au format JSON                                                                              */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_CH_to_json ( JsonNode *element, struct DLS_CH *bit )
  { Json_node_add_string ( element, "tech_id",   bit->tech_id );
    Json_node_add_string ( element, "acronyme",  bit->acronyme );
    Json_node_add_int    ( element, "valeur",    bit->valeur );
    Json_node_add_bool   ( element, "etat",      bit->etat );
    Json_node_add_int    ( element, "last_arch", bit->last_arch );
  };
/******************************************************************************************************************************/
/* Dls_all_CH_to_json: Transforme tous les bits en JSON                                                                       */
/* Entrée: target                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_all_AI_to_json ( gpointer array, struct DLS_PLUGIN *plugin )
  { JsonArray *RootArray = array;
    GSList *liste = plugin->Dls_data_CH;
    while ( liste )
     { struct DLS_AI *bit = liste->data;
       JsonNode *element = Json_node_create();
       Dls_AI_to_json ( element, bit );
       Json_array_add_element ( RootArray, element );
       liste = g_slist_next(liste);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
