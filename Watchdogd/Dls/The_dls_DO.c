/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_DO.c        Déclaration des fonctions pour la gestion des Sorties TOR                                */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    25.03.2019 14:16:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_DO.c
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

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Dls_data_DO_create_by_array : Création d'un DO pour le plugin                                                              */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_DO_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_DO *bit = g_try_malloc0 ( sizeof(struct DLS_DO) );
    if (!bit)
     { Info_new( __func__, Config.log_dls, LOG_ERR, "Memory error for '%s:%s'", tech_id, acronyme );
       return;
     }
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    bit->etat = Json_get_bool ( element, "etat" );
    bit->mono = Json_get_bool ( element, "mono" );
    plugin->Dls_data_DO = g_slist_prepend ( plugin->Dls_data_DO, bit );
    Info_new( __func__, Config.log_dls, LOG_INFO,
              "Create bit DLS_DO '%s:%s'=%d (%s) mono=%d", bit->tech_id, bit->acronyme, bit->etat, bit->libelle, bit->mono );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_DO: Recherche un DO dans les plugins DLS                                                                   */
/* Entrée: le tech_id, l'acronyme                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_DO *Dls_data_lookup_DO ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp( plugin->tech_id, tech_id ))
        { GSList *liste = plugin->Dls_data_DO;
          while (liste)
           { struct DLS_DO *bit = liste->data;
             if ( !strcasecmp ( bit->acronyme, acronyme ) ) return(bit);
             liste = g_slist_next(liste);
           }
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_get_DO: Remonte l'etat d'une sortie tor                                                                           */
/* Sortie : TRUE sur la sortie est UP                                                                                         */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DO ( struct DLS_DO *dout )
  { if (!dout) return(FALSE);
    return( dout->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_set_DO: Positionne une bit de sortie TOR                                                                          */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_DO ( struct DLS_TO_PLUGIN *vars, struct DLS_DO *dout, gboolean etat )
  { if (!dout) return;
    if (dout->etat == etat) return;
    dout->etat = etat;
    Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
              "ligne %04d: Changing DLS_DO '%s:%s'=%d ",
              (vars ? vars->num_ligne : -1), dout->tech_id, dout->acronyme, dout->etat );

    JsonNode *RootNode = Json_node_create ();
    if (RootNode)
     { Dls_DO_to_json ( RootNode, dout );
       pthread_mutex_lock( &Partage->com_msrv.synchro );                          /* Envoie au MSRV pour dispatch aux threads */
       Partage->com_msrv.Liste_DO = g_slist_append ( Partage->com_msrv.Liste_DO, RootNode );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
     }
    else Info_new( __func__, Config.log_msrv, LOG_ERR, "JSon RootNode creation failed" );

    if (etat == TRUE && dout->mono) /* Si sortie de type monostable, elle redescend tout de suite */
     { JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { Dls_DO_to_json ( RootNode, dout );
          Json_node_add_bool ( RootNode, "etat", FALSE );                                    /* Passage a zero dans la foulée */
          pthread_mutex_lock( &Partage->com_msrv.synchro );                       /* Envoie au MSRV pour dispatch aux threads */
          Partage->com_msrv.Liste_DO = g_slist_append ( Partage->com_msrv.Liste_DO, RootNode );
          pthread_mutex_unlock( &Partage->com_msrv.synchro );
        }
       else Info_new( __func__, Config.log_msrv, LOG_ERR, "JSon RootNode creation failed" );
     }
    Partage->audit_bit_interne_per_sec++;
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool_up: Remonte le front montant d'un boolean                                                                */
/* Sortie : TRUE sur le boolean vient de passer à UP                                                                          */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DO_up ( struct DLS_DO *dout )
  { if (!dout) return(FALSE);
    return( dout->edge_up );
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool_down: Remonte le front descendant d'un boolean                                                           */
/* Sortie : TRUE sur le boolean vient de passer à DOWN                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DO_down ( struct DLS_DO *dout )
  { if (!dout) return(FALSE);
    return( dout->edge_down );
  }
/******************************************************************************************************************************/
/* Dls_DO_to_json : Formate un bit au format JSON                                                                             */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_DO_to_json ( JsonNode *element, struct DLS_DO *bit )
  { Json_node_add_string ( element, "tech_id",  bit->tech_id );
    Json_node_add_string ( element, "acronyme", bit->acronyme );
    Json_node_add_bool   ( element, "etat",     bit->etat );
  }
/******************************************************************************************************************************/
/* Dls_all_DO_to_json: Transforme tous les bits en JSON                                                                       */
/* Entrée: target                                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_all_DO_to_json ( gpointer array, struct DLS_PLUGIN *plugin )
  { JsonArray *RootArray = array;
    GSList *liste = plugin->Dls_data_DO;
    while ( liste )
     { struct DLS_DO *bit = liste->data;
       JsonNode *element = Json_node_create();
       Dls_DO_to_json ( element, bit );
       Json_array_add_element ( RootArray, element );
       liste = g_slist_next(liste);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
