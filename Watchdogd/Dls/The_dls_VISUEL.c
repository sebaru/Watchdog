/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_VISUEL.c             Gestion des visuels                                                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    22.03.2017 10:29:53 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_VISUEL.c
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
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, tech_id, acronyme );
       return;
     }
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->libelle,  sizeof(bit->libelle),  "%s", Json_get_string ( element, "libelle" ) );
    plugin->Dls_data_VISUEL = g_slist_prepend ( plugin->Dls_data_VISUEL, bit );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
              "%s: Create bit DLS_VISUEL '%s:%s' (%s)", __func__, bit->tech_id, bit->acronyme, bit->libelle );
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
       GSList *liste = plugin->Dls_data_VISUEL;
       while (liste)
        { struct DLS_VISUEL *bit = liste->data;
          if ( !strcasecmp ( bit->acronyme, acronyme ) && !strcasecmp( bit->tech_id, tech_id ) ) return(bit);
          liste = g_slist_next(liste);
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
                            gchar *mode, gchar *color, gboolean cligno, gchar *libelle )
  { if (!visu) return;

    if ( strcmp ( visu->mode, mode ) || strcmp( visu->color, color ) || visu->cligno != cligno )
     { if ( visu->last_change + 50 <= Partage->top )                                 /* Si pas de change depuis plus de 5 sec */
        { visu->changes = 0; }

       if ( visu->changes <= 10 )                                                          /* Si moins de 10 changes en 5 sec */
        { if ( visu->changes == 10 )                                                /* Est-ce le dernier change avant blocage */
           { g_snprintf( visu->mode,  sizeof(visu->mode),  "hors_comm" ); }
          else { g_snprintf( visu->mode,    sizeof(visu->mode), "%s", mode );/* Sinon on recopie ce qui est demandé par le plugin DLS */
                 g_snprintf( visu->color,   sizeof(visu->color), "%s", color );
                 g_snprintf( visu->libelle, sizeof(visu->libelle), "%s", libelle );
                 Convert_libelle_dynamique ( visu->tech_id, visu->libelle, sizeof(visu->libelle) );
                 visu->cligno  = cligno;
               }

          visu->last_change = Partage->top;                                                             /* Date de la photo ! */
          pthread_mutex_lock( &Partage->com_msrv.synchro );                             /* Ajout dans la liste de i a traiter */
          Partage->com_msrv.liste_visuel = g_slist_append( Partage->com_msrv.liste_visuel, visu );
          pthread_mutex_unlock( &Partage->com_msrv.synchro );
          Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                    "%s: ligne %04d: Changing DLS_VISUEL '%s:%s'-> mode %s color %s cligno %d libelle = %s", __func__,
                    (vars ? vars->num_ligne : -1), visu->tech_id, visu->acronyme, visu->mode, visu->color, visu->cligno, visu->libelle );
        }
       visu->changes++;                                                                                /* Un change de plus ! */
       Partage->audit_bit_interne_per_sec++;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/