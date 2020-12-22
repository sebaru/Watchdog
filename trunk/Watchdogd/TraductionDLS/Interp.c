/******************************************************************************************************************************/
/* Watchdogd/TraductionDLS/Interp.c          Interpretation du langage DLS                                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          dim 05 avr 2009 12:47:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Interp.c
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

 #include <glib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/prctl.h>
 #include <fcntl.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <string.h>
 #include <locale.h>

 #include "watchdogd.h"
 #include "lignes.h"

 static GSList *Alias=NULL;                                                  /* Liste des alias identifiés dans le source DLS */
 static gchar *Buffer=NULL;
 static gint Buffer_used=0, Buffer_taille=0;
 static int Id_log;                                                                     /* Pour la creation du fichier de log */
 static int nbr_erreur;
 static struct CMD_TYPE_PLUGIN_DLS Dls_plugin;

/******************************************************************************************************************************/
/* New_chaine: Alloue une certaine quantité de mémoire pour utiliser des chaines de caractères                                */
/* Entrées: la longueur souhaitée                                                                                             */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 char *New_chaine( int longueur )
  { char *chaine;
    chaine = g_try_malloc0( longueur );
    if (!chaine) { return(NULL); }
    return(chaine);
  }
/******************************************************************************************************************************/
/* Emettre: Met a jour le fichier temporaire en code intermédiaire                                                            */
/* Entrées: la ligne d'instruction à mettre                                                                                   */
/* Sortie: void                                                                                                               */
/******************************************************************************************************************************/
 void Emettre( char *chaine )
  { int taille;
    taille = strlen(chaine);
    if ( Buffer_used + taille > Buffer_taille)
     { gchar *new_Buffer;
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG,
                "%s: buffer too small, trying to expand it to %d)", __func__, Buffer_taille + taille );
       new_Buffer = g_try_realloc( Buffer, Buffer_taille + taille );
       if (!new_Buffer)
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Fail to expand buffer. skipping", __func__ );
          return;
        }
       Buffer = new_Buffer;
       Buffer_taille = Buffer_taille + taille;
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Buffer expanded to %d bytes", __func__, Buffer_taille );
     }
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: ligne %d : %s", __func__, DlsScanner_get_lineno(), chaine );
    memcpy ( Buffer + Buffer_used, chaine, taille );                                             /* Recopie du bout de buffer */
    Buffer_used += taille;
  }
/******************************************************************************************************************************/
/* DlsScanner_error: Appellé par le scanner en cas d'erreur de syntaxe (et non une erreur de grammaire !)                     */
/* Entrée : la chaine source de l'erreur de syntaxe                                                                           */
/* Sortie : appel de la fonction Emettre_erreur_new en backend                                                                */
/******************************************************************************************************************************/
 int DlsScanner_error ( char *s )
  { Emettre_erreur_new( "Ligne %d: %s", DlsScanner_get_lineno(), s );
    return(0);
  }
/******************************************************************************************************************************/
/* Emettre_erreur_new: collecte des erreurs de traduction D.L.S                                                               */
/* Entrée: le numéro de ligne, le format et les paramètres associés                                                           */
/******************************************************************************************************************************/
 void Emettre_erreur_new( gchar *format, ... )
  { static gchar *too_many="Too many events. Limiting output...\n";
    gchar log[256], chaine[256];
    va_list ap;

    if (nbr_erreur<15)
     { va_start( ap, format );
       g_vsnprintf( chaine, sizeof(chaine), format, ap );
       va_end ( ap );
       g_snprintf( log, sizeof(log), "Ligne %d: %s\n", DlsScanner_get_lineno(), chaine );
       write( Id_log, log, strlen(log) );

       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Ligne %d : %s", __func__, DlsScanner_get_lineno(), chaine );
     }
    else if (nbr_erreur==15)
     { write( Id_log, too_many, strlen(too_many)+1 ); }
    nbr_erreur++;
  }
/******************************************************************************************************************************/
/* New_option: Alloue une certaine quantité de mémoire pour les options                                                       */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct COMPARATEUR *New_comparateur( void )
  { struct COMPARATEUR *comparateur;
    comparateur=(struct COMPARATEUR *)g_try_malloc0( sizeof(struct COMPARATEUR) );
    return(comparateur);
  }
/******************************************************************************************************************************/
/* New_option: Alloue une certaine quantité de mémoire pour les options                                                       */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct OPTION *New_option( void )
  { struct OPTION *option;
    option=(struct OPTION *)g_try_malloc0( sizeof(struct OPTION) );
    return(option);
  }
/******************************************************************************************************************************/
/* Get_option_entier: Cherche une option et renvoie sa valeur                                                                 */
/* Entrées: la liste des options, le type a rechercher                                                                        */
/* Sortie: -1 si pas trouvé                                                                                                   */
/******************************************************************************************************************************/
 static int Get_option_entier( GList *liste_options, gint type )
  { struct OPTION *option;
    GList *liste;
    liste = liste_options;
    while (liste)
     { option=(struct OPTION *)liste->data;
       if ( option->type == type )
        { return (option->entier); }
       liste = liste->next;
     }
    return(-1);
  }
