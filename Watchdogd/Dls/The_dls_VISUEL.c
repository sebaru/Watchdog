/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_VISUEL.c             Gestion des visuels                                                             */
/* Projet Abls-Habitat version 4.5       Gestion d'habitat                                                22.03.2017 10:29:53 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_VISUEL.c
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
 #include <locale.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Dls_data_VISUEL_create_by_array : Création d'un VISUEL pour le plugin                                                      */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_VISUEL_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_VISUEL *bit = g_try_malloc0 ( sizeof(struct DLS_VISUEL) );
    if (!bit)
     { Info_new( __func__, Config.log_dls, LOG_ERR, "Memory error for '%s:%s'", tech_id, acronyme );
       return;
     }
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    g_snprintf( bit->forme,    sizeof(bit->forme),    "%s", Json_get_string ( element, "forme" ) );
    plugin->Dls_data_VISUEL = g_slist_prepend ( plugin->Dls_data_VISUEL, bit );
    Info_new( __func__, Config.log_dls, LOG_INFO,
              "Create bit DLS_VISUEL '%s:%s' (%s)", bit->tech_id, bit->acronyme, bit->libelle );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_VISUEL: Recherche un VISUEL dans les plugins DLS                                                           */
/* Entrée: le tech_id, l'acronyme                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_VISUEL *Dls_data_lookup_VISUEL ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp( plugin->tech_id, tech_id ))
        { GSList *liste = plugin->Dls_data_VISUEL;
          while (liste)
           { struct DLS_VISUEL *bit = liste->data;
             if ( !strcasecmp ( bit->acronyme, acronyme ) ) return(bit);
             liste = g_slist_next(liste);
           }
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_set_visuel : Gestion du positionnement des visuels en mode dynamique                                              */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et la valeur on ou off de la tempo                            */
/******************************************************************************************************************************/
 void Dls_data_set_VISUEL ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu,
                            gchar *mode, gchar *color, gdouble valeur, gboolean cligno, gboolean noshow, gchar *libelle, gboolean disable )
  { if (!visu) return;
    if (    visu->mode    != mode
         || visu->color   != color                            /* On ne peut pas checker le libellé car il peut etre dynamique */
         || visu->cligno  != cligno
         || visu->noshow  != noshow
         || visu->disable != disable
         || visu->valeur  != valeur
       )
     { visu->mode    = mode;                                         /* Sinon on recopie ce qui est demandé par le plugin DLS */
       visu->color   = color;
       visu->valeur  = valeur;
       visu->cligno  = cligno;
       visu->noshow  = noshow;
       visu->disable = disable;
       visu->changed = TRUE;
       gchar *libelle_new = Convert_libelle_dynamique ( libelle );                        /* mise a jour du libelle dynamique */
       g_snprintf( visu->libelle, sizeof(visu->libelle), "%s", libelle_new );
       g_free(libelle_new);
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_VISUEL '%s:%s'-> mode='%s' color='%s' valeur='%f' ('%s') "
                 "cligno=%d noshow=%d libelle='%s', disable=%d",
                 (vars ? vars->num_ligne : -1), visu->tech_id, visu->acronyme,
                  visu->mode, visu->color, visu->valeur, visu->unite, visu->cligno, visu->noshow, visu->libelle, visu->disable );
       Partage->audit_bit_interne_per_sec++;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_set_VISUEL_bdage : Gestion du badge d'un visuel                                                                   */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et la valeur on ou off de la tempo                            */
/******************************************************************************************************************************/
 void Dls_data_set_VISUEL_badge ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, gchar *badge )
  { if (!visu) return;
    if ( badge != visu->badge )                                      /* Comparaison possible car les chaines sont statiques ! */
     { visu->badge = badge;
       visu->changed = TRUE;
       Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "ligne %04d: Changing DLS_VISUEL '%s:%s'-> badge='%s'",
                 (vars ? vars->num_ligne : -1), visu->tech_id, visu->acronyme, badge );
       Partage->audit_bit_interne_per_sec++;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_set_VISUEL_for_CI : Met un jour un visuel accroché a un compteur d'impulsion                                      */
/* Entrée : le dls en cours, le visuel, le registre et les parametre du visuel                                                */
/******************************************************************************************************************************/
 void Dls_data_set_VISUEL_for_CI ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, struct DLS_CI *src,
                                   gchar *mode, gchar *color, gboolean cligno, gboolean noshow, gchar *libelle, gboolean disable )
  { if (!visu) return;
    if (!src) return;

    gint valeur   = Dls_data_get_CI ( src );
    g_snprintf( visu->unite, sizeof(visu->unite), "%s", src->unite );
    Dls_data_set_VISUEL ( vars, visu, mode, color, 1.0*valeur, cligno, noshow, libelle, disable );
  }
