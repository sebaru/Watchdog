/******************************************************************************************************************************/
/* Watchdogd/TraductionDLS/Interp.c          Interpretation du langage DLS                                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          dim 05 avr 2009 12:47:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Interp.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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

 #include "watchdogd.h"
 #include "lignes.h"

 static GSList *Alias=NULL;                                                  /* Liste des alias identifi�s dans le source DLS */
 static GSList *Liste_Actions_bit    = NULL;                              /* Liste des actions rencontr�es dans le source DLS */
 static GSList *Liste_Actions_num    = NULL;                              /* Liste des actions rencontr�es dans le source DLS */
 static GSList *Liste_Actions_msg    = NULL;                              /* Liste des actions rencontr�es dans le source DLS */
 static GSList *Liste_edge_up_bi     = NULL;                               /* Liste des bits B utilis�s avec l'option EDGE_UP */
 static GSList *Liste_edge_up_entree = NULL;                               /* Liste des bits E utilis�s avec l'option EDGE_UP */
 static gchar *Buffer=NULL;
 static gint Buffer_used=0, Buffer_taille=0;
 static int Id_log;                                                                     /* Pour la creation du fichier de log */
 static int nbr_erreur;
 static struct CMD_TYPE_PLUGIN_DLS Dls_plugin;

/******************************************************************************************************************************/
/* New_chaine: Alloue une certaine quantit� de m�moire pour utiliser des chaines de caract�res                                */
/* Entr�es: la longueur souhait�e                                                                                             */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 char *New_chaine( int longueur )
  { char *chaine;
    chaine = g_try_malloc0( longueur );
    if (!chaine) { return(NULL); }
    return(chaine);
  }
/******************************************************************************************************************************/
/* Emettre: Met a jour le fichier temporaire en code interm�diaire                                                            */
/* Entr�es: la ligne d'instruction � mettre                                                                                   */
/* Sortie: void                                                                                                               */
/******************************************************************************************************************************/
 void Emettre( char *chaine )
  { int taille;
    taille = strlen(chaine);
    if ( Buffer_used + taille > Buffer_taille)
     { gchar *new_Buffer;
       Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                "%s: buffer too small, trying to expand it to %d", __func__, Buffer_taille + 1024 );
       new_Buffer = g_try_realloc( Buffer, Buffer_taille + 1024 );
       if (!new_Buffer)
        { Info_new( Config.log, Config.log_dls, LOG_ERR,
                   "%s: Fail to expand buffer. skipping", __func__ );
        }
       Buffer = new_Buffer;
       Buffer_taille = Buffer_taille + 1024;
       Info_new( Config.log, Config.log_dls, LOG_INFO,
                "%s: Buffer expanded to %d bytes", __func__, Buffer_taille );
     }
    Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s: ligne %d : %s", __func__, DlsScanner_get_lineno(), chaine );
    memcpy ( Buffer + Buffer_used, chaine, taille );                                          /* Recopie du bout de buffer */
    Buffer_used += taille;
  }
/******************************************************************************************************************************/
/* DlsScanner_error: Appell� par le scanner en cas d'erreur de syntaxe (et non une erreur de grammaire !)                     */
/* Entr�e : la chaine source de l'erreur de syntaxe                                                                           */
/* Sortie : appel de la fonction Emettre_erreur_new en backend                                                                */
/******************************************************************************************************************************/
 int DlsScanner_error ( char *s )
  { Emettre_erreur_new( "Ligne %d: %s", DlsScanner_get_lineno(), s );
    return(0);
  }
/******************************************************************************************************************************/
/* Emettre_erreur_new: collecte des erreurs de traduction D.L.S                                                               */
/* Entr�e: le num�ro de ligne, le format et les param�tres associ�s                                                           */
/******************************************************************************************************************************/
 void Emettre_erreur_new( gchar *format, ... )
  { static gchar *too_many="Too many events. Limiting output...\n";
    gchar log[256], chaine[256];
    va_list ap;

    if (nbr_erreur<15)
     { va_start( ap, format );
       g_vsnprintf( chaine, sizeof(chaine), format, ap );
       va_end ( ap );
       g_snprintf( log, sizeof(log), "%s\n", chaine );
       write( Id_log, log, strlen(log) );

       Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: Ligne %d : %s", __func__, DlsScanner_get_lineno(), chaine );
     }
    else if (nbr_erreur==15)
     { write( Id_log, too_many, strlen(too_many)+1 ); }
    nbr_erreur++;
  }
