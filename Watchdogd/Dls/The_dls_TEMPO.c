/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_TEMPO.c              Déclaration des fonctions pour la gestion des tempo.c                           */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                     sam. 09 mars 2013 11:47:18 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_TEMPO.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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
/* Dls_data_TEMPO_create_by_array : Création d'un TEMPO pour le plugin                                                        */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_TEMPO_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_TEMPO *bit = g_try_malloc0 ( sizeof(struct DLS_TEMPO) );
    if (!bit)
     { Info_new( __func__, Config.log_dls, LOG_ERR, "Memory error for '%s:%s'", tech_id, acronyme );
       return;
     }
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    plugin->Dls_data_TEMPO = g_slist_prepend ( plugin->Dls_data_TEMPO, bit );
    Info_new( __func__, Config.log_dls, LOG_INFO,
              "Create bit DLS_TEMPO '%s:%s' (%s)", bit->tech_id, bit->acronyme, bit->libelle );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_TEMPO: Recherche un TEMPO dans les plugins DLS                                                             */
/* Entrée: le tech_id, l'acronyme                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_TEMPO *Dls_data_lookup_TEMPO ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp( plugin->tech_id, tech_id ))
        { GSList *liste = plugin->Dls_data_TEMPO;
          while (liste)
           { struct DLS_TEMPO *bit = liste->data;
             if ( !strcasecmp ( bit->acronyme, acronyme ) ) return(bit);
             liste = g_slist_next(liste);
           }
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* STR_local: Positionnement d'une Tempo retard DLS                                                                           */
/* Entrée: la structure tempo et son etat                                                                                     */
/* Sortie: Neant                                                                                                              */
/******************************************************************************************************************************/
 static void ST_local( struct DLS_TO_PLUGIN *vars, struct DLS_TEMPO *tempo, int etat )
  { static guint seed;
    if (tempo->status == DLS_TEMPO_NOT_COUNTING && etat == 1)
     { tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_ON;
       if (tempo->random)
        { gfloat ratio;
          ratio = (gfloat)rand_r(&seed)/RAND_MAX;
          tempo->delai_on  = (gint)(tempo->random * ratio);
          if (tempo->delai_on<10) tempo->delai_on = 10;
          tempo->min_on    = 0;
          tempo->max_on    = 0;
          tempo->delai_off = 0;
        }
       tempo->date_on = Partage->top + tempo->delai_on;
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_TEMPO '%s:%s'=%d, WAIT_FOR_DELAI_ON",
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_ON && etat == 0)
     { tempo->status = DLS_TEMPO_NOT_COUNTING;
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_TEMPO '%s:%s'=%d, NOT_COUNTING",
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_ON && tempo->date_on <= Partage->top)
     { tempo->status = DLS_TEMPO_WAIT_FOR_MIN_ON;
       tempo->state = TRUE;
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_MIN_ON",
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MIN_ON && etat == 0 &&
        Partage->top < tempo->date_on + tempo->min_on )
     { if (Partage->top+tempo->delai_off <= tempo->date_on + tempo->min_on)
            { tempo->date_off = tempo->date_on+tempo->min_on; }
       else { tempo->date_off = Partage->top+tempo->delai_off; }
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_DELAI_OFF",
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MIN_ON && etat == 0 &&
        tempo->date_on + tempo->min_on <= Partage->top )
     { tempo->date_off = Partage->top+tempo->delai_off;
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_DELAI_OFF",
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MIN_ON && etat == 1 &&
        tempo->date_on + tempo->min_on <= Partage->top )
     { tempo->status = DLS_TEMPO_WAIT_FOR_MAX_ON;
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_DELAI_MAX_ON",
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MAX_ON && etat == 0 )
     { if (tempo->max_on)
            { if (Partage->top+tempo->delai_off < tempo->date_on+tempo->max_on)
                   { tempo->date_off = Partage->top + tempo->delai_off; }
              else { tempo->date_off = tempo->date_on+tempo->max_on; }
            }
       else { tempo->date_off = Partage->top+tempo->delai_off; }
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_DELAI_OFF",
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MAX_ON && etat == 1 && tempo->max_on &&
        tempo->date_on + tempo->max_on <= Partage->top )
     { tempo->date_off = tempo->date_on+tempo->max_on;
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_DELAI_OFF",
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_OFF && tempo->date_off <= Partage->top )
     { tempo->date_on = tempo->date_off = 0;
       tempo->status = DLS_TEMPO_WAIT_FOR_COND_OFF;
       tempo->state = FALSE;
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_COND_OFF",
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_COND_OFF && etat == 0 )
     { tempo->status = DLS_TEMPO_NOT_COUNTING;
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_TEMPO '%s:%s'=%d NOT_COUNTING",
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }
  }
