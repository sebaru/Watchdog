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
 static struct DLS_PLUGIN Dls_plugin;

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
       Info_new( Config.log, Config.log_trad, LOG_DEBUG,
                "%s: buffer too small, trying to expand it to %d)", __func__, Buffer_taille + taille );
       new_Buffer = g_try_realloc( Buffer, Buffer_taille + taille );
       if (!new_Buffer)
        { Info_new( Config.log, Config.log_trad, LOG_ERR, "%s: Fail to expand buffer. skipping", __func__ );
          return;
        }
       Buffer = new_Buffer;
       Buffer_taille = Buffer_taille + taille;
       Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: Buffer expanded to %d bytes", __func__, Buffer_taille );
     }
    Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: ligne %d : %s", __func__, DlsScanner_get_lineno(), chaine );
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

       Info_new( Config.log, Config.log_trad, LOG_ERR, "%s: Ligne %d : %s", __func__, DlsScanner_get_lineno(), chaine );
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
 static int Get_option_entier( GList *liste_options, gint token, gint defaut )
  { struct OPTION *option;
    GList *liste;
    liste = liste_options;
    while (liste)
     { option=(struct OPTION *)liste->data;
       if ( option->token == token )
        { return (option->val_as_int); }
       liste = liste->next;
     }
    return(defaut);
  }
