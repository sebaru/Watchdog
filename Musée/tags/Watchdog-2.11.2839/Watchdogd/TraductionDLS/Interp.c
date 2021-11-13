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

 GList *Alias=NULL;
 static int Id_cible;                                                               /* Pour la création du fichier temporaire */
 static int Id_log;                                                                     /* Pour la creation du fichier de log */
 static int nbr_erreur;

 extern gboolean Interpreter_source_dls ( gchar *source );
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
    Info_new( Config.log, Config.log_dls, LOG_DEBUG, "Emettre %s", chaine );
    write( Id_cible, chaine, taille );
  }
/******************************************************************************************************************************/
/* Emettre: Met a jour le fichier temporaire en code intermédiaire                                                            */
/* Entrées: la ligne d'instruction à mettre                                                                                   */
/* Sortie: void                                                                                                               */
/******************************************************************************************************************************/
 void Emettre_erreur( char *chaine )
  { static gchar *too_many="Too many errors...";
    int taille;
    if ( nbr_erreur < 15 )
     { taille = strlen(chaine);
       Info_new( Config.log, Config.log_dls, LOG_ERR, "Emettre_erreur %s", chaine );
       write( Id_log, chaine, taille );
     } else
    if ( nbr_erreur == 15 )
     { taille = strlen(too_many);
       Info_new( Config.log, Config.log_dls, LOG_ERR, "Emettre_erreur: %s", too_many );
       write( Id_log, too_many, taille );
     }
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
    int taille;

    taille = 15;
    action = New_action();
    action->alors = New_chaine( taille );
    g_snprintf( action->alors, taille, "MSG(%d,1);", num );
    action->sinon = New_chaine( taille );
    g_snprintf( action->sinon, taille, "MSG(%d,0);", num );
    return(action);
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
 struct ACTION *New_action_cpt_h( int num )
  { struct ACTION *action;
    int taille;

    taille = 15;
    action = New_action();
    action->alors = New_chaine( taille );
    action->sinon = New_chaine( taille );

    g_snprintf( action->alors, taille, "SCH(%d,1);", num );
    g_snprintf( action->sinon, taille, "SCH(%d,0);", num );
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
    Alias = g_list_append( Alias, alias );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Get_alias: Recherche un alias donné en paramètre                                                                           */
/* Entrées: le nom de l'alias                                                                                                 */
/* Sortie: NULL si probleme                                                                                                   */
/******************************************************************************************************************************/
 struct ALIAS *Get_alias_par_nom( char *nom )
  { struct ALIAS *alias;
    GList *liste;
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
 void Liberer_memoire( void )
  { g_list_foreach( Alias, (GFunc) Liberer_alias, NULL );
    g_list_free( Alias );
    Alias = NULL;
  }
/******************************************************************************************************************************/
/* Traduire: Traduction du fichier en paramètre du langage DLS vers le langage C                                              */
/* Entrée: l'id du modul, new = true si il faut compiler le .dls.new                                                          */
/* Sortie: TRAD_DLS_OK, _WARNING ou _ERROR                                                                                    */
/******************************************************************************************************************************/
 gint Traduire_DLS( gboolean new, gint id )
  { gchar source[80], source_ok[80], cible[80], log[80];
    struct ALIAS *alias;
    gint retour;
    GList *liste;

    g_snprintf( source,    sizeof(source),    "Dls/%d.dls.new", id );
    g_snprintf( source_ok, sizeof(source_ok), "Dls/%d.dls", id );
    g_snprintf( cible,     sizeof(cible),     "Dls/%d.c",   id );
    g_snprintf( log,       sizeof(log),       "Dls/%d.log", id );
    unlink ( cible );
    unlink ( log );
    Info_new( Config.log, Config.log_dls, LOG_DEBUG, "Traduire_DLS: new=%d, id=%d, source=%s, source_ok=%s cible=%s, log=%s",
              new, id, source, source_ok, cible, log );

    Id_cible = open( cible, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (Id_cible<0)
     { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                "Traduire_DLS: Target creation failed %s (%s)", cible, strerror(errno) ); 
       return(TRAD_DLS_ERROR_FILE);
     }

    Id_log = open( log, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (Id_log<0)
     { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                "Traduire_DLS: Log creation failed %s (%s)", cible, strerror(errno) ); 
       close(Id_cible);
       return(TRAD_DLS_ERROR_FILE);
     }

    pthread_mutex_lock( &Partage->com_dls.synchro_traduction );                           /* Attente unicité de la traduction */

    Alias = NULL;                                                                                  /* Par défaut, pas d'alias */
    nbr_erreur = 0;                                                                   /* Au départ, nous n'avons pas d'erreur */
    if ( Interpreter_source_dls( (new ? source : source_ok) ) )
         { retour = TRAD_DLS_OK; }
    else { retour = TRAD_DLS_ERROR; }

    if (retour==TRAD_DLS_OK)                                                      /* Si pas d'erreur, on regarde les warnings */
     { liste = Alias;
       while(liste)
        { alias = (struct ALIAS *)liste->data;
          if ( (!alias->used) )
           { gchar chaine[128];
             g_snprintf(chaine, sizeof(chaine), "Warning: %s not used\n", alias->nom );
             Emettre_erreur( chaine ); 
             retour = TRAD_DLS_WARNING;
           }
          liste = liste->next;
        }
     }

    close(Id_cible);
    close(Id_log);
    Liberer_memoire();

    if (retour != TRAD_DLS_ERROR && new)
     { Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                "Traduire_DLS: Renaming '%s' to '%s'", source, source_ok );
       unlink ( source_ok );                                                   /* Recopie sur le fichier "officiel" du plugin */
       rename( source, source_ok );
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro_traduction );                                         /* Libération Mutex */
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
