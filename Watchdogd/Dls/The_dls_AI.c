/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_AI.c  Gestion des Analog Input                                                                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    30.01.2022 14:07:24 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_AI.c
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

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Dls_data_AI_lookup : Recupere la structure AI selon tech_id/acronyme                                                       */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 static struct DLS_AI *Dls_data_AI_lookup ( gchar *tech_id, gchar *acronyme, gpointer *ai_p )
  { struct DLS_AI *ai;
    if (ai_p && *ai_p)                                                               /* Si pointeur d'acceleration disponible */
     { ai = (struct DLS_AI *)*ai_p;
       return( ai );
     }
    if (!tech_id || !acronyme) return(NULL);

    GSList *liste = Partage->Dls_data_AI;
    while (liste)                                                                               /* A la recherche du message. */
     { ai = (struct DLS_AI *)liste->data;
       if ( !strcasecmp( ai->tech_id, tech_id ) && !strcasecmp( ai->acronyme, acronyme ) )
        { if (ai_p) *ai_p = (gpointer)ai;                                           /* Sauvegarde pour acceleration si besoin */
          return(ai);
        }
       liste = g_slist_next(liste);
     }

    ai = g_try_malloc0 ( sizeof(struct DLS_AI) );
    if (!ai)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, tech_id, acronyme );
       return(NULL);
     }
    g_snprintf( ai->acronyme, sizeof(ai->acronyme), "%s", acronyme );
    g_snprintf( ai->tech_id,  sizeof(ai->tech_id),  "%s", tech_id );
    pthread_mutex_lock( &Partage->com_dls.synchro_data );
    Partage->Dls_data_AI = g_slist_prepend ( Partage->Dls_data_AI, ai );
    pthread_mutex_unlock( &Partage->com_dls.synchro_data );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: adding AI '%s:%s'", __func__, tech_id, acronyme );
    return(ai);
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gdouble Dls_data_get_AI ( gchar *tech_id, gchar *acronyme, gpointer *ai_p )
  { struct DLS_AI *ai = Dls_data_AI_lookup ( tech_id, acronyme, ai_p );
    if (!ai) return(0.0);
    return( ai->valeur );
  }
/******************************************************************************************************************************/
/* Met à jour l'entrée analogique num à partir de sa valeur avant mise a l'echelle                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_AI ( gchar *tech_id, gchar *acronyme, gpointer *ai_p, gdouble valeur, gboolean in_range )
  { struct DLS_AI *ai;
    ai = Dls_data_AI_lookup ( tech_id, acronyme, ai_p );
    if (!ai) return;
    if (ai_p) *ai_p = (gpointer)ai;                                                 /* Sauvegarde pour acceleration si besoin */

    ai->valeur  = valeur;
    ai->inrange = in_range;
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_AI_inrange ( gchar *tech_id, gchar *acronyme, gpointer *ai_p )
  { struct DLS_AI *ai = Dls_data_AI_lookup ( tech_id, acronyme, ai_p );
    if (!ai) return(FALSE);
    return( ai->inrange );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
