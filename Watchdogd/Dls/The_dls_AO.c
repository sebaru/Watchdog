/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_AO.c        Déclaration des fonctions pour la gestion des AO                                         */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                25.03.2019 14:16:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_AO.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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

/******************************************************************************************************************************/
/* Dls_data_AO_create_by_array : Création d'un AO pour le plugin                                                              */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_AO_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_AO *bit = g_try_malloc0 ( sizeof(struct DLS_AO) );
    if (!bit)
     { Info_new( __func__, Config.log_dls, LOG_ERR, "Memory error for '%s:%s'", tech_id, acronyme );
       return;
     }
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    g_snprintf( bit->unite,    sizeof(bit->unite),    "%s", Json_get_string ( element, "unite" ) );
    bit->archivage = Json_get_int    ( element, "archivage" );
    bit->valeur    = Json_get_double ( element, "valeur" );
    plugin->Dls_data_AO = g_slist_prepend ( plugin->Dls_data_AO, bit );
    Info_new( __func__, Config.log_dls, LOG_INFO,
              "Create bit DLS_AO '%s:%s'=%f %s (%s) archivage=%d",
               bit->tech_id, bit->acronyme, bit->valeur, bit->unite, bit->libelle, bit->archivage );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_AO : Recherche un CH dans les plugins DLS                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 struct DLS_AO *Dls_data_lookup_AO ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp( plugin->tech_id, tech_id ))
        { GSList *liste = plugin->Dls_data_AO;
          while (liste)
           { struct DLS_AO *bit = liste->data;
             if ( !strcasecmp ( bit->acronyme, acronyme ) ) return(bit);
             liste = g_slist_next(liste);
           }
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_get_AO : Recupere la valeur de l'AO en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gdouble Dls_data_get_AO ( struct DLS_AO *ao )
  { if (!ao) return(0.0);
    return( ao->valeur );
  }
/******************************************************************************************************************************/
/* Met à jour la sortie analogique à partir de sa valeur avant mise a l'echelle                                               */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_AO ( struct DLS_TO_PLUGIN *vars, struct DLS_AO *bit, gdouble valeur )
  { if (!bit) return;
    if (bit->valeur == valeur) return;
    bit->valeur = valeur;                                                           /* Archive au mieux toutes les 5 secondes */
    Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
              "ligne %04d: Changing DLS_AO '%s:%s'=%f %s",
              (vars ? vars->num_ligne : -1), bit->tech_id, bit->acronyme, bit->valeur, bit->unite );
    JsonNode *RootNode = Json_node_create ();
    if (RootNode)
     { Dls_AO_to_json ( RootNode, bit );
       pthread_rwlock_wrlock( &Partage->Liste_AO_synchro );                           /* Ajout dans la liste des AO a traiter */
       Partage->Liste_AO = g_slist_append( Partage->Liste_AO, RootNode );
       pthread_rwlock_unlock( &Partage->Liste_AO_synchro );
     }
    else Info_new( __func__, Config.log_msrv, LOG_ERR, "JSon RootNode creation failed" );
    Partage->audit_bit_interne_per_sec++;
    Dls_AO_export_to_API ( bit );                                                                            /* envoi a l'API */
  }
/******************************************************************************************************************************/
/* Dls_AO_to_json: Convertir un AO en JSON                                                                                    */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_AO_to_json ( JsonNode *element, struct DLS_AO *bit )
  { Json_node_add_string ( element, "classe",       "AO" );
    Json_node_add_string ( element, "tech_id",      bit->tech_id );
    Json_node_add_string ( element, "acronyme",     bit->acronyme );
    Json_node_add_string ( element, "unite",        bit->unite );
    Json_node_add_double ( element, "valeur",       bit->valeur );
    Json_node_add_int    ( element, "archivage",    bit->archivage );
  }
/******************************************************************************************************************************/
/* Dls_all_AO_to_json: Transforme tous les bits en JSON                                                                       */
/* Entrée: target                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_all_AO_to_json ( gpointer array, struct DLS_PLUGIN *plugin )
  { JsonArray *RootArray = array;
    GSList *liste = plugin->Dls_data_AO;
    while ( liste )
     { struct DLS_AO *bit = liste->data;
       JsonNode *element = Json_node_create();
       Dls_AO_to_json ( element, bit );
       Json_array_add_element ( RootArray, element );
       liste = g_slist_next(liste);
     }
  }
/******************************************************************************************************************************/
/* Dls_AO_export_to_API : Formate un bit au format JSON                                                                       */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_AO_export_to_API ( struct DLS_AO *bit )
  { JsonNode *element = Json_node_create ();
    if (element)
     { Json_node_add_double ( element, "valeur", bit->valeur );
       MQTT_Send_to_API ( element, "DLS_REPORT/AO/%s/%s", bit->tech_id, bit->acronyme );
       Json_node_unref ( element );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
