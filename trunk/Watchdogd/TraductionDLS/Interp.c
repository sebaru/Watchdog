/******************************************************************************************************************************/
/* Watchdogd/TraductionDLS/Interp.c          Interpretation du langage DLS                                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          dim 05 avr 2009 12:47:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Interp.c
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

 static GSList *Alias=NULL;                                                  /* Liste des alias identifiés dans le source DLS */
 static GSList *Liste_Actions_bit    = NULL;                              /* Liste des actions rencontrées dans le source DLS */
 static GSList *Liste_Actions_num    = NULL;                              /* Liste des actions rencontrées dans le source DLS */
 static GSList *Liste_Actions_msg    = NULL;                              /* Liste des actions rencontrées dans le source DLS */
 static GSList *Liste_edge_up_bi     = NULL;                               /* Liste des bits B utilisés avec l'option EDGE_UP */
 static GSList *Liste_edge_up_entree = NULL;                               /* Liste des bits E utilisés avec l'option EDGE_UP */
 static gchar *Buffer=NULL;
 static gint Buffer_used=0, Buffer_taille=0;
 static int Id_log;                                                                     /* Pour la creation du fichier de log */
 static int nbr_erreur;
 static int Dls_id;                                                                /* numéro du plugin en cours de traduction */

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
    Info_new( Config.log, Config.log_dls, LOG_DEBUG, "Emettre %s", chaine );
    memcpy ( Buffer + Buffer_used, chaine, taille );                                          /* Recopie du bout de buffer */
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
       g_snprintf( log, sizeof(log), "%s\n", chaine );
       write( Id_log, log, strlen(log) );

       Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: %s", __func__, chaine );
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
/* Entrées: rien                                                                                                              */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 int Get_option_entier( GList *liste_options, gint type )
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
/* Check_msg_ownership: Vérifie la propriété du bit interne MSG en action                                                     */
/* Entrées: le numéro du message positionné en action dans la ligne dls                                                       */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Check_msg_ownership ( gint num )
  { struct CMD_TYPE_MESSAGE *message;
    gchar chaine[80];
    gboolean retour;
    retour = FALSE;
    message = Rechercher_messageDB ( num );
    Info_new( Config.log, Config.log_dls, LOG_DEBUG,
             "%s: Test Message %d for id %d: mnemo %p", __func__, num, Dls_id, message ); 
    if (message)
     { if (message->dls_id == Dls_id) retour=TRUE;
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
/* Check_ownership: Vérifie la propriété du bit interne en action                                                             */
/* Entrées: le type et numéro du bit interne a testet                                                                         */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Check_ownership ( gint type, gint num )
  { struct CMD_TYPE_NUM_MNEMONIQUE critere;
    struct CMD_TYPE_MNEMO_BASE *mnemo;
    critere.type = type;
    critere.num  = num;
    gchar chaine[80];
    gboolean retour;
    retour = FALSE;
    mnemo = Rechercher_mnemo_baseDB_type_num ( &critere );
    Info_new( Config.log, Config.log_dls, LOG_DEBUG,
             "%s: Test Mnemo %d %d for id %d: mnemo %p", __func__, critere.type, critere.num, Dls_id, mnemo ); 
    if (mnemo)
     { if (mnemo->dls_id == Dls_id) retour=TRUE;
       g_free(mnemo);
     }
    
    if(retour == FALSE)
     { switch (type)
        { case MNEMO_BISTABLE:
               g_snprintf( chaine, sizeof(chaine), "Ligne %d: B%04d not owned by plugin", DlsScanner_get_lineno(), num );
               break;
          case MNEMO_CPTH:
               g_snprintf( chaine, sizeof(chaine), "Ligne %d: CH%04d not owned by plugin", DlsScanner_get_lineno(), num );
               break;
          case MNEMO_CPT_IMP:
               g_snprintf( chaine, sizeof(chaine), "Ligne %d: CI%04d not owned by plugin", DlsScanner_get_lineno(), num );
               break;
          case MNEMO_MONOSTABLE:
               g_snprintf( chaine, sizeof(chaine), "Ligne %d: M%04d not owned by plugin", DlsScanner_get_lineno(), num );
               break;
          case MNEMO_MOTIF:
               g_snprintf( chaine, sizeof(chaine), "Ligne %d: I%04d not owned by plugin", DlsScanner_get_lineno(), num );
               break;
          case MNEMO_REGISTRE:
               g_snprintf( chaine, sizeof(chaine), "Ligne %d: R%04d not owned by plugin", DlsScanner_get_lineno(), num );
               break;
          case MNEMO_SORTIE:
               g_snprintf( chaine, sizeof(chaine), "Ligne %d: A%04d not owned by plugin", DlsScanner_get_lineno(), num );
               break;
          case MNEMO_SORTIE_ANA:
               g_snprintf( chaine, sizeof(chaine), "Ligne %d: AA%04d not owned by plugin", DlsScanner_get_lineno(), num );
               break;
          case MNEMO_TEMPO:
               g_snprintf( chaine, sizeof(chaine), "Ligne %d: T%04d not owned by plugin", DlsScanner_get_lineno(), num );
               break;
          default: Emettre_erreur_new( "Ligne %d: Ownership problem (but bit type unknown)", DlsScanner_get_lineno() );
               break;
        }
       Emettre_erreur_new( "%s", chaine );
       return(FALSE);
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* New_condition_bi: Prepare la chaine de caractere associée à la condition, en respectant les options                        */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_bi( int barre, int num, GList *options )
  { gchar *result;
    gint taille;
    taille = 24;
    result = New_chaine( taille );
    if (Get_option_entier( options, T_EDGE_UP) == 1)
     { Liste_edge_up_bi = g_slist_prepend ( Liste_edge_up_bi, GINT_TO_POINTER(num) );
       if (barre) g_snprintf( result, taille, "!B%d_edge_up_value", num );
             else g_snprintf( result, taille, "B%d_edge_up_value", num );
     }
    else { if (barre) g_snprintf( result, taille, "!B(%d)", num );
                 else g_snprintf( result, taille, "B(%d)", num );
         }
    return(result);
  }
/******************************************************************************************************************************/
/* New_condition_bi: Prepare la chaine de caractere associée à la condition, en respectant les options                        */
/* Entrées: numero du bit bistable et sa liste d'options                                                                      */
/* Sortie: la chaine de caractere en C                                                                                        */
/******************************************************************************************************************************/
 gchar *New_condition_entree( int barre, int num, GList *options )
  { gchar *result;
    gint taille;
    taille = 24;
    result = New_chaine( taille );
    if (Get_option_entier( options, T_EDGE_UP) == 1)
     { Liste_edge_up_entree = g_slist_prepend ( Liste_edge_up_entree, GINT_TO_POINTER(num) );
       if (barre) g_snprintf( result, taille, "!E%d_edge_up_value", num );
             else g_snprintf( result, taille, "E%d_edge_up_value", num );
     }
    else { if (barre) g_snprintf( result, taille, "!E(%d)", num );
                 else g_snprintf( result, taille, "E(%d)", num );
         }
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
/* New_action_msg: Prepare une struct action avec une commande MSG                                                            */
/* Entrées: numero du message                                                                                                 */
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
     { Liste_Actions_bit = g_slist_prepend ( Liste_Actions_bit, GINT_TO_POINTER(type) );
       Liste_Actions_num = g_slist_prepend ( Liste_Actions_num, GINT_TO_POINTER(num) );
       return(TRUE);
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* New_action_sortie: Prepare une struct action avec une commande SA                                                          */
/* Entrées: numero de la sortie, sa logique                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_sortie( int num, int barre )
  { struct ACTION *action;
    int taille;

    taille = 20;
    if (Add_bit_to_list(MNEMO_SORTIE, num)) Check_ownership ( MNEMO_SORTIE, num );
    action = New_action();
    action->alors = New_chaine( taille );
    g_snprintf( action->alors, taille, "SA(%d,%d);", num, !barre );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_mono( int num )
  { struct ACTION *action;
    int taille;

    taille = 15;
    if (Add_bit_to_list(MNEMO_MONOSTABLE, num)) Check_ownership ( MNEMO_MONOSTABLE, num );
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "SM(%d,1);", num );
    g_snprintf( action->sinon, taille, "SM(%d,0);", num );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_mono: Prepare une struct action avec une commande SM                                                            */
/* Entrées: numero du monostable, sa logique                                                                                  */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_cpt_h( int num, GList *options )
  { struct ACTION *action;
    int taille, reset;

    reset = Get_option_entier ( options, RESET ); if (reset == -1) reset = 0;
    taille = 15;
    if (Add_bit_to_list(MNEMO_CPTH, num)) Check_ownership ( MNEMO_CPTH, num );
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "SCH(%d,1,%d);", num, reset );
    g_snprintf( action->sinon, taille, "SCH(%d,0,%d);", num, reset );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_cpt_imp: Prepare une struct action avec une commande SCI                                                        */
/* Entrées: numero du compteur d'impulsion, sa logique, son reset                                                             */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_cpt_imp( int num, GList *options )
  { struct ACTION *action;
    int taille, reset, ratio;
    reset = Get_option_entier ( options, RESET ); if (reset == -1) reset = 0;
    ratio = Get_option_entier ( options, RATIO ); if (ratio == -1) ratio = 1;

    taille = 20;
    if (Add_bit_to_list(MNEMO_CPT_IMP, num)) Check_ownership ( MNEMO_CPT_IMP, num );
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "SCI(%d,1,%d,%d);", num, reset, ratio );
    g_snprintf( action->sinon, taille, "SCI(%d,0,%d,%d);", num, reset, ratio );
    return(action);
  }
/******************************************************************************************************************************/
/* New_action_icone: Prepare une struct action avec une commande SI                                                           */
/* Entrées: numero du motif                                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_icone( int num, GList *options )
  { struct ACTION *action;
    int taille, rouge, vert, bleu, val, coul, cligno;

    val    = Get_option_entier ( options, MODE   ); if (val    == -1) val = 0;
    coul   = Get_option_entier ( options, COLOR  ); if (coul   == -1) coul = 0;
    cligno = Get_option_entier ( options, CLIGNO ); if (cligno == -1) cligno = 0;
    taille = 40;
    if (Add_bit_to_list(MNEMO_MOTIF, num)) Check_ownership ( MNEMO_MOTIF, num );
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
    g_snprintf( action->alors, taille, "SI(%d,%d,%d,%d,%d,%d);",
                num, val, rouge, vert, bleu, cligno );
    return(action);
  }

/******************************************************************************************************************************/
/* New_action_tempo: Prepare une struct action avec une commande TR                                                           */
/* Entrées: numero de la tempo, sa consigne                                                                                   */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_tempo( int num, GList *options )
  { struct ACTION *action;
    int taille;

    if (Add_bit_to_list(MNEMO_TEMPO, num)) Check_ownership ( MNEMO_TEMPO, num );
    action = New_action();
    taille = 40;
    action->alors = New_chaine( taille );
    g_snprintf( action->alors, taille, "ST(%d,1);", num );
    action->sinon = New_chaine( taille );
    g_snprintf( action->sinon, taille, "ST(%d,0);", num );
    return(action);
  }

/******************************************************************************************************************************/
/* New_action_bi: Prepare une struct action avec une commande SB                                                              */
/* Entrées: numero du bistable, sa logique                                                                                    */
/* Sortie: la structure action                                                                                                */
/******************************************************************************************************************************/
 struct ACTION *New_action_bi( int num, int barre )
  { struct ACTION *action;
    int taille;

    taille = 20;
    if (Add_bit_to_list(MNEMO_BISTABLE, num)) Check_ownership ( MNEMO_BISTABLE, num );
    action = New_action();
    action->alors = New_chaine( taille );
       
    g_snprintf( action->alors, taille, "SB(%d,%d);", num, !barre );
    return(action);
  }
/******************************************************************************************************************************/
/* New_alias: Alloue une certaine quantité de mémoire pour utiliser des alias                                                 */
/* Entrées: le nom de l'alias, le tableau et le numero du bit                                                                 */
/* Sortie: False si il existe deja, true sinon                                                                                */
/******************************************************************************************************************************/
 gboolean New_alias( char *nom, int bit, int num, int barre, GList *options )
  { struct ALIAS *alias;

    if (Get_alias_par_nom( nom )) return(FALSE);                                                         /* ID deja definit ? */

    alias=(struct ALIAS *)g_try_malloc0( sizeof(struct ALIAS) );
    if (!alias) { return(FALSE); }
    alias->nom = nom;
    alias->bit = bit;
    alias->num = num;
    alias->barre = barre;
    alias->options = options;
    alias->used = 0;
    Alias = g_slist_prepend( Alias, alias );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Get_alias: Recherche un alias donné en paramètre                                                                           */
/* Entrées: le nom de l'alias                                                                                                 */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct ALIAS *Get_alias_par_nom( char *nom )
  { struct ALIAS *alias;
    GSList *liste;
    liste = Alias;
    while(liste)
     { alias = (struct ALIAS *)liste->data;
       if (!strcmp(alias->nom, nom)) { alias->used++; return(alias); }                      /* Si deja present, on renvoie un */
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
  { if (options)
     { g_list_foreach( options, (GFunc)g_free, NULL );
       g_list_free( options );
     }
  }
/******************************************************************************************************************************/
/* Liberer_alias: Liberation de toutes les zones de mémoire précédemment allouées                                             */
/* Entrées: kedal                                                                                                             */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Liberer_alias ( struct ALIAS *alias )
  { Liberer_options( alias->options );
    g_free(alias->nom);
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
    g_slist_free(Liste_Actions_msg);    Liste_Actions_msg    = NULL;
    g_slist_free(Liste_Actions_bit);    Liste_Actions_bit    = NULL;
    g_slist_free(Liste_Actions_num);    Liste_Actions_num    = NULL;
    g_slist_free(Liste_edge_up_bi);     Liste_edge_up_bi     = NULL;
    g_slist_free(Liste_edge_up_entree); Liste_edge_up_entree = NULL;
  }
/******************************************************************************************************************************/
/* Traduire: Traduction du fichier en paramètre du langage DLS vers le langage C                                              */
/* Entrée: l'id du modul                                                                                                      */
/* Sortie: TRAD_DLS_OK, _WARNING ou _ERROR                                                                                    */
/******************************************************************************************************************************/
 gint Traduire_DLS( int id )
  { gchar source[80], cible[80], log[80];
    struct ALIAS *alias;
    GSList *liste;
    gint retour;
    FILE *rc;

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

    pthread_mutex_lock( &Partage->com_dls.synchro_traduction );                           /* Attente unicité de la traduction */

    Dls_id = id;                                     /* Sauvegarde, pour notamment les tests de d'ownership des bits internes */
    Alias = NULL;                                                                                  /* Par défaut, pas d'alias */
    Liste_Actions_bit = NULL;                                                                    /* Par défaut, pas d'actions */
    Liste_Actions_num = NULL;                                                                    /* Par défaut, pas d'actions */
    Liste_Actions_msg = NULL;                                                                    /* Par défaut, pas d'actions */
    Liste_edge_up_bi  = NULL;                                               /* Liste des bits B utilisé avec l'option EDGE UP */
    DlsScanner_set_lineno(1);                                                                     /* Reset du numéro de ligne */
    nbr_erreur = 0;                                                                   /* Au départ, nous n'avons pas d'erreur */
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
       Emettre_erreur_new( "No error found" );                        /* Pas d'erreur rencontré (mais peu etre des warning !) */
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
          gchar *Start_Go = " void Go ( gint start, gint debug )\n"
                            "  {\n"
                            "    Update_edge_up_value();\n"
                            "    if (debug) Dls_print_debug( Dls_id, (int *)&Tableau_bit, (int *)&Tableau_num, (float *)&Tableau_val );\n";
          gchar *End_Go =   "  }\n";
          gchar chaine[4096];
          gint cpt=0;                                                                                   /* Compteur d'actions */

          write(fd, include, strlen(include));

          cpt = g_slist_length(Liste_Actions_bit);
          if (cpt==0) cpt=1;
          g_snprintf( chaine, sizeof(chaine), " static gfloat Tableau_val[%d];\n", cpt );
          write(fd, chaine, strlen(chaine) );                                                         /* Ecriture du prologue */

          g_snprintf( chaine, sizeof(chaine), " static gint Dls_id = %d;\n", id );
          write(fd, chaine, strlen(chaine) );                                                         /* Ecriture du prologue */

          write(fd, Chaine_bit, strlen(Chaine_bit) );                                                 /* Ecriture du prologue */
          liste = Liste_Actions_bit;                                       /* Initialise les tableaux des actions rencontrées */
          while(liste)
           { gchar chaine[12];
             g_snprintf(chaine, sizeof(chaine), "%d, ", GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             liste = liste->next;
           }
          write(fd, Tableau_end, strlen(Tableau_end) );                                               /* Ecriture du prologue */

          write(fd, Chaine_num, strlen(Chaine_num) );                                                 /* Ecriture du prologue */
          liste = Liste_Actions_num;                                       /* Initialise les tableaux des actions rencontrées */
          while(liste)
           { gchar chaine[12];
             g_snprintf(chaine, sizeof(chaine), "%d, ", GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             liste = liste->next;
           }
          write(fd, Tableau_end, strlen(Tableau_end) );                                               /* Ecriture du prologue */

          write(fd, Chaine_msg, strlen(Chaine_msg) );                                                 /* Ecriture du prologue */
          liste = Liste_Actions_msg;                                       /* Initialise les tableaux des actions rencontrées */
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
             liste = liste->next;
           }

          liste = Liste_edge_up_entree;                            /* Initialise les fonctions de gestion des fronts montants */
          while(liste)
           { g_snprintf(chaine, sizeof(chaine),
                      " static int E%d_edge_up_value = 0;\n", GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             liste = liste->next;
           }


          g_snprintf(chaine, sizeof(chaine),
                    "/*******************************************************/\n"
                    " static void Update_edge_up_value (void)\n"
                    "  { int new_value=0;\n" );
          write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */

          liste = Liste_edge_up_bi;                                /* Initialise les fonctions de gestion des fronts montants */
          while(liste)
           { gchar chaine[1024];
             g_snprintf(chaine, sizeof(chaine),
                      " new_value = B(%d);\n"
                      " if (new_value == 0) B%d_edge_up_value = 0;\n"
                      " else { if (B%d_edge_up_value==0 && new_value == 1) { B%d_edge_up_value=1; }\n"
                      "                                               else { B%d_edge_up_value=0; }\n"
                      "      }\n",
                      GPOINTER_TO_INT(liste->data), GPOINTER_TO_INT(liste->data),
                      GPOINTER_TO_INT(liste->data), GPOINTER_TO_INT(liste->data),
                      GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             liste = liste->next;
           }
          liste = Liste_edge_up_entree;                            /* Initialise les fonctions de gestion des fronts montants */
          while(liste)
           { gchar chaine[1024];
             g_snprintf(chaine, sizeof(chaine),
                      " new_value = E(%d);\n"
                      " if (new_value == 0) E%d_edge_up_value = 0;\n"
                      " else { if (E%d_edge_up_value==0 && new_value == 1) { E%d_edge_up_value=1; }\n"
                      "                                               else { E%d_edge_up_value=0; }\n"
                      "      }\n",
                      GPOINTER_TO_INT(liste->data), GPOINTER_TO_INT(liste->data),
                      GPOINTER_TO_INT(liste->data), GPOINTER_TO_INT(liste->data),
                      GPOINTER_TO_INT(liste->data) );
             write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */
             liste = liste->next;
           }
          g_snprintf(chaine, sizeof(chaine), "  }\n" );
          write(fd, chaine, strlen(chaine) );                                                      /* Ecriture du prologue */

          write( fd, Start_Go, strlen(Start_Go) );
          write(fd, Buffer, Buffer_used );                                                     /* Ecriture du buffer resultat */
          write( fd, End_Go, strlen(End_Go) );
          close(fd);
        }

       liste = Alias;                                           /* Libération des alias, et remonté d'un Warning si il y en a */
       while(liste)
        { alias = (struct ALIAS *)liste->data;
          if ( (!alias->used) )
           { Emettre_erreur_new( "Warning: %s not used", alias->nom );
             retour = TRAD_DLS_WARNING;
           }
          liste = liste->next;
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