/******************************************************************************************************************************/
/* Dls_data_set_visuel_for_registre : Met un jour un visuel accroché a un registre                                            */
/* Entrée : le dls en cours, le visuel, le registre et les parametre du visuel                                                */
/******************************************************************************************************************************/
 void Dls_data_set_VISUEL_for_REGISTRE ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, struct DLS_REGISTRE *src,
                                         gchar *mode, gchar *color, gboolean cligno, gboolean noshow, gchar *libelle, gboolean disable )
  { if (!visu) return;
    if (!src) return;

    gdouble valeur = Dls_data_get_REGISTRE ( src );
    g_snprintf( visu->unite, sizeof(visu->unite), "%s", src->unite );
    Dls_data_set_VISUEL ( vars, visu, mode, color, valeur, cligno, noshow, libelle, disable );
  }
/******************************************************************************************************************************/
/* Dls_data_set_visuel_for_watchdog : Met un jour un visuel accroché a un watchdog                                            */
/* Entrée : le dls en cours, le visuel, le watchdog et les parametre du visuel                                                */
/******************************************************************************************************************************/
 void Dls_data_set_VISUEL_for_WATCHDOG ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, struct DLS_WATCHDOG *src,
                                         gchar *mode, gchar *color, gboolean cligno, gboolean noshow, gchar *libelle, gboolean disable )
  { if (!visu) return;
    if (!src) return;

    gdouble valeur = Dls_data_get_WATCHDOG_time ( src );
    g_snprintf( visu->unite, sizeof(visu->unite), "s" );
    Dls_data_set_VISUEL ( vars, visu, mode, color, valeur, cligno, noshow, libelle, disable );
  }
/******************************************************************************************************************************/
/* Dls_data_set_visuel_for_tempo : Met un jour un visuel accroché a une temporisation                                         */
/* Entrée : le dls en cours, le visuel, la temporisation et les parametre du visuel                                           */
/******************************************************************************************************************************/
 void Dls_data_set_VISUEL_for_TEMPO ( struct DLS_TO_PLUGIN *vars, struct DLS_VISUEL *visu, struct DLS_TEMPO *src,
                                      gchar *mode, gchar *color, gboolean cligno, gboolean noshow, gchar *libelle, gboolean disable )
  { if (!visu) return;
    if (!src) return;

    gdouble valeur = Dls_data_get_TEMPO_time ( src );
    g_snprintf( visu->unite, sizeof(visu->unite), "s" );
    Dls_data_set_VISUEL ( vars, visu, mode, color, valeur, cligno, noshow, libelle, disable );
  }
/******************************************************************************************************************************/
/* Dls_VISUEL_to_json : Formate un bit au format JSON                                                                         */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_VISUEL_to_json ( JsonNode *RootNode, struct DLS_VISUEL *bit )
  { Json_node_add_string ( RootNode, "tech_id",   bit->tech_id );
    Json_node_add_string ( RootNode, "acronyme",  bit->acronyme );
    Json_node_add_string ( RootNode, "mode",      bit->mode  );
    Json_node_add_string ( RootNode, "color",     bit->color );
    Json_node_add_double ( RootNode, "valeur",    bit->valeur );
    Json_node_add_bool   ( RootNode, "cligno",    bit->cligno );
    Json_node_add_bool   ( RootNode, "noshow",    bit->noshow );
    Json_node_add_bool   ( RootNode, "disable",   bit->disable );
    Json_node_add_string ( RootNode, "libelle",   bit->libelle );
    Json_node_add_string ( RootNode, "unite",     bit->unite );
    Json_node_add_string ( RootNode, "badge",     bit->badge );
  }
/******************************************************************************************************************************/
/* Dls_data_VISUEL_apply: Met à jour les visuels du plugin                                                                    */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_VISUEL_apply ( struct DLS_PLUGIN *plugin )
  { if (!plugin) return;

    GSList *liste = plugin->Dls_data_VISUEL;
    while ( liste )
     { struct DLS_VISUEL *visu = liste->data;
       if (visu->changed && (Partage->top >= visu->next_send))
        { pthread_rwlock_wrlock( &Partage->Liste_visuel_synchro );                      /* Ajout dans la liste de i a traiter */
          Partage->Liste_visuel = g_slist_append( Partage->Liste_visuel, visu );
          pthread_rwlock_unlock( &Partage->Liste_visuel_synchro );
          visu->changed = FALSE;
          visu->next_send = Partage->top + 10;                                                  /* Next update dans 1 seconde */
        }
       liste = g_slist_next(liste);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
