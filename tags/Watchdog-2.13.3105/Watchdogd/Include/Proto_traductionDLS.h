/**********************************************************************************************************/
/* Watchdogd/Include/Proto_traductionDLS.h     Interpretation du langage DLS                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                       ven 23 nov 2007 20:33:19 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Proto_traductionDLS.h
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
 
 #ifndef _PROTO_TRADDLS_H_
 #define _PROTO_TRADDLS_H_
 
 #include <glib.h>
 #include "Erreur.h"

 enum { TRAD_DLS_OK,                                               /* Retour de la fonction Traduire DLS. */
        TRAD_DLS_WARNING,
        TRAD_DLS_ERROR,
        TRAD_DLS_ERROR_FILE
      };

 struct ACTION
  { gchar *alors;                                      /* Chaine pointant sur le nom du tableau (B/M/E..) */
    gchar *sinon;
  };
 

 struct OPTION
  { gint type;
    union { int entier;
          };
  };

 struct COMPARATEUR
  { gint type;
    union { gint val;
            gfloat valf;
          };
  };

 struct ALIAS
  { gchar *nom;
    gint bit;                                                            /* Type de tableau (E/A/B/M....) */
    gint num;                                                              /* Numero du bit interne ciblé */
    int barre;                                                           /* Represente la negation ou pas */
    GList *options;
    gint used;
  };

/*********************************************** Prototypes ***********************************************/
 extern gint Traduire_DLS( gboolean new, gint id );                                      /* Dans Interp.c */
 extern char *New_chaine( int longueur );
 extern void Emettre( char *chaine );
 extern void Emettre_erreur( char *chaine );
 extern void Emettre_init_alias( void );
 extern struct COMPARATEUR *New_comparateur( void );
 extern struct ACTION *New_action( void );
 extern struct ACTION *New_action_msg( int num );
 extern struct ACTION *New_action_sortie( int num, int barre );
 extern struct ACTION *New_action_mono( int num );
 extern struct ACTION *New_action_icone( int num, GList *options );
 extern struct ACTION *New_action_tempo( int num, GList *options );
 extern struct ACTION *New_action_bi( int num, int barre );
 extern struct ACTION *New_action_cpt_h( int num, GList *options );
 extern struct ACTION *New_action_cpt_imp( int num, GList *options );
 extern gboolean New_alias( char *nom, int bit, int num, int barre, GList *options );
 extern struct ALIAS *Get_alias_par_nom( char *nom );
 extern struct OPTION *New_option( void );
 extern int Get_option_entier( GList *liste_options, gint type );
 extern void Liberer_options ( GList *options );

 extern int Dls_error ( char *s );
 extern void Dls_restart ( FILE *input_file );
 #endif
/*--------------------------------------------------------------------------------------------------------*/
