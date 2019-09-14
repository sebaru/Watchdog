/******************************************************************************************************************************/
/* Watchdogd/Include/Scenario_DB.h        Déclaration structure internes des scenario                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          dim 03 aoû 2008 13:15:58 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Scenario_DB.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 
#ifndef _SCENARIO_H_
 #define _SCENARIO_H_

 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_SCENARIO_TICK "scenario_ticks"

 struct SCENARIO_TICK
  { gint id;
    gint num;
    gint minute;
    gint heure;
    gint jour;
    gint date;
    gint mois;
    gint mnemo_id;
    /* Champs chargés depuis un innerjoin */
    gint mnemo_num;
    gchar mnemo_libelle[120];
  };

/************************************************ Définitions des prototypes **************************************************/
 extern gboolean Set_scenario_detailsDB ( gint num, GSList *Liste );
 extern gboolean Recuperer_scenario_detailsDB ( struct DB **db_retour, gint num );
 extern struct SCENARIO_TICK *Recuperer_scenario_detailsDB_suite( struct DB **db_orig );
 extern void Check_scenario_tick_thread ( void );
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
