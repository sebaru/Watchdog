/******************************************************************************************************************************/
/* Watchdogd/Include/Module_dls.h -> Déclaration des prototypes de fonctions                                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          jeu 31 jui 2003 11:49:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Module_dls.h
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

 #ifndef _MODULE_DLS_H_
 #define _MODULE_DLS_H_
 #include <glib.h>

 struct DLS_TO_PLUGIN                                                 /* structure dechange de données entre DLS et le plugin */
  { gboolean resetted;                                  /* 1 si les bits internes "start" du plugins doivent etre positionnés */
    gboolean debug;                                                 /* TRUE si le plugin doit logguer ses changements de bits */
    gint     num_ligne;                                                         /* N° de ligne du plugin en cours d'execution */
    gpointer bit_comm;
    gpointer bit_msg_comm_ok;
    gpointer bit_msg_comm_hs;
    gpointer bit_defaut;
    gpointer bit_defaut_fixe;
    gpointer bit_alarme;
    gpointer bit_alarme_fixe;
    gpointer bit_veille;
    gpointer bit_alerte;
    gpointer bit_alerte_fixe;
    gpointer bit_derangement;
    gpointer bit_derangement_fixe;
    gpointer bit_danger;
    gpointer bit_danger_fixe;
    gpointer bit_activite_ok;
    gpointer bit_secupers_ok;
    gpointer bit_alerte_fugitive;
    gpointer bit_acquit;
  };

 extern gboolean Dls_get_top_alerte ( void );
 extern gboolean Dls_get_top_alerte_fugitive ( void );
 extern gboolean Dls_data_get_BI        ( gchar *tech_id, gchar *acronyme, gpointer *bool_p );
 extern gboolean Dls_data_get_MONO      ( gchar *tech_id, gchar *acronyme, gpointer *bool_p );
 extern gboolean Dls_data_get_bool_up   ( gchar *tech_id, gchar *acronyme, gpointer *bool_p );
 extern gboolean Dls_data_get_bool_down ( gchar *tech_id, gchar *acronyme, gpointer *bool_p );
 extern void     Dls_data_set_BI        ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *bool_p, gboolean valeur );
 extern void     Dls_data_set_MONO      ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *bool_p, gboolean valeur );
 extern gboolean Dls_data_get_DI        ( gchar *tech_id, gchar *acronyme, gpointer *di_p );
 extern gboolean Dls_data_get_DI_up     ( gchar *tech_id, gchar *acronyme, gpointer *di_p );
 extern gboolean Dls_data_get_DI_down   ( gchar *tech_id, gchar *acronyme, gpointer *di_p );
 extern void     Dls_data_set_DO        ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *dout_p, gboolean valeur );
 extern void     Dls_data_set_MSG       ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *msg_p, gboolean update, gboolean etat );
 extern void     Dls_data_set_MSG_groupe( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *msg_p, gint groupe );
 extern void     Dls_data_set_tempo     ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *tempo_p, gboolean etat,
                                          gint delai_on, gint min_on, gint max_on, gint delai_off, gint random);
 extern gdouble  Dls_data_get_AO        ( gchar *tech_id, gchar *acronyme, gpointer *ao_p );
 extern void     Dls_data_set_AO        ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *ao_p, gdouble valeur );
 extern gboolean Dls_data_get_tempo     ( gchar *tech_id, gchar *acronyme, gpointer *tempo_p );
 extern gboolean Dls_data_get_WATCHDOG ( gchar *tech_id, gchar *acronyme, gpointer *wtd_p );
 extern void     Dls_data_set_WATCHDOG ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *wtd_p, gint consigne );
 extern void Dls_data_set_bus ( gchar *tech_id, gchar *acronyme, gpointer *bus_p, gchar *target_tech_id, gchar *json_parametre );
 extern gdouble  Dls_data_get_AI        ( gchar *tech_id, gchar *acronyme, gpointer *ai_p );
 extern gboolean Dls_data_get_AI_inrange ( gchar *tech_id, gchar *acronyme, gpointer *ai_p );
 extern void Dls_data_set_CI ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *cpt_imp_p, gboolean etat, gint reset, gint ratio );
 extern gint Dls_data_get_CI ( gchar *tech_id, gchar *acronyme, gpointer *cpt_imp_p );
 extern void Dls_data_set_CH ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *cpt_h_p, gboolean etat, gint reset );
 extern gint Dls_data_get_CH ( gchar *tech_id, gchar *acronyme, gpointer *cpt_h_p );
 extern void    Dls_data_set_REGISTRE ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *r_p, gdouble valeur );
 extern gdouble Dls_data_get_REGISTRE ( gchar *tech_id, gchar *acronyme, gpointer *r_p );
 extern void Dls_data_set_VISUEL ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *visuel_p,
                                   gchar *mode, gchar *color, gboolean cligno, gchar *libelle );
 extern void Dls_PID_reset ( gchar *input_tech_id, gchar *input_acronyme, gpointer *r_input );
 extern gdouble Dls_PID ( gchar *input_tech_id, gchar *input_acronyme, gpointer *r_input,
                          gchar *consigne_tech_id, gchar *consigne_acronyme, gpointer *r_consigne,
                          gchar *kp_tech_id, gchar *kp_acronyme, gpointer *r_kp,
                          gchar *ki_tech_id, gchar *ki_acronyme, gpointer *r_ki,
                          gchar *kd_tech_id, gchar *kd_acronyme, gpointer *r_kd,
                          gchar *outputmin_tech_id, gchar *outputmin_acronyme, gpointer *r_outputmin,
                          gchar *outputmax_tech_id, gchar *outputmax_acronyme, gpointer *r_outputmax
                        );

 extern gint Dls_get_top( void );                                                                             /* donne le top */
 extern int Heure( int heure, int minute );                                                        /* Tester l'heure actuelle */
 extern int Heure_avant( int heure, int minute );
 extern int Heure_apres( int heure, int minute );
 extern int Jour_semaine( int jour );                                     /* Sommes nous le jour de la semaine en parametre ? */
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
