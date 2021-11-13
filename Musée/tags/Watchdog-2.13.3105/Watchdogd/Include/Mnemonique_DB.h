/******************************************************************************************************************************/
/* Watchdogd/Include/Mnemonique.h        Déclaration structure internes des mnemoniques watchdog                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           mer 21 jan 2004 18:45:59 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemonique_DB.h
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
 
#ifndef _MNEMONIQUE_H_
 #define _MNEMONIQUE_H_

 #include <glib.h>
 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_MNEMO          "mnemos"
 #define NOM_TABLE_MNEMO_AI       "mnemos_AnalogInput"
 #define NOM_TABLE_MNEMO_CPTH     "mnemos_CptHoraire"
 #define NOM_TABLE_MNEMO_CPTIMP   "mnemos_CptImp"
 #define NOM_TABLE_MNEMO_TEMPO    "mnemos_Tempo"
 #define NOM_TABLE_MNEMO_REGISTRE "mnemos_Registre"

/***************************************************** Définitions des prototypes *********************************************/
 extern struct CMD_TYPE_MNEMO_BASE *Rechercher_mnemo_baseDB ( guint id );
 extern gboolean Recuperer_mnemo_baseDB ( struct DB **db );
 extern gboolean Recuperer_mnemo_baseDB_by_command_text ( struct DB **db_retour, gchar *commande_pure );
  
 extern struct CMD_TYPE_MNEMO_BASE *Recuperer_mnemo_baseDB_suite( struct DB **db );
 extern gint Ajouter_mnemo_fullDB ( struct CMD_TYPE_MNEMO_FULL *mnemo );
 extern gboolean Retirer_mnemo_baseDB ( struct CMD_TYPE_MNEMO_BASE *mnemo );
 extern struct CMD_TYPE_MNEMO_BASE *Rechercher_mnemo_baseDB_type_num ( struct CMD_TYPE_NUM_MNEMONIQUE *critere );
 extern struct CMD_TYPE_MNEMO_FULL *Rechercher_mnemo_fullDB ( guint id );
 extern gboolean Modifier_mnemo_fullDB ( struct CMD_TYPE_MNEMO_FULL *mnemo_full );

 extern void Charger_analogInput ( void );                                                                 /* Dans Mnemo_AI.c */
 extern struct CMD_TYPE_MNEMO_AI *Rechercher_mnemo_aiDB ( guint id );
 extern gboolean Modifier_mnemo_aiDB( struct CMD_TYPE_MNEMO_FULL *option_mnemo );


 extern void Updater_cpt_impDB ( void );                                                              /* Dans Mnemo_CPT_IMP.c */
 extern void Charger_cpt_imp ( void );
 extern struct CMD_TYPE_MNEMO_CPT_IMP *Rechercher_mnemo_cptimpDB ( guint id );
 extern gboolean Modifier_mnemo_cptimpDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full );

 extern void Updater_cpthDB ( void );                                                                   /* Dans Mnemo_CPT_H.c */
 extern void Charger_cpth ( void );
 extern struct CMD_TYPE_MNEMO_CPT_H *Rechercher_mnemo_cpthDB ( guint id );
 extern gboolean Modifier_mnemo_cpthDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full );

 extern void Charger_tempo ( void );                                                                    /* Dans Mnemo_tempo.c */
 extern struct CMD_TYPE_MNEMO_TEMPO *Rechercher_mnemo_tempoDB ( guint id );
 extern gboolean Modifier_mnemo_tempoDB( struct CMD_TYPE_MNEMO_FULL *option_mnemo );

 extern void Charger_registre ( void );                                                              /* Dans Mnemo_registre.c */
 extern struct CMD_TYPE_MNEMO_REGISTRE *Rechercher_mnemo_registreDB ( guint id );
 extern gboolean Modifier_mnemo_registreDB( struct CMD_TYPE_MNEMO_FULL *option_mnemo );

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