/******************************************************************************************************************************/
/* Get_option_chaine: Cherche une option de type chaine et renvoie sa valeur                                                  */
/* Entrées: la liste des options, le type a rechercher                                                                        */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 static gchar *Get_option_chaine( GList *liste_options, gint type )
  { struct OPTION *option;
    GList *liste;
    liste = liste_options;
    while (liste)
     { option=(struct OPTION *)liste->data;
       if ( option->type == type )
        { return (option->chaine); }
       liste = liste->next;
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* New_condition_bi: Prepare la chaine de caractere associée àla condition, en respectant les options                        */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_bi( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    taille = 256;
    result = New_chaine( taille ); /* 10 caractères max */
    if (Get_option_entier( options, T_EDGE_UP) == 1)
     { g_snprintf( result, taille, "%sDls_data_get_bool_up ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else if (Get_option_entier( options, T_EDGE_DOWN) == 1)
     { g_snprintf( result, taille, "%sDls_data_get_bool_down ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( result, taille, "%sDls_data_get_bool ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    return(result);
  }
/******************************************************************************************************************************/
/* New_condition_entree: Prepare la chaine de caractere associée à la condition, en respectant les options                    */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_entree( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    taille = 256;
    result = New_chaine( taille ); /* 10 caractères max */
    if (Get_option_entier( options, T_EDGE_UP) == 1)
     { g_snprintf( result, taille, "%sDls_data_get_DI_up ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else if (Get_option_entier( options, T_EDGE_DOWN) == 1)
     { g_snprintf( result, taille, "%sDls_data_get_DI_down ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( result, taille, "%sDls_data_get_DI ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    return(result);
  }
/******************************************************************************************************************************/
/* New_condition_entree_ana: Prepare la chaine de caractere associée à la condition, en respectant les options                */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_entree_ana( int barre, struct ALIAS *alias, GList *options, struct COMPARATEUR *comparateur )
  { gint taille, in_range = Get_option_entier ( options, T_IN_RANGE );
    gchar *result;

    if (in_range==1)
     { taille = 256;
       result = New_chaine( taille ); /* 10 caractères max */
       if (barre) g_snprintf( result, taille, "!Dls_data_get_AI_inrange(\"%s\",\"%s\",&_%s_%s)",
                              alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme );
             else g_snprintf( result, taille, "Dls_data_get_AI_inrange(\"%s\",\"%s\",&_%s_%s)",
                              alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme );
       return(result);
     }
    if (!comparateur)                                                    /* Vérification des bits obligatoirement comparables */
     { Emettre_erreur_new( "Ligne %d: '%s' ne peut s'utiliser qu'avec une comparaison", DlsScanner_get_lineno(), alias->acronyme );
       result=New_chaine(2);
       g_snprintf( result, 2, "0" );
       return(result);
     }

    if (comparateur->type == T_EGAL)
     { Emettre_erreur_new( "Ligne %d: '%s' ne peut s'utiliser avec le comparateur '='", DlsScanner_get_lineno(), alias->acronyme );
       result=New_chaine(2);
       g_snprintf( result, 2, "0" );
       return(result);
     }

    taille = 512;
    result = New_chaine( taille ); /* 10 caractères max */
    setlocale(LC_ALL, "C");
    switch(comparateur->type)
     { case INF:         g_snprintf( result, taille, "(Dls_data_get_AI_inrange(\"%s\",\"%s\",&_%s_%s) &&"
                                                     " (Dls_data_get_AI(\"%s\",\"%s\",&_%s_%s)<%f))",
                                     alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme,
                                     alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, comparateur->valf );
                         break;
       case SUP:         g_snprintf( result, taille, "(Dls_data_get_AI_inrange(\"%s\",\"%s\",&_%s_%s) &&"
                                                     " (Dls_data_get_AI(\"%s\",\"%s\",&_%s_%s)>%f))",
                                     alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme,
                                     alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, comparateur->valf );
                         break;
       case INF_OU_EGAL: g_snprintf( result, taille, "(Dls_data_get_AI_inrange(\"%s\",\"%s\",&_%s_%s) &&"
                                                     " (Dls_data_get_AI(\"%s\",\"%s\",&_%s_%s)<=%f))",
                                     alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme,
                                     alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, comparateur->valf );
                         break;
       case SUP_OU_EGAL: g_snprintf( result, taille, "(Dls_data_get_AI_inrange(\"%s\",\"%s\",&_%s_%s) &&"
                                                     " (Dls_data_get_AI(\"%s\",\"%s\",&_%s_%s)>=%f))",
                                     alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme,
                                     alias->tech_id, alias->acronyme,alias->tech_id, alias->acronyme, comparateur->valf );
                         break;
     }
    return(result);
  }
/******************************************************************************************************************************/
/* New_condition_entree_ana: Prepare la chaine de caractere associée à la condition, en respectant les options                */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_sortie_ana( int barre, struct ALIAS *alias, GList *options, struct COMPARATEUR *comparateur )
  { gchar *result;
    gint taille;

    if (!comparateur)                                                    /* Vérification des bits obligatoirement comparables */
     { Emettre_erreur_new( "Ligne %d: '%s' ne peut s'utiliser qu'avec une comparaison", DlsScanner_get_lineno(), alias->acronyme );
       result=New_chaine(2);
       g_snprintf( result, 2, "0" );
       return(result);
     }

    taille = 256;
    result = New_chaine( taille ); /* 10 caractères max */
    setlocale(LC_ALL, "C");
    switch(comparateur->type)
     { case T_EGAL:      g_snprintf( result, taille, "Dls_data_get_AO(\"%s\",\"%s\",&_%s_%s)==%f",
                                     alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, comparateur->valf );
                         break;
       case INF:         g_snprintf( result, taille, "Dls_data_get_AO(\"%s\",\"%s\",&_%s_%s)<%f",
                                     alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, comparateur->valf );
                         break;
       case SUP:         g_snprintf( result, taille, "Dls_data_get_AO(\"%s\",\"%s\",&_%s_%s)>%f",
                                     alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, comparateur->valf );
                         break;
       case INF_OU_EGAL: g_snprintf( result, taille, "Dls_data_get_AO(\"%s\",\"%s\",&_%s_%s)<=%f",
                                     alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, comparateur->valf );
                         break;
       case SUP_OU_EGAL: g_snprintf( result, taille, "Dls_data_get_AO(\"%s\",\"%s\",&_%s_%s)>=%f",
                                     alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, comparateur->valf );
                         break;
     }
    return(result);
  }
/******************************************************************************************************************************/
/* New_condition_mono: Prepare la chaine de caractere associée à la condition, en respectant les options                      */
/* Entrées: l'alias du monostable et sa liste d'options                                                                       */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_mono( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    taille = 256;
    result = New_chaine( taille ); /* 10 caractères max */
    if ( (!barre) )
         { g_snprintf( result, taille, "Dls_data_get_bool ( \"%s\", \"%s\", &_%s_%s )",
                       alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
    else { g_snprintf( result, taille, "!Dls_data_get_bool ( \"%s\", \"%s\", &_%s_%s )",
                       alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
   return(result);
 }
/******************************************************************************************************************************/
/* New_condition_tempo: Prepare la chaine de caractere associée à la condition, en respectant les options                     */
/* Entrées: l'alias de la temporisatio et sa liste d'options                                                                  */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_tempo( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    taille = 128;
    result = New_chaine( taille );
    g_snprintf( result, taille, "%sDls_data_get_tempo ( \"%s\", \"%s\", &_%s_%s )",
                (barre==1 ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    return(result);
  }
/******************************************************************************************************************************/
/* New_condition_horloge: Prepare la chaine de caractere associée à la condition, en respectant les options                   */
/* Entrées: l'alias de l'horloge et sa liste d'options                                                                        */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_horloge( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    taille = 256;                                                                               /* Alias par nom uniquement ! */
    result = New_chaine( taille ); /* 10 caractères max */
    if ( !barre )
         { g_snprintf( result, taille, "Dls_data_get_DI ( \"%s\", \"%s\", &_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
    else { g_snprintf( result, taille, "!Dls_data_get_DI ( \"%s\", \"%s\", &_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
   return(result);
 }
/******************************************************************************************************************************/
/* New_condition_horloge: Prepare la chaine de caractere associée à la condition, en respectant les options                   */
/* Entrées: l'alias de l'horloge et sa liste d'options                                                                        */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_WATCHDOG( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    taille = 256;                                                                               /* Alias par nom uniquement ! */
    result = New_chaine( taille ); /* 10 caractères max */
    if ( !barre )
         { g_snprintf( result, taille, "Dls_data_get_WATCHDOG ( \"%s\", \"%s\", &_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
    else { g_snprintf( result, taille, "!Dls_data_get_WATCHDOG ( \"%s\", \"%s\", &_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
   return(result);
 }
/******************************************************************************************************************************/
/* New_condition_vars: formate une condition avec le nom de variable en parametre                                             */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 gchar *New_condition_vars( int barre, gchar *nom )
  { gchar *result;
    int taille;

    taille = strlen(nom)+5;
    result = New_chaine( taille ); /* 10 caractères max */
    if (!barre) { g_snprintf( result, taille, "%s", nom ); }
           else { g_snprintf( result, taille, "(!%s)", nom ); }
    return(result);
  }
/******************************************************************************************************************************/
/* New_action: Alloue une certaine quantité de mémoire pour les actions DLS                                                   */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct ACTION *New_action( void )
  { struct ACTION *action;
    action=(struct ACTION *)g_try_malloc0( sizeof(struct ACTION) );
    if (!action) { return(NULL); }
    action->alors = NULL;
    action->sinon = NULL;
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_msg_by_alias: Prepare une struct action avec une commande de type MSG                                           */
/* Entrées: L'alias decouvert                                                                                                 */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_msg( struct ALIAS *alias, GList *options )
  { struct ACTION *action;
    int taille;

    taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    gint update = Get_option_entier ( options, T_UPDATE );
    if (update==-1) update=0;

    g_snprintf( action->alors, taille, "   Dls_data_set_MSG ( vars, \"%s\", \"%s\", &_%s_%s, %s, TRUE );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, (update ? "TRUE" : "FALSE") );
    g_snprintf( action->sinon, taille, "   Dls_data_set_MSG ( vars, \"%s\", \"%s\", &_%s_%s, %s, FALSE );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, (update ? "TRUE" : "FALSE") );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_sortie: Prepare la structure ACTION associée à l'alias en paramètre                                             */
/* Entrées: l'alias, le complement si besoin, les options                                                                     */
/* Sortie: la structure ACTION associée                                                                                       */
/******************************************************************************************************************************/
 struct ACTION *New_action_sortie( struct ALIAS *alias, int barre, GList *options )
  { struct ACTION *action = New_action();
    gint taille = 128;
    action->alors = New_chaine( taille );
    if ( (!barre) )
         { g_snprintf( action->alors, taille, "   Dls_data_set_DO ( vars, \"%s\", \"%s\", &_%s_%s, 1 );\n",
                       alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
    else { g_snprintf( action->alors, taille, "   Dls_data_set_DO ( vars, \"%s\", \"%s\", &_%s_%s, 0 );\n",
                       alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_digital_output: Prepare la structure ACTION associée à l'alias en paramètre                                     */
/* Entrées: l'alias, le complement si besoin, les options                                                                     */
/* Sortie: la structure ACTION associée                                                                                       */
/******************************************************************************************************************************/
 struct ACTION *New_action_digital_output( struct ALIAS *alias, GList *options )
  { struct ACTION *action;
    gint taille = 128;

    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_DO ( vars, \"%s\", \"%s\", &_%s_%s, TRUE );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    g_snprintf( action->sinon, taille, "   Dls_data_set_DO ( vars, \"%s\", \"%s\", &_%s_%s, FALSE );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_vars_mono: Prepare une struct action avec une commande SM                                                       */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_vars_mono( gchar *nom )
  { struct ACTION *action;
    int taille;

    taille = strlen(nom)+5;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "%s=1;", nom );
    g_snprintf( action->sinon, taille, "%s=0;", nom );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_mono( struct ALIAS *alias )
  { struct ACTION *action;
    int taille;

    taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_bool ( vars, \"%s\", \"%s\", &_%s_%s, TRUE );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    g_snprintf( action->sinon, taille, "   Dls_data_set_bool ( vars, \"%s\", \"%s\", &_%s_%s, FALSE );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_cpt_h( struct ALIAS *alias, GList *options )
  { struct ACTION *action;
    int taille, reset;

    reset = Get_option_entier ( options, RESET ); if (reset == -1) reset = 0;
    taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_CH ( vars, \"%s\", \"%s\", &_%s_%s, TRUE, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, reset );
    g_snprintf( action->sinon, taille, "   Dls_data_set_CH ( vars, \"%s\", \"%s\", &_%s_%s, FALSE, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, reset );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_cpt_imp( struct ALIAS *alias, GList *options )
  { struct ACTION *action;
    int taille, reset, ratio;

    reset = Get_option_entier ( options, RESET ); if (reset == -1) reset = 0;
    ratio = Get_option_entier ( options, RATIO ); if (ratio == -1) ratio = 1;

    taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_CI ( vars, \"%s\", \"%s\", &_%s_%s, TRUE, %d, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, reset, ratio );
    g_snprintf( action->sinon, taille, "   Dls_data_set_CI ( vars, \"%s\", \"%s\", &_%s_%s, FALSE, %d, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, reset, ratio );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_WATCHDOG: Prepare une struct action pour une action de type WATCHDOG                                            */
/* Entrées: l'alias source, et ses options                                                                                    */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_WATCHDOG( struct ALIAS *alias, GList *options )
  { struct ACTION *action;

    gint consigne = Get_option_entier ( options, T_CONSIGNE ); if (consigne == -1) consigne = 600;
    gint taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_WATCHDOG ( vars, \"%s\", \"%s\", &_%s_%s, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, consigne );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_icone: Prepare une struct action avec une commande SI                                                           */
/* Entrées: numero du motif                                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_icone( struct ALIAS *alias, GList *options )
  { struct ACTION *action;
    int taille, mode, coul, cligno;
    gchar *color;

    mode   = Get_option_entier ( options, MODE   ); if (mode   == -1) mode   = 0;
    coul   = Get_option_entier ( options, COLOR  ); if (coul   == -1) coul   = 0;
    cligno = Get_option_entier ( options, CLIGNO ); if (cligno == -1) cligno = 0;
    taille = 512;
    action = New_action();
    action->alors = New_chaine( taille );
    switch (coul)
     { case ROUGE   : color="red"; break;
       case VERT    : color="lime"; break;
       case BLEU    : color="blue"; break;
       case JAUNE   : color="yellow"; break;
       case ORANGE  : color="orange"; break;
       case BLANC   : color="white"; break;
       case GRIS    : color="lightgray"; break;
       case KAKI    : color="brown"; break;
       default      : color="black";
     }
    g_snprintf( action->alors, taille,
                "  Dls_data_set_VISUEL( vars, \"%s\", \"%s\", &_%s_%s, %d, \"%s\", %d );\n",
                  alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, mode, color, cligno );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_tempo: Prepare une struct action avec une commande TR                                                           */
/* Entrées: numero de la tempo, sa consigne                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_bus( struct ALIAS *alias, GList *options )
  { struct ACTION *result;
    gint taille;
    gchar *host, *thread, *tag, *param1;

    host   = Get_option_chaine ( options, T_HOST   ); if(!host) host="*";
    thread = Get_option_chaine ( options, T_THREAD ); if (!thread) thread="*";
    tag    = Get_option_chaine ( options, T_TAG    ); if (!tag) tag="no tag";
    param1 = Get_option_chaine ( options, T_PARAM1 );                            /* Param1 est un appel de fonction ou NULL ! */

    result = New_action();
    taille = 256;
    result->alors = New_chaine( taille );
    g_snprintf( result->alors, taille,
                "   Dls_data_set_bus ( \"%s\", \"%s\", &_%s_%s, 1, \"%s\", \"%s\", \"%s\", %s );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                host, thread, tag, (param1?param1:"NULL") );
    result->sinon = New_chaine( taille );
    g_snprintf( result->sinon, taille,
                "   Dls_data_set_bus ( \"%s\", \"%s\", &_%s_%s, 0, \"%s\", \"%s\", \"%s\", NULL );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                host, thread, tag );
    return(result);
  }
/******************************************************************************************************************************/
/* New_action_tempo: Prepare une struct action avec une commande TR                                                           */
/* Entrées: numero de la tempo, sa consigne                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_tempo( struct ALIAS *alias, GList *options )
  { struct ACTION *action;
    int taille, daa, dma, dMa, dad, random;

    daa    = Get_option_entier ( options, T_DAA );      if (daa == -1)    daa = 0;
    dma    = Get_option_entier ( options, T_DMINA );    if (dma == -1)    dma = 0;
    dMa    = Get_option_entier ( options, T_DMAXA );    if (dMa == -1)    dMa = 0;
    dad    = Get_option_entier ( options, T_DAD );      if (dad == -1)    dad = 0;
    random = Get_option_entier ( options, T_RANDOM );   if (random == -1) random = 0;

    action = New_action();
    taille = 256;
    action->alors = New_chaine( taille );
    g_snprintf( action->alors, taille,
                "   Dls_data_set_tempo ( \"%s\", \"%s\", &_%s_%s, 1, %d, %d, %d, %d, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                daa, dma, dMa, dad, random );
    action->sinon = New_chaine( taille );
    g_snprintf( action->sinon, taille,
                "   Dls_data_set_tempo ( \"%s\", \"%s\", &_%s_%s, 0, %d, %d, %d, %d, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                daa, dma, dMa, dad, random );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_bi( struct ALIAS *alias, gint barre )
  { struct ACTION *action;
    int taille;

    action = New_action();
    taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );
    if (barre)
     { g_snprintf( action->alors, taille, "   Dls_data_set_bool ( vars, \"%s\", \"%s\", &_%s_%s, FALSE );\n",
                                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( action->alors, taille, "   Dls_data_set_bool ( vars, \"%s\", \"%s\", &_%s_%s, TRUE );\n",
                                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    return(action);
  }
/******************************************************************************************************************************/
/* New_alias: Alloue une certaine quantité de mémoire pour utiliser des alias                                                 */
/* Entrées: le nom de l'alias, le tableau et le numero du bit                                                                 */
/* Sortie: False si il existe deja, true sinon                                                                                */
/******************************************************************************************************************************/
 gboolean New_alias ( gchar *tech_id, gchar *acronyme, gint bit, GList *options )
  { struct ALIAS *alias;

    if (Get_alias_par_acronyme( tech_id, acronyme )) return(FALSE);                                      /* ID deja definit ? */

    alias=(struct ALIAS *)g_try_malloc0( sizeof(struct ALIAS) );
    if (!alias) { return(FALSE); }
    if (!tech_id) alias->tech_id = g_strdup(Dls_plugin.tech_id);
             else alias->tech_id = g_strdup(tech_id);
    alias->acronyme = g_strdup(acronyme);
    alias->classe   = bit;
    alias->options  = options;
    alias->used     = 0;
    Alias = g_slist_prepend( Alias, alias );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* New_option: Alloue une certaine quantité de mémoire pour les options                                                       */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct OPTION *New_option_chaine( gint type, gchar *chaine )
  { struct OPTION *option;
    option=(struct OPTION *)g_try_malloc0( sizeof(struct OPTION) );
    if (option)
     { option->type   = type;
       option->chaine = chaine;
     }
    return(option);
  }
/******************************************************************************************************************************/
/* New_alias: Alloue une certaine quantité de mémoire pour utiliser des alias                                                 */
/* Entrées: le nom de l'alias, le tableau et le numero du bit                                                                 */
/* Sortie: False si il existe deja, true sinon                                                                                */
/******************************************************************************************************************************/
 static gboolean New_alias_internal ( gchar *acronyme, gint bit, gchar *libelle )
  { struct ALIAS *alias;

    if (Get_alias_par_acronyme( Dls_plugin.tech_id, acronyme )) return(FALSE);                           /* ID deja definit ? */

    alias=(struct ALIAS *)g_try_malloc0( sizeof(struct ALIAS) );
    if (!alias) { return(FALSE); }
    alias->tech_id  = g_strdup(Dls_plugin.tech_id);
    alias->acronyme = g_strdup(acronyme);
    alias->classe   = bit;
    struct OPTION *option = New_option_chaine ( T_LIBELLE, strdup(libelle) );
    alias->options  = g_list_append ( NULL, option );
    alias->used     = 1;                                                       /* Un bit internal est obligatoirement utilisé */
    Alias = g_slist_prepend( Alias, alias );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* New_alias: Alloue une certaine quantité de mémoire pour utiliser des alias                                                 */
/* Entrées: le nom de l'alias, le tableau et le numero du bit                                                                 */
/* Sortie: False si il existe deja, true sinon                                                                                */
/******************************************************************************************************************************/
 struct ALIAS *Set_new_external_alias( gchar *tech_id, gchar *acronyme )
  { struct ALIAS *alias;
    gint type;

    alias=(struct ALIAS *)g_try_malloc0( sizeof(struct ALIAS) );
    if (!alias) { return(NULL); }
    alias->options  = NULL;
    alias->used     = 1;
    alias->external = TRUE;

    if (!tech_id) tech_id=Dls_plugin.tech_id;

    if ( (type=Rechercher_DICO_type ( tech_id, acronyme )) != -1 )
     { alias->tech_id  = g_strdup(tech_id);
       alias->acronyme = g_strdup(acronyme);
       alias->classe   = type;
     }
    else
     { g_free(alias);
       return(NULL);
     }
    Alias = g_slist_prepend( Alias, alias );
    return(alias);
  }
/******************************************************************************************************************************/
/* Get_alias: Recherche un alias donné en paramètre                                                                           */
/* Entrées: le nom de l'alias                                                                                                 */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct ALIAS *Get_alias_par_acronyme( gchar *tech_id, gchar *acronyme )
  { struct ALIAS *alias;
    GSList *liste;
    liste = Alias;
    while(liste)
     { alias = (struct ALIAS *)liste->data;
       if (!strcmp(alias->acronyme, acronyme) && (tech_id==NULL || !strcmp(alias->tech_id,tech_id)) )
        { alias->used++; return(alias); }                                          /* Si deja present, on fait ++ sur le used */
       liste = liste->next;
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Liberer_alias: Liberation de toutes les zones de mémoire précédemment allouées                                             */
/* Entrées: kedal                                                                                                             */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Liberer_options ( GList *options )
  { while (options)
     { struct OPTION *option = (struct OPTION *)options->data;
       options = g_list_remove (options, option);
       switch (option->type)
        { case T_FORME:
          case T_LIBELLE:
          case T_ETIQUETTE:
          case T_HOST:
          case T_THREAD:
          case T_TAG:
          case T_PARAM1:
               g_free(option->chaine);
               break;
        }
       g_free(option);
     }
  }
/******************************************************************************************************************************/
/* Liberer_alias: Liberation de toutes les zones de mémoire précédemment allouées                                             */
/* Entrées: kedal                                                                                                             */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Liberer_alias ( struct ALIAS *alias )
  { Liberer_options(alias->options);
    g_free(alias->tech_id);
    g_free(alias->acronyme);
    g_free(alias);
  }
/******************************************************************************************************************************/
/* Liberer_memoire: Liberation de toutes les zones de mémoire précédemment allouées                                           */
/* Entrées: kedal                                                                                                             */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Liberer_memoire( void )
  { g_slist_foreach( Alias, (GFunc) Liberer_alias, NULL );
    g_slist_free( Alias );
    Alias = NULL;
  }
/******************************************************************************************************************************/
/* Trad_dls_set_debug: Positionne le flag de debug Bison/Flex                                                                 */
/* Entrée : TRUE ou FALSE                                                                                                     */
/******************************************************************************************************************************/
 void Trad_dls_set_debug ( gboolean actif )
  { DlsScanner_debug = actif; }                                                                   /* Debug de la traduction ?? */
/******************************************************************************************************************************/
/* Traduire: Traduction du fichier en paramètre du langage DLS vers le langage C                                              */
/* Entrée: l'id du modul                                                                                                      */
/* Sortie: TRAD_DLS_OK, _WARNING ou _ERROR                                                                                    */
/******************************************************************************************************************************/
 gint Traduire_DLS( gchar *tech_id )
  { gchar source[80], cible[80], log[80], *requete;
    struct CMD_TYPE_PLUGIN_DLS *plugin;
    struct ALIAS *alias;
    GSList *liste;
    gint retour, nb_car;
    FILE *rc;

    plugin = Rechercher_plugin_dlsDB ( tech_id );
    if (!plugin)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: plugin '%s' not found.", __func__, tech_id );
       return (TRAD_DLS_ERROR_NO_FILE);
     }
    memcpy ( &Dls_plugin, plugin, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
    g_free(plugin);

    Buffer_taille = 1024;
    Buffer = g_try_malloc0( Buffer_taille );                                             /* Initialisation du buffer resultat */
    if (!Buffer) return ( TRAD_DLS_ERROR_NO_FILE );
    Buffer_used = 0;

    g_snprintf( source, sizeof(source), "Dls/%s.dls", tech_id );
    g_snprintf( log,    sizeof(log),    "Dls/%s.log", tech_id );
    g_snprintf( cible,  sizeof(cible),  "Dls/%s.c", tech_id );
    unlink ( log );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: tech_id='%s', source='%s', log='%s'", __func__,
              tech_id, source, log );

    Id_log = open( log, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (Id_log<0)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                "%s: Log creation failed %s (%s)", __func__, log, strerror(errno) );
       close(Id_log);
       return(TRAD_DLS_ERROR_NO_FILE);
     }

    pthread_mutex_lock( &Partage->com_dls.synchro_traduction );                           /* Attente unicité de la traduction */

    Alias = NULL;                                                                                  /* Par défaut, pas d'alias */
    DlsScanner_set_lineno(1);                                                                     /* Reset du numéro de ligne */
    nbr_erreur = 0;                                                                   /* Au départ, nous n'avons pas d'erreur */
    rc = fopen( source, "r" );
    if (!rc) retour = TRAD_DLS_ERROR_NO_FILE;
    else
     { gchar *libelle;
       setlocale(LC_ALL, "C");

/*---------------------------------------- Création des mnemoniques permanents -----------------------------------------------*/
       libelle = "Statut de la communication du module";
       New_alias_internal ( "_COMM", MNEMO_MONOSTABLE, libelle );
       Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, Dls_plugin.tech_id, "_COMM", libelle );

       libelle = "Communication OK";
       New_alias_internal ( "_MSG_COMM_OK", MNEMO_MSG, libelle );
       Mnemo_auto_create_MSG ( Dls_plugin.tech_id, "MSG_COMM_OK", libelle, MSG_ETAT );

       libelle = "Communication Hors Service";
       New_alias_internal ( "_MSG_COMM_HS", MNEMO_MSG, libelle );
       Mnemo_auto_create_MSG ( Dls_plugin.tech_id, "MSG_COMM_HS", libelle, MSG_DEFAUT );

       DlsScanner_restart(rc);
       DlsScanner_parse();                                                                       /* Parsing du fichier source */
       fclose(rc);
     }

    if (nbr_erreur)
     { Emettre_erreur_new( "%d error%s found", nbr_erreur, (nbr_erreur>1 ? "s" : "") );
       retour = TRAD_DLS_SYNTAX_ERROR;
     }
    else
     { gchar chaine[4096], date[64];
       gint fd;
       Emettre_erreur_new( "No error found" );                      /* Pas d'erreur rencontré (mais peut etre des warnings !) */
       retour = TRAD_DLS_OK;

       unlink ( cible );
       fd = open( cible, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );             /* Enregistrement du buffer resultat sur disque */
       if (fd<0)
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                   "%s: Target creation failed %s (%s)", __func__, cible, strerror(errno) );
          retour = TRAD_DLS_ERROR_NO_FILE;
        }
       else
        { gchar *include = " #include <Module_dls.h>\n";
          gchar *Start_Go = " void Go ( struct DLS_TO_PLUGIN *vars )\n"
                            "  {\n";
          gchar *End_Go =   "  }\n";
          struct tm *temps;
          time_t ltime;

          write(fd, include, strlen(include));


          liste = Alias;
          while(liste)
           { alias = (struct ALIAS *)liste->data;
             nb_car = g_snprintf(chaine, sizeof(chaine), " gpointer _%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
             write (fd, chaine, nb_car);
             liste = liste->next;
           }

          time(&ltime);
          temps = localtime( (time_t *)&ltime );
          if (temps) { strftime( date, sizeof(date), "%F %T", temps ); }
                else { g_snprintf(date, sizeof(date), "Erreur"); }

          g_snprintf(chaine, sizeof(chaine),
                    "/*******************************************************/\n"
                    " gchar *version (void)\n"
                    "  { return(\"V%s - %s\"); \n  }\n", WTD_VERSION, date );
          write(fd, chaine, strlen(chaine) );                                                         /* Ecriture du prologue */

/*----------------------------------------------- Ecriture de la fonction Go -------------------------------------------------*/
          write( fd, Start_Go, strlen(Start_Go) );                                                 /* Ecriture de de l'entete */

/*----------------------------------------------- Ecriture du buffer C -------------------------------------------------------*/
          write(fd, Buffer, Buffer_used );                                                     /* Ecriture du buffer resultat */

/*----------------------------------------------- Ecriture de la fin de fonction Go ------------------------------------------*/
          write( fd, End_Go, strlen(End_Go) );
          close(fd);
        }

       gchar *Liste_BOOL = NULL, *Liste_DI = NULL, *Liste_DO = NULL, *Liste_AO = NULL, *Liste_AI = NULL;
       gchar *Liste_TEMPO = NULL, *Liste_HORLOGE = NULL, *Liste_REGISTRE = NULL, *Liste_WATCHDOG = NULL, *Liste_MESSAGE = NULL;
       gchar *Liste_CI = NULL, *Liste_CH = NULL;
       gchar *old_liste = NULL;
       liste = Alias;                                           /* Libération des alias, et remonté d'un Warning si il y en a */
       while(liste)
        { alias = (struct ALIAS *)liste->data;
          if ( (!alias->used) )
           { Emettre_erreur_new( "Warning: %s not used", alias->acronyme );
             retour = TRAD_DLS_WARNING;
           }
                                                                                               /* Alias Dynamiques uniquement */
          if (!strcmp(alias->tech_id, Dls_plugin.tech_id) && alias->external == FALSE )
           { gchar *libelle = Get_option_chaine( alias->options, T_LIBELLE );
             if (!libelle) libelle="no libelle";

             switch(alias->classe)
              { case MNEMO_BUS:
                   break;
                case MNEMO_BISTABLE:
                case MNEMO_MONOSTABLE:
                 { Mnemo_auto_create_BOOL ( TRUE, alias->classe, Dls_plugin.tech_id, alias->acronyme, libelle );
                   if (!Liste_BOOL) Liste_BOOL = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_BOOL;
                      Liste_BOOL = g_strconcat ( Liste_BOOL, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
                case MNEMO_ENTREE:
                 { Mnemo_auto_create_DI ( TRUE, Dls_plugin.tech_id, alias->acronyme, libelle );
                   if (!Liste_DI) Liste_DI = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_DI;
                      Liste_DI = g_strconcat ( Liste_DI, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
                case MNEMO_SORTIE:
                 { Mnemo_auto_create_DO ( TRUE, Dls_plugin.tech_id, alias->acronyme, libelle );
                   if (!Liste_DO) Liste_DO = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_DO;
                      Liste_DO = g_strconcat ( Liste_DO, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
                case MNEMO_SORTIE_ANA:
                 { Mnemo_auto_create_AO ( TRUE, Dls_plugin.tech_id, alias->acronyme, libelle );
                   if (!Liste_AO) Liste_AO = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_AO;
                      Liste_AO = g_strconcat ( Liste_AO, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
                case MNEMO_ENTREE_ANA:
                 { Mnemo_auto_create_AI ( TRUE, Dls_plugin.tech_id, alias->acronyme, libelle, NULL );
                   if (!Liste_AI) Liste_AI = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_AI;
                      Liste_AI = g_strconcat ( Liste_AI, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
                case MNEMO_TEMPO:
                 { Mnemo_auto_create_TEMPO ( Dls_plugin.tech_id, alias->acronyme, libelle );
                   if (!Liste_TEMPO) Liste_TEMPO = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_TEMPO;
                      Liste_TEMPO = g_strconcat ( Liste_TEMPO, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
                case MNEMO_HORLOGE:
                 { Mnemo_auto_create_HORLOGE ( Dls_plugin.tech_id, alias->acronyme, libelle );
                   if (!Liste_HORLOGE) Liste_HORLOGE = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_HORLOGE;
                      Liste_HORLOGE = g_strconcat ( Liste_HORLOGE, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
                case MNEMO_REGISTRE:
                 { gchar *unite = Get_option_chaine( alias->options, T_UNITE );
                   Mnemo_auto_create_REGISTRE ( Dls_plugin.tech_id, alias->acronyme, libelle, unite );
                   if (!Liste_REGISTRE) Liste_REGISTRE = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_REGISTRE;
                      Liste_REGISTRE = g_strconcat ( Liste_REGISTRE, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
                case MNEMO_WATCHDOG:
                 { Mnemo_auto_create_WATCHDOG ( TRUE, Dls_plugin.tech_id, alias->acronyme, libelle );
                   if (!Liste_WATCHDOG) Liste_WATCHDOG = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_WATCHDOG;
                      Liste_WATCHDOG = g_strconcat ( Liste_WATCHDOG, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
                case MNEMO_MOTIF:
                 { gchar *forme = Get_option_chaine( alias->options, T_FORME );
                   if (!forme) forme="none";
                   /*Synoptique_auto_create_VISUEL ( Dls_plugin.tech_id, alias->acronyme, libelle, forme );*/
                   break;
                 }
                case MNEMO_CPT_IMP:
                 { Mnemo_auto_create_CI ( Dls_plugin.tech_id, alias->acronyme, libelle );
                   if (!Liste_CI) Liste_CI = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_CI;
                      Liste_CI = g_strconcat ( Liste_CI, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
                case MNEMO_CPTH:
                 { Mnemo_auto_create_CH ( Dls_plugin.tech_id, alias->acronyme, libelle );
                   if (!Liste_CH) Liste_CH = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_CH;
                      Liste_CH = g_strconcat ( Liste_CH, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
                case MNEMO_MSG:
                 { gint param;
                   param = Get_option_entier ( alias->options, T_TYPE );
                   Mnemo_auto_create_MSG ( Dls_plugin.tech_id, alias->acronyme, libelle, (param!=-1 ? param : MSG_ETAT) );
                   if (!Liste_MESSAGE) Liste_MESSAGE = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_MESSAGE;
                      Liste_MESSAGE = g_strconcat ( Liste_MESSAGE, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
              }
           }
          liste = liste->next;
        }
/*--------------------------------------- Suppression des mnemoniques non utilisés -------------------------------------------*/
       requete = g_strconcat ( "DELETE FROM mnemos_AI WHERE deletable=1 AND tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_AI?Liste_AI:"''") , ")", NULL );
       if (Liste_AI) g_free(Liste_AI);
       SQL_Write ( requete );
       g_free(requete);

       requete = g_strconcat ( "DELETE FROM mnemos_AO WHERE deletable=1 AND tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_AO?Liste_AO:"''") , ")", NULL );
       if (Liste_AO) g_free(Liste_AO);
       SQL_Write ( requete );
       g_free(requete);

       requete = g_strconcat ( "DELETE FROM mnemos_DI WHERE deletable=1 AND tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_DI?Liste_DI:"''") , ")", NULL );
       if (Liste_DI) g_free(Liste_DI);
       SQL_Write ( requete );
       g_free(requete);

       requete = g_strconcat ( "DELETE FROM mnemos_DO WHERE deletable=1 AND tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_DO?Liste_DO:"''") , ")", NULL );
       if (Liste_DO) g_free(Liste_DO);
       SQL_Write ( requete );
       g_free(requete);

       requete = g_strconcat ( "DELETE FROM mnemos_R WHERE tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_REGISTRE?Liste_REGISTRE:"''") , ")", NULL );
       if (Liste_REGISTRE) g_free(Liste_REGISTRE);
       SQL_Write ( requete );
       g_free(requete);

       requete = g_strconcat ( "DELETE FROM mnemos_BOOL WHERE deletable=1 AND tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_BOOL?Liste_BOOL:"''") , ")", NULL );
       if (Liste_BOOL) g_free(Liste_BOOL);
       SQL_Write ( requete );
       g_free(requete);

       requete = g_strconcat ( "DELETE FROM mnemos_Tempo WHERE tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_TEMPO?Liste_TEMPO:"''") , ")", NULL );
       if (Liste_TEMPO) g_free(Liste_TEMPO);
       SQL_Write ( requete );
       g_free(requete);

       requete = g_strconcat ( "DELETE FROM mnemos_CI WHERE tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_CI?Liste_CI:"''") , ")", NULL );
       if (Liste_CI) g_free(Liste_CI);
       SQL_Write ( requete );
       g_free(requete);

       requete = g_strconcat ( "DELETE FROM mnemos_CH WHERE tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_CH?Liste_CH:"''") , ")", NULL );
       if (Liste_CH) g_free(Liste_CH);
       SQL_Write ( requete );
       g_free(requete);

       requete = g_strconcat ( "DELETE FROM msgs WHERE tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_MESSAGE?Liste_MESSAGE:"''") , ")", NULL );
       if (Liste_MESSAGE) g_free(Liste_MESSAGE);
       SQL_Write ( requete );
       g_free(requete);

       requete = g_strconcat ( "DELETE FROM mnemos_HORLOGE WHERE tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_HORLOGE?Liste_HORLOGE:"''") , ")", NULL );
       if (Liste_HORLOGE) g_free(Liste_HORLOGE);
       SQL_Write ( requete );
       g_free(requete);

       requete = g_strconcat ( "DELETE FROM mnemos_WATCHDOG WHERE deletable=1 AND tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_WATCHDOG?Liste_WATCHDOG:"''") , ")", NULL );
       if (Liste_WATCHDOG) g_free(Liste_WATCHDOG);
       SQL_Write ( requete );
       g_free(requete);
     }
    close(Id_log);
    Liberer_memoire();
    g_free(Buffer);
    Buffer = NULL;
    pthread_mutex_unlock( &Partage->com_dls.synchro_traduction );                                         /* Libération Mutex */
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
