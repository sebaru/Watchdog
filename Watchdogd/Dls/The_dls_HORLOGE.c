/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_HORLOGE.c        Déclaration des fonctions pour la gestion des Horloges                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    03.07.2018 21:25:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_HORLOGE.c
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
/* Dls_data_HORLOGE_create_by_array : Création d'un HORLOGE pour le plugin                                                    */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_HORLOGE_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_HORLOGE *bit = g_try_malloc0 ( sizeof(struct DLS_HORLOGE) );
    if (!bit)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, tech_id, acronyme );
       return;
     }
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    plugin->Dls_data_HORLOGE = g_slist_prepend ( plugin->Dls_data_HORLOGE, bit );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
              "%s: Create bit DLS_HORLOGE '%s:%s' (%s)", __func__, bit->tech_id, bit->acronyme, bit->libelle );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_HORLOGE: Recherche un HORLOGE dans les plugins DLS                                                         */
/* Entrée: le tech_id, l'acronyme                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_HORLOGE *Dls_data_lookup_HORLOGE ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp( plugin->tech_id, tech_id ))
        { GSList *liste = plugin->Dls_data_HORLOGE;
          while (liste)
           { struct DLS_HORLOGE *bit = liste->data;
             if ( !strcasecmp ( bit->acronyme, acronyme ) ) return(bit);
             liste = g_slist_next(liste);
           }
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_get_HORLOGE : Recupere la valeur de l'horloge en parametre                                                        */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_HORLOGE ( struct DLS_HORLOGE *bit )
  { if (!bit) return(FALSE);
    gboolean found = FALSE;
    GSList *Horloges = Partage->com_dls.HORLOGE_actives;
    while (Horloges)
     { struct DLS_HORLOGE *horloge = Horloges->data;
       if ( !strcasecmp ( horloge->acronyme, bit->acronyme ) && !strcasecmp( horloge->tech_id, bit->tech_id ) ) found = TRUE;
       Horloges = g_slist_next(Horloges);
     }
    return(found);
  }
/******************************************************************************************************************************/
/* Dls_data_set_HORLOGE : Active une horloge                                                                                  */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_clear_HORLOGE ()
  { if (Partage->com_dls.HORLOGE_actives)
     { g_slist_free_full ( Partage->com_dls.HORLOGE_actives, (GDestroyNotify) g_free );
       Partage->com_dls.HORLOGE_actives = NULL;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_activer_une_horloge: teste l'horloge en paramètre et l'active si c'est l'heure                                    */
/* Entrée: Nier                                                                                                               */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Dls_data_activer_une_horloge ( JsonArray *array, guint index_, JsonNode *element, gpointer user_data )
  { gint heure  = Json_get_int ( element, "heure" );
    gint minute = Json_get_int ( element, "minute" );
    struct tm tm;
    time_t temps;
    time(&temps);
    localtime_r( &temps, &tm );
    if (heure == tm.tm_hour && minute == tm.tm_min)   /*num_jour_semaine = tm.tm_wday;*/
     { gchar *tech_id  = Json_get_string ( element, "tech_id" );
       gchar *acronyme = Json_get_string ( element, "acronyme" );
       struct DLS_HORLOGE *bit = Dls_data_lookup_HORLOGE ( tech_id, acronyme );
       if (bit) Partage->com_dls.HORLOGE_actives = g_slist_append ( Partage->com_dls.HORLOGE_actives, bit );
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Mise à un de l'horloge %s:%s", __func__, tech_id, acronyme );
     }
  }
/******************************************************************************************************************************/
/* Dls_data_activer_horloge: Recherche toutes les horloges actives à date et les positionne dans la mémoire partagée          */
/* Entrée: rien                                                                                                               */
/* Sortie: Les horloges sont directement pilotées dans la structure DLS_DATA                                                  */
/******************************************************************************************************************************/
 void Dls_data_activer_horloge ( void )
  { Json_node_foreach_array_element ( Partage->com_dls.HORLOGE_ticks, "horloges", Dls_data_activer_une_horloge, NULL ); }
/******************************************************************************************************************************/
/* Dls_Load_horloge_ticks: Charge les horloges depuis l'API                                                                   */
/* Entrée: rien                                                                                                               */
/* Sortie: Les horloges sont directement stockées dans la structure partagée                                                  */
/******************************************************************************************************************************/
 void Dls_Load_horloge_ticks ( void )
  { JsonNode *api_result = Http_Post_to_global_API ( "/run/horloge/load", NULL );
    if (api_result && Json_get_int ( api_result, "api_status" ) == SOUP_STATUS_OK)
     { pthread_mutex_lock ( &Partage->com_dls.synchro_data );
       Json_node_unref ( Partage->com_dls.HORLOGE_ticks );
       Partage->com_dls.HORLOGE_ticks = api_result;
       pthread_mutex_unlock ( &Partage->com_dls.synchro_data );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: HORLOGE ticks loaded.", __func__ );
     }
    else Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: API Request for HORLOGE TICKS failed.", __func__ );
  }

/*----------------------------------------------------------------------------------------------------------------------------*/