/******************************************************************************************************************************/
/* New_option: Alloue une certaine quantit� de m�moire pour les options                                                       */
/* Entr�es: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct COMPARATEUR *New_comparateur( void )
  { struct COMPARATEUR *comparateur;
    comparateur=(struct COMPARATEUR *)g_try_malloc0( sizeof(struct COMPARATEUR) );
    return(comparateur);
  }
/******************************************************************************************************************************/
/* New_option: Alloue une certaine quantit� de m�moire pour les options                                                       */
/* Entr�es: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct OPTION *New_option( void )
  { struct OPTION *option;
    option=(struct OPTION *)g_try_malloc0( sizeof(struct OPTION) );
    return(option);
  }
/******************************************************************************************************************************/
/* Get_option_entier: Cherche une option et renvoie sa valeur                                                                 */
/* Entr�es: la liste des options, le type a rechercher                                                                        */
/* Sortie: -1 si pas trouv�                                                                                                   */
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
/* Entr�es: la liste des options, le type a rechercher                                                                        */
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
    return("no string");
  }
/******************************************************************************************************************************/
/* Check_msg_ownership: V�rifie la propri�t� du bit interne MSG en action                                                     */
/* Entr�es: le num�ro du message positionn� en action dans la ligne dls                                                       */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Check_msg_ownership ( gint num )
  { struct CMD_TYPE_MESSAGE *message;
    gchar chaine[80];
    gboolean retour;
    retour = FALSE;
    message = Rechercher_messageDB ( num );
    Info_new( Config.log, Config.log_dls, LOG_DEBUG,
             "%s: Test Message %d for id %d: mnemo %p", __func__, num, Dls_plugin.id, message ); 
    if (message)
     { if (message->dls_id == Dls_plugin.id) retour=TRUE;
       g_free(message);
     }
    
    if(retour == FALSE)
     { g_snprintf( chaine, sizeof(chaine), "Ligne %d: MSG%04d not owned by plugin", DlsScanner_get_lineno(), num );
       Emettre_erreur_new( "%s", chaine );
       return(FALSE);
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* New_condition_bi: Prepare la chaine de caractere associ�e � la condition, en respectant les options                        */
/* Entr�es: numero du bit bistable et sa liste d'options                                                                      */
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
/* New_condition_bi: Prepare la chaine de caractere associ�e � la condition, en respectant les options                        */
/* Entr�es: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_bi( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    if (alias->num != -1) /* Alias par num�ro ? */
     { return(New_condition_bi_old( barre, alias->num, options)); }
    else /* Alias par nom */
     { taille = 100;
       result = New_chaine( taille ); /* 10 caract�res max */
       if ( (!barre && !alias->barre) || (barre && alias->barre) )
            { g_snprintf( result, taille, "Dls_data_get_bool ( \"%s\", \"%s\", &_B_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
            }
       else { g_snprintf( result, taille, "!Dls_data_get_bool ( \"%s\", \"%s\", &_B_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
            }
     }
    return(result);
  }
/******************************************************************************************************************************/
/* New_condition_bi: Prepare la chaine de caractere associ�e � la condition, en respectant les options                        */
/* Entr�es: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_entree_old( int barre, int num, GList *options )
  { gchar *result;
    gint taille;
    taille = 24;
    result = New_chaine( taille );
    if (Get_option_entier( options, T_EDGE_UP) == 1)
     { if ( g_slist_find ( Liste_edge_up_entree, GINT_TO_POINTER(num) ) == NULL )
        { Liste_edge_up_entree = g_slist_prepend ( Liste_edge_up_entree, GINT_TO_POINTER(num) ); }
       if (barre) g_snprintf( result, taille, "!E%d_edge_up_value", num );
             else g_snprintf( result, taille, "E%d_edge_up_value", num );
     }
    else { if (barre) g_snprintf( result, taille, "!E(%d)", num );
                 else g_snprintf( result, taille, "E(%d)", num );
         }
    return(result);
  }
/******************************************************************************************************************************/
/* New_condition_bi: Prepare la chaine de caractere associ�e � la condition, en respectant les options                        */
/* Entr�es: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_entree( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    if (alias->num != -1) /* Alias par num�ro ? */
     { return(New_condition_entree_old( barre, alias->num, options)); }
    else /* Alias par nom */
     { taille = 100;
       result = New_chaine( taille ); /* 10 caract�res max */
       if ( (!barre && !alias->barre) || (barre && alias->barre) )
            { g_snprintf( result, taille, "Dls_data_get_bool ( \"%s\", \"%s\", &_E_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
            }
       else { g_snprintf( result, taille, "!Dls_data_get_bool ( \"%s\", \"%s\", &_E_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
            }
     }
    return(result);
  }
/******************************************************************************************************************************/
/* New_condition_mono: Prepare la chaine de caractere associ�e � la condition, en respectant les options                      */
/* Entr�es: l'alias du monostable et sa liste d'options                                                                       */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_mono( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    if (alias->num != -1) /* Alias par num�ro ? */
     { taille = 15;
       result = New_chaine( taille ); /* 10 caract�res max */
       if ( (!barre && !alias->barre) || (barre && alias->barre) )
            { g_snprintf( result, taille, "M(%d)", alias->num ); }
       else { g_snprintf( result, taille, "!M(%d)", alias->num ); }
     }
    else /* Alias par nom */
     { taille = 100;
       result = New_chaine( taille ); /* 10 caract�res max */
       if ( (!barre && !alias->barre) || (barre && alias->barre) )
            { g_snprintf( result, taille, "Dls_data_get_bool ( \"%s\", \"%s\", &_M_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
            }
       else { g_snprintf( result, taille, "!Dls_data_get_bool ( \"%s\", \"%s\", &_M_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
            }
     }
   return(result);
 }
/******************************************************************************************************************************/
/* New_condition_tempo: Prepare la chaine de caractere associ�e � la condition, en respectant les options                     */
/* Entr�es: l'alias de la temporisatio et sa liste d'options                                                                  */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_tempo( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    taille = 128;
    result = New_chaine( taille );
    if ( alias->type == ALIAS_TYPE_DYNAMIC)
     { g_snprintf( result, taille, "%sDls_data_get_tempo ( \"%s\", \"%s\", &_T_%s_%s )",
                   (barre==1 ? "!" : ""), alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    else
     { g_snprintf( result, taille, "%sT(%d)",
                   (barre==1 ? "!" : ""), alias->num );
     }
    return(result);
  }
/******************************************************************************************************************************/
/* New_condition_horloge: Prepare la chaine de caractere associ�e � la condition, en respectant les options                   */
/* Entr�es: l'alias de l'horloge et sa liste d'options                                                                        */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_horloge( int barre, struct ALIAS *alias, GList *options )
  { gchar *result;
    gint taille;
    taille = 100;                                                                               /* Alias par nom uniquement ! */
    result = New_chaine( taille ); /* 10 caract�res max */
    if ( !barre )
         { g_snprintf( result, taille, "Dls_data_get_bool ( \"%s\", \"%s\", &_HOR_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
    else { g_snprintf( result, taille, "!Dls_data_get_bool ( \"%s\", \"%s\", &_HOR_%s_%s )",
                          alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
   return(result);
 }
/******************************************************************************************************************************/
/* New_condition_vars: formate une condition avec le nom de variable en parametre                                             */
/* Entr�es: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 gchar *New_condition_vars( int barre, gchar *nom )
  { gchar *result;
    int taille;

    taille = strlen(nom)+5;
    result = New_chaine( taille ); /* 10 caract�res max */
    if (!barre) { g_snprintf( result, taille, "%s", nom ); }
           else { g_snprintf( result, taille, "!%s", nom ); }
    return(result);
  }
/******************************************************************************************************************************/
/* New_action: Alloue une certaine quantit� de m�moire pour les actions DLS                                                   */
/* Entr�es: rien                                                                                                              */
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
/* New_action_msg: Prepare une struct action avec une commande MSG                                                            */
/* Entr�es: numero du message                                                                                                 */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_msg( int num )
  { struct ACTION *action;
    GSList *liste;
    int taille;

    taille = 15;
    liste = Liste_Actions_msg;
    while (liste)
     { if (GPOINTER_TO_INT(liste->data) == num) break;
       liste=liste->next;
     }
    if(!liste)
     { Liste_Actions_msg = g_slist_prepend ( Liste_Actions_msg, GINT_TO_POINTER(num) );
       Check_msg_ownership ( num );
     }
    action = New_action();
    action->alors = New_chaine( taille );
    g_snprintf( action->alors, taille, "MSG(%d,1);", num );
    action->sinon = New_chaine( taille );
    g_snprintf( action->sinon, taille, "MSG(%d,0);", num );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_msg_by_alias: Prepare une struct action avec une commande de type MSG                                           */
/* Entr�es: L'alias decouvert                                                                                                 */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_msg_by_alias( struct ALIAS *alias )
  { struct ACTION *action;
    int taille;

    if (alias->type == ALIAS_TYPE_STATIC)                                                               /* Alias par num�ro ? */
     { return(New_action_msg ( alias->num )); }

    taille = 100;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "Dls_data_set_MSG ( \"%s\", \"%s\", &_MSG_%s_%s, TRUE );",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    g_snprintf( action->sinon, taille, "Dls_data_set_MSG ( \"%s\", \"%s\", &_MSG_%s_%s, FALSE );",
                alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
    return(action);
  }
/******************************************************************************************************************************/
/* Add_bit_to_list: Ajoute un bit dans la liste des bits utilis�                                                              */
/* Entr�es: le type de bit et son num�ro                                                                                      */
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
/* New_action_sortie: Prepare une struct action avec une commande SA                                                          */
/* Entr�es: numero de la sortie, sa logique                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 static struct ACTION *New_action_sortie_old( int num, int barre )
  { struct ACTION *action;
    int taille;

    taille = 20;
    Add_bit_to_list(MNEMO_SORTIE, num);
    action = New_action();
    action->alors = New_chaine( taille );
    g_snprintf( action->alors, taille, "SA(%d,%d);", num, !barre );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_sortie: Prepare la structure ACTION associ�e � l'alias en paremetre                                             */
/* Entr�es: l'alias, le complement si besoin, les options                                                                     */
/* Sortie: la structure ACTION associ�e                                                                                       */
/******************************************************************************************************************************/
 struct ACTION *New_action_sortie( struct ALIAS *alias, int barre, GList *options )
  { if (alias->num != -1) /* Alias par num�ro ? */
     { return(New_action_sortie_old( alias->num, barre )); }
    /* Alias par nom */
    struct ACTION *action = New_action();
    gint taille = 128;
    action->alors = New_chaine( taille );
    g_snprintf( action->alors, taille, "SA(%d,%d);", alias->num, !barre );
    if ( (!barre && !alias->barre) || (barre && alias->barre) )
         { g_snprintf( action->alors, taille, "Dls_data_set_bool ( \"%s\", \"%s\", &_A_%s_%s, 1 ); ",
                       alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
    else { g_snprintf( action->alors, taille, "Dls_data_set_bool ( \"%s\", \"%s\", &_A_%s_%s, 0 ); ",
                       alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
         }
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_vars_mono: Prepare une struct action avec une commande SM                                                       */
/* Entr�es: numero du monostable, sa logique                                                                                  */
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
/* Entr�es: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_mono_by_alias( struct ALIAS *alias )
  { struct ACTION *action;
    int taille;

    if (alias->type == ALIAS_TYPE_STATIC)                                                               /* Alias par num�ro ? */
     { taille = 15;
       Add_bit_to_list(MNEMO_MONOSTABLE, alias->num);
       action = New_action();
       action->alors = New_chaine( taille );
       action->sinon = New_chaine( taille );

       g_snprintf( action->alors, taille, "SM(%d,1);", alias->num );
       g_snprintf( action->sinon, taille, "SM(%d,0);", alias->num );
     }
    else /* Alias par nom */
     { taille = 100;
       action = New_action();
       action->alors = New_chaine( taille );
       action->sinon = New_chaine( taille );

       g_snprintf( action->alors, taille, "Dls_data_set_bool ( \"%s\", \"%s\", &_M_%s_%s, TRUE );",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
       g_snprintf( action->sinon, taille, "Dls_data_set_bool ( \"%s\", \"%s\", &_M_%s_%s, FALSE );",
                   alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme );
     }
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entr�es: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_cpt_h( int num, GList *options )
  { struct ACTION *action;
    int taille, reset;

    reset = Get_option_entier ( options, RESET ); if (reset == -1) reset = 0;
    taille = 15;
    Add_bit_to_list(MNEMO_CPTH, num);
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "SCH(%d,1,%d);", num, reset );
    g_snprintf( action->sinon, taille, "SCH(%d,0,%d);", num, reset );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_cpt_imp: Prepare une struct action avec une commande SCI                                                        */
/* Entr�es: numero du compteur d'impulsion, sa logique, son reset                                                             */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_cpt_imp( int num, GList *options )
  { struct ACTION *action;
    int taille, reset, ratio;
    reset = Get_option_entier ( options, RESET ); if (reset == -1) reset = 0;
    ratio = Get_option_entier ( options, RATIO ); if (ratio == -1) ratio = 1;

    taille = 20;
    Add_bit_to_list(MNEMO_CPT_IMP, num);
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "SCI(%d,1,%d,%d);", num, reset, ratio );
    g_snprintf( action->sinon, taille, "SCI(%d,0,%d,%d);", num, reset, ratio );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_icone: Prepare une struct action avec une commande SI                                                           */
/* Entr�es: numero du motif                                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_icone( int num, GList *options )
  { struct ACTION *action;
    int taille, rouge, vert, bleu, val, coul, cligno;

    val    = Get_option_entier ( options, MODE   ); if (val    == -1) val = 0;
    coul   = Get_option_entier ( options, COLOR  ); if (coul   == -1) coul = 0;
    cligno = Get_option_entier ( options, CLIGNO ); if (cligno == -1) cligno = 0;
    taille = 128;
    Add_bit_to_list(MNEMO_MOTIF, num);
    action = New_action();
    action->alors = New_chaine( taille );
    switch (coul)
     { case ROUGE   : rouge = 255; vert =   0; bleu =   0; break;
       case VERT    : rouge =   0; vert = 255; bleu =   0; break;
       case BLEU    : rouge =   0; vert =   0; bleu = 255; break;
       case JAUNE   : rouge = 255; vert = 255; bleu =   0; break;
       case ORANGE  : rouge = 255; vert = 190; bleu =   0; break;
       case BLANC   : rouge = 255; vert = 255; bleu = 255; break;
       case GRIS    : rouge = 127; vert = 127; bleu = 127; break;
       case KAKI    : rouge =   0; vert = 100; bleu =   0; break;
       default      : rouge = vert = bleu = 0;
     }
    g_snprintf( action->alors, taille,
               " if (vars->bit_comm_out) SI(%d, 0, 0, 100, 0, 1); else SI(%d,%d,%d,%d,%d,%d);",
                num, num, val, rouge, vert, bleu, cligno );
    return(action);
  }

/******************************************************************************************************************************/
/* New_action_tempo: Prepare une struct action avec une commande TR                                                           */
/* Entr�es: numero de la tempo, sa consigne                                                                                   */
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
    taille = 128;
    if (alias->type == ALIAS_TYPE_DYNAMIC)
     { action->alors = New_chaine( taille );
       g_snprintf( action->alors, taille, "Dls_data_set_tempo ( \"%s\", \"%s\", &_T_%s_%s, 1, %d, %d, %d, %d, %d );",
                                           alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                                           daa, dma, dMa, dad, random );
       action->sinon = New_chaine( taille );
       g_snprintf( action->sinon, taille, "Dls_data_set_tempo ( \"%s\", \"%s\", &_T_%s_%s, 0, %d, %d, %d, %d, %d );",
                                           alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme,
                                           daa, dma, dMa, dad, random );
     }
    else
     { taille = 40;
       action->alors = New_chaine( taille );
       g_snprintf( action->alors, taille, "ST(%d,1);", alias->num );
       action->sinon = New_chaine( taille );
       g_snprintf( action->sinon, taille, "ST(%d,0);", alias->num );
    }
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entr�es: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_bi_by_alias( struct ALIAS *alias, gint barre )
  { struct ACTION *action;
    int taille;

    action = New_action();
    if (alias->type == ALIAS_TYPE_STATIC)                                                               /* Alias par num�ro ? */
     { taille = 20;
       Add_bit_to_list(MNEMO_BISTABLE, alias->num);
       action->alors = New_chaine( taille );
       g_snprintf( action->alors, taille, "SB(%d,%d);", alias->num, !barre );
     }
    else /* Alias par nom */
     { taille = 100;
       action = New_action();
       action->alors = New_chaine( taille );
       g_snprintf( action->alors, taille, "Dls_data_set_bool ( \"%s\", \"%s\", &_B_%s_%s, %d );",
                                           alias->tech_id, alias->acronyme, alias->tech_id, alias->acronyme, !barre );
     }
    return(action);
  }
/******************************************************************************************************************************/
/* New_alias: Alloue une certaine quantit� de m�moire pour utiliser des alias                                                 */
/* Entr�es: le nom de l'alias, le tableau et le numero du bit                                                                 */
/* Sortie: False si il existe deja, true sinon                                                                                */
/******************************************************************************************************************************/
 gboolean New_alias( gint type, gchar *tech_id, gchar *acronyme, gint bit, gint num, gint barre, GList *options )
  { struct ALIAS *alias;

    if (Get_alias_par_acronyme( tech_id, acronyme )) return(FALSE);                                           /* ID deja definit ? */

    alias=(struct ALIAS *)g_try_malloc0( sizeof(struct ALIAS) );
    if (!alias) { return(FALSE); }
    alias->type = type;
    if (!tech_id) alias->tech_id = g_strdup(Dls_plugin.tech_id);
             else alias->tech_id = g_strdup(tech_id);
    alias->acronyme = g_strdup(acronyme);
    alias->bit      = bit;
    alias->num      = num;
    alias->barre    = barre;
    alias->options  = options;
    alias->used     = 0;
    Alias = g_slist_prepend( Alias, alias );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* New_alias: Alloue une certaine quantit� de m�moire pour utiliser des alias                                                 */
/* Entr�es: le nom de l'alias, le tableau et le numero du bit                                                                 */
/* Sortie: False si il existe deja, true sinon                                                                                */
/******************************************************************************************************************************/
 struct ALIAS *Set_new_external_alias( gchar *tech_id, gchar *acronyme )
  { struct CMD_TYPE_MNEMO_BASE *mnemo;
    struct ALIAS *alias;

    alias=(struct ALIAS *)g_try_malloc0( sizeof(struct ALIAS) );
    if (!alias) { return(NULL); }

    if (!strcmp(tech_id,"THIS")) tech_id=Dls_plugin.tech_id;

    mnemo = Rechercher_mnemo_baseDB_by_acronyme ( tech_id, acronyme );
    if (!mnemo)
     { g_free(alias);
       Emettre_erreur_new( "Bit %s:%s not found", tech_id, acronyme );
       return(NULL);
     }
    alias->type     = ALIAS_TYPE_DYNAMIC;
    alias->tech_id  = g_strdup(mnemo->dls_tech_id);
    alias->acronyme = g_strdup(mnemo->acronyme);
    alias->bit      = mnemo->type;
    alias->num      = -1;
    alias->barre    = 0;
    alias->options  = NULL;
    alias->used     = 1;
    Alias = g_slist_prepend( Alias, alias );
    g_free(mnemo);
    return(alias);
  }
/******************************************************************************************************************************/
/* Get_alias: Recherche un alias donn� en param�tre                                                                           */
/* Entr�es: le nom de l'alias                                                                                                 */
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
/* Liberer_alias: Liberation de toutes les zones de m�moire pr�c�demment allou�es                                             */
/* Entr�es: kedal                                                                                                             */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Liberer_options ( GList *options )
  { while (options)
     { struct OPTION *option = (struct OPTION *)options->data;
       options = g_list_remove (options, option);
       switch (option->type)
        { case T_LIBELLE: g_free(option->chaine); break;
          case T_ETIQUETTE: g_free(option->chaine); break;
        }
       g_free(option);
     }
  }
/******************************************************************************************************************************/
/* Liberer_alias: Liberation de toutes les zones de m�moire pr�c�demment allou�es                                             */
/* Entr�es: kedal                                                                                                             */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Liberer_alias ( struct ALIAS *alias )
  { GList *liste;
    Liberer_options(alias->options);
    g_free(alias->tech_id);
    g_free(alias->acronyme);
    g_free(alias);
  }
/******************************************************************************************************************************/
/* Liberer_memoire: Liberation de toutes les zones de m�moire pr�c�demment allou�es                                           */
/* Entr�es: kedal                                                                                                             */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Liberer_memoire( void )
  { g_slist_foreach( Alias, (GFunc) Liberer_alias, NULL );
    g_slist_free( Alias );
    Alias = NULL;
    g_slist_free(Liste_Actions_msg);    Liste_Actions_msg    = NULL;
    g_slist_free(Liste_Actions_bit);    Liste_Actions_bit    = NULL;
    g_slist_free(Liste_Actions_num);    Liste_Actions_num    = NULL;
    g_slist_free(Liste_edge_up_bi);     Liste_edge_up_bi     = NULL;
    g_slist_free(Liste_edge_up_entree); Liste_edge_up_entree = NULL;
  }
/******************************************************************************************************************************/
/* Trad_dls_set_debug: Positionne le flag de debug Bison/Flex                                                                 */
/* Entr�e : TRUE ou FALSE                                                                                                     */
/******************************************************************************************************************************/
 void Trad_dls_set_debug ( gboolean actif )
  { DlsScanner_debug = actif; }                                                                   /* Debug de la traduction ?? */
/******************************************************************************************************************************/
/* Traduire: Traduction du fichier en param�tre du langage DLS vers le langage C                                              */
/* Entr�e: l'id du modul                                                                                                      */
/* Sortie: TRAD_DLS_OK, _WARNING ou _ERROR                                                                                    */
/******************************************************************************************************************************/
 gint Traduire_DLS( int id )
  { gchar source[80], cible[80], log[80];
    struct CMD_TYPE_PLUGIN_DLS *plugin;
    struct ALIAS *alias;
    GSList *liste;
    gint retour, nb_car;
    FILE *rc;

    plugin = Rechercher_plugin_dlsDB ( id );
    if (!plugin) return (TRAD_DLS_ERROR);
    memcpy ( &Dls_plugin, plugin, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
    g_free(plugin);
    
    Buffer_taille = 1024;
    Buffer = g_try_malloc0( Buffer_taille );                                             /* Initialisation du buffer resultat */
    if (!Buffer) return ( TRAD_DLS_ERROR );
    Buffer_used = 0;
    
    g_snprintf( source, sizeof(source), "Dls/%06d.dls", id );
    g_snprintf( log,    sizeof(log),    "Dls/%06d.log", id );
    g_snprintf( cible,  sizeof(cible),  "Dls/%06d.c", id );
    unlink ( log );
    Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s: id=%d, source=%s, log=%s", __func__, id, source, log );

    Id_log = open( log, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (Id_log<0)
     { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                "%s: Log creation failed %s (%s)", __func__, log, strerror(errno) ); 
       close(Id_log);
       return(TRAD_DLS_ERROR_FILE);
     }

    pthread_mutex_lock( &Partage->com_dls.synchro_traduction );                           /* Attente unicit� de la traduction */

    Alias = NULL;                                                                                  /* Par d�faut, pas d'alias */
    Liste_Actions_bit = NULL;                                                                    /* Par d�faut, pas d'actions */
    Liste_Actions_num = NULL;                                                                    /* Par d�faut, pas d'actions */
    Liste_Actions_msg = NULL;                                                                    /* Par d�faut, pas d'actions */
    Liste_edge_up_bi  = NULL;                                               /* Liste des bits B utilis� avec l'option EDGE UP */
    DlsScanner_set_lineno(1);                                                                     /* Reset du num�ro de ligne */
    nbr_erreur = 0;                                                                   /* Au d�part, nous n'avons pas d'erreur */
    rc = fopen( source, "r" );
    if (!rc) retour = TRAD_DLS_ERROR;
    else
     { DlsScanner_debug = 0;                                                                     /* Debug de la traduction ?? */
       DlsScanner_restart(rc);
       DlsScanner_parse();                                                                       /* Parsing du fichier source */
       fclose(rc);
     }

    if (nbr_erreur)
     { Emettre_erreur_new( "%d error%s found", nbr_erreur, (nbr_erreur>1 ? "s" : "") );
       retour = TRAD_DLS_ERROR;
     }
    else
     { gint fd;
       Emettre_erreur_new( "No error found" );                        /* Pas d'erreur rencontr� (mais peu etre des warning !) */
       retour = TRAD_DLS_OK;

       unlink ( cible );
       fd = open( cible, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );             /* Enregistrement du buffer resultat sur disque */
       if (fd<0)
        { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                   "%s: Target creation failed %s (%s)", __func__, cible, strerror(errno) ); 
          retour = TRAD_DLS_ERROR_FILE;
        }
       else
        { gchar *include = " #include <Module_dls.h>\n";
          gchar *Chaine_bit= " static gint Tableau_bit[]= { ";
          gchar *Chaine_num= " static gint Tableau_num[]= { ";
          gchar *Chaine_msg= " static gint Tableau_msg[]= { ";
          gchar *Tableau_end=" -1 };\n";
          gchar *Fonction= " gint Get_Tableau_bit(int n) { return(Tableau_bit[n]); }\n"
                           " gint Get_Tableau_num(int n) { return(Tableau_num[n]); }\n"
                           " gint Get_Tableau_msg(int n) { return(Tableau_msg[n]); }\n";
          gchar *Start_Go = " void Go ( struct DLS_TO_PLUGIN *vars )\n"
                            "  {\n"
                            "    Update_edge_up_value();\n"
                            "    if (vars->debug) Dls_print_debug( Dls_id, (int *)&Tableau_bit, (int *)&Tableau_num, (float *)&Tableau_val );\n";
          gchar *End_Go =   "  }\n";
          gchar chaine[4096];
          gint cpt=0;                                                                                   /* Compteur d'actions */

          write(fd, include, strlen(include));

          liste = Alias;
          while(liste)
           { alias = (struct ALIAS *)liste->data;
             if (alias->type == ALIAS_TYPE_DYNAMIC)                      /* alias par nom ? creation du pointeur de raccourci */
              { switch (alias->bit)
                 { case MNEMO_MONOSTABLE:
                        nb_car = g_snprintf(chaine, sizeof(chaine), " gpointer _M_%s_%s;\n", alias->tech_id, alias->acronyme );
                        write (fd, chaine, nb_car);
                        break;
                   case MNEMO_BISTABLE:
                        nb_car = g_snprintf(chaine, sizeof(chaine), " gpointer _B_%s_%s;\n", alias->tech_id, alias->acronyme );
                        write (fd, chaine, nb_car);
                        break;
                   case MNEMO_ENTREE:
                        nb_car = g_snprintf(chaine, sizeof(chaine), " gpointer _E_%s_%s;\n", alias->tech_id, alias->acronyme );
                        write (fd, chaine, nb_car);
                        break;
                   case MNEMO_SORTIE:
                        nb_car = g_snprintf(chaine, sizeof(chaine), " gpointer _A_%s_%s;\n", alias->tech_id, alias->acronyme );
                        write (fd, chaine, nb_car);
                        break;
                   case MNEMO_TEMPO:
                        nb_car = g_snprintf(chaine, sizeof(chaine), " gpointer _T_%s_%s;\n", alias->tech_id, alias->acronyme );
                        write (fd, chaine, nb_car);
                        break;
                   case MNEMO_HORLOGE:
                        nb_car = g_snprintf(chaine, sizeof(chaine), " gpointer _HOR_%s_%s;\n", alias->tech_id, alias->acronyme );
                        write (fd, chaine, nb_car);
                        break;
                   case MNEMO_MSG:
                        nb_car = g_snprintf(chaine, sizeof(chaine), " gpointer _MSG_%s_%s;\n", alias->tech_id, alias->acronyme );
                        write (fd, chaine, nb_car);
                        break;
                 }
              }
             liste = liste->next;
           }

     
          cpt = g_slist_length(Liste_Actions_bit);
          if (cpt==0) cpt=1;
          g_snprintf( chaine, sizeof(chaine), " static gfloat Tableau_val[%d];\n", cpt );
          write(fd, chaine, strlen(chaine) );                                                         /* Ecriture du prologue */

          g_snprintf( chaine, sizeof(chaine), " static gint Dls_id = %d;\n", id );
          write(fd, chaine, strlen(chaine) );                                                         /* Ecriture du prologue */

          write(fd, Chaine_bit, strlen(Chaine_bit) );                                                 /* Ecriture du prologue */
          liste = Liste_Actions_bit;                                       /* Initialise les tableaux des actions rencontr�es */
          while(liste)
           { gchar chaine[12];
             g_snprintf(chaine, sizeof(chaine), "%d, ", GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             liste = liste->next;
           }
          write(fd, Tableau_end, strlen(Tableau_end) );                                               /* Ecriture du prologue */

          write(fd, Chaine_num, strlen(Chaine_num) );                                                 /* Ecriture du prologue */
          liste = Liste_Actions_num;                                       /* Initialise les tableaux des actions rencontr�es */
          while(liste)
           { gchar chaine[12];
             g_snprintf(chaine, sizeof(chaine), "%d, ", GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             liste = liste->next;
           }
          write(fd, Tableau_end, strlen(Tableau_end) );                                               /* Ecriture du prologue */

          write(fd, Chaine_msg, strlen(Chaine_msg) );                                                 /* Ecriture du prologue */
          liste = Liste_Actions_msg;                                       /* Initialise les tableaux des actions rencontr�es */
          while(liste)
           { gchar chaine[12];
             g_snprintf(chaine, sizeof(chaine), "%d, ", GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             liste = liste->next;
           }
          write(fd, Tableau_end, strlen(Tableau_end) );                                               /* Ecriture du prologue */

          write(fd, Fonction, strlen(Fonction) );                                                     /* Ecriture du prologue */

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

          g_snprintf(chaine, sizeof(chaine), "    if (vars->starting)\n     {\n" );
          write( fd, chaine, strlen(chaine) );                                                     /* Ecriture de de l'entete */
          liste = Liste_Actions_msg;                               /* Initialise les fonctions de gestion des fronts montants */
          while(liste)
           { gchar chaine[128];
             g_snprintf(chaine, sizeof(chaine), "       MSG(%d,0);\n", GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             liste = liste->next;
           }
          g_snprintf(chaine, sizeof(chaine), "     }\n" );
          write(fd, chaine, strlen(chaine) );                                                         /* Ecriture du prologue */

          write(fd, Buffer, Buffer_used );                                                     /* Ecriture du buffer resultat */
          write( fd, End_Go, strlen(End_Go) );
          close(fd);
        }

       liste = Alias;                                           /* Lib�ration des alias, et remont� d'un Warning si il y en a */
       while(liste)
        { struct CMD_TYPE_MNEMO_FULL mnemo;
          alias = (struct ALIAS *)liste->data;
          if ( (!alias->used) )
           { Emettre_erreur_new( "Warning: %s not used", alias->acronyme );
             retour = TRAD_DLS_WARNING;
           }
          if (alias->type == ALIAS_TYPE_DYNAMIC && !strcmp(alias->tech_id, Dls_plugin.tech_id))/* Alias Dynamiques uniquement */
           { switch(alias->bit)
              { case MNEMO_MSG:
                 { struct CMD_TYPE_MESSAGE msg;
                   g_snprintf( msg.acronyme, sizeof(msg.acronyme), "%s", alias->acronyme );
                   g_snprintf( msg.libelle,  sizeof(msg.libelle), "%s", Get_option_chaine( alias->options, T_LIBELLE ) );
                   msg.dls_id = Dls_plugin.id;
                   msg.type = Get_option_entier ( alias->options, T_TYPE );
                   Ajouter_messageDB_for_dls ( &msg );
                   break;
                 }
                default: g_snprintf( mnemo.mnemo_base.acronyme, sizeof(mnemo.mnemo_base.acronyme), "%s", alias->acronyme );
                         g_snprintf( mnemo.mnemo_base.libelle,  sizeof(mnemo.mnemo_base.libelle),
                                     "%s", Get_option_chaine( alias->options, T_LIBELLE ) );
                         g_snprintf( mnemo.mnemo_base.acro_syn, sizeof(mnemo.mnemo_base.acro_syn),
                                     "%s", Get_option_chaine( alias->options, T_ETIQUETTE ) );
                         mnemo.mnemo_base.dls_id = Dls_plugin.id;
                         mnemo.mnemo_base.type = alias->bit;
                         Mnemo_auto_create_for_dls ( &mnemo );
                         break;
              }
           }
          liste = liste->next;
        }
     }
    close(Id_log);
    Liberer_memoire();
    g_free(Buffer);
    Buffer = NULL;
    pthread_mutex_unlock( &Partage->com_dls.synchro_traduction );                                         /* Lib�ration Mutex */
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
