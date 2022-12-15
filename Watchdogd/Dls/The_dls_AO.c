/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_AO.c        Déclaration des fonctions pour la gestion des AO                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    25.03.2019 14:16:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_AO.c
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
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, tech_id, acronyme );
       return;
     }
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    bit->valeur = Json_get_double ( element, "valeur" );
    if (!strcasecmp ( tech_id, "SYS" ) ) bit->archivage = 2;            /* Si AO du plugin SYS, on archive toutes les minutes */
    plugin->Dls_data_AO = g_slist_prepend ( plugin->Dls_data_AO, bit );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
              "%s: Create bit DLS_AO '%s:%s'=%f (%s)", __func__, bit->tech_id, bit->acronyme, bit->valeur, bit->libelle );
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
       GSList *liste = plugin->Dls_data_AO;
       while (liste)
        { struct DLS_AO *bit = liste->data;
          if ( !strcasecmp ( bit->acronyme, acronyme ) && !strcasecmp( bit->tech_id, tech_id ) ) return(bit);
          liste = g_slist_next(liste);
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
 void Dls_data_set_AO ( struct DLS_TO_PLUGIN *vars, struct DLS_AO *ao, gdouble valeur )
  { if (!ao) return;
    ao->valeur = valeur;                                                            /* Archive au mieux toutes les 5 secondes */
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                 /* Ajout dans la liste de msg a traiter */
    Partage->com_msrv.Liste_AO = g_slist_append( Partage->com_msrv.Liste_AO, ao );
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
              "%s: ligne %04d: Changing DLS_AO '%s:%s'=%f", __func__,
              (vars ? vars->num_ligne : -1), ao->tech_id, ao->acronyme, ao->valeur );
  }
/******************************************************************************************************************************/
/* Dls_AO_to_json: Convertir un AO en JSON                                                                                    */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_AO_to_json ( JsonNode *element, struct DLS_AO *bit )
  { Json_node_add_string ( element, "tech_id",      bit->tech_id );
    Json_node_add_string ( element, "acronyme",     bit->acronyme );
    Json_node_add_double ( element, "valeur_brute", bit->valeur );
    Json_node_add_double ( element, "valeur_min",   bit->min );
    Json_node_add_double ( element, "valeur_max",   bit->max );
    Json_node_add_double ( element, "valeur",       bit->valeur );
    Json_node_add_int    ( element, "type",         bit->type );
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
/*----------------------------------------------------------------------------------------------------------------------------*/
