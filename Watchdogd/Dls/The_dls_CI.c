/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_CI.c      Déclaration des fonctions pour la gestion des compteurs d'impulsions                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mar. 07 déc. 2010 17:26:52 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_CI.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
 #include <locale.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Dls_data_CI_create_by_array : Création d'un CI pour le plugin                                                              */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_CI_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_CI *bit = g_try_malloc0 ( sizeof(struct DLS_CI) );
    if (!bit)
     { Info_new( __func__, Config.log_dls, LOG_ERR, "Memory error for '%s:%s'", tech_id, acronyme );
       return;
     }
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    g_snprintf( bit->unite,    sizeof(bit->unite),    "%s", Json_get_string ( element, "unite" ) );
    bit->valeur    = Json_get_int ( element, "valeur" );
    bit->multi     = Json_get_double ( element, "multi" );
    bit->archivage = Json_get_int ( element, "archivage" );
    bit->etat      = Json_get_bool ( element, "etat" );
    plugin->Dls_data_CI = g_slist_prepend ( plugin->Dls_data_CI, bit );
    Info_new( __func__, Config.log_dls, LOG_INFO,
              "Create bit DLS_CI '%s:%s'=%d (%s)", bit->tech_id, bit->acronyme, bit->valeur, bit->libelle );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_CI : Recherche un CH dans les plugins DLS                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 struct DLS_CI *Dls_data_lookup_CI ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp( plugin->tech_id, tech_id ))
        { GSList *liste = plugin->Dls_data_CI;
          while (liste)
           { struct DLS_CI *bit = liste->data;
             if ( !strcasecmp ( bit->acronyme, acronyme ) ) return(bit);
             liste = g_slist_next(liste);
           }
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_set_CI: Positionne un compteur d'impulsion                                                                        */
/* Entrée: le tech_id, l'acronyme, le pointeur d'accélération et la valeur entière                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_CI ( struct DLS_TO_PLUGIN *vars, struct DLS_CI *cpt_imp, gboolean etat, gint reset, gint ratio )
  { if (!cpt_imp) return;
    if (etat)
     { if (reset)                                                                       /* Le compteur doit-il etre resetté ? */
        { if (cpt_imp->valeur!=0)
           { cpt_imp->val_en_cours1 = 0;                                           /* Valeur transitoire pour gérer les ratio */
             cpt_imp->valeur = 0;                                                  /* Valeur transitoire pour gérer les ratio */
           }
        }
       else if ( cpt_imp->etat == FALSE )                                                                 /* Passage en actif */
        { cpt_imp->etat = TRUE;
          Partage->audit_bit_interne_per_sec++;
          cpt_imp->val_en_cours1++;
          if (cpt_imp->val_en_cours1>=ratio)
           { cpt_imp->valeur++;
             cpt_imp->val_en_cours1=0;                                                        /* RAZ de la valeur de calcul 1 */
             if (cpt_imp->abonnement) Dls_cadran_send_CI_to_API ( cpt_imp );
             Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                       "ligne %04d: Changing DLS_CI '%s:%s'=%d",
                       (vars ? vars->num_ligne : -1), cpt_imp->tech_id, cpt_imp->acronyme, cpt_imp->valeur );
           }
        }
     }
    else
     { if (reset==0) cpt_imp->etat = FALSE; }
  }
/******************************************************************************************************************************/
/* Dls_data_get_CI : Recupere la valeur du compteur en parametre                                                              */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gint Dls_data_get_CI ( struct DLS_CI *cpt_imp )
  { if (!cpt_imp) return(0);
    return( cpt_imp->valeur );
  }
/******************************************************************************************************************************/
/* Dls_cadran_send_CI_to_API: Ennvoi un CI à l'API pour affichage des cadrans                                                 */
/* Entrées: la structure DLs_AI                                                                                               */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_cadran_send_CI_to_API ( struct DLS_CI *bit )
  { if (!bit) return;
    JsonNode *RootNode = Json_node_create();
    Dls_CI_to_json ( RootNode, bit );
    pthread_mutex_lock ( &Partage->abonnements_synchro );
    Partage->abonnements = g_slist_append ( Partage->abonnements, RootNode );
    pthread_mutex_unlock ( &Partage->abonnements_synchro );
  }
/******************************************************************************************************************************/
/* Dls_CI_to_json : Formate un CI au format JSON                                                                              */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_CI_to_json ( JsonNode *element, struct DLS_CI *bit )
  { Json_node_add_string ( element, "classe",   "CI" );
    Json_node_add_string ( element, "tech_id",   bit->tech_id );
    Json_node_add_string ( element, "acronyme",  bit->acronyme );
    Json_node_add_int    ( element, "valeur",    bit->valeur );
    Json_node_add_double ( element, "multi",     bit->multi );
    Json_node_add_string ( element, "unite",     bit->unite );
    Json_node_add_bool   ( element, "etat",      bit->etat );
    Json_node_add_int    ( element, "archivage", bit->archivage );
    Json_node_add_string ( element, "libelle",   bit->libelle );
  };
/******************************************************************************************************************************/
/* Dls_all_CI_to_json: Transforme tous les bits en JSON                                                                       */
/* Entrée: target                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_all_CI_to_json ( gpointer array, struct DLS_PLUGIN *plugin )
  { JsonArray *RootArray = array;
    GSList *liste = plugin->Dls_data_CI;
    while ( liste )
     { struct DLS_CI *bit = liste->data;
       JsonNode *element = Json_node_create();
       Dls_CI_to_json ( element, bit );
       Json_array_add_element ( RootArray, element );
       liste = g_slist_next(liste);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
