/******************************************************************************************************************************/
/* Watchdogd/TraductionDLS/Proto_traductionDLS.h     Interpretation du langage DLS                                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           ven 23 nov 2007 20:33:19 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Proto_traductionDLS.h
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

 #ifndef _PROTO_TRADDLS_H_
 #define _PROTO_TRADDLS_H_

 #include <glib.h>
 #include <stdio.h>
 #include "Erreur.h"

 struct ACTION
  { gint taille_alors;
    gchar *alors;                                                          /* Chaine pointant sur le nom du tableau (B/M/E..) */
    gint taille_sinon;
    gchar *sinon;
  };

 struct OPTION
  { gint token;
    gint token_classe;
    union { gint val_as_int;
            gchar *chaine;
            gdouble val_as_double;
            struct ALIAS *val_as_alias;
          };
  };

 struct CONDITION
  { gboolean is_bool;
    gint taille;
    gchar *chaine;
  };

 struct INSTRUCTION
  { struct CONDITION *condition;
    GList *options;
    struct ACTION *actions;
    gint line_number;
  };

 struct ALIAS
  { gchar *tech_id;
    gchar *acronyme;
    gint classe;                                                                             /* Type de tableau (E/A/B/M....) */
    GList *options;
    gint used;
  };

/****************************************************** Prototypes ************************************************************/
 extern char *New_chaine( int longueur );
 extern void Emettre( void *scan_instance, char *chaine );
 extern void Emettre_erreur_new( void *scan_instance, gchar *format, ... );
 extern void Emettre_init_alias( void );
 extern gchar *New_condition_vars( int barre, gchar *nom );
 extern gchar *New_calcul_PID ( void *scan_instance, GList *options );
 extern struct CONDITION *New_condition_entier( gint entier );
 extern struct CONDITION *New_condition_valf( gdouble valf );
 extern struct CONDITION *New_condition( gboolean is_bool, gint taille );
 extern struct INSTRUCTION *New_instruction( void *scan_instance, struct CONDITION *condition, GList *options, struct ACTION *actions );
 extern void Del_instruction( struct INSTRUCTION *instr );
 extern void Del_condition( struct CONDITION *condition );
 extern void Del_actions( struct ACTION *action );
 extern struct CONDITION *New_condition_comparaison( void *scan_instance, struct CONDITION *condition_g, gint ordre, struct CONDITION *condition_d );
 extern struct CONDITION *New_condition_alias( void *scan_instance, gint barre, struct ALIAS *alias, GList *options );
 extern gint Get_option_entier( GList *liste_options, gint token, gint defaut );
 extern struct ACTION *New_action( void );
 extern struct ACTION *New_action_msg( struct ALIAS *alias, GList *options );
 extern struct ACTION *New_action_sortie( struct ALIAS *alias, int barre, GList *options );
 extern struct ACTION *New_action_digital_output( struct ALIAS *alias, GList *options );
 extern struct ACTION *New_action_vars_mono( gchar *nom );
 extern struct ACTION *New_action_bus( struct ALIAS *alias, GList *options );
 extern struct ACTION *New_action_mono( struct ALIAS *alias );
 extern struct ACTION *New_action_visuel( struct ALIAS *alias, GList *options );
 extern struct ACTION *New_action_tempo( struct ALIAS *alias, GList *options );
 extern struct ACTION *New_action_bi( struct ALIAS *alias, gint barre );
 extern struct ACTION *New_action_cpt_h( struct ALIAS *alias, GList *options );
 extern struct ACTION *New_action_cpt_imp( struct ALIAS *alias, GList *options );
 extern struct ACTION *New_action_WATCHDOG( struct ALIAS *alias, GList *options );
 extern struct ACTION *New_action_REGISTRE( struct ALIAS *alias, GList *options );
 extern struct ACTION *New_action_AO( struct ALIAS *alias, GList *options );
 extern struct ACTION *New_action_PID ( void *scan_instance, GList *options );
 extern struct ALIAS *New_alias( gchar *tech_id, gchar *acronyme, gint classe, GList *options );
 extern struct ALIAS *New_external_alias( gchar *tech_id, gchar *acronyme, GList *options );
 extern struct ALIAS *Get_local_alias( gchar *tech_id, gchar *acronyme );
 extern struct OPTION *New_option( void );
 /*extern int Get_option_entier( GList *liste_options, gint type );*/
 extern void Liberer_options ( GList *options );
 extern int  DlsScanner_error ( void *scan_instance ,char *s );
/* Fonctions mise a disposition par Flex et Bison */
 extern int  DlsScanner_lex ( YYSTYPE *lvalp, void *scan_instance );
 extern int  DlsScanner_lex_init ( void **scan_instance );
 extern int  DlsScanner_lex_destroy ( void *scan_instance );
 extern void DlsScanner_restart (FILE * input_file, void *scan_instance );
 extern int  DlsScanner_get_lineno (void *scan_instance);
 extern void DlsScanner_set_lineno (int _line_number, void *scan_instance );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