/******************************************************************************************************************************/
/* Dls_data_set_TEMPO : Gestion du positionnement des tempos DLS en mode dynamique                                            */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et la valeur on ou off de la tempo                            */
/******************************************************************************************************************************/
 void Dls_data_set_TEMPO ( struct DLS_TO_PLUGIN *vars, struct DLS_TEMPO *tempo, gboolean etat,
                           gint delai_on, gint min_on, gint max_on, gint delai_off, gint random)
  { if (!tempo) return;
    if (tempo->init == FALSE)
     { tempo->delai_on  = delai_on;
       tempo->min_on    = min_on;
       tempo->max_on    = max_on;
       tempo->delai_off = delai_off;
       tempo->random    = random;
       tempo->init      = TRUE;
       Info_new( __func__, Config.log_dls, LOG_DEBUG, "%s: Initializing TEMPO '%s:%s'",
                 __func__, tempo->tech_id, tempo->acronyme );
     }
    ST_local ( vars, tempo, etat );                                                               /* Recopie dans la variable */
  }
/******************************************************************************************************************************/
/* Dls_data_get_TEMPO : Gestion du positionnement des tempos DLS en mode dynamique                                            */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_TEMPO ( struct DLS_TEMPO *tempo )
  { if (!tempo) return(FALSE);
    return( tempo->state );
  }
/******************************************************************************************************************************/
/* Dls_data_get_TEMPO_time: Renvoie le temps de decompte restant de la tempo                                                  */
/* Sortie : le temps, ou 0 si pas trouvé                                                                                      */
/******************************************************************************************************************************/
 gint Dls_data_get_TEMPO_time ( struct DLS_TEMPO *bit )
  { if (!bit) return(0);
    return( ((bit->date_on - Partage->top) > 0 ? (bit->date_on - Partage->top) : 0) );
  }
/******************************************************************************************************************************/
/* Dls_TEMPO_to_json : Formate un bit au format JSON                                                                          */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_TEMPO_to_json ( JsonNode *element, struct DLS_TEMPO *bit )
  { Json_node_add_string ( element, "tech_id",  bit->tech_id );
    Json_node_add_string ( element, "acronyme", bit->acronyme );
    Json_node_add_bool ( element, "etat",   bit->state );
    Json_node_add_int  ( element, "daa", bit->delai_on );
    Json_node_add_int  ( element, "dma", bit->min_on );
    Json_node_add_int  ( element, "dMa", bit->max_on );
    Json_node_add_int  ( element, "dad", bit->delai_off );
    Json_node_add_int  ( element, "date_on", bit->date_on );
    Json_node_add_int  ( element, "date_off", bit->date_off );
    gchar *status;
    switch ( bit->status )
     { default:
       case DLS_TEMPO_NOT_COUNTING:       status = "NOT COUNTING";       break;
       case DLS_TEMPO_WAIT_FOR_DELAI_ON:  status = "WAIT_FOR_DELAI_ON";  break;
       case DLS_TEMPO_WAIT_FOR_MIN_ON:    status = "WAIT_FOR_MIN_ON";    break;
       case DLS_TEMPO_WAIT_FOR_DELAI_OFF: status = "WAIT_FOR_DELAI_OFF"; break;
       case DLS_TEMPO_WAIT_FOR_MAX_ON:    status = "WAIT_FOR_MAX_ON";    break;
       case DLS_TEMPO_WAIT_FOR_COND_OFF:  status = "WAIT_FOR_COND_OFF";  break;
     }
    Json_node_add_string ( element, "status", status );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
