/**********************************************************************************************************/
/* Watchdogd/Include/Scenario_DB.h        Déclaration structure internes des scenario                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 03 aoû 2008 13:15:58 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Scenario_DB.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - sebastien
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
 #include "Cst_scenario.h"

 #define NOM_TABLE_SCENARIO    "dls_scenario"

 struct SCENARIO_DB
  { guint  id;                                                                      /* Numero du compteur */
    guint  actif;                                                                   /* Valeur du compteur */
    guint  bit_m;                                                                   /* Valeur du compteur */
    gboolean ts_jour;                                                               /* Valeur du compteur */
    gboolean ts_mois;                                                               /* Valeur du compteur */
    gboolean lundi;                                                                 /* Valeur du compteur */
    gboolean mardi;                                                                 /* Valeur du compteur */
    gboolean mercredi;                                                              /* Valeur du compteur */
    gboolean jeudi;                                                                 /* Valeur du compteur */
    gboolean vendredi;                                                              /* Valeur du compteur */
    gboolean samedi;                                                                /* Valeur du compteur */
    gboolean dimanche;                                                              /* Valeur du compteur */
    gboolean janvier;                                                               /* Valeur du compteur */
    gboolean fevrier;                                                               /* Valeur du compteur */
    gboolean mars;                                                                  /* Valeur du compteur */
    gboolean avril;                                                                 /* Valeur du compteur */
    gboolean mai;                                                                   /* Valeur du compteur */
    gboolean juin;                                                                  /* Valeur du compteur */
    gboolean juillet;                                                               /* Valeur du compteur */
    gboolean aout;                                                                  /* Valeur du compteur */
    gboolean septembre;                                                             /* Valeur du compteur */
    gboolean octobre;                                                               /* Valeur du compteur */
    gboolean novembre;                                                              /* Valeur du compteur */
    gboolean decembre;                                                              /* Valeur du compteur */
    guint  heure;                                                                   /* Valeur du compteur */
    guint  minute;                                                                  /* Valeur du compteur */
    guchar libelle[NBR_CARAC_LIBELLE_SCENARIO_UTF8+1];
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Charger_scenario ( void );
 extern gint Ajouter_scenarioDB ( struct LOG *log, struct DB *db, struct CMD_ADD_SCENARIO *sc );
 extern gboolean Modifier_scenarioDB ( struct LOG *log, struct DB *db, struct CMD_EDIT_SCENARIO *scenario );
 extern gboolean Retirer_scenarioDB ( struct LOG *log, struct DB *db, struct CMD_ID_SCENARIO *sc );
 extern struct SCENARIO_DB *Rechercher_scenarioDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Recuperer_scenarioDB ( struct LOG *log, struct DB *db );
 extern struct SCENARIO_DB *Recuperer_scenarioDB_suite( struct LOG *log, struct DB *db );
 extern gboolean Recuperer_scenarioDB_par_bitm ( struct LOG *log, struct DB *db,
                                                 struct CMD_WANT_SCENARIO_MOTIF *sce );

#endif
/*--------------------------------------------------------------------------------------------------------*/
