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
 static GSList *Liste_Actions_bit    = NULL;                              /* Liste des actions rencontrées dans le source DLS */
 static GSList *Liste_Actions_num    = NULL;                              /* Liste des actions rencontrées dans le source DLS */
 static GSList *Liste_edge_up_bi     = NULL;                               /* Liste des bits B utilisés avec l'option EDGE_UP */
 static GSList *Liste_edge_up_entree = NULL;                               /* Liste des bits E utilisés avec l'option EDGE_UP */
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
/* New_condition_bi: Prepare la chaine de caractere associée à la condition, en respectant les options                        */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 static gchar *New_condition_bi_old( int barre, int num, GList *options )
  { gchar *result;
    gint taille;
    taille = 24;
    result = New_chaine( taille );
    if (Get_option_entier( options, T_EDGE_UP) == 1)
     { if ( g_slist_find ( Liste_edge_up_bi, GINT_TO_POINTER(num) ) == NULL )
         { Liste_edge_up_bi = g_slist_prepend ( Liste_edge_up_bi, GINT_TO_POINTER(num) ); }
       if (barre) g_snprintf( result, taille, "!B%d_edge_up_value", num );
             else g_snprintf( result, taille, "B%d_edge_up_value", num );
     }
    else { if (barre) g_snprintf( result, taille, "!B(%d)", num );
                 else g_snprintf( result, taille, "B(%d)", num );
         }
    return(result);
  }