/******************************************************************************************************************************/
/* Get_option_entier: Cherche une option et renvoie sa valeur                                                                 */
/* Entrées: la liste des options, le type a rechercher                                                                        */
/* Sortie: -1 si pas trouvé                                                                                                   */
/******************************************************************************************************************************/
 static struct ALIAS *Get_option_alias( GList *liste_options, gint token )
  { struct OPTION *option;
    GList *liste;
    liste = liste_options;
    while (liste)
     { option=(struct OPTION *)liste->data;
       if ( option->token == token && option->token_classe == ID )
        { return (option->val_as_alias); }
       liste = liste->next;
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Get_option_chaine: Cherche une option de type chaine et renvoie sa valeur                                                  */
/* Entrées: la liste des options, le type a rechercher                                                                        */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 static gchar *Get_option_chaine( GList *liste_options, gint token, gchar *defaut )
  { struct OPTION *option;
    GList *liste;
    liste = liste_options;
    while (liste)
     { option=(struct OPTION *)liste->data;
       if ( option->token == token && option->token_classe == T_CHAINE )
        { return (option->chaine); }
       liste = liste->next;
     }
    return(defaut);
  }
/******************************************************************************************************************************/
/* New_condition_bi: Prepare la chaine de caractere associée àla condition, en respectant les options                         */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_bi( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    taille = 256;
    result = New_chaine( taille ); /* 10 caractères max */
    if (Get_option_entier( options, T_EDGE_UP, 0) == 1)
     { g_snprintf( result, taille, "%sDls_data_get_bool_up ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else if (Get_option_entier( options, T_EDGE_DOWN, 0) == 1)
     { g_snprintf( result, taille, "%sDls_data_get_bool_down ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( result, taille, "%sDls_data_get_BI ( \"%s\", \"%s\", &_%s_%s )",
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
    if (Get_option_entier( options, T_EDGE_UP, 0) == 1)
     { g_snprintf( result, taille, "%sDls_data_get_DI_up ( \"%s\", \"%s\", &_%s_%s )",
                   (barre ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else if (Get_option_entier( options, T_EDGE_DOWN, 0) == 1)
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
/* New_condition_sortie_ana: Prepare la chaine de caractere associée à la condition, en respectant les options                */
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
    switch(comparateur->ordre)
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
         { g_snprintf( result, taille, "Dls_data_get_MONO ( \"%s\", \"%s\", &_%s_%s )",
                       alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
    else { g_snprintf( result, taille, "!Dls_data_get_MONO ( \"%s\", \"%s\", &_%s_%s )",
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
/* New_condition_comparateur: Prepare la chaine de caractere associée à la condition de comparateur                           */
/* Entrées: le tech_id/acronyme, ses options, son comparateur                                                                 */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_comparateur( gchar *id, gchar *suffixe, GList *options_g, struct COMPARATEUR *comparateur )
  { struct ALIAS *alias_g, *alias_d;
    gchar *tech_id_g, *acro_g, *tech_id_d, *acro_d;

    if (suffixe) { tech_id_g = id;   acro_g = suffixe; }
            else { tech_id_g = NULL; acro_g = id; }

    alias_g = Get_alias_par_acronyme(tech_id_g,acro_g);                                                /* On recupere l'alias */
    if (!alias_g)
     { alias_g = Set_new_external_alias(tech_id_g,acro_g); }                         /* Si dependance externe, on va chercher */

    if (!alias_g)
     { if (tech_id_g) Emettre_erreur_new( "'%s:%s' is not defined", tech_id_g, acro_g );/* si l'alias n'existe pas */
                 else Emettre_erreur_new( "'%s' is not defined", acro_g );          /* si l'alias n'existe pas */
       return(NULL);
     }

    if (alias_g->classe!=MNEMO_SORTIE_ANA &&                     /* Vérification des bits non comparables */
        alias_g->classe!=MNEMO_ENTREE_ANA &&
        alias_g->classe!=MNEMO_REGISTRE &&
        alias_g->classe!=MNEMO_CPT_IMP &&
        alias_g->classe!=MNEMO_CPTH
       )
     { Emettre_erreur_new( "'%s:%s' n'est pas comparable", alias_g->tech_id, alias_g->acronyme );
       return(NULL);
     }

    if (comparateur->token_classe == ID )
     { if (comparateur->has_tech_id) { tech_id_d = comparateur->tech_id; acro_d = comparateur->acronyme; }
                                else { tech_id_d = NULL;                 acro_d = comparateur->acronyme; }

       alias_d = Get_alias_par_acronyme(tech_id_d,acro_d);                                             /* On recupere l'alias */
       if (!alias_d)
        { alias_d = Set_new_external_alias(tech_id_d,acro_d); }                      /* Si dependance externe, on va chercher */

       if (!alias_d)
        { if (tech_id_d) Emettre_erreur_new( "'%s:%s' is not defined", tech_id_d, acro_d );        /* si l'alias n'existe pas */
                    else Emettre_erreur_new( "'%s' is not defined", acro_d );                      /* si l'alias n'existe pas */
          return(NULL);
        }

       if (alias_d->classe!=MNEMO_SORTIE_ANA &&                     /* Vérification des bits non comparables */
           alias_d->classe!=MNEMO_ENTREE_ANA &&
           alias_d->classe!=MNEMO_REGISTRE &&
           alias_d->classe!=MNEMO_CPT_IMP &&
           alias_d->classe!=MNEMO_CPTH
          )
        { Emettre_erreur_new( "'%s:%s' n'est pas comparable", alias_d->tech_id, alias_d->acronyme );
          return(NULL);
        }
     }

    gchar partie_g[512], partie_d[512];
    switch(alias_g->classe)                                                /* On traite que ce qui peut passer en "condition" */
     { case MNEMO_ENTREE_ANA :
        { gint in_range = Get_option_entier ( options_g, T_IN_RANGE, 0 );
          if (in_range==1)
           { Emettre_erreur_new( "'%s'(in_range) ne peut s'utiliser dans une comparaison", alias_g->acronyme );
             return(NULL);
           }
          g_snprintf ( partie_g, sizeof(partie_g),
                       "(Dls_data_get_AI_inrange(\"%s\",\"%s\",&_%s_%s) && "
                       " (Dls_data_get_AI(\"%s\",\"%s\",&_%s_%s)",
                       alias_g->tech_id, alias_g->acronyme, alias_g->tech_id, alias_g->acronyme,
                       alias_g->tech_id, alias_g->acronyme, alias_g->tech_id, alias_g->acronyme );
          break;
        }
     }

    switch(comparateur->ordre)
     { case INF:         g_strlcat ( partie_g, " < ", sizeof(partie_g) ); break;
       case SUP:         g_strlcat ( partie_g, " > ", sizeof(partie_g) ); break;
       case INF_OU_EGAL: g_strlcat ( partie_g, " <= ", sizeof(partie_g) ); break;
       case SUP_OU_EGAL: g_strlcat ( partie_g, " >= ", sizeof(partie_g) ); break;
     }

    if (comparateur->token_classe == ID)
     { switch(alias_d->classe)                              /* On traite que ce qui peut passer en "condition" */
        { case MNEMO_ENTREE_ANA :
           { gint in_range = Get_option_entier ( comparateur->alias->options, T_IN_RANGE, 0 );
             if (in_range==1)
              { Emettre_erreur_new( "'%s'(in_range) ne peut s'utiliser dans une comparaison", alias_d->acronyme ); }
             g_snprintf ( partie_d, sizeof(partie_d),
                          "Dls_data_get_AI(\"%s\",\"%s\",&_%s_%s)) && "
                          " Dls_data_get_AI_inrange(\"%s\",\"%s\",&_%s_%s) )",
                          alias_d->tech_id, alias_d->acronyme, alias_d->tech_id, alias_d->acronyme,
                          alias_d->tech_id, alias_d->acronyme, alias_d->tech_id, alias_d->acronyme );
             break;
           }
        }
     }
    else if (comparateur->token_classe == T_VALF)
     { g_snprintf( partie_d, sizeof(partie_d), "%f )", comparateur->valf ); }
     
    return( g_strconcat( partie_g, partie_d, NULL ) );
  }
/******************************************************************************************************************************/
/* New_condition_comparateur: Prepare la chaine de caractere associée à la condition de comparateur                           */
/* Entrées: le tech_id/acronyme, ses options, son comparateur                                                                 */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_simple( gint barre, gchar *id, gchar *suffixe, GList *options )
  { gchar *tech_id, *acro;
    struct ALIAS *alias;
    if (suffixe) { tech_id = id; acro = suffixe; }
            else { tech_id = NULL; acro = id; }

    alias = Get_alias_par_acronyme(tech_id,acro);                                       /* On recupere l'alias */
    if (!alias)
     { alias = Set_new_external_alias(tech_id,acro); }                /* Si dependance externe, on va chercher */

    if (!alias)
     { if (tech_id) Emettre_erreur_new( "'%s:%s' is not defined", tech_id, acro );/* si l'alias n'existe pas */
               else Emettre_erreur_new( "'%s' is not defined", acro );/* si l'alias n'existe pas */
       return(NULL);
     }

    if ( alias->classe!=MNEMO_TEMPO &&
         alias->classe!=MNEMO_ENTREE &&
         alias->classe!=MNEMO_BISTABLE &&
         alias->classe!=MNEMO_MONOSTABLE &&
         alias->classe!=MNEMO_HORLOGE &&
         alias->classe!=MNEMO_WATCHDOG
       )
     { Emettre_erreur_new( "'%s' ne peut s'utiliser seul (avec une comparaison ?)", acro );
       return(NULL);
     }

     switch(alias->classe)                              /* On traite que ce qui peut passer en "condition" */
      { case MNEMO_TEMPO :     return ( New_condition_tempo( barre, alias, options ) );
        case MNEMO_ENTREE:     return ( New_condition_entree( barre, alias, options ) );
        case MNEMO_BISTABLE:   return ( New_condition_bi( barre, alias, options ) );
        case MNEMO_MONOSTABLE: return ( New_condition_mono( barre, alias, options ) );
        case MNEMO_HORLOGE:    return ( New_condition_horloge( barre, alias, options ) );
        case MNEMO_WATCHDOG:   return ( New_condition_WATCHDOG( barre, alias, options ) );
        default:
         { Emettre_erreur_new( "'%s' n'est pas une condition valide", acro );
           return(NULL);
         }
      }
  }
/******************************************************************************************************************************/
/* New_calcul_PID: Calcul un PID                                                                                              */
/* Entrées: la liste d'option associée au PID                                                                                 */
/* Sortie: la chaine de calcul DLS                                                                                            */
/******************************************************************************************************************************/
 gchar *New_calcul_PID ( GList *options )
  { struct ALIAS *input = Get_option_alias ( options, T_INPUT );
    if (!input)
     { Emettre_erreur_new ( "PID : input unknown. Select one R." );
       return(g_strdup("0"));
     }
    if ( input->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( "PID : input must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *consigne = Get_option_alias ( options, T_CONSIGNE );
    if (!consigne)
     { Emettre_erreur_new ( "PID : consigne unknown. Select one R." );
       return(g_strdup("0"));
     }
    if ( consigne->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( "PID : consigne must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *kp = Get_option_alias ( options, T_KP );
    if (!kp)
     { Emettre_erreur_new ( "PID : kp. Select one R." );
       return(g_strdup("0"));
     }
    if ( kp->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( "PID : kp must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *ki = Get_option_alias ( options, T_KD );
    if (!ki)
     { Emettre_erreur_new ( "PID : ki. Select one R." );
       return(g_strdup("0"));
     }
    if ( ki->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( "PID : ki must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *kd = Get_option_alias ( options, T_KI );
    if (!kd)
     { Emettre_erreur_new ( "PID : kd. Select one R." );
       return(g_strdup("0"));
     }
    if ( kd->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( "PID : kd must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *output_min = Get_option_alias ( options, T_MIN );
    if (!output_min)
     { Emettre_erreur_new ( "PID : output_min. Select one R." );
       return(g_strdup("0"));
     }
    if ( output_min->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( "PID : output_min must be R." );
       return(g_strdup("0"));
     }

    struct ALIAS *output_max = Get_option_alias ( options, T_MAX );
    if (!output_max)
     { Emettre_erreur_new ( "PID : output_max. Select one R." );
       return(g_strdup("0"));
     }
    if ( output_max->classe != MNEMO_REGISTRE )
     { Emettre_erreur_new ( "PID : output_max must be R." );
       return(g_strdup("0"));
     }

    gint taille=512;
    gchar *chaine = New_chaine ( taille );
    g_snprintf( chaine, taille, "Dls_PID ( \"%s\", \"%s\", &_%s_%s, "
                                          "\"%s\", \"%s\", &_%s_%s, "
                                          "\"%s\", \"%s\", &_%s_%s, "
                                          "\"%s\", \"%s\", &_%s_%s, "
                                          "\"%s\", \"%s\", &_%s_%s, "
                                          "\"%s\", \"%s\", &_%s_%s, "
                                          "\"%s\", \"%s\", &_%s_%s )",
                input->tech_id, input->acronyme, input->tech_id, input->acronyme,
                consigne->tech_id, consigne->acronyme, consigne->tech_id, consigne->acronyme,
                kp->tech_id, kp->acronyme, kp->tech_id, kp->acronyme,
                ki->tech_id, ki->acronyme, ki->tech_id, ki->acronyme,
                kd->tech_id, kd->acronyme, kd->tech_id, kd->acronyme,
                output_min->tech_id, output_min->acronyme, output_min->tech_id, output_min->acronyme,
                output_max->tech_id, output_max->acronyme, output_max->tech_id, output_max->acronyme );
    return(chaine);
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

    gint update = Get_option_entier ( options, T_UPDATE, 0 );

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
    action->sinon = NULL;

    g_snprintf( action->alors, taille, "   Dls_data_set_MONO ( vars, \"%s\", \"%s\", &_%s_%s, TRUE );\n",
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

    reset = Get_option_entier ( options, RESET, 0 );
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

    reset = Get_option_entier ( options, RESET, 0 );
    ratio = Get_option_entier ( options, T_RATIO, 1 );

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

    gint consigne = Get_option_entier ( options, T_CONSIGNE, 600 );
    gint taille = 256;
    action = New_action();
    action->alors = New_chaine( taille );

    g_snprintf( action->alors, taille, "   Dls_data_set_WATCHDOG ( vars, \"%s\", \"%s\", &_%s_%s, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, consigne );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_visuel: Prepare une struct action avec une commande SI                                                           */
/* Entrées: numero du motif                                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_visuel( struct ALIAS *alias, GList *options )
  { struct ACTION *action;
    int taille, mode, coul, cligno;
    gchar *color;

    gchar *mode_string = Get_option_chaine ( options, MODE, NULL );
    if (mode_string == NULL) mode = Get_option_entier ( options, MODE, 0   );
    coul   = Get_option_entier ( options, COLOR, 0  );
    cligno = Get_option_entier ( options, CLIGNO, 0 );
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
       case KAKI    : color="darkgreen"; break;
       default      : color="black";
     }
    if (mode_string==NULL)
     { g_snprintf( action->alors, taille,
                   "  Dls_data_set_VISUEL( vars, \"%s\", \"%s\", &_%s_%s, \"%d\", \"%s\", %d );\n",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, mode, color, cligno );
     }
    else
     { g_snprintf( action->alors, taille,
                   "  Dls_data_set_VISUEL( vars, \"%s\", \"%s\", &_%s_%s, \"%s\", \"%s\", %d );\n",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, mode_string, color, cligno );
     }

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
#ifdef bouh
    host   = Get_option_chaine ( options, T_HOST   ); if(!host) host="*";
    thread = Get_option_chaine ( options, T_THREAD ); if (!thread) thread="*";
    tag    = Get_option_chaine ( options, T_TAG    ); if (!tag) tag="no tag";
/*  param1 = Get_option_chaine ( options, T_PARAM1 );                            /* Param1 est un appel de fonction ou NULL ! */

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
#endif
return(NULL);
  }
/******************************************************************************************************************************/
/* New_action_tempo: Prepare une struct action avec une commande TR                                                           */
/* Entrées: numero de la tempo, sa consigne                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_tempo( struct ALIAS *alias, GList *options )
  { struct ACTION *action;
    int taille, daa, dma, dMa, dad, random;

    daa    = Get_option_entier ( options, T_DAA, 0 );
    dma    = Get_option_entier ( options, T_DMINA, 0 );
    dMa    = Get_option_entier ( options, T_DMAXA, 0 );
    dad    = Get_option_entier ( options, T_DAD, 0 );
    random = Get_option_entier ( options, T_RANDOM, 0 );

    action = New_action();
    taille = 256;
    action->alors = New_chaine( taille );
    g_snprintf( action->alors, taille,
                "   Dls_data_set_tempo ( vars, \"%s\", \"%s\", &_%s_%s, 1, %d, %d, %d, %d, %d );\n",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                daa, dma, dMa, dad, random );
    action->sinon = New_chaine( taille );
    g_snprintf( action->sinon, taille,
                "   Dls_data_set_tempo ( vars, \"%s\", \"%s\", &_%s_%s, 0, %d, %d, %d, %d, %d );\n",
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
     { g_snprintf( action->alors, taille, "   Dls_data_set_BI ( vars, \"%s\", \"%s\", &_%s_%s, FALSE );\n",
                                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( action->alors, taille, "   Dls_data_set_BI ( vars, \"%s\", \"%s\", &_%s_%s, TRUE );\n",
                                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    return(action);
  }
/******************************************************************************************************************************/
/* New_option: Alloue une certaine quantité de mémoire pour les options                                                       */
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct OPTION *New_option_chaine( gint token, gchar *chaine )
  { struct OPTION *option;
    option=(struct OPTION *)g_try_malloc0( sizeof(struct OPTION) );
    if (option)
     { option->token        = token;
       option->token_classe = T_CHAINE;
       option->chaine       = chaine;
     }
    return(option);
  }
/******************************************************************************************************************************/
/* Get_option_entier: Cherche une option et renvoie sa valeur                                                                 */
/* Entrées: la liste des options, le type a rechercher                                                                        */
/* Sortie: -1 si pas trouvé                                                                                                   */
/******************************************************************************************************************************/
 static gdouble Get_option_double( GList *liste_options, gint token, gdouble defaut )
  { struct OPTION *option;
    GList *liste;
    liste = liste_options;
    while (liste)
     { option=(struct OPTION *)liste->data;
       if ( option->token == token && option->token_classe == T_VALF )
        { return (option->val_as_double); }
       liste = liste->next;
     }
    return(defaut);
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
    Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: '%s:%s'", __func__, alias->tech_id, alias->acronyme );

    if (bit != MNEMO_MOTIF) return(TRUE);

    gchar *forme_src = Get_option_chaine ( options, T_FORME, NULL );
    gchar ss_chaine[128], ss_acronyme[64];
    GList *ss_options;
    if (!forme_src) return(TRUE);

    gchar *forme = Normaliser_chaine ( forme_src );
    if (!forme) return(TRUE);

    JsonNode *RootNode = Json_node_create();
    SQL_Select_to_json_node ( RootNode, NULL, "SELECT ihm_affichage FROM icone WHERE forme='%s'", forme );
    gchar *ihm_affichage = Json_get_string ( RootNode, "ihm_affichage" );
    if (ihm_affichage)
     { if (!strcasecmp ( ihm_affichage, "simple" ))
        { g_snprintf( ss_acronyme, sizeof(ss_acronyme), "%s_CLIC", acronyme );
          g_snprintf( ss_chaine, sizeof(ss_chaine), "Clic sur l'icone depuis l'IHM" );
          ss_options = g_list_append ( NULL, New_option_chaine ( T_LIBELLE, ss_chaine ) );
          New_alias ( tech_id, ss_acronyme, MNEMO_ENTREE, ss_options );
        }
     }
    g_free(forme);
    json_node_unref(RootNode);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* New_alias: Alloue une certaine quantité de mémoire pour utiliser des alias                                                 */
/* Entrées: le nom de l'alias, le tableau et le numero du bit                                                                 */
/* Sortie: False si il existe deja, true sinon                                                                                */
/******************************************************************************************************************************/
 static gboolean New_alias_permanent ( gchar *acronyme, gint bit, gchar *libelle )
  { struct ALIAS *alias;

    if (Get_alias_par_acronyme( Dls_plugin.tech_id, acronyme )) return(FALSE);                           /* ID deja definit ? */

    alias=(struct ALIAS *)g_try_malloc0( sizeof(struct ALIAS) );
    if (!alias) { return(FALSE); }
    alias->tech_id   = g_strdup(Dls_plugin.tech_id);
    alias->acronyme  = g_strdup(acronyme);
    alias->classe    = bit;
    struct OPTION *option = New_option_chaine ( T_LIBELLE, strdup(libelle) );
    alias->options   = g_list_append ( NULL, option );
    alias->used      = 1;                                                      /* Un bit internal est obligatoirement utilisé */
    alias->permanent = 1;                                                      /* Un bit internal est obligatoirement utilisé */
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
    else if ( (type=Rechercher_DICO_type ( "SYS", acronyme )) != -1 )
     { alias->tech_id  = g_strdup(tech_id);
       alias->acronyme = g_strdup(acronyme);
       alias->classe   = type;
     }
    else { g_free(alias); return(NULL); }                                          /* Si pas trouvé en externe, retourne NULL */
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
       if (option->token_classe == T_CHAINE) g_free(option->chaine);
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
/* Traduire: Traduction du fichier en paramètre du langage DLS vers le langage C                                              */
/* Entrée: l'id du modul                                                                                                      */
/* Sortie: TRAD_DLS_OK, _WARNING ou _ERROR                                                                                    */
/******************************************************************************************************************************/
 gint Traduire_DLS( gchar *tech_id )
  { gchar source[80], cible[80], log[80], *requete;
    struct DLS_PLUGIN *plugin;
    struct ALIAS *alias;
    GSList *liste;
    gint retour, nb_car;
    FILE *rc;

    plugin = Rechercher_plugin_dlsDB ( tech_id );
    if (!plugin)
     { Info_new( Config.log, Config.log_trad, LOG_ERR, "%s: plugin '%s' not found.", __func__, tech_id );
       return (TRAD_DLS_ERROR_NO_FILE);
     }
    memcpy ( &Dls_plugin, plugin, sizeof(struct DLS_PLUGIN) );
    g_free(plugin);

    Buffer_taille = 1024;
    Buffer = g_try_malloc0( Buffer_taille );                                             /* Initialisation du buffer resultat */
    if (!Buffer) return ( TRAD_DLS_ERROR_NO_FILE );
    Buffer_used = 0;

    g_snprintf( source, sizeof(source), "Dls/%s.dls", tech_id );
    g_snprintf( log,    sizeof(log),    "Dls/%s.log", tech_id );
    g_snprintf( cible,  sizeof(cible),  "Dls/%s.c", tech_id );
    unlink ( log );
    Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: tech_id='%s', source='%s', log='%s'", __func__,
              tech_id, source, log );

    Id_log = open( log, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (Id_log<0)
     { Info_new( Config.log, Config.log_trad, LOG_WARNING,
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
       libelle = "Statut de la Communication de l'I/O Thread lié au module";
       New_alias_permanent ( "IO_COMM", MNEMO_MONOSTABLE, libelle );
       Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, Dls_plugin.tech_id, "IO_COMM", libelle );

       libelle = "Statut de Synthèse de la communication du module";
       New_alias_permanent ( "COMM", MNEMO_MONOSTABLE, libelle );
       Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, Dls_plugin.tech_id, "COMM", libelle );

       libelle = "Communication OK";
       New_alias_permanent ( "MSG_COMM_OK", MNEMO_MSG, libelle );
       Mnemo_auto_create_MSG ( FALSE, Dls_plugin.tech_id, "MSG_COMM_OK", libelle, MSG_ETAT );

       libelle = "Communication Hors Service";
       New_alias_permanent ( "MSG_COMM_HS", MNEMO_MSG, libelle );
       Mnemo_auto_create_MSG ( FALSE, Dls_plugin.tech_id, "MSG_COMM_HS", libelle, MSG_DEFAUT );

       DlsScanner_debug = Config.log_trad;
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
        { Info_new( Config.log, Config.log_trad, LOG_WARNING,
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
                                                                       /* Alias Dynamiques, local et non permanent uniquement */
          if (!strcmp(alias->tech_id, Dls_plugin.tech_id) && alias->external == FALSE && alias->permanent == FALSE)
           { gchar *libelle = Get_option_chaine( alias->options, T_LIBELLE, "no libelle" );
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

                   gchar *cadran = Get_option_chaine( alias->options, T_CADRAN, NULL );
                   if (cadran)
                    { Synoptique_auto_create_CADRAN ( &Dls_plugin, alias->acronyme, cadran,
                                                      Get_option_double ( alias->options, T_MIN, 0.0 ),
                                                      Get_option_double ( alias->options, T_MAX, 100.0 ),
                                                      Get_option_entier ( alias->options, T_DECIMAL, 2 )
                                                    );
                    }

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
                 { Mnemo_auto_create_HORLOGE ( TRUE, Dls_plugin.tech_id, alias->acronyme, libelle );
                   if (!Liste_HORLOGE) Liste_HORLOGE = g_strconcat( "'", alias->acronyme, "'", NULL );
                   else
                    { old_liste = Liste_HORLOGE;
                      Liste_HORLOGE = g_strconcat ( Liste_HORLOGE, ", '", alias->acronyme, "'", NULL );
                      g_free(old_liste);
                    }
                   break;
                 }
                case MNEMO_REGISTRE:
                 { gchar *unite = Get_option_chaine( alias->options, T_UNITE, "no unit" );
                   Mnemo_auto_create_REGISTRE ( Dls_plugin.tech_id, alias->acronyme, libelle, unite );

                   gchar *cadran = Get_option_chaine( alias->options, T_CADRAN, NULL );
                   if (cadran)
                    { Synoptique_auto_create_CADRAN ( &Dls_plugin, alias->acronyme, cadran,
                                                      Get_option_double ( alias->options, T_MIN, 0.0 ),
                                                      Get_option_double ( alias->options, T_MAX, 100.0 ),
                                                      Get_option_entier ( alias->options, T_DECIMAL, 2 )
                                                    );
                    }

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
                 { gchar *forme = Get_option_chaine( alias->options, T_FORME, "none" );
                   Synoptique_auto_create_VISUEL ( &Dls_plugin, alias->acronyme, libelle, forme );
                   break;
                 }
                case MNEMO_CPT_IMP:
                 { Mnemo_auto_create_CI ( Dls_plugin.tech_id, alias->acronyme, libelle,
                                          Get_option_chaine ( alias->options, T_UNITE, "fois" ),
                                          Get_option_double ( alias->options, T_MULTI, 1.0 ) );
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
                   param = Get_option_entier ( alias->options, T_TYPE, MSG_ETAT );
                   Mnemo_auto_create_MSG ( TRUE, Dls_plugin.tech_id, alias->acronyme, libelle, param );
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

       requete = g_strconcat ( "DELETE FROM msgs WHERE deletable=1 AND tech_id='", tech_id, "' ",
                               " AND acronyme NOT IN (", (Liste_MESSAGE?Liste_MESSAGE:"''") , ")", NULL );
       if (Liste_MESSAGE) g_free(Liste_MESSAGE);
       SQL_Write ( requete );
       g_free(requete);

       requete = g_strconcat ( "DELETE FROM mnemos_HORLOGE WHERE deletable=1 AND tech_id='", tech_id, "' ",
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
