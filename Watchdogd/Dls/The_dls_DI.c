/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls_DI.c  Gestion des Analog Input                                                                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    30.01.2022 14:07:24 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_DI.c
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
 * GNU General Public License for more detdils.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Dls_data_DI_lookup : Recupere la structure DI selon tech_id/acronyme                                                       */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 static struct DLS_DI *Dls_data_DI_lookup ( gchar *tech_id, gchar *acronyme, gpointer *di_p )
  { struct DLS_DI *di = NULL;
    if (di_p && *di_p)                                                               /* Si pointeur d'acceleration disponible */
     { di = (struct DLS_DI *)*di_p;
       return( di );
     }
    if (!tech_id || !acronyme) return(NULL);

    pthread_mutex_lock( &Partage->com_dls.synchro_data );
    GSList *liste = Partage->Dls_data_DI;
    while (liste)                                                                               /* A la recherche du message. */
     { di = (struct DLS_DI *)liste->data;
       if ( !strcasecmp( di->tech_id, tech_id ) && !strcasecmp( di->acronyme, acronyme ) )
        { if (di_p) *di_p = (gpointer)di;                                           /* Sauvegarde pour acceleration si besoin */
          break;
        }
       liste = g_slist_next(liste);
     }

    if (liste)
     { pthread_mutex_unlock( &Partage->com_dls.synchro_data );
       return(di);
     }

    di = g_try_malloc0 ( sizeof(struct DLS_DI) );
    if (di)
     { g_snprintf( di->acronyme, sizeof(di->acronyme), "%s", acronyme );
       g_snprintf( di->tech_id,  sizeof(di->tech_id),  "%s", tech_id );
       Partage->Dls_data_DI = g_slist_prepend ( Partage->Dls_data_DI, di );
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: adding DI '%s:%s'", __func__, tech_id, acronyme );
     }
    else Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, tech_id, acronyme );
    pthread_mutex_unlock( &Partage->com_dls.synchro_data );
    return(di);
  }
/******************************************************************************************************************************/
/* Dls_data_get_DI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DI ( gchar *tech_id, gchar *acronyme, gpointer *di_p )
  { struct DLS_DI *di = Dls_data_DI_lookup ( tech_id, acronyme, di_p );
    if (!di) return(FALSE);
    return( di->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_get_DI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DI_up ( gchar *tech_id, gchar *acronyme, gpointer *di_p )
  { struct DLS_DI *di = Dls_data_DI_lookup ( tech_id, acronyme, di_p );
    if (!di) return(FALSE);
    return( di->edge_up );
  }
/******************************************************************************************************************************/
/* Dls_data_get_DI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DI_down ( gchar *tech_id, gchar *acronyme, gpointer *di_p )
  { struct DLS_DI *di = Dls_data_DI_lookup ( tech_id, acronyme, di_p );
    if (!di) return(FALSE);
    return( di->edge_down );
  }
/******************************************************************************************************************************/
/* Met à jour l'entrée analogique num à partir de sa valeur avant mise a l'echelle                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_DI ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *di_p, gboolean valeur )
  { struct DLS_DI *di;
    di = Dls_data_DI_lookup ( tech_id, acronyme, di_p );
    if (!di) return;
    if (di_p) *di_p = (gpointer)di;                                                 /* Sauvegarde pour acceleration si besoin */

    if (di->etat != valeur)
     { Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG, "%s: Changing DLS_DI '%s:%s'=%d up %d down %d",
                 __func__, di->tech_id, di->acronyme, valeur, di->edge_up, di->edge_down );
       if (valeur == TRUE) Partage->com_dls.Set_Dls_DI_Edge_up   = g_slist_prepend ( Partage->com_dls.Set_Dls_DI_Edge_up, di );
                      else Partage->com_dls.Set_Dls_DI_Edge_down = g_slist_prepend ( Partage->com_dls.Set_Dls_DI_Edge_down, di );
       Partage->audit_bit_interne_per_sec++;
     }
    di->etat = valeur;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