/******************************************************************************************************************************/
/* New_condition_bi: Prepare la chaine de caractere associée àla condition, en respectant les options                        */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_bi( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    if (alias->num != -1) /* Alias par numéro ? */
     { return(New_condition_bi_old( barre, alias->num, options)); }
    else /* Alias par nom */
     { taille = 256;
       result = New_chaine( taille ); /* 10 caractères max */
       if ( (!barre && !alias->barre) || (barre && alias->barre) )
            { g_snprintf( result, taille, "Dls_data_get_bool ( \"%s\", \"%s\", &_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
            }
       else { g_snprintf( result, taille, "!Dls_data_get_bool ( \"%s\", \"%s\", &_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
            }
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
    if (alias->num != -1) /* Alias par numéro ? */
     { taille = 24;
       result = New_chaine( taille );
       if (Get_option_entier( options, T_EDGE_UP) == 1)
        { if ( g_slist_find ( Liste_edge_up_entree, GINT_TO_POINTER(alias->num) ) == NULL )
           { Liste_edge_up_entree = g_slist_prepend ( Liste_edge_up_entree, GINT_TO_POINTER(alias->num) ); }
          if ((!barre && !alias->barre) || (barre && alias->barre) )
               g_snprintf( result, taille, "E%d_edge_up_value", alias->num );
          else g_snprintf( result, taille, "!E%d_edge_up_value", alias->num );
        }
       else { if ((!barre && !alias->barre) || (barre && alias->barre) )
                   g_snprintf( result, taille, "E(%d)", alias->num );
              else g_snprintf( result, taille, "!E(%d)", alias->num );
            }
     }
    else /* Alias par nom */
     { taille = 256;
       result = New_chaine( taille ); /* 10 caractères max */
       if ( (!barre && !alias->barre) || (barre && alias->barre) )
            { if (Get_option_entier( options, T_EDGE_UP) == 1)
               { g_snprintf( result, taille, "Dls_data_get_DI_up ( \"%s\", \"%s\", &_%s_%s )",
                             alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
               }
              else
               { g_snprintf( result, taille, "Dls_data_get_DI ( \"%s\", \"%s\", &_%s_%s )",
                             alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
               }
            }
       else { if (Get_option_entier( options, T_EDGE_UP) == 1)
               { g_snprintf( result, taille, "!Dls_data_get_DI_up ( \"%s\", \"%s\", &_%s_%s )",
                             alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
               }
              else
               { g_snprintf( result, taille, "!Dls_data_get_DI ( \"%s\", \"%s\", &_%s_%s )",
                             alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
               }
            }
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
    if (alias->num != -1) /* Alias par numéro ? */
     { taille = 15;
       result = New_chaine( taille ); /* 10 caractères max */
       if ( (!barre && !alias->barre) || (barre && alias->barre) )
            { g_snprintf( result, taille, "M(%d)", alias->num ); }
       else { g_snprintf( result, taille, "!M(%d)", alias->num ); }
     }
    else /* Alias par nom */
     { taille = 256;
       result = New_chaine( taille ); /* 10 caractères max */
       if ( (!barre && !alias->barre) || (barre && alias->barre) )
            { g_snprintf( result, taille, "Dls_data_get_bool ( \"%s\", \"%s\", &_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
            }
       else { g_snprintf( result, taille, "!Dls_data_get_bool ( \"%s\", \"%s\", &_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
            }
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
    if ( alias->type == ALIAS_TYPE_DYNAMIC)
     { g_snprintf( result, taille, "%sDls_data_get_tempo ( \"%s\", \"%s\", &_%s_%s )",
                   (barre==1 ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( result, taille, "%sT(%d)",
                   (barre==1 ? "!" : ""), alias->num );
     }
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
/* Add_bit_to_list: Ajoute un bit dans la liste des bits utilisé                                                              */
/* Entrées: le type de bit et son numéro                                                                                      */
/* Sortie: FALSE si le bit est deja dans la liste                                                                             */
/******************************************************************************************************************************/
 static gboolean Add_bit_to_list( int type, int num )
  { GSList *liste_bit, *liste_num;
    liste_bit = Liste_Actions_bit;
    liste_num = Liste_Actions_num;
    while (liste_bit)
     { if ( GPOINTER_TO_INT(liste_bit->data) == type && GPOINTER_TO_INT(liste_num->data) == num ) break;
       liste_bit=liste_bit->next;
       liste_num=liste_num->next;
     }
    if(!liste_bit)
     { Liste_Actions_bit = g_slist_append ( Liste_Actions_bit, GINT_TO_POINTER(type) );
       Liste_Actions_num = g_slist_append ( Liste_Actions_num, GINT_TO_POINTER(num) );
       return(TRUE);
     }
    return(FALSE);
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
    if ( (!barre && !alias->barre) || (barre && alias->barre) )
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

    if (alias->type == ALIAS_TYPE_STATIC)                                                               /* Alias par numéro ? */
     { taille = 15;
       Add_bit_to_list(MNEMO_MONOSTABLE, alias->num);
       action = New_action();
       action->alors = New_chaine( taille );
       action->sinon = New_chaine( taille );

       g_snprintf( action->alors, taille, "SM(%d,1);", alias->num );
       g_snprintf( action->sinon, taille, "SM(%d,0);", alias->num );
     }
    else /* Alias par nom */
     { taille = 256;
       action = New_action();
       action->alors = New_chaine( taille );
       action->sinon = New_chaine( taille );

       g_snprintf( action->alors, taille, "   Dls_data_set_bool ( vars, \"%s\", \"%s\", &_%s_%s, TRUE );\n",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
       g_snprintf( action->sinon, taille, "   Dls_data_set_bool ( vars, \"%s\", \"%s\", &_%s_%s, FALSE );\n",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
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
    taille = 256;
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
    if (alias->num==-1)
     { g_snprintf( action->alors, taille,
                   "  if (vars->bit_comm_out) Dls_data_set_VISUEL( vars, \"%s\", \"%s\", &_%s_%s, 0, \"darkgreen\", 1 );"
                   " else Dls_data_set_VISUEL( vars, \"%s\", \"%s\", &_%s_%s, %d, \"%s\", %d );\n",
                     alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                     alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, mode, color, cligno );
     }
    else
     { g_snprintf( action->alors, taille,
                   "  if (vars->bit_comm_out) Dls_data_set_VISUEL( vars, \"OLD_I\", \"%d\", &_%s_%s, 0, \"darkgreen\", 1 );"
                   " else Dls_data_set_VISUEL( vars, \"OLD_I\", \"%d\", &_%s_%s, %d, \"%s\", %d );\n",
                     alias->num, alias->tech_id, alias->acronyme,
                     alias->num, alias->tech_id, alias->acronyme, mode, color, cligno );
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

    host   = Get_option_chaine ( options, T_HOST   ); if(!host) host="*";
    thread = Get_option_chaine ( options, T_THREAD ); if (!thread) thread="*";
    tag    = Get_option_chaine ( options, T_TAG    ); if (!tag) tag="no tag";
    param1 = Get_option_chaine ( options, T_PARAM1 );                            /* Param1 est un appel de fonction ou NULL ! */

    result = New_action();
    taille = 256;
    if (alias->type == ALIAS_TYPE_DYNAMIC)
     { result->alors = New_chaine( taille );
       g_snprintf( result->alors, taille,
                   "   Dls_data_set_bus ( \"%s\", \"%s\", &_%s_%s, 1, \"%s\", \"%s\", \"%s\", %s );\n",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                   host, thread, tag, (param1?param1:"NULL") );
       result->sinon = New_chaine( taille );
       g_snprintf( result->sinon, taille,
                   "   Dls_data_set_bus ( \"%s\", \"%s\", &_%s_%s, 0, \"%s\", \"%s\", \"%s\", NULL );\n",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                   host, thread, tag );
     }
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
    if (alias->type == ALIAS_TYPE_STATIC)                                                               /* Alias par numéro ? */
     { taille = 20;
       Add_bit_to_list(MNEMO_BISTABLE, alias->num);
       action->alors = New_chaine( taille );
       g_snprintf( action->alors, taille, "SB(%d,%d);", alias->num, !barre );
     }
    else /* Alias par nom */
     { taille = 256;
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
     }
    return(action);
  }
/******************************************************************************************************************************/
/* New_alias: Alloue une certaine quantité de mémoire pour utiliser des alias                                                 */
/* Entrées: le nom de l'alias, le tableau et le numero du bit                                                                 */
/* Sortie: False si il existe deja, true sinon                                                                                */
/******************************************************************************************************************************/
 gboolean New_alias( gint type, gchar *tech_id, gchar *acronyme, gint bit, gint num, gint barre, GList *options )
  { struct ALIAS *alias;

    if (Get_alias_par_acronyme( tech_id, acronyme )) return(FALSE);                                      /* ID deja definit ? */

    alias=(struct ALIAS *)g_try_malloc0( sizeof(struct ALIAS) );
    if (!alias) { return(FALSE); }
    alias->type = type;
    if (!tech_id) alias->tech_id = g_strdup(Dls_plugin.tech_id);
             else alias->tech_id = g_strdup(tech_id);
    alias->acronyme = g_strdup(acronyme);
    alias->type_bit = bit;
    alias->num      = num;
    alias->barre    = barre;
    alias->options  = options;
    alias->used     = 0;
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
    struct DB *db;

    alias=(struct ALIAS *)g_try_malloc0( sizeof(struct ALIAS) );
    if (!alias) { return(NULL); }
    alias->type     = ALIAS_TYPE_DYNAMIC;
    alias->num      = -1;
    alias->barre    = 0;
    alias->options  = NULL;
    alias->used     = 1;
    alias->external = TRUE;

    if (!strcmp(tech_id,"THIS")) tech_id=Dls_plugin.tech_id;

    if ( (db=Rechercher_BOOL ( tech_id, acronyme )) != NULL )
     { alias->tech_id  = g_strdup(tech_id);
       alias->acronyme = g_strdup(acronyme);
       alias->type_bit = atoi(db->row[0]);
       Libere_DB_SQL (&db);
     }
    else if ( (db=Rechercher_AI ( tech_id, acronyme )) != NULL )
     { alias->tech_id  = g_strdup(tech_id);
       alias->acronyme = g_strdup(acronyme);
       alias->type_bit = MNEMO_ENTREE_ANA;
       Libere_DB_SQL (&db);
     }
    else if ( (db=Rechercher_DI ( tech_id, acronyme )) != NULL )
     { alias->tech_id  = g_strdup(tech_id);
       alias->acronyme = g_strdup(acronyme);
       alias->type_bit = atoi(db->row[0]);
       Libere_DB_SQL (&db);
     }
    else if ( (db=Rechercher_DO ( tech_id, acronyme )) != NULL )
     { alias->tech_id  = g_strdup(tech_id);
       alias->acronyme = g_strdup(acronyme);
       alias->type_bit = MNEMO_DIGITAL_OUTPUT;
       Libere_DB_SQL (&db);
     }
    else if ( (db=Rechercher_CI ( tech_id, acronyme )) != NULL )
     { alias->tech_id  = g_strdup(tech_id);
       alias->acronyme = g_strdup(acronyme);
       alias->type_bit = MNEMO_CPT_IMP;
       Libere_DB_SQL (&db);
     }
    else if ( (db=Rechercher_CH ( tech_id, acronyme )) != NULL )
     { alias->tech_id  = g_strdup(tech_id);
       alias->acronyme = g_strdup(acronyme);
       alias->type_bit = MNEMO_CPTH;
       Libere_DB_SQL (&db);
     }
    else
     { g_free(alias);
       Emettre_erreur_new( "Bit %s:%s not found", tech_id, acronyme );
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
    g_slist_free(Liste_Actions_bit);    Liste_Actions_bit    = NULL;
    g_slist_free(Liste_Actions_num);    Liste_Actions_num    = NULL;
    g_slist_free(Liste_edge_up_bi);     Liste_edge_up_bi     = NULL;
    g_slist_free(Liste_edge_up_entree); Liste_edge_up_entree = NULL;
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
    Liste_Actions_bit = NULL;                                                                    /* Par défaut, pas d'actions */
    Liste_Actions_num = NULL;                                                                    /* Par défaut, pas d'actions */
    Liste_edge_up_bi  = NULL;                                               /* Liste des bits B utilisé avec l'option EDGE UP */
    DlsScanner_set_lineno(1);                                                                     /* Reset du numéro de ligne */
    nbr_erreur = 0;                                                                   /* Au départ, nous n'avons pas d'erreur */
    rc = fopen( source, "r" );
    if (!rc) retour = TRAD_DLS_ERROR_NO_FILE;
    else
     { setlocale(LC_ALL, "C");
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
                            "  {\n"
                            "    Update_edge_up_value();\n";
          gchar *End_Go =   "  }\n";
          struct tm *temps;
          time_t ltime;

          write(fd, include, strlen(include));

          liste = Alias;
          while(liste)
           { alias = (struct ALIAS *)liste->data;
             if (alias->type == ALIAS_TYPE_DYNAMIC)                      /* alias par nom ? creation du pointeur de raccourci */
              { nb_car = g_snprintf(chaine, sizeof(chaine), " gpointer _%s_%s = NULL;\n", alias->tech_id, alias->acronyme );
                write (fd, chaine, nb_car);
              }
             liste = liste->next;
           }

          liste = Liste_edge_up_bi;                                /* Initialise les fonctions de gestion des fronts montants */
          while(liste)
           { g_snprintf(chaine, sizeof(chaine),
                      " static int B%d_edge_up_value = 0;\n", GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             g_snprintf(chaine, sizeof(chaine),
                      " static int B%d_verrou = 0;\n", GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             liste = liste->next;
           }

          liste = Liste_edge_up_entree;                            /* Initialise les fonctions de gestion des fronts montants */
          while(liste)
           { g_snprintf(chaine, sizeof(chaine),
                      " static int E%d_edge_up_value = 0;\n", GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             g_snprintf(chaine, sizeof(chaine),
                      " static int E%d_verrou = 0;\n", GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
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
          write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */

          g_snprintf(chaine, sizeof(chaine),
                    "/*******************************************************/\n"
                    " static void Update_edge_up_value (void)\n"
                    "  { \n" );
          write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */

          liste = Liste_edge_up_bi;                                /* Initialise les fonctions de gestion des fronts montants */
          while(liste)
           { gchar chaine[1024];
             g_snprintf(chaine, sizeof(chaine),
                      " if (B(%d) == 0)\n"
                      "      { B%d_verrou = 0; B%d_edge_up_value = 0; }\n"
                      " else { if (B%d_verrou==0) { B%d_verrou=1; B%d_edge_up_value=1; }\n"
                      "                      else { B%d_edge_up_value=0; }\n"
                      "      }\n",
                      GPOINTER_TO_INT(liste->data), GPOINTER_TO_INT(liste->data), GPOINTER_TO_INT(liste->data),
                      GPOINTER_TO_INT(liste->data), GPOINTER_TO_INT(liste->data), GPOINTER_TO_INT(liste->data),
                      GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             liste = liste->next;
           }
          liste = Liste_edge_up_entree;                            /* Initialise les fonctions de gestion des fronts montants */
          while(liste)
           { gchar chaine[1024];
             g_snprintf(chaine, sizeof(chaine),
                      " if (E(%d) == 0)\n"
                      "      { E%d_verrou = 0; E%d_edge_up_value = 0; }\n"
                      " else { if (E%d_verrou==0) { E%d_verrou=1; E%d_edge_up_value=1; }\n"
                      "                      else { E%d_edge_up_value=0; }\n"
                      "      }\n",
                      GPOINTER_TO_INT(liste->data), GPOINTER_TO_INT(liste->data), GPOINTER_TO_INT(liste->data),
                      GPOINTER_TO_INT(liste->data), GPOINTER_TO_INT(liste->data), GPOINTER_TO_INT(liste->data),
                      GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             liste = liste->next;
           }
          g_snprintf(chaine, sizeof(chaine), "  }\n" );
          write(fd, chaine, strlen(chaine) );                                                         /* Ecriture du prologue */

          write( fd, Start_Go, strlen(Start_Go) );                                                 /* Ecriture de de l'entete */

          write(fd, Buffer, Buffer_used );                                                     /* Ecriture du buffer resultat */
          write( fd, End_Go, strlen(End_Go) );
          close(fd);
        }

       gchar *Liste_acronyme = NULL;
       gchar *old_Liste_acronyme = NULL;
       liste = Alias;                                           /* Libération des alias, et remonté d'un Warning si il y en a */
       while(liste)
        { alias = (struct ALIAS *)liste->data;
          if ( (!alias->used) )
           { Emettre_erreur_new( "Warning: %s not used", alias->acronyme );
             retour = TRAD_DLS_WARNING;
           }
                                                                                               /* Alias Dynamiques uniquement */
          if (alias->type == ALIAS_TYPE_DYNAMIC && !strcmp(alias->tech_id, Dls_plugin.tech_id) && alias->external == FALSE )
           { gchar *libelle = Get_option_chaine( alias->options, T_LIBELLE );
             if (!libelle) libelle="no libelle";

             if (!Liste_acronyme) Liste_acronyme = g_strconcat( "'", alias->acronyme, "'", NULL );
             else
              { old_Liste_acronyme = Liste_acronyme;
                Liste_acronyme = g_strconcat ( Liste_acronyme, ", '", alias->acronyme, "'", NULL );
                g_free(old_Liste_acronyme);
              }

             switch(alias->type_bit)
              { case MNEMO_BUS:
                   break;
                case MNEMO_BISTABLE:
                case MNEMO_MONOSTABLE:
                 { Mnemo_auto_create_BOOL ( TRUE, alias->type_bit, Dls_plugin.tech_id, alias->acronyme, libelle );
                   break;
                 }
                case MNEMO_ENTREE:
                 { Mnemo_auto_create_DI ( TRUE, Dls_plugin.tech_id, alias->acronyme, libelle );
                   break;
                 }
                case MNEMO_SORTIE:
                 { Mnemo_auto_create_DO ( TRUE, Dls_plugin.tech_id, alias->acronyme, libelle );
                   break;
                 }
                case MNEMO_SORTIE_ANA:
                 { Mnemo_auto_create_AO ( TRUE, Dls_plugin.tech_id, alias->acronyme, libelle );
                   break;
                 }
                case MNEMO_ENTREE_ANA:
                 { Mnemo_auto_create_AI ( TRUE, Dls_plugin.tech_id, alias->acronyme, libelle, NULL );
                   break;
                 }
                case MNEMO_TEMPO:
                 { Mnemo_auto_create_TEMPO ( Dls_plugin.tech_id, alias->acronyme, libelle );
                   break;
                 }
                case MNEMO_HORLOGE:
                 { Mnemo_auto_create_HORLOGE ( Dls_plugin.tech_id, alias->acronyme, libelle );
                   break;
                 }
                case MNEMO_REGISTRE:
                 { gchar *unite = Get_option_chaine( alias->options, T_UNITE );
                   Mnemo_auto_create_REGISTRE ( Dls_plugin.tech_id, alias->acronyme, libelle, unite );
                   break;
                 }
                case MNEMO_MOTIF:
                 { gchar *forme = Get_option_chaine( alias->options, T_FORME );
                   if (!forme) forme="none";
                   Synoptique_auto_create_VISUEL ( Dls_plugin.tech_id, alias->acronyme, libelle, forme );
                   break;
                 }
                case MNEMO_CPT_IMP:
                 { Mnemo_auto_create_CI ( Dls_plugin.tech_id, alias->acronyme, libelle );
                   break;
                 }
                case MNEMO_CPTH:
                 { Mnemo_auto_create_CH ( Dls_plugin.tech_id, alias->acronyme, libelle );
                   break;
                 }
                case MNEMO_MSG:
                 { struct CMD_TYPE_MESSAGE msg;
                   gint param;
                   g_snprintf( msg.tech_id,  sizeof(msg.tech_id),  "%s", Dls_plugin.tech_id );
                   g_snprintf( msg.acronyme, sizeof(msg.acronyme), "%s", alias->acronyme );
                   g_snprintf( msg.libelle,  sizeof(msg.libelle),  "%s", libelle );
                   param = Get_option_entier ( alias->options, T_TYPE );
                   if (param!=-1) msg.type = param;
                             else msg.type = MSG_ETAT;
                   Mnemo_auto_create_MSG ( &msg );
                   break;
                 }
              }
           }
          liste = liste->next;
        }

       if (Liste_acronyme)
        { g_snprintf( chaine, sizeof(chaine), "DELETE FROM mnemos_AI WHERE deletable=1 AND tech_id='%s' AND acronyme NOT IN ", tech_id );
          requete = g_strconcat ( chaine, "(", Liste_acronyme, ")", NULL );
          SQL_Write ( requete );
          g_free(requete);

          g_snprintf( chaine, sizeof(chaine), "DELETE FROM mnemos_AO WHERE deletable=1 AND tech_id='%s' AND acronyme NOT IN ", tech_id );
          requete = g_strconcat ( chaine, "(", Liste_acronyme, ")", NULL );
          SQL_Write ( requete );
          g_free(requete);

          g_snprintf( chaine, sizeof(chaine), "DELETE FROM mnemos_DI WHERE deletable=1 AND tech_id='%s' AND acronyme NOT IN ", tech_id );
          requete = g_strconcat ( chaine, "(", Liste_acronyme, ")", NULL );
          SQL_Write ( requete );
          g_free(requete);

          g_snprintf( chaine, sizeof(chaine), "DELETE FROM mnemos_DO WHERE deletable=1 AND tech_id='%s' AND acronyme NOT IN ", tech_id );
          requete = g_strconcat ( chaine, "(", Liste_acronyme, ")", NULL );
          SQL_Write ( requete );
          g_free(requete);

          g_snprintf( chaine, sizeof(chaine), "DELETE FROM mnemos_R WHERE tech_id='%s' AND acronyme NOT IN ", tech_id );
          requete = g_strconcat ( chaine, "(", Liste_acronyme, ")", NULL );
          SQL_Write ( requete );
          g_free(requete);

          g_snprintf( chaine, sizeof(chaine), "DELETE FROM mnemos_BOOL WHERE  deletable=1 AND tech_id='%s' AND acronyme NOT IN ", tech_id );
          requete = g_strconcat ( chaine, "(", Liste_acronyme, ")", NULL );
          SQL_Write ( requete );
          g_free(requete);

          g_snprintf( chaine, sizeof(chaine), "DELETE FROM mnemos_Tempo WHERE tech_id='%s' AND acronyme NOT IN ", tech_id );
          requete = g_strconcat ( chaine, "(", Liste_acronyme, ")", NULL );
          SQL_Write ( requete );
          g_free(requete);

          g_snprintf( chaine, sizeof(chaine), "DELETE FROM mnemos_CI WHERE tech_id='%s' AND acronyme NOT IN ", tech_id );
          requete = g_strconcat ( chaine, "(", Liste_acronyme, ")", NULL );
          SQL_Write ( requete );
          g_free(requete);

          g_snprintf( chaine, sizeof(chaine), "DELETE FROM mnemos_CH WHERE tech_id='%s' AND acronyme NOT IN ", tech_id );
          requete = g_strconcat ( chaine, "(", Liste_acronyme, ")", NULL );
          SQL_Write ( requete );
          g_free(requete);

          g_snprintf( chaine, sizeof(chaine), "DELETE FROM msgs WHERE tech_id='%s' AND acronyme NOT IN ", tech_id );
          requete = g_strconcat ( chaine, "(", Liste_acronyme, ")", NULL );
          SQL_Write ( requete );
          g_free(requete);

          g_snprintf( chaine, sizeof(chaine), "DELETE FROM mnemos_HORLOGE WHERE tech_id='%s' AND acronyme NOT IN ", tech_id );
          requete = g_strconcat ( chaine, "(", Liste_acronyme, ")", NULL );
          SQL_Write ( requete );
          g_free(requete);

          g_free(Liste_acronyme);
        }
     }
    close(Id_log);
    Liberer_memoire();
    g_free(Buffer);
    Buffer = NULL;
    pthread_mutex_unlock( &Partage->com_dls.synchro_traduction );                                         /* Libération Mutex */
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
