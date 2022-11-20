/******************************************************************************************************************************/
/* Watchdogd/Include/Mnemonique.h        Déclaration structure internes des mnemoniques watchdog                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           mer 21 jan 2004 18:45:59 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemonique_DB.h
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

#ifndef _MNEMONIQUE_H_
 #define _MNEMONIQUE_H_

 #include <glib.h>
 #include "Db.h"

 #define NOM_TABLE_MNEMO          "mnemos"
 #define NOM_TABLE_MNEMO_CPTH     "mnemos_CptHoraire"
 #define NOM_TABLE_MNEMO_CPTIMP   "mnemos_CptImp"
 #define NOM_TABLE_MNEMO_REGISTRE "mnemos_Registre"

/***************************************************** Définitions des prototypes *********************************************/
 extern gint Rechercher_DICO_type ( gchar *tech_id, gchar *acronyme );                                   /* Dans Mnemonique.c */
 extern JsonNode *Rechercher_DICO ( gchar *tech_id, gchar *acronyme );

 extern void Dls_AI_to_json ( JsonNode *element, struct DLS_AI *bit );                                    /* Dans Mnemos_AI.c */
 extern void Dls_all_AI_to_json ( JsonNode *target );
 extern JsonNode *Mnemo_create_thread_AI ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage );
 extern void Charger_confDB_AI ( gchar *tech_id, gchar *acronyme );

 extern void Activer_horlogeDB ( void );                                                            /* Dans Mnemos_Horloges.c */
 extern gboolean Mnemo_auto_create_HORLOGE ( gint deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern gboolean Horloge_del_all_ticks ( gchar *tech_id, gchar *acronyme );
 extern gboolean Horloge_add_tick ( gchar *tech_id, gchar *acronyme, gint heure, gint minute );

 extern void Charger_confDB_CI ( struct DLS_CI *cpt_imp );                                                 /* Dans Mnemo_CI.c */
 extern gboolean Mnemo_auto_create_CI ( gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src, gdouble multi );
 extern void Dls_CI_to_json ( JsonNode *element, struct DLS_CI *bit );
 extern void Dls_all_CI_to_json ( JsonNode *target );

 extern void Charger_confDB_CH ( struct DLS_CH *cpt_h );                                                   /* Dans Mnemo_CH.c */
 extern gboolean Mnemo_auto_create_CH ( gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern void Dls_CH_to_json ( JsonNode *element, struct DLS_CH *bit );
 extern void Dls_all_CH_to_json ( JsonNode *target );

 extern gboolean Mnemo_auto_create_TEMPO ( gchar *tech_id, gchar *acronyme, gchar *libelle_src );       /* Dans Mnemo_tempo.c */
 extern void Dls_TEMPO_to_json ( JsonNode *element, struct DLS_TEMPO *bit );

 extern void Charger_confDB_Registre ( gchar *tech_id );                                             /* Dans Mnemo_registre.c */
 extern gboolean Mnemo_auto_create_REGISTRE ( gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src );
 extern gboolean Recuperer_mnemos_R_by_map_question_vocale ( struct DB **db_retour, gchar *map_snips );
 extern gboolean Recuperer_mnemos_R_suite( struct DB **db_orig );
 extern void Dls_REGISTRE_to_json ( JsonNode *element, struct DLS_REGISTRE *bit );
 extern void Dls_all_REGISTRE_to_json ( JsonNode *target );

 extern JsonNode *Mnemo_create_thread_DI ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle );/* Dans mnemos_DI.c */
 extern void Dls_DI_to_json ( JsonNode *element, struct DLS_DI *bit );
 extern void Dls_all_DI_to_json ( JsonNode *target );

 extern gboolean Mnemo_auto_create_DO ( gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle );
 extern void Mnemo_create_thread_DO ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle );    /* Dans mnemos_DO.c */
 extern void Dls_DO_to_json ( JsonNode *element, struct DLS_DO *bit );
 extern void Dls_all_DO_to_json ( JsonNode *target );

 extern void Charger_confDB_MONO ( void );                                                              /* Dans mnemos_MONO.c */
 extern gboolean Mnemo_auto_create_MONO ( gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern void Dls_MONO_to_json ( JsonNode *element, struct DLS_MONO *bit );
 extern void Dls_all_MONO_to_json ( JsonNode *target );

 extern void Charger_confDB_BI ( void );                                                                  /* Dans mnemos_BI.c */
 extern gboolean Mnemo_auto_create_BI ( gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gint groupe );
 extern void Dls_BI_to_json ( JsonNode *element, struct DLS_BI *bit );
 extern void Dls_all_BI_to_json ( JsonNode *target );

 extern void Dls_AO_to_json ( JsonNode *element, struct DLS_AO *bit );                                    /* Dans mnemos_AO.c */
 extern void Dls_all_AO_to_json ( JsonNode *target );
 extern JsonNode *Mnemo_create_thread_AI ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage );

 extern gboolean Mnemo_auto_create_WATCHDOG ( gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src );
 extern void Dls_WATCHDOG_to_json ( JsonNode *element, struct DLS_WATCHDOG *bit );

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
