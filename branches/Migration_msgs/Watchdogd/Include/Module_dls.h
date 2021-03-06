/******************************************************************************************************************************/
/* Watchdogd/Include/Module_dls.h -> D�claration des prototypes de fonctions                                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          jeu 31 jui 2003 11:49:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Module_dls.h
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
 
 #ifndef _MODULE_DLS_H_
 #define _MODULE_DLS_H_
 #include <glib.h>

 struct DLS_TO_PLUGIN                                                 /* structure dechange de donn�es entre DLS et le plugin */
  { gint starting;                                      /* 1 si les bits internes "start" du plugins doivent etre positionn�s */
    gboolean debug;                                                 /* TRUE si le plugin doit logguer ses changements de bits */
    gboolean bit_comm_out;
    gboolean bit_defaut;
    gboolean bit_defaut_fixe;
    gboolean bit_alarme;
    gboolean bit_alarme_fixe;
    gboolean bit_dispo;
    gboolean bit_veille;
    gboolean bit_alerte;
    gboolean bit_alerte_fixe;
    gboolean bit_derangement;
    gboolean bit_derangement_fixe;
    gboolean bit_danger;
    gboolean bit_danger_fixe;
    gboolean bit_acquit;
    gboolean bit_activite_down;
  };

 extern void     Dls_print_debug ( gint id, gint *Tableau_bit, gint *Tableau_num, gfloat *Tableau_val );
 extern gboolean Dls_get_top_alerte ( void );
 extern gboolean Dls_data_get_bool      ( gchar *tech_id, gchar *acronyme, gpointer **bool_p );
 extern gboolean Dls_data_get_bool_up   ( gchar *tech_id, gchar *acronyme, gpointer **bool_p );
 extern gboolean Dls_data_get_bool_down ( gchar *tech_id, gchar *acronyme, gpointer **bool_p );
 extern void     Dls_data_set_bool      ( gchar *tech_id, gchar *acronyme, gpointer **data_p, gboolean valeur );
 extern void     Dls_data_set_MSG       ( gchar *tech_id, gchar *acronyme, gpointer **msg_p, gboolean etat );
 extern void     Dls_data_set_tempo     ( gchar *tech_id, gchar *acronyme, gpointer **tempo_p, gboolean etat,
                                          gint delai_on, gint min_on, gint max_on, gint delai_off, gint random);
 extern gboolean Dls_data_get_tempo     ( gchar *tech_id, gchar *acronyme, gpointer **tempo_p );
 extern gfloat   Dls_data_get_AI        ( gchar *tech_id, gchar *acronyme, gpointer **ai_p );
 extern int E( int num );
 extern int B( int num );
 extern int M( int num );
 extern int T( int num );
 extern int EA_ech_inf( float val, int num );
 extern int EA_ech_sup( float val, int num );
 extern int EA_ech_inf_egal( float val, int num );
 extern int EA_ech_sup_egal( float val, int num );
 extern void SEA( int num, float val_avant_ech );
 extern void SR( int num, float val );
 extern float EA_ech( int num );
 extern float R( int num );
 extern float CI( int num );
 extern void SI( int num, int etat, int rouge, int vert, int bleu, int cligno );
 extern void SB( int num, int etat );
 extern void ST( int num, int etat );
 extern void SCH( int num, int etat, int reset );
 extern void SCI( int num, int etat, int reset, int ratio );
 extern void SM( int num, int etat );
 extern void SA( int num, int etat );
 extern void MSG( int num, int etat );
                           
 extern int Heure( int heure, int minute );                                                        /* Tester l'heure actuelle */
 extern int Heure_avant( int heure, int minute );
 extern int Heure_apres( int heure, int minute );
 extern int Jour_semaine( int jour );                                     /* Sommes nous le jour de la semaine en parametre ? */
 #endif 
/*----------------------------------------------------------------------------------------------------------------------------*/
