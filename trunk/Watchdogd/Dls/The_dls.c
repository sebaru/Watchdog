/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls.c  Gestion et execution des plugins DLS Watchdgo 2.0                                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       mar. 06 juil. 2010 18:31:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls.c
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
 #include <fcntl.h>
 #include <string.h>
 #include <signal.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <semaphore.h>
 #include <locale.h>
 #include <math.h>

 #include "watchdogd.h"

 #define DLS_LIBRARY_VERSION  "20201101"

/******************************************************************************************************************************/
/* Http_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                   */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gboolean Dls_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Creer_configDB ( "dls", "compil_at_boot", "false" );                                       /* Settings default parameters */
    Partage->com_dls.Compil_at_boot = FALSE;                                                   /* Settings default parameters */
    Creer_configDB ( "dls", "debug", "false" );                                                /* Settings default parameters */
    Partage->com_dls.Thread_debug   = FALSE;                                                   /* Settings default parameters */
    Creer_configDB ( "dls", "library_version", DLS_LIBRARY_VERSION );                          /* Settings default parameters */
    g_snprintf( Partage->com_dls.Library_version, sizeof(Partage->com_dls.Library_version), DLS_LIBRARY_VERSION );

    if ( ! Recuperer_configDB( &db, "dls" ) )                                               /* Connexion a la base de données */
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     {      if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Partage->com_dls.Thread_debug = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "compil_at_boot" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Partage->com_dls.Compil_at_boot = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "library_version" ) )
        { g_snprintf( Partage->com_dls.Library_version, sizeof(Partage->com_dls.Library_version), "%s", valeur ); }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Dls_get_top_alerte: Remonte la valeur du plus haut bit d'alerte dans l'arbre DLS                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: TRUE ou FALSe                                                                                                      */
/******************************************************************************************************************************/
 gboolean Dls_get_top_alerte ( void )
  { return( Partage->com_dls.Dls_tree->syn_vars.bit_alerte ); }
/******************************************************************************************************************************/
/* Dls_get_top_alerte_fugitive: Remonte la valeur du plus haut bit d'alerte fugitive dans l'arbre DLS                         */
/* Entrée: Rien                                                                                                               */
/* Sortie: TRUE ou FALSe                                                                                                      */
/******************************************************************************************************************************/
 gboolean Dls_get_top_alerte_fugitive ( void )
  { return( Partage->com_dls.Dls_tree->syn_vars.bit_alerte_fugitive ); }
/******************************************************************************************************************************/
/* Chrono: renvoi la difference de temps entre deux structures timeval                                                        */
/* Entrée: le temps avant, et le temps apres l'action                                                                         */
/* Sortie: un float                                                                                                           */
/******************************************************************************************************************************/
 static float Chrono ( struct timeval *avant, struct timeval *apres )
  { if (!(avant && apres)) return(0.0);
    else return( apres->tv_sec - avant->tv_sec + (apres->tv_usec - avant->tv_usec)/1000000.0 );
  }
/******************************************************************************************************************************/
/* STR_local: Positionnement d'une Tempo retard DLS                                                                           */
/* Entrée: la structure tempo et son etat                                                                                     */
/* Sortie: Neant                                                                                                              */
/******************************************************************************************************************************/
 static void ST_local( struct DLS_TEMPO *tempo, int etat )
  { static guint seed;
    if (tempo->status == DLS_TEMPO_NOT_COUNTING && etat == 1)
     { tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_ON;
       if (tempo->random)
        { gfloat ratio;
          ratio = (gfloat)rand_r(&seed)/RAND_MAX;
          tempo->delai_on  = (gint)(tempo->random * ratio);
          if (tempo->delai_on<10) tempo->delai_on = 10;
          tempo->min_on    = 0;
          tempo->max_on    = 0;
          tempo->delai_off = 0;
        }
       tempo->date_on = Partage->top + tempo->delai_on;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_ON && etat == 0)
     { tempo->status = DLS_TEMPO_NOT_COUNTING; }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_ON && tempo->date_on <= Partage->top)
     { tempo->status = DLS_TEMPO_WAIT_FOR_MIN_ON;
       tempo->state = TRUE;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MIN_ON && etat == 0 &&
        Partage->top < tempo->date_on + tempo->min_on )
     { if (Partage->top+tempo->delai_off <= tempo->date_on + tempo->min_on)
            { tempo->date_off = tempo->date_on+tempo->min_on; }
       else { tempo->date_off = Partage->top+tempo->delai_off; }
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MIN_ON && etat == 0 &&
        tempo->date_on + tempo->min_on <= Partage->top )
     { tempo->date_off = Partage->top+tempo->delai_off;
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MIN_ON && etat == 1 &&
        tempo->date_on + tempo->min_on <= Partage->top )
     { tempo->status = DLS_TEMPO_WAIT_FOR_MAX_ON;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MAX_ON && etat == 0 )
     { if (tempo->max_on)
            { if (Partage->top+tempo->delai_off < tempo->date_on+tempo->max_on)
                   { tempo->date_off = Partage->top + tempo->delai_off; }
              else { tempo->date_off = tempo->date_on+tempo->max_on; }
            }
       else { tempo->date_off = Partage->top+tempo->delai_off; }
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MAX_ON && etat == 1 && tempo->max_on &&
        tempo->date_on + tempo->max_on <= Partage->top )
     { tempo->date_off = tempo->date_on+tempo->max_on;
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_OFF && tempo->date_off <= Partage->top )
     { tempo->date_on = tempo->date_off = 0;
       tempo->status = DLS_TEMPO_WAIT_FOR_COND_OFF;
       tempo->state = FALSE;
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_COND_OFF && etat == 0 )
     { tempo->status = DLS_TEMPO_NOT_COUNTING; }
  }
/******************************************************************************************************************************/
/* Envoyer_commande_dls_data: Gestion des envois de commande DLS via dls_data                                                 */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Envoyer_commande_dls_data ( gchar *tech_id, gchar *acronyme )
  { gpointer di=NULL;
    Dls_data_get_DI ( tech_id, acronyme, &di );
    if (!di) { Dls_data_set_DI ( NULL, tech_id, acronyme, &di, TRUE ); }

    pthread_mutex_lock( &Partage->com_dls.synchro );
    Partage->com_dls.Set_Dls_Data = g_slist_append ( Partage->com_dls.Set_Dls_Data, di );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Mise a un du bit DI '%s:%s' demandée", __func__, tech_id, acronyme );
  }
/******************************************************************************************************************************/
/* Set_cde_exterieure: Mise à un des bits de commande exterieure                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Set_cde_exterieure ( void )
  { pthread_mutex_lock( &Partage->com_dls.synchro );
    while( Partage->com_dls.Set_Dls_Data )                                                  /* A-t-on une entrée a allumer ?? */
     { struct DLS_DI *di = Partage->com_dls.Set_Dls_Data->data;
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Mise a 1 du bit DI %s:%s",
                 __func__, di->tech_id, di->acronyme );
       Partage->com_dls.Set_Dls_Data = g_slist_remove ( Partage->com_dls.Set_Dls_Data, di );
       Partage->com_dls.Reset_Dls_Data = g_slist_append ( Partage->com_dls.Reset_Dls_Data, di );
       Dls_data_set_DI ( NULL, NULL, NULL, (gpointer *)&di, TRUE );                              /* Mise a un du bit d'entrée */
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Reset_cde_exterieure: Mise à zero des bits de commande exterieure                                                          */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Reset_cde_exterieure ( void )
  { pthread_mutex_lock( &Partage->com_dls.synchro );
    while( Partage->com_dls.Reset_Dls_Data )                                            /* A-t-on un monostable a éteindre ?? */
     { struct DLS_DI *di = Partage->com_dls.Reset_Dls_Data->data;
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Mise a 0 du bit DI %s:%s",
                 __func__, di->tech_id, di->acronyme );
       Partage->com_dls.Reset_Dls_Data = g_slist_remove ( Partage->com_dls.Reset_Dls_Data, di );
       Dls_data_set_DI ( NULL, NULL, NULL, (gpointer *)&di, FALSE );                             /* Mise a un du bit d'entrée */
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Set_cde_exterieure: Mise à un des bits de commande exterieure                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Set_edge ( void )
  { while( Partage->com_dls.Set_Dls_Bool_Edge_up )                                       /* A-t-on un boolean up a allumer ?? */
     { struct DLS_BOOL *bool = Partage->com_dls.Set_Dls_Bool_Edge_up->data;
       Partage->com_dls.Set_Dls_Bool_Edge_up   = g_slist_remove  ( Partage->com_dls.Set_Dls_Bool_Edge_up, bool );
       Partage->com_dls.Reset_Dls_Bool_Edge_up = g_slist_prepend ( Partage->com_dls.Reset_Dls_Bool_Edge_up, bool );
       bool->edge_up = TRUE;
     }
    while( Partage->com_dls.Set_Dls_Bool_Edge_down )                                       /* A-t-on un boolean down a allumer ?? */
     { struct DLS_BOOL *bool = Partage->com_dls.Set_Dls_Bool_Edge_down->data;
       Partage->com_dls.Set_Dls_Bool_Edge_down   = g_slist_remove  ( Partage->com_dls.Set_Dls_Bool_Edge_down, bool );
       Partage->com_dls.Reset_Dls_Bool_Edge_down = g_slist_prepend ( Partage->com_dls.Reset_Dls_Bool_Edge_down, bool );
       bool->edge_down = TRUE;
     }
    while( Partage->com_dls.Set_Dls_DI_Edge_up )                                       /* A-t-on un boolean up a allumer ?? */
     { struct DLS_DI *di = Partage->com_dls.Set_Dls_DI_Edge_up->data;
       Partage->com_dls.Set_Dls_DI_Edge_up   = g_slist_remove  ( Partage->com_dls.Set_Dls_DI_Edge_up, di );
       Partage->com_dls.Reset_Dls_DI_Edge_up = g_slist_prepend ( Partage->com_dls.Reset_Dls_DI_Edge_up, di );
       di->edge_up = TRUE;
     }
    while( Partage->com_dls.Set_Dls_DI_Edge_down )                                       /* A-t-on un boolean down a allumer ?? */
     { struct DLS_DI *di = Partage->com_dls.Set_Dls_DI_Edge_down->data;
       Partage->com_dls.Set_Dls_DI_Edge_down   = g_slist_remove  ( Partage->com_dls.Set_Dls_DI_Edge_down, di );
       Partage->com_dls.Reset_Dls_DI_Edge_down = g_slist_prepend ( Partage->com_dls.Reset_Dls_DI_Edge_down, di );
       di->edge_down = TRUE;
     }
  }
/******************************************************************************************************************************/
/* Reset_cde_exterieure: Mise à zero des bits de commande exterieure                                                          */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Reset_edge ( void )
  { while( Partage->com_dls.Reset_Dls_Bool_Edge_up )                                     /* A-t-on un boolean up a allumer ?? */
     { struct DLS_BOOL *bool = Partage->com_dls.Reset_Dls_Bool_Edge_up->data;
       Partage->com_dls.Reset_Dls_Bool_Edge_up = g_slist_remove ( Partage->com_dls.Reset_Dls_Bool_Edge_up, bool );
       bool->edge_up = FALSE;
     }
    while( Partage->com_dls.Reset_Dls_Bool_Edge_down )                                 /* A-t-on un boolean down a allumer ?? */
     { struct DLS_BOOL *bool = Partage->com_dls.Reset_Dls_Bool_Edge_down->data;
       Partage->com_dls.Reset_Dls_Bool_Edge_down = g_slist_remove ( Partage->com_dls.Reset_Dls_Bool_Edge_down, bool );
       bool->edge_down = FALSE;
     }
    while( Partage->com_dls.Reset_Dls_DI_Edge_up )                                       /* A-t-on un boolean up a allumer ?? */
     { struct DLS_DI *di = Partage->com_dls.Reset_Dls_DI_Edge_up->data;
       Partage->com_dls.Reset_Dls_DI_Edge_up = g_slist_remove ( Partage->com_dls.Reset_Dls_DI_Edge_up, di );
       di->edge_up = FALSE;
     }
    while( Partage->com_dls.Reset_Dls_DI_Edge_down )                                   /* A-t-on un boolean down a allumer ?? */
     { struct DLS_DI *di = Partage->com_dls.Reset_Dls_DI_Edge_down->data;
       Partage->com_dls.Reset_Dls_DI_Edge_down = g_slist_remove ( Partage->com_dls.Reset_Dls_DI_Edge_down, di );
       di->edge_down = FALSE;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************************************************************/
/* Dls_data_set_bool: Positionne un boolean                                                                                   */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 void Dls_data_set_bool ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *bool_p, gboolean valeur )
  { struct DLS_BOOL *bool;

    if (!bool_p || !*bool_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_BOOL;
       while (liste)
        { bool = (struct DLS_BOOL *)liste->data;
          if ( !strcasecmp ( bool->acronyme, acronyme ) && !strcasecmp( bool->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { bool = g_try_malloc0 ( sizeof(struct DLS_BOOL) );
          if (!bool)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( bool->acronyme, sizeof(bool->acronyme), "%s", acronyme );
          g_snprintf( bool->tech_id,  sizeof(bool->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_BOOL = g_slist_prepend ( Partage->Dls_data_BOOL, bool );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s : adding DLS_BOOL '%s:%s'", __func__, tech_id, acronyme );
        }
       if (bool_p) *bool_p = (gpointer)bool;                                        /* Sauvegarde pour acceleration si besoin */
      }
    else bool = (struct DLS_BOOL *)*bool_p;

    if (bool->etat != valeur)
     { Info_new( Config.log, (vars ? vars->debug : Partage->com_dls.Thread_debug), LOG_DEBUG, "%s : Changing DLS_BOOL '%s:%s'=%d up %d down %d",
                 __func__, bool->tech_id, bool->acronyme, valeur, bool->edge_up, bool->edge_down );
       if (valeur == TRUE) Partage->com_dls.Set_Dls_Bool_Edge_up   = g_slist_prepend ( Partage->com_dls.Set_Dls_Bool_Edge_up, bool );
                      else Partage->com_dls.Set_Dls_Bool_Edge_down = g_slist_prepend ( Partage->com_dls.Set_Dls_Bool_Edge_down, bool );
       Partage->audit_bit_interne_per_sec++;
     }
    bool->etat = valeur;
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool: Remonte l'etat d'un boolean                                                                             */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_bool ( gchar *tech_id, gchar *acronyme, gpointer *bool_p )
  { struct DLS_BOOL *bool;
    GSList *liste;
    if (bool_p && *bool_p)                                                           /* Si pointeur d'acceleration disponible */
     { bool = (struct DLS_BOOL *)*bool_p;
       return( bool->etat );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_BOOL;
    while (liste)
     { bool = (struct DLS_BOOL *)liste->data;
       if ( !strcasecmp ( bool->acronyme, acronyme ) && !strcasecmp( bool->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (bool_p) *bool_p = (gpointer)bool;                                           /* Sauvegarde pour acceleration si besoin */
    return( bool->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool_up: Remonte le front montant d'un boolean                                                                */
/* Sortie : TRUE sur le boolean vient de passer à UP                                                                          */
/******************************************************************************************************************************/
 gboolean Dls_data_get_bool_up ( gchar *tech_id, gchar *acronyme, gpointer *bool_p )
  { struct DLS_BOOL *bool;
    GSList *liste;
    if (bool_p && *bool_p)                                                           /* Si pointeur d'acceleration disponible */
     { bool = (struct DLS_BOOL *)*bool_p;
       return( bool->edge_up );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_BOOL;
    while (liste)
     { bool = (struct DLS_BOOL *)liste->data;
       if ( !strcasecmp ( bool->acronyme, acronyme ) && !strcasecmp( bool->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (bool_p) *bool_p = (gpointer)bool;                                           /* Sauvegarde pour acceleration si besoin */
    return( bool->edge_up );
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool_down: Remonte le front descendant d'un boolean                                                           */
/* Sortie : TRUE sur le boolean vient de passer à DOWN                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_bool_down ( gchar *tech_id, gchar *acronyme, gpointer *bool_p )
  { struct DLS_BOOL *bool;
    GSList *liste;
    if (bool_p && *bool_p)                                                           /* Si pointeur d'acceleration disponible */
     { bool = (struct DLS_BOOL *)*bool_p;
       return( bool->edge_down );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_BOOL;
    while (liste)
     { bool = (struct DLS_BOOL *)liste->data;
       if ( !strcasecmp ( bool->acronyme, acronyme ) && !strcasecmp( bool->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (bool_p) *bool_p = (gpointer)bool;                                           /* Sauvegarde pour acceleration si besoin */
    return( bool->edge_down );
  }
/******************************************************************************************************************************/
/* Dls_data_set_bool: Positionne un boolean                                                                                   */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 void Dls_data_set_DI ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *di_p, gboolean valeur )
  { struct DLS_DI *di;

    if (!di_p || !*di_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_DI;
       while (liste)
        { di = (struct DLS_DI *)liste->data;
          if ( !strcasecmp ( di->acronyme, acronyme ) && !strcasecmp( di->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { di = g_try_malloc0 ( sizeof(struct DLS_DI) );
          if (!di)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( di->acronyme, sizeof(di->acronyme), "%s", acronyme );
          g_snprintf( di->tech_id,  sizeof(di->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_DI = g_slist_prepend ( Partage->Dls_data_DI, di );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s : adding DLS_DI '%s:%s'=%d", __func__, tech_id, acronyme, valeur );
        }
       if (di_p) *di_p = (gpointer)di;                                              /* Sauvegarde pour acceleration si besoin */
      }
    else di = (struct DLS_DI *)*di_p;

    if (di->etat != valeur)
     { Info_new( Config.log, (vars ? vars->debug : Partage->com_dls.Thread_debug), LOG_DEBUG, "%s : Changing DLS_DI '%s:%s'=%d up %d down %d",
                 __func__, di->tech_id, di->acronyme, valeur, di->edge_up, di->edge_down );
       if (valeur == TRUE) Partage->com_dls.Set_Dls_DI_Edge_up   = g_slist_prepend ( Partage->com_dls.Set_Dls_DI_Edge_up, di );
                      else Partage->com_dls.Set_Dls_DI_Edge_down = g_slist_prepend ( Partage->com_dls.Set_Dls_DI_Edge_down, di );
       Partage->audit_bit_interne_per_sec++;
     }
    di->etat = valeur;
  }
/******************************************************************************************************************************/
/* Dls_data_get_DI: Remonte l'etat d'une entrée TOR                                                                           */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DI ( gchar *tech_id, gchar *acronyme, gpointer *di_p )
  { struct DLS_DI *di;
    GSList *liste;
    if (di_p && *di_p)                                                               /* Si pointeur d'acceleration disponible */
     { di = (struct DLS_DI *)*di_p;
       return( di->etat );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_DI;
    while (liste)
     { di = (struct DLS_DI *)liste->data;
       if ( !strcasecmp ( di->acronyme, acronyme ) && !strcasecmp( di->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (di_p) *di_p = (gpointer)di;                                                 /* Sauvegarde pour acceleration si besoin */
    return( di->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool_up: Remonte le front montant d'un boolean                                                                */
/* Sortie : TRUE sur le boolean vient de passer à UP                                                                          */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DI_up ( gchar *tech_id, gchar *acronyme, gpointer *di_p )
  { struct DLS_DI *di;
    GSList *liste;
    if (di_p && *di_p)                                                               /* Si pointeur d'acceleration disponible */
     { di = (struct DLS_DI *)*di_p;
       return( di->edge_up );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_DI;
    while (liste)
     { di = (struct DLS_DI *)liste->data;
       if ( !strcasecmp ( di->acronyme, acronyme ) && !strcasecmp( di->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (di_p) *di_p = (gpointer)di;                                                 /* Sauvegarde pour acceleration si besoin */
    return( di->edge_up );
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool_down: Remonte le front descendant d'un boolean                                                           */
/* Sortie : TRUE sur le boolean vient de passer à DOWN                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DI_down ( gchar *tech_id, gchar *acronyme, gpointer *di_p )
  { struct DLS_DI *di;
    GSList *liste;
    if (di_p && *di_p)                                                               /* Si pointeur d'acceleration disponible */
     { di = (struct DLS_DI *)*di_p;
       return( di->edge_down );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_DI;
    while (liste)
     { di = (struct DLS_DI *)liste->data;
       if ( !strcasecmp ( di->acronyme, acronyme ) && !strcasecmp( di->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (di_p) *di_p = (gpointer)di;                                                 /* Sauvegarde pour acceleration si besoin */
    return( di->edge_down );
  }
/******************************************************************************************************************************/
/* Dls_data_get_DO: Remonte l'etat d'une sortie tor                                                                           */
/* Sortie : TRUE sur la sortie est UP                                                                                         */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DO ( gchar *tech_id, gchar *acronyme, gpointer *dout_p )
  { struct DLS_DO *dout;
    GSList *liste;
    if (dout_p && *dout_p)                                                           /* Si pointeur d'acceleration disponible */
     { dout = (struct DLS_DO *)*dout_p;
       return( dout->etat );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_DO;
    while (liste)
     { dout = (struct DLS_DO *)liste->data;
       if ( !strcasecmp ( dout->acronyme, acronyme ) && !strcasecmp( dout->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (dout_p) *dout_p = (gpointer)dout;                                           /* Sauvegarde pour acceleration si besoin */
    return( dout->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_set_DO: Positionne une bit de sortie TOR                                                                          */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_DO ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *dout_p, gboolean etat )
  { struct DLS_DO *dout;

    if (!dout_p || !*dout_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_DO;
       while (liste)
        { dout = (struct DLS_DO *)liste->data;
          if ( !strcasecmp ( dout->acronyme, acronyme ) && !strcasecmp( dout->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { dout = g_try_malloc0 ( sizeof(struct DLS_DO) );
          if (!dout)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( dout->acronyme, sizeof(dout->acronyme), "%s", acronyme );
          g_snprintf( dout->tech_id,  sizeof(dout->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_DO = g_slist_prepend ( Partage->Dls_data_DO, dout );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s : adding DLS_DO '%s:%s'", __func__, tech_id, acronyme );
        }
       if (dout_p) *dout_p = (gpointer)dout;                                              /* Sauvegarde pour acceleration si besoin */
      }
    else dout = (struct DLS_DO *)*dout_p;

    if (dout->etat != etat)
     { Info_new( Config.log, (vars ? vars->debug : Partage->com_dls.Thread_debug), LOG_DEBUG, "%s : Changing DLS_DO '%s:%s'=%d ",
                 __func__, dout->tech_id, dout->acronyme );
       if (etat)
        { pthread_mutex_lock( &Partage->com_msrv.synchro );
          Partage->com_msrv.Liste_DO = g_slist_prepend ( Partage->com_msrv.Liste_DO, dout );
          pthread_mutex_unlock( &Partage->com_msrv.synchro );
          Partage->audit_bit_interne_per_sec++;
        }
     }
    dout->etat = etat;
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool_up: Remonte le front montant d'un boolean                                                                */
/* Sortie : TRUE sur le boolean vient de passer à UP                                                                          */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DO_up ( gchar *tech_id, gchar *acronyme, gpointer *dout_p )
  { struct DLS_DO *dout;
    GSList *liste;
    if (dout_p && *dout_p)                                                           /* Si pointeur d'acceleration disponible */
     { dout = (struct DLS_DO *)*dout_p;
       return( dout->edge_up );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_DO;
    while (liste)
     { dout = (struct DLS_DO *)liste->data;
       if ( !strcasecmp ( dout->acronyme, acronyme ) && !strcasecmp( dout->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (dout_p) *dout_p = (gpointer)dout;                                           /* Sauvegarde pour acceleration si besoin */
    return( dout->edge_up );
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool_down: Remonte le front descendant d'un boolean                                                           */
/* Sortie : TRUE sur le boolean vient de passer à DOWN                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_DO_down ( gchar *tech_id, gchar *acronyme, gpointer *dout_p )
  { struct DLS_DO *dout;
    GSList *liste;
    if (dout_p && *dout_p)                                                           /* Si pointeur d'acceleration disponible */
     { dout = (struct DLS_DO *)*dout_p;
       return( dout->edge_down );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_DO;
    while (liste)
     { dout = (struct DLS_DO *)liste->data;
       if ( !strcasecmp ( dout->acronyme, acronyme ) && !strcasecmp( dout->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (dout_p) *dout_p = (gpointer)dout;                                           /* Sauvegarde pour acceleration si besoin */
    return( dout->edge_down );
  }
/******************************************************************************************************************************/
/* Met à jour l'entrée analogique num à partir de sa valeur avant mise a l'echelle                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_AI ( gchar *tech_id, gchar *acronyme, gpointer *ai_p, float val_avant_ech, gboolean in_range )
  { struct DLS_AI *ai;
    gboolean need_arch;

    if (!ai_p || !*ai_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_AI;
       while (liste)
        { ai = (struct DLS_AI *)liste->data;
          if ( !strcasecmp ( ai->acronyme, acronyme ) && !strcasecmp( ai->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { ai = g_try_malloc0 ( sizeof(struct DLS_AI) );
          if (!ai)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( ai->acronyme, sizeof(ai->acronyme), "%s", acronyme );
          g_snprintf( ai->tech_id,  sizeof(ai->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_AI = g_slist_prepend ( Partage->Dls_data_AI, ai );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s : adding AI '%s:%s'", __func__, tech_id, acronyme );
        }
       if (ai_p) *ai_p = (gpointer)ai;                                              /* Sauvegarde pour acceleration si besoin */
      }
    else ai = (struct DLS_AI *)*ai_p;

    need_arch = FALSE;
    if ( (ai->val_avant_ech != val_avant_ech) || (ai->inrange != in_range) )
     { ai->val_avant_ech = val_avant_ech;                                           /* Archive au mieux toutes les 5 secondes */
       if ( ai->last_arch + ARCHIVE_EA_TEMPS_SI_VARIABLE < Partage->top ) { need_arch = TRUE; }

       switch ( ai->type )
        { case ENTREEANA_NON_INTERP:
               ai->val_ech = val_avant_ech;                                                        /* Pas d'interprétation !! */
               ai->inrange = in_range;
               break;
          case ENTREEANA_4_20_MA_10BITS:
               if (val_avant_ech < 100)                                                /* 204) Modification du range pour 4mA */
                { ai->val_ech = 0.0;                                                                    /* Valeur à l'echelle */
                  ai->inrange = 0;
                }
               else
                { if (val_avant_ech < 204) val_avant_ech = 204;                                         /* Valeur à l'echelle */
                  ai->val_ech = (gfloat) ((val_avant_ech-204)*(ai->max - ai->min))/820.0 + ai->min;
                  ai->inrange = 1;
                }
               break;
          case ENTREEANA_4_20_MA_12BITS:
               if (val_avant_ech < 400)
                { ai->val_ech = 0.0;                                                                    /* Valeur à l'echelle */
                  ai->inrange = 0;
                }
               else
                { if (val_avant_ech < 816) val_avant_ech = 816;                                         /* Valeur à l'echelle */
                  ai->val_ech = (gfloat) ((val_avant_ech-816)*(ai->max - ai->min))/3280.0 + ai->min;
                  ai->inrange = 1;
                }
               break;
          case ENTREEANA_WAGO_750455:                                                                              /* 4/20 mA */
               if (in_range)
                { ai->val_ech = (gfloat) (val_avant_ech*(ai->max - ai->min))/4095.0 + ai->min; }
               ai->inrange = in_range;                                           /* InRange dependant d'un autre champ ModBus */
               break;
          case ENTREEANA_WAGO_750461:                                                                          /* Borne PT100 */
               if (val_avant_ech > -2000 && val_avant_ech < 8500)
                { ai->val_ech = (gfloat)(val_avant_ech/10.0);                                           /* Valeur à l'echelle */
                  ai->inrange = 1;
                }
               else ai->inrange = 0;
               break;
          default:
               ai->val_ech = 0.0;
               ai->inrange = 0;
        }
     }
    else if ( ai->last_arch + ARCHIVE_EA_TEMPS_SI_CONSTANT < Partage->top )
     { need_arch = TRUE; }                                                               /* Archive au pire toutes les 10 min */

    if (need_arch)
     { Ajouter_arch_by_nom( ai->acronyme, ai->tech_id, ai->val_ech );                              /* Archivage si besoin */
       ai->last_arch = Partage->top;
     }
  }
/******************************************************************************************************************************/
/* Met à jour la sortie analogique à partir de sa valeur avant mise a l'echelle                                               */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_AO ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *ao_p, float val_avant_ech )
  { struct DLS_AO *ao;
    gboolean need_arch;

    if (!ao_p || !*ao_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_AO;
       while (liste)
        { ao = (struct DLS_AO *)liste->data;
          if ( !strcasecmp ( ao->acronyme, acronyme ) && !strcasecmp( ao->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { ao = g_try_malloc0 ( sizeof(struct DLS_AO) );
          if (!ao)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( ao->acronyme, sizeof(ao->acronyme), "%s", acronyme );
          g_snprintf( ao->tech_id,  sizeof(ao->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_AO = g_slist_prepend ( Partage->Dls_data_AO, ao );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s : adding AO '%s:%s'", __func__, tech_id, acronyme );
        }
       if (ao_p) *ao_p = (gpointer)ao;                                              /* Sauvegarde pour acceleration si besoin */
      }
    else ao = (struct DLS_AO *)*ao_p;

    need_arch = FALSE;
    if (ao->val_avant_ech != val_avant_ech)
     { ao->val_avant_ech = val_avant_ech;                                           /* Archive au mieux toutes les 5 secondes */
       if ( ao->last_arch + ARCHIVE_EA_TEMPS_SI_VARIABLE < Partage->top ) { need_arch = TRUE; }

       switch ( ao->type )
        { case 0: /*SORTIEANA_NON_INTERP:*/
               ao->val_ech = val_avant_ech;                                                        /* Pas d'interprétation !! */
               break;
#ifdef bouh
          case ENTREEANA_4_20_MA_10BITS:
               if (val_avant_ech < 100)                                                /* 204) Modification du range pour 4mA */
                { ai->val_ech = 0.0;                                                                    /* Valeur à l'echelle */
                  ai->inrange = 0;
                }
               else
                { if (val_avant_ech < 204) val_avant_ech = 204;                                         /* Valeur à l'echelle */
                  ai->val_ech = (gfloat) ((val_avant_ech-204)*(ai->max - ai->min))/820.0 + ai->min;
                  ai->inrange = 1;
                }
               break;
          case ENTREEANA_4_20_MA_12BITS:
               if (val_avant_ech < 400)
                { ai->val_ech = 0.0;                                                                    /* Valeur à l'echelle */
                  ai->inrange = 0;
                }
               else
                { if (val_avant_ech < 816) val_avant_ech = 816;                                         /* Valeur à l'echelle */
                  ai->val_ech = (gfloat) ((val_avant_ech-816)*(ai->max - ai->min))/3280.0 + ai->min;
                  ai->inrange = 1;
                }
               break;
          case ENTREEANA_WAGO_750455:                                                                              /* 4/20 mA */
               ai->val_ech = (gfloat) (val_avant_ech*(ai->max - ai->min))/4095.0 + ai->min;
               ai->inrange = 1;
               break;
          case ENTREEANA_WAGO_750461:                                                                          /* Borne PT100 */
               if (val_avant_ech > -32767 && val_avant_ech < 8500)
                { ai->val_ech = (gfloat)(val_avant_ech/10.0);                                           /* Valeur à l'echelle */
                  ai->inrange = 1;
                }
               else ai->inrange = 0;
               break;
#endif
          default:
               ao->val_ech = 0.0;
        }
       pthread_mutex_lock( &Partage->com_msrv.synchro );                        /* Ajout dans la liste de msg a traiter */
       Partage->com_msrv.Liste_AO = g_slist_append( Partage->com_msrv.Liste_AO, ao );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       Info_new( Config.log, (vars ? vars->debug : Partage->com_dls.Thread_debug), LOG_DEBUG, "%s : Changing DLS_AO '%s:%s'=%f/%f",
                 __func__, ao->tech_id, ao->acronyme, ao->val_avant_ech, ao->val_ech );
     }
    else if ( ao->last_arch + ARCHIVE_EA_TEMPS_SI_CONSTANT < Partage->top )
     { need_arch = TRUE; }                                                               /* Archive au pire toutes les 10 min */

    if (need_arch)
     { Ajouter_arch_by_nom( ao->acronyme, ao->tech_id, ao->val_ech );                                  /* Archivage si besoin */
       ao->last_arch = Partage->top;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_set_INT: Positionne un integer dans la mémoire DLS                                                                */
/* Entrée: le tech_id, l'acronyme, le pointeur d'accélération et la valeur entière                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_CI ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *cpt_imp_p, gboolean etat, gint reset, gint ratio )
  { struct DLS_CI *cpt_imp;

    if (!cpt_imp_p || !*cpt_imp_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_CI;
       while (liste)
        { cpt_imp = (struct DLS_CI *)liste->data;
          if ( !strcasecmp ( cpt_imp->acronyme, acronyme ) && !strcasecmp( cpt_imp->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { cpt_imp = g_try_malloc0 ( sizeof(struct DLS_CI) );
          if (!cpt_imp)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( cpt_imp->acronyme, sizeof(cpt_imp->acronyme), "%s", acronyme );
          g_snprintf( cpt_imp->tech_id,  sizeof(cpt_imp->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_CI = g_slist_prepend ( Partage->Dls_data_CI, cpt_imp );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s : adding CI '%s:%s'", __func__, tech_id, acronyme );
          Charger_conf_CI ( cpt_imp );                                     /* Chargement des valeurs en base pour ce compteur */
        }
       if (cpt_imp_p) *cpt_imp_p = (gpointer)cpt_imp;                   /* Sauvegarde pour acceleration si besoin */
      }
    else cpt_imp = (struct DLS_CI *)*cpt_imp_p;

    gboolean need_arch = FALSE;
    if (etat)
     { if (reset)                                                                       /* Le compteur doit-il etre resetté ? */
        { if (cpt_imp->valeur!=0)
           { cpt_imp->val_en_cours1 = 0;                                           /* Valeur transitoire pour gérer les ratio */
             cpt_imp->valeur = 0;                                                  /* Valeur transitoire pour gérer les ratio */
             need_arch = TRUE;
           }
        }
       else if ( cpt_imp->etat == FALSE )                                                                 /* Passage en actif */
        { cpt_imp->etat = TRUE;
          Partage->audit_bit_interne_per_sec++;
          cpt_imp->val_en_cours1++;
          if (cpt_imp->val_en_cours1>=ratio)
           { cpt_imp->valeur++;
             cpt_imp->val_en_cours1=0;                                                        /* RAZ de la valeur de calcul 1 */
             need_arch = TRUE;
             Info_new( Config.log, (vars ? vars->debug : Partage->com_dls.Thread_debug), LOG_DEBUG, "%s : Changing DLS_CI '%s:%s'=%d",
                       __func__, cpt_imp->tech_id, cpt_imp->acronyme, cpt_imp->valeur );
           }
        }
     }
    else
     { if (reset==0) cpt_imp->etat = FALSE; }

    if ( cpt_imp->last_update + 10 <= Partage->top )                                                    /* Toutes les secondes */
     { memcpy( &cpt_imp->valeurs[0], &cpt_imp->valeurs[1], 59*sizeof(cpt_imp->valeurs[0]) );
       cpt_imp->valeurs[59] = cpt_imp->valeur;
       cpt_imp->imp_par_minute = cpt_imp->valeur - cpt_imp->valeurs[0];
     }

    if ( (cpt_imp->archivage == 1 && need_arch == TRUE) ||
         (cpt_imp->archivage == 2 && cpt_imp->last_arch + 600    <= Partage->top) ||
         (cpt_imp->archivage == 3 && cpt_imp->last_arch + 36000  <= Partage->top) ||
         (cpt_imp->archivage == 4 && cpt_imp->last_arch + 864000 <= Partage->top)
       )
     { Ajouter_arch_by_nom( cpt_imp->acronyme, cpt_imp->tech_id, cpt_imp->valeur*1.0 );             /* Archivage si besoin */
       cpt_imp->last_arch = Partage->top;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_CI : Recupere la valeur de l'EA en parametre                                                             */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gint Dls_data_get_CI ( gchar *tech_id, gchar *acronyme, gpointer *cpt_imp_p )
  { struct DLS_CI *cpt_imp;
    GSList *liste;
    if (cpt_imp_p && *cpt_imp_p)                                                     /* Si pointeur d'acceleration disponible */
     { cpt_imp = (struct DLS_CI *)*cpt_imp_p;
       return( cpt_imp->valeur );
     }
    if (!tech_id || !acronyme) return(0.0);

    liste = Partage->Dls_data_CI;
    while (liste)
     { cpt_imp = (struct DLS_CI *)liste->data;
       if ( !strcasecmp ( cpt_imp->acronyme, acronyme ) && !strcasecmp( cpt_imp->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(0);
    if (cpt_imp_p) *cpt_imp_p = (gpointer)cpt_imp;                                  /* Sauvegarde pour acceleration si besoin */
    return( cpt_imp->valeur );
  }
/******************************************************************************************************************************/
/* Dls_data_set_INT: Positionne un integer dans la mémoire DLS                                                                */
/* Entrée: le tech_id, l'acronyme, le pointeur d'accélération et la valeur entière                                            */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_CH ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *cpt_h_p, gboolean etat, gint reset )
  { struct DLS_CH *cpt_h;

    if (!cpt_h_p || !*cpt_h_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_CH;
       while (liste)
        { cpt_h = (struct DLS_CH *)liste->data;
          if ( !strcasecmp ( cpt_h->acronyme, acronyme ) && !strcasecmp( cpt_h->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { cpt_h = g_try_malloc0 ( sizeof(struct DLS_CH) );
          if (!cpt_h)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( cpt_h->acronyme, sizeof(cpt_h->acronyme), "%s", acronyme );
          g_snprintf( cpt_h->tech_id,  sizeof(cpt_h->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_CH = g_slist_prepend ( Partage->Dls_data_CH, cpt_h );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s : adding CH '%s:%s'", __func__, tech_id, acronyme );
          Charger_conf_CH ( cpt_h );                                       /* Chargement des valeurs en base pour ce compteur */
        }
       if (cpt_h_p) *cpt_h_p = (gpointer)cpt_h;                                     /* Sauvegarde pour acceleration si besoin */
      }
    else cpt_h = (struct DLS_CH *)*cpt_h_p;

    if (reset)
     { if (etat)
        { cpt_h->valeur = 0;
          cpt_h->etat = FALSE;
        }
     }
    else if (etat)
     { if ( ! cpt_h->etat )
        { cpt_h->etat = TRUE;
          cpt_h->old_top = Partage->top;
        }
       else
        { int new_top, delta;
          new_top = Partage->top;
          delta = new_top - cpt_h->old_top;
          if (delta >= 10)                                                              /* On compte +1 toutes les secondes ! */
           { cpt_h->valeur++;
             cpt_h->old_top = new_top;
             Info_new( Config.log, (vars ? vars->debug : Partage->com_dls.Thread_debug), LOG_DEBUG, "%s : Changing DLS_CH '%s:%s'=%d",
                       __func__, cpt_h->tech_id, cpt_h->acronyme, cpt_h->valeur );
             Partage->audit_bit_interne_per_sec++;
           }
          if (cpt_h->last_arch + 600 < Partage->top)
           { Ajouter_arch_by_nom( cpt_h->acronyme, cpt_h->tech_id, 1.0*cpt_h->valeur );
             cpt_h->last_arch = Partage->top;
           }
        }
     }
    else
     { cpt_h->etat = FALSE; }
  }
/******************************************************************************************************************************/
/* Dls_data_get_CI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gint Dls_data_get_CH ( gchar *tech_id, gchar *acronyme, gpointer *cpt_h_p )
  { struct DLS_CH *cpt_h;
    GSList *liste;
    if (cpt_h_p && *cpt_h_p)                                                         /* Si pointeur d'acceleration disponible */
     { cpt_h = (struct DLS_CH *)*cpt_h_p;
       return( cpt_h->valeur );
     }
    if (!tech_id || !acronyme) return(0.0);

    liste = Partage->Dls_data_CH;
    while (liste)
     { cpt_h = (struct DLS_CH *)liste->data;
       if ( !strcasecmp ( cpt_h->acronyme, acronyme ) && !strcasecmp( cpt_h->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(0);
    if (cpt_h_p) *cpt_h_p = (gpointer)cpt_h;                                        /* Sauvegarde pour acceleration si besoin */
    return( cpt_h->valeur );
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gfloat Dls_data_get_AI ( gchar *tech_id, gchar *acronyme, gpointer *ai_p )
  { struct DLS_AI *ai;
    GSList *liste;
    if (ai_p && *ai_p)                                                               /* Si pointeur d'acceleration disponible */
     { ai = (struct DLS_AI *)*ai_p;
       return( ai->val_ech );
     }
    if (!tech_id || !acronyme) return(0.0);

    liste = Partage->Dls_data_AI;
    while (liste)
     { ai = (struct DLS_AI *)liste->data;
       if ( !strcasecmp ( ai->acronyme, acronyme ) && !strcasecmp( ai->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(0.0);
    if (ai_p) *ai_p = (gpointer)ai;                                                 /* Sauvegarde pour acceleration si besoin */
    return( ai->val_ech );
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gfloat Dls_data_get_AO ( gchar *tech_id, gchar *acronyme, gpointer *ao_p )
  { struct DLS_AO *ao;
    GSList *liste;
    if (ao_p && *ao_p)                                                               /* Si pointeur d'acceleration disponible */
     { ao = (struct DLS_AO *)*ao_p;
       return( ao->val_avant_ech );
     }
    if (!tech_id || !acronyme) return(0.0);

    liste = Partage->Dls_data_AO;
    while (liste)
     { ao = (struct DLS_AO *)liste->data;
       if ( !strcasecmp ( ao->acronyme, acronyme ) && !strcasecmp( ao->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(0.0);
    if (ao_p) *ao_p = (gpointer)ao;                                                 /* Sauvegarde pour acceleration si besoin */
    return( ao->val_avant_ech );
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_AI_inrange ( gchar *tech_id, gchar *acronyme, gpointer *ai_p )
  { struct DLS_AI *ai;
    Dls_data_get_AI ( tech_id, acronyme, ai_p );
    if (ai_p && *ai_p)                                                               /* Si pointeur d'acceleration disponible */
     { ai = (struct DLS_AI *)*ai_p;
       return( ai->inrange );
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Dls_data_set_tempo : Gestion du positionnement des tempos DLS en mode dynamique                                            */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et la valeur on ou off de la tempo                            */
/******************************************************************************************************************************/
 void Dls_data_set_tempo ( gchar *tech_id, gchar *acronyme, gpointer *tempo_p, gboolean etat,
                           gint delai_on, gint min_on, gint max_on, gint delai_off, gint random)
  { struct DLS_TEMPO *tempo;

    if (!tempo_p || !*tempo_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_TEMPO;
       while (liste)
        { tempo = (struct DLS_TEMPO *)liste->data;
          if ( !strcasecmp ( tempo->acronyme, acronyme ) && !strcasecmp( tempo->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { tempo = g_try_malloc0 ( sizeof(struct DLS_TEMPO) );
          if (!tempo)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( tempo->acronyme, sizeof(tempo->acronyme), "%s", acronyme );
          g_snprintf( tempo->tech_id,  sizeof(tempo->tech_id),  "%s", tech_id );
          tempo->init = FALSE;
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_TEMPO = g_slist_prepend ( Partage->Dls_data_TEMPO, tempo );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s : adding TEMPO '%s:%s'", __func__, tech_id, acronyme );
        }
       if (tempo_p) *tempo_p = (gpointer)tempo;                                     /* Sauvegarde pour acceleration si besoin */
      }
    else tempo = (struct DLS_TEMPO *)*tempo_p;

    if (tempo->init == FALSE)
     { tempo->delai_on  = delai_on;
       tempo->min_on    = min_on;
       tempo->max_on    = max_on;
       tempo->delai_off = delai_off;
       tempo->random    = random;
       tempo->init      = TRUE;
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s : Initializing TEMPO '%s:%s'", __func__, tech_id, acronyme );
     }
    ST_local ( tempo, etat );                                                                     /* Recopie dans la variable */
  }
/******************************************************************************************************************************/
/* Dls_data_get_tempo : Gestion du positionnement des tempos DLS en mode dynamique                                            */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_tempo ( gchar *tech_id, gchar *acronyme, gpointer *tempo_p )
  { struct DLS_TEMPO *tempo;
    GSList *liste;
    if (tempo_p && *tempo_p)                                                         /* Si pointeur d'acceleration disponible */
     { tempo = (struct DLS_TEMPO *)*tempo_p;
       return( tempo->state );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_TEMPO;
    while (liste)
     { tempo = (struct DLS_TEMPO *)liste->data;
       if ( !strcasecmp ( tempo->acronyme, acronyme ) && !strcasecmp( tempo->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (tempo_p) *tempo_p = (gpointer)tempo;                                        /* Sauvegarde pour acceleration si besoin */
    return( tempo->state );
  }
/******************************************************************************************************************************/
/* Dls_data_set_bus : Envoi un message sur le bus système                                                                     */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et les paramètres du message                                  */
/******************************************************************************************************************************/
 void Dls_data_set_bus ( gchar *tech_id, gchar *acronyme, gpointer *bus_p, gboolean etat,
                         gchar *host, gchar *thread, gchar *tag, gchar *param1)
  { Dls_data_set_bool ( NULL, tech_id, acronyme, bus_p, etat );                                   /* Utilisation d'un boolean */
    if (Dls_data_get_bool_up(tech_id, acronyme, bus_p))
     { if (param1)
        { Send_zmq_with_tag ( Partage->com_dls.zmq_to_master, NULL, "dls", host, thread, tag, param1, strlen(param1)+1 ); }
       else
        { Send_zmq_with_tag ( Partage->com_dls.zmq_to_master, NULL, "dls", host, thread, tag, NULL, 0 ); }
     }
    if (param1) g_free(param1);                                       /* Param1 est issu d'un g_strdup ou d'un Dls_dyn_string */
  }
/******************************************************************************************************************************/
/* Met à jour le message en parametre                                                                                         */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_MSG ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *msg_p, gboolean update, gboolean etat )
  { struct DLS_MESSAGES *msg;

    if (!msg_p || !*msg_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_MSG;
       while (liste)
        { msg = (struct DLS_MESSAGES *)liste->data;
          if ( !strcasecmp ( msg->acronyme, acronyme ) && !strcasecmp( msg->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { msg = g_try_malloc0 ( sizeof(struct DLS_MESSAGES) );
          if (!msg)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( msg->acronyme, sizeof(msg->acronyme), "%s", acronyme );
          g_snprintf( msg->tech_id,  sizeof(msg->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_MSG = g_slist_prepend ( Partage->Dls_data_MSG, msg );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s : adding MSG '%s:%s'", __func__, tech_id, acronyme );
        }
       if (msg_p) *msg_p = (gpointer)msg;                                           /* Sauvegarde pour acceleration si besoin */
      }
    else msg = (struct DLS_MESSAGES *)*msg_p;

    if ( update )
     { if (etat == FALSE) { msg->etat_update = FALSE; }
       else if (msg->etat == TRUE && msg->etat_update == FALSE)
        { struct DLS_MESSAGES_EVENT *event;
          msg->etat_update = TRUE;
          event = (struct DLS_MESSAGES_EVENT *)g_try_malloc0( sizeof (struct DLS_MESSAGES_EVENT) );
          if (!event)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
                      "%s: malloc Event failed. Memory error for Updating MSG'%s:%s'", __func__, msg->tech_id, msg->acronyme );
           }
          else
           { event->etat = FALSE;                                                       /* Recopie de l'état dans l'evenement */
             event->msg  = msg;
             pthread_mutex_lock( &Partage->com_msrv.synchro );                        /* Ajout dans la liste de msg a traiter */
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
             pthread_mutex_unlock( &Partage->com_msrv.synchro );
           }
          event = (struct DLS_MESSAGES_EVENT *)g_try_malloc0( sizeof (struct DLS_MESSAGES_EVENT) );
          if (!event)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
                      "%s: malloc Event failed. Memory error for Updating MSG'%s:%s'", __func__, msg->tech_id, msg->acronyme );
           }
          else
           { event->etat = TRUE;                                                        /* Recopie de l'état dans l'evenement */
             event->msg  = msg;
             pthread_mutex_lock( &Partage->com_msrv.synchro );                        /* Ajout dans la liste de msg a traiter */
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
             pthread_mutex_unlock( &Partage->com_msrv.synchro );
           }
        }
     }
    else if ( msg->etat != etat )
     { msg->etat = etat;
       if (etat) msg->etat_update = TRUE;

       if ( msg->last_change + 10 <= Partage->top ) { msg->changes = 0; }            /* Si pas de change depuis plus de 1 sec */

       if ( msg->changes > 5 && !(Partage->top % 50) )              /* Si persistence d'anomalie on prévient toutes les 5 sec */
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: last_change trop tot for MSG '%s':'%s' !", __func__,
                    msg->tech_id, msg->acronyme );
        }
       else
        { struct DLS_MESSAGES_EVENT *event;
          event = (struct DLS_MESSAGES_EVENT *)g_try_malloc0( sizeof (struct DLS_MESSAGES_EVENT) );
          if (!event)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
                      "%s: malloc Event failed. Memory error for MSG'%s:%s'", __func__, msg->tech_id, msg->acronyme );
           }
          else
           { event->etat = etat;                                                        /* Recopie de l'état dans l'evenement */
             event->msg  = msg;
             pthread_mutex_lock( &Partage->com_msrv.synchro );                        /* Ajout dans la liste de msg a traiter */
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
             pthread_mutex_unlock( &Partage->com_msrv.synchro );
           }

          Info_new( Config.log, (vars ? vars->debug : Partage->com_dls.Thread_debug), LOG_DEBUG, "%s : Changing DLS_MSG '%s:%s'=%d",
                    __func__, msg->tech_id, msg->acronyme, msg->etat );
          msg->changes++;
          msg->last_change = Partage->top;
          Partage->audit_bit_interne_per_sec++;
        }
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gboolean Dls_data_get_MSG ( gchar *tech_id, gchar *acronyme, gpointer *msg_p )
  { struct DLS_MESSAGES *msg;
    GSList *liste;
    if (msg_p && *msg_p)                                                             /* Si pointeur d'acceleration disponible */
     { msg = (struct DLS_MESSAGES *)*msg_p;
       return( msg->etat );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_MSG;
    while (liste)
     { msg = (struct DLS_MESSAGES *)liste->data;
       if ( !strcasecmp ( msg->acronyme, acronyme ) && !strcasecmp( msg->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (msg_p) *msg_p = (gpointer)msg;                                              /* Sauvegarde pour acceleration si besoin */
    return( msg->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_set_tempo : Gestion du positionnement des tempos DLS en mode dynamique                                            */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et la valeur on ou off de la tempo                            */
/******************************************************************************************************************************/
 void Dls_data_set_VISUEL ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *visu_p, gint mode,
                            gchar *color, gboolean cligno )
  { struct DLS_VISUEL *visu;

    if (!visu_p || !*visu_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_VISUEL;
       while (liste)
        { visu = (struct DLS_VISUEL *)liste->data;
          if ( !strcasecmp ( visu->acronyme, acronyme ) && !strcasecmp( visu->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { visu = g_try_malloc0 ( sizeof(struct DLS_VISUEL) );
          if (!visu)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( visu->acronyme, sizeof(visu->acronyme), "%s", acronyme );
          g_snprintf( visu->tech_id,  sizeof(visu->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_VISUEL = g_slist_prepend ( Partage->Dls_data_VISUEL, visu );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s : adding VISUEL '%s:%s'", __func__, tech_id, acronyme );
        }
       if (visu_p) *visu_p = (gpointer)visu;                                        /* Sauvegarde pour acceleration si besoin */
      }
    else visu = (struct DLS_VISUEL *)*visu_p;

    if (visu->mode != mode || strcmp( visu->color, color ) || visu->cligno != cligno )
     { if ( visu->last_change + 50 <= Partage->top )                                 /* Si pas de change depuis plus de 5 sec */
        { visu->changes = 0; }

       if ( visu->changes <= 10 )                                                          /* Si moins de 10 changes en 5 sec */
        { if ( visu->changes == 10 )                                                /* Est-ce le dernier change avant blocage */
           { visu->mode   = 0;                                                   /* Si oui, on passe le visuel en kaki cligno */
             g_snprintf( visu->color, sizeof(visu->color), "brown" );
             visu->cligno = 1;                                                                                  /* Clignotant */
           }
          else { visu->mode   = mode;                                /* Sinon on recopie ce qui est demandé par le plugin DLS */
                 g_snprintf( visu->color, sizeof(visu->color), "%s", color );
                 visu->cligno = cligno;
               }

          visu->last_change = Partage->top;                                                             /* Date de la photo ! */
          pthread_mutex_lock( &Partage->com_msrv.synchro );                             /* Ajout dans la liste de i a traiter */
          Partage->com_msrv.liste_visuel = g_slist_append( Partage->com_msrv.liste_visuel, visu );
          pthread_mutex_unlock( &Partage->com_msrv.synchro );
          Info_new( Config.log, (vars ? vars->debug : Partage->com_dls.Thread_debug), LOG_DEBUG, "%s : Changing DLS_VISUEL '%s:%s'-> mode %d color %s cligne %d",
                    __func__, visu->tech_id, visu->acronyme, visu->mode, visu->color, visu->cligno );
        }
       visu->changes++;                                                                                /* Un change de plus ! */
       Partage->audit_bit_interne_per_sec++;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gint Dls_data_get_VISUEL ( gchar *tech_id, gchar *acronyme, gpointer *visu_p )
  { struct DLS_VISUEL *visu;
    GSList *liste;
    if (visu_p && *visu_p)                                                             /* Si pointeur d'acceleration disponible */
     { visu = (struct DLS_VISUEL *)*visu_p;
       return( visu->mode );
     }
    if (!tech_id || !acronyme) return(0);

    liste = Partage->Dls_data_VISUEL;
    while (liste)
     { visu = (struct DLS_VISUEL *)liste->data;
       if ( !strcasecmp ( visu->acronyme, acronyme ) && !strcasecmp( visu->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(0);
    if (visu_p) *visu_p = (gpointer)visu;                                           /* Sauvegarde pour acceleration si besoin */
    return( visu->mode );
  }
/******************************************************************************************************************************/
/* Dls_data_set_R: Positionne un registre                                                                                     */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_R ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *r_p, gfloat valeur )
  { struct DLS_REGISTRE *reg;

    if (!r_p || !*r_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_REGISTRE;
       while (liste)
        { reg = (struct DLS_REGISTRE *)liste->data;
          if ( !strcasecmp ( reg->acronyme, acronyme ) && !strcasecmp( reg->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { reg = g_try_malloc0 ( sizeof(struct DLS_REGISTRE) );
          if (!reg)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
                       "%s : Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( reg->acronyme, sizeof(reg->acronyme), "%s", acronyme );
          g_snprintf( reg->tech_id,  sizeof(reg->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_REGISTRE = g_slist_prepend ( Partage->Dls_data_REGISTRE, reg );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
                    "%s : adding DLS_REGISTRE '%s:%s'", __func__, tech_id, acronyme );
        }
       if (r_p) *r_p = (gpointer)reg;                                               /* Sauvegarde pour acceleration si besoin */
      }
    else reg = (struct DLS_REGISTRE *)*r_p;

    gboolean need_arch = FALSE;
    if (valeur != reg->valeur)
     { reg->valeur = valeur;
       need_arch = TRUE;
       Info_new( Config.log, (vars ? vars->debug : Partage->com_dls.Thread_debug), LOG_DEBUG, "%s : Changing DLS_REGISTRE '%s:%s'=%f",
                 __func__, reg->tech_id, reg->acronyme, reg->valeur );
       Partage->audit_bit_interne_per_sec++;
     }

    if ( (reg->archivage == 1 && need_arch == TRUE) ||
         (reg->archivage == 2 && reg->last_arch + 600    <= Partage->top) ||
         (reg->archivage == 3 && reg->last_arch + 36000  <= Partage->top) ||
         (reg->archivage == 4 && reg->last_arch + 864000 <= Partage->top)
       )
     { Ajouter_arch_by_nom( reg->acronyme, reg->tech_id, reg->valeur );                                /* Archivage si besoin */
       reg->last_arch = Partage->top;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_reg: Remonte l'etat d'un registre                                                                             */
/* Sortie : TRUE sur le regean est UP                                                                                         */
/******************************************************************************************************************************/
 gfloat Dls_data_get_R ( gchar *tech_id, gchar *acronyme, gpointer *r_p )
  { struct DLS_REGISTRE *reg;
    GSList *liste;
    if (r_p && *r_p)                                                             /* Si pointeur d'acceleration disponible */
     { reg = (struct DLS_REGISTRE *)*r_p;
       return( reg->valeur );
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_REGISTRE;
    while (liste)
     { reg = (struct DLS_REGISTRE *)liste->data;
       if ( !strcasecmp ( reg->acronyme, acronyme ) && !strcasecmp( reg->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(FALSE);
    if (r_p) *r_p = (gpointer)reg;                                              /* Sauvegarde pour acceleration si besoin */
    return( reg->valeur );
  }
/******************************************************************************************************************************/
/* Dls_dyn_string: Formate la chaine en parametre avec le bit également en parametre                                          */
/* Entrée : La chaine source, le type de bit, le tech_id/acronyme, le pointeur de raccourci                                   */
/* sortie : Une nouvelle chaine de caractere à g_freer                                                                        */
/******************************************************************************************************************************/
 gchar *Dls_dyn_string ( gchar *format, gint type_bit, gchar *tech_id, gchar *acronyme, gpointer *dlsdata_p )
  { gchar result[128], *debut, chaine[64];
    debut = g_strrstr ( format, "$1" );                            /* Début pointe sur le $ de "$1" si présent dans la chaine */
    if (!debut) return(g_strdup(format));
    g_snprintf( result, debut-format+1, "%s", format );                                                           /* Prologue */
    switch (type_bit)
     { case MNEMO_CPT_IMP:
             { struct DLS_CI *ci = *dlsdata_p;
               g_snprintf( chaine, sizeof(chaine), "%d %s", ci->valeur, ci->unite ); /* Row1 = unite */
             }
            break;
       case MNEMO_ENTREE_ANA:
            if (!strcasecmp(tech_id, "SYS") && !strcasecmp(acronyme, "TIME"))
             { struct tm tm;
               time_t temps;
               time(&temps);
               localtime_r( &temps, &tm );
               g_snprintf( chaine, sizeof(chaine), "%d heure et %d minute", tm.tm_hour, tm.tm_min );
             }
            else
             { Dls_data_get_AI ( tech_id, acronyme, dlsdata_p );
               struct DLS_AI *ai = *dlsdata_p;
               if (ai)
                { if (ai->val_ech-roundf(ai->val_ech) == 0.0)
                   { g_snprintf( chaine, sizeof(chaine), "%.0f %s", ai->val_ech, ai->unite ); }
                  else
                   { g_snprintf( chaine, sizeof(chaine), "%.2f %s", ai->val_ech, ai->unite ); }
                }
               else g_snprintf( chaine, sizeof(chaine), "erreur" );
             }
            break;
       case MNEMO_REGISTRE:
             { Dls_data_get_R ( tech_id, acronyme, dlsdata_p );
               struct DLS_REGISTRE *reg = *dlsdata_p;
               if (reg)
                { if (reg->valeur-roundf(reg->valeur) == 0.0)
                   { g_snprintf( chaine, sizeof(chaine), "%.0f %s", reg->valeur, reg->unite ); }
                  else
                   { g_snprintf( chaine, sizeof(chaine), "%.2f %s", reg->valeur, reg->unite ); }
                }
               else g_snprintf( chaine, sizeof(chaine), "erreur" );
             }
            break;
       default: return(NULL);
     }
    g_strlcat ( result, chaine, sizeof(result) );
    g_strlcat ( result, debut+2, sizeof(result) );
    return(g_strdup(result));
  }
/******************************************************************************************************************************/
/* Dls_run_dls_tree: Fait tourner les DLS synoptique en parametre + les sous DLS                                              */
/* Entrée : le Dls_tree correspondant                                                                                         */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_run_dls_tree ( struct DLS_TREE *dls_tree )
  { struct timeval tv_avant, tv_apres;
    gboolean bit_comm_out, bit_defaut, bit_defaut_fixe, bit_alarme, bit_alarme_fixe;                              /* Activité */
    gboolean bit_veille_partielle, bit_veille_totale, bit_alerte, bit_alerte_fixe;             /* Synthese Sécurité des Biens */
    gboolean bit_alerte_fugitive;
    gboolean bit_derangement, bit_derangement_fixe, bit_danger, bit_danger_fixe;           /* synthèse Sécurité des Personnes */
    GSList *liste;

    bit_comm_out = bit_defaut = bit_defaut_fixe = bit_alarme = bit_alarme_fixe = FALSE;
    bit_veille_partielle = FALSE;
    bit_veille_totale = TRUE;
    bit_alerte = bit_alerte_fixe = bit_alerte_fugitive = FALSE;
    bit_derangement = bit_derangement_fixe = bit_danger = bit_danger_fixe = FALSE;

    liste = dls_tree->Liste_plugin_dls;
    while(liste)                                                                     /* On execute tous les modules un par un */
     { struct PLUGIN_DLS *plugin_actuel;
       plugin_actuel = (struct PLUGIN_DLS *)liste->data;

       if (plugin_actuel->plugindb.on && plugin_actuel->go)
        { gettimeofday( &tv_avant, NULL );
          Partage->top_cdg_plugin_dls = 0;                                                      /* On reset le cdg plugin DLS */
          plugin_actuel->go( &plugin_actuel->vars );                                                    /* On appel le plugin */
          gettimeofday( &tv_apres, NULL );
          plugin_actuel->conso+=Chrono( &tv_avant, &tv_apres );
          plugin_actuel->vars.starting = 0;

          plugin_actuel->vars.bit_acquit = 0;                                                 /* On arrete l'acquit du plugin */
                                                                                                  /* Bit de synthese activite */
          bit_comm_out         |= plugin_actuel->vars.bit_comm_out;
          bit_defaut           |= plugin_actuel->vars.bit_defaut;
          bit_defaut_fixe      |= plugin_actuel->vars.bit_defaut_fixe;
          bit_alarme           |= plugin_actuel->vars.bit_alarme;
          bit_alarme_fixe      |= plugin_actuel->vars.bit_alarme_fixe;
          plugin_actuel->vars.bit_activite_ok = !(bit_comm_out | bit_defaut | bit_defaut_fixe | bit_alarme | bit_alarme_fixe);

          bit_veille_partielle |= plugin_actuel->vars.bit_veille;
          bit_veille_totale    &= plugin_actuel->vars.bit_veille;
          bit_alerte           |= plugin_actuel->vars.bit_alerte;
          bit_alerte_fixe      |= plugin_actuel->vars.bit_alerte_fixe;
          bit_alerte_fugitive  |= plugin_actuel->vars.bit_alerte_fugitive;

          bit_derangement      |= plugin_actuel->vars.bit_derangement;
          bit_derangement_fixe |= plugin_actuel->vars.bit_derangement_fixe;
          bit_danger           |= plugin_actuel->vars.bit_danger;
          bit_danger_fixe      |= plugin_actuel->vars.bit_danger_fixe;
          plugin_actuel->vars.bit_secupers_ok = !(bit_derangement | bit_derangement_fixe | bit_danger | bit_danger_fixe);
        }
       liste = liste->next;
     }
    liste = dls_tree->Liste_dls_tree;
    while (liste)
     { struct DLS_TREE *sub_tree;
       sub_tree = (struct DLS_TREE *)liste->data;
       Dls_run_dls_tree ( sub_tree );
       bit_comm_out         |= sub_tree->syn_vars.bit_comm_out;
       bit_defaut           |= sub_tree->syn_vars.bit_defaut;
       bit_defaut_fixe      |= sub_tree->syn_vars.bit_defaut_fixe;
       bit_alarme           |= sub_tree->syn_vars.bit_alarme;
       bit_alarme_fixe      |= sub_tree->syn_vars.bit_alarme_fixe;
       bit_veille_partielle |= sub_tree->syn_vars.bit_veille_partielle;
       bit_veille_totale    &= sub_tree->syn_vars.bit_veille_totale;
       bit_alerte           |= sub_tree->syn_vars.bit_alerte;
       bit_alerte_fixe      |= sub_tree->syn_vars.bit_alerte_fixe;
       bit_alerte_fugitive  |= sub_tree->syn_vars.bit_alerte_fugitive;
       bit_derangement      |= sub_tree->syn_vars.bit_derangement;
       bit_derangement_fixe |= sub_tree->syn_vars.bit_derangement_fixe;
       bit_danger           |= sub_tree->syn_vars.bit_danger;
       bit_danger_fixe      |= sub_tree->syn_vars.bit_danger_fixe;
       liste = liste->next;
     }

    if ( bit_comm_out         != dls_tree->syn_vars.bit_comm_out ||                              /* Detection des changements */
         bit_defaut           != dls_tree->syn_vars.bit_defaut ||
         bit_defaut_fixe      != dls_tree->syn_vars.bit_defaut_fixe ||
         bit_alarme           != dls_tree->syn_vars.bit_alarme ||
         bit_alarme_fixe      != dls_tree->syn_vars.bit_alarme_fixe ||
         bit_veille_partielle != dls_tree->syn_vars.bit_veille_partielle ||
         bit_veille_totale    != dls_tree->syn_vars.bit_veille_totale ||
         bit_alerte           != dls_tree->syn_vars.bit_alerte ||
         bit_alerte_fixe      != dls_tree->syn_vars.bit_alerte_fixe ||
         bit_alerte_fugitive  != dls_tree->syn_vars.bit_alerte_fugitive ||
         bit_derangement      != dls_tree->syn_vars.bit_derangement ||
         bit_derangement_fixe != dls_tree->syn_vars.bit_derangement_fixe ||
         bit_danger           != dls_tree->syn_vars.bit_danger ||
         bit_danger_fixe      != dls_tree->syn_vars.bit_danger_fixe )
     { dls_tree->syn_vars.bit_comm_out         = bit_comm_out;                           /* Recopie et envoi aux threads SSRV */
       dls_tree->syn_vars.bit_defaut           = bit_defaut;
       dls_tree->syn_vars.bit_defaut_fixe      = bit_defaut_fixe;
       dls_tree->syn_vars.bit_alarme           = bit_alarme;
       dls_tree->syn_vars.bit_alarme_fixe      = bit_alarme_fixe;
       dls_tree->syn_vars.bit_veille_partielle = bit_veille_partielle;
       dls_tree->syn_vars.bit_veille_totale    = bit_veille_totale;
       dls_tree->syn_vars.bit_alerte           = bit_alerte;
       dls_tree->syn_vars.bit_alerte_fixe      = bit_alerte_fixe;
       dls_tree->syn_vars.bit_alerte_fugitive  = bit_alerte_fugitive;
       dls_tree->syn_vars.bit_derangement      = bit_derangement;
       dls_tree->syn_vars.bit_derangement_fixe = bit_derangement_fixe;
       dls_tree->syn_vars.bit_danger           = bit_danger;
       dls_tree->syn_vars.bit_danger_fixe      = bit_danger_fixe;
       Send_zmq_with_tag ( Partage->com_dls.zmq_to_master,
                           NULL, "dls", "*", "ssrv", "SET_SYN_VARS",
                          &dls_tree->syn_vars, sizeof(struct CMD_TYPE_SYN_VARS) );
     }
 }
/******************************************************************************************************************************/
/* Main: Fonction principale du DLS                                                                                           */
/******************************************************************************************************************************/
 void Run_dls ( void )
  { gint last_top_10sec, last_top_5sec, last_top_2sec, last_top_1sec, last_top_2hz, last_top_5hz, last_top_1min;
    gint Update_heure=0;

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    prctl(PR_SET_NAME, "W-DLS", 0, 0, 0 );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );
    Partage->com_dls.Thread_run = TRUE;                                                                 /* Le thread tourne ! */
    Dls_Lire_config ();                                                     /* Lecture de la configuration logiciel du thread */
    Prendre_heure();                                                     /* On initialise les variables de gestion de l'heure */

    if (strcmp ( Partage->com_dls.Library_version, DLS_LIBRARY_VERSION ) )
     { Partage->com_dls.Compil_at_boot = TRUE;
       if (Modifier_configDB ( "dls", "library_version", DLS_LIBRARY_VERSION ))
        { Info_new( Config.log, Config.log_db, LOG_NOTICE, "%s: updating library version OK", __func__ ); }
       else
        { Info_new( Config.log, Config.log_db, LOG_NOTICE, "%s: update library error" ); }
     }
    Charger_plugins();                                                                          /* Chargement des modules dls */
    Mnemo_auto_create_AI ( FALSE, "SYS", "DLS_BIT_PER_SEC", "nb bit par seconde", "bit par seconde" );
    Mnemo_auto_create_AI ( FALSE, "SYS", "DLS_WAIT", "delai d'attente DLS", "micro seconde" );
    Mnemo_auto_create_AI ( FALSE, "SYS", "DLS_TOUR_PER_SEC", "Nombre de tour dls par seconde", "tour par seconde" );
    Mnemo_auto_create_AI ( FALSE, "SYS", "TIME", "Represente l'heure/minute actuelles", "hh:mm" );
    Mnemo_auto_create_AI ( FALSE, "SYS", "NBR_MSG_QUEUE", "Nombre de messages dans la file de traitement", "messages" );
    Mnemo_auto_create_AI ( FALSE, "SYS", "NBR_VISUEL_QUEUE", "Nombre de visuels dans la file de traitement", "visuels" );
    Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, "SYS", "TOP_1MIN", "Impulsion toutes les minutes" );
    Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, "SYS", "TOP_1SEC", "Impulsion toutes les secondes" );
    Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, "SYS", "TOP_5SEC", "Impulsion toutes les 5 secondes" );
    Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, "SYS", "TOP_10SEC", "Impulsion toutes les 10 secondes" );
    Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, "SYS", "TOP_2HZ", "Impulsion toutes les demi-secondes" );
    Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, "SYS", "TOP_5HZ", "Impulsion toutes les 1/5 secondes" );
    Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, "SYS", "FLIPFLOP_2SEC", "Creneaux d'une durée de deux secondes" );
    Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, "SYS", "FLIPFLOP_1SEC", "Creneaux d'une durée d'une seconde" );
    Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, "SYS", "FLIPFLOP_2HZ",  "Creneaux d'une durée d'une demi seconde" );
    Mnemo_auto_create_BOOL ( FALSE, MNEMO_MONOSTABLE, "SYS", "FLIPFLOP_5HZ",  "Creneaux d'une durée d'un 5ième de seconde" );

    gint wait=30;
    while( Partage->com_dls.Thread_run == TRUE && wait )                                     /* On tourne tant que necessaire */
     { sleep(1); wait--; }        /* attente 30 secondes pour initialisation des bit internes et collection des infos modules */

    Partage->com_dls.zmq_to_master = Connect_zmq ( ZMQ_PUB, "pub-to-master", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

    last_top_2sec = last_top_1sec = last_top_2hz = last_top_5hz = last_top_1min = Partage->top;
    while(Partage->com_dls.Thread_run == TRUE)                                               /* On tourne tant que necessaire */
     { gpointer dls_top_10sec=NULL, dls_top_5sec=NULL, dls_top_1sec=NULL, dls_top_2hz=NULL, dls_top_5hz=NULL, dls_top_1min=NULL;
       gpointer dls_flipflop_1sec=NULL, dls_flipflop_2hz=NULL;
       gpointer dls_flipflop_2sec=NULL, dls_flipflop_5hz=NULL;
       gpointer dls_wait = NULL, dls_tour_per_sec = NULL, dls_bit_per_sec = NULL;
       gpointer dls_nbr_msg_queue = NULL, dls_nbr_visuel_queue = NULL;

       if (Partage->com_dls.Thread_reload)
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: RELOADING", __func__ );
          Dls_Lire_config();
          Decharger_plugins();
          Charger_plugins();
          Partage->com_dls.Thread_reload = FALSE;
        }

       if (Partage->top-last_top_5hz>=2)                                                           /* Toutes les 1/5 secondes */
        { Dls_data_set_bool ( NULL, "SYS", "TOP_5HZ", &dls_top_5hz, TRUE );
          Dls_data_set_bool ( NULL, "SYS", "FLIPFLOP_5HZ", &dls_flipflop_5hz,
                              !Dls_data_get_bool ( "SYS", "FLIPFLOP_5HZ", &dls_flipflop_5hz) );
          last_top_5hz = Partage->top;
        }
       if (Partage->top-last_top_2hz>=5)                                                           /* Toutes les 1/2 secondes */
        { Dls_data_set_bool ( NULL, "SYS", "TOP_2HZ", &dls_top_2hz, TRUE );
          Dls_data_set_bool ( NULL, "SYS", "FLIPFLOP_2HZ", &dls_flipflop_2hz,
                              !Dls_data_get_bool ( "SYS", "FLIPFLOP_2HZ", &dls_flipflop_2hz) );
          last_top_2hz = Partage->top;
        }
       if (Partage->top-last_top_1sec>=10)                                                             /* Toutes les secondes */
        { Dls_data_set_bool ( NULL, "SYS", "TOP_1SEC", &dls_top_1sec, TRUE );
          Dls_data_set_bool ( NULL, "SYS", "FLIPFLOP_1SEC", &dls_flipflop_1sec,
                              !Dls_data_get_bool ( "SYS", "FLIPFLOP_1SEC", &dls_flipflop_1sec) );
          last_top_1sec = Partage->top;

          Partage->audit_bit_interne_per_sec_hold += Partage->audit_bit_interne_per_sec;
          Partage->audit_bit_interne_per_sec_hold = Partage->audit_bit_interne_per_sec_hold >> 1;
          Partage->audit_bit_interne_per_sec = 0;                                                               /* historique */
          Dls_data_set_AI ( "SYS", "DLS_BIT_PER_SEC", &dls_bit_per_sec, Partage->audit_bit_interne_per_sec_hold, TRUE );

          Partage->audit_tour_dls_per_sec_hold += Partage->audit_tour_dls_per_sec;
          Partage->audit_tour_dls_per_sec_hold = Partage->audit_tour_dls_per_sec_hold >> 1;
          Partage->audit_tour_dls_per_sec = 0;
          Dls_data_set_AI ( "SYS", "DLS_TOUR_PER_SEC", &dls_tour_per_sec, Partage->audit_tour_dls_per_sec_hold, TRUE );
          if (Partage->audit_tour_dls_per_sec_hold > 100)                                           /* Moyennage tour DLS/sec */
           { Partage->com_dls.temps_sched += 50; }
          else if (Partage->audit_tour_dls_per_sec_hold < 80)
           { if (Partage->com_dls.temps_sched) Partage->com_dls.temps_sched -= 10; }
          Dls_data_set_AI ( "SYS", "DLS_WAIT", &dls_wait, Partage->com_dls.temps_sched, TRUE );                 /* historique */
        }
       if (Partage->top-last_top_2sec>=20)                                                             /* Toutes les secondes */
        { Dls_data_set_bool ( NULL, "SYS", "FLIPFLOP_2SEC", &dls_flipflop_2sec,
                              !Dls_data_get_bool ( "SYS", "FLIPFLOP_2SEC", &dls_flipflop_2sec) );
          last_top_2sec = Partage->top;
        }
       if (Partage->top-last_top_5sec>=50)                                                           /* Toutes les 5 secondes */
        { Dls_data_set_bool ( NULL, "SYS", "TOP_5SEC", &dls_top_5sec, TRUE );
          last_top_5sec = Partage->top;
        }
       if (Partage->top-last_top_10sec>=100)                                                              /* Toutes les secondes */
        { Dls_data_set_bool ( NULL, "SYS", "TOP_10SEC", &dls_top_10sec, TRUE );
          last_top_10sec = Partage->top;
        }
       if (Partage->top-last_top_1min>=600)                                                             /* Toutes les minutes */
        { Dls_data_set_bool ( NULL, "SYS", "TOP_1MIN", &dls_top_1min, TRUE );
          Dls_data_set_AI ( "SYS", "NBR_MSG_QUEUE", &dls_nbr_msg_queue, g_slist_length(Partage->com_msrv.liste_msg), TRUE );
          Dls_data_set_AI ( "SYS", "NBR_VISUEL_QUEUE", &dls_nbr_visuel_queue, g_slist_length(Partage->com_msrv.liste_visuel), TRUE );
          last_top_1min = Partage->top;
        }

       if (Partage->top-Update_heure>=600)                          /* Gestion des changements d'horaire (toutes les minutes) */
        { Prendre_heure ();                                                /* Mise à jour des variables de gestion de l'heure */
          Update_heure=Partage->top;
        }

       Set_edge();                                                                     /* Mise à zero des bit de egde up/down */
       Set_cde_exterieure();                                            /* Mise à un des bit de commande exterieure (furtifs) */

       pthread_mutex_lock( &Partage->com_dls.synchro );
       Dls_run_dls_tree( Partage->com_dls.Dls_tree );
       pthread_mutex_unlock( &Partage->com_dls.synchro );
       Dls_data_set_bool ( NULL, "SYS", "TOP_1SEC", &dls_top_1sec, FALSE );
       Dls_data_set_bool ( NULL, "SYS", "TOP_5SEC", &dls_top_5sec, FALSE );
       Dls_data_set_bool ( NULL, "SYS", "TOP_10SEC", &dls_top_10sec, FALSE );
       Dls_data_set_bool ( NULL, "SYS", "TOP_2HZ", &dls_top_2hz, FALSE );
       Dls_data_set_bool ( NULL, "SYS", "TOP_5HZ", &dls_top_5hz, FALSE );
       Dls_data_set_bool ( NULL, "SYS", "TOP_1MIN", &dls_top_1min, FALSE );
       Partage->com_dls.Top_check_horaire = FALSE;                         /* Cotrole horaire effectué un fois par minute max */
       Reset_edge();                                                                   /* Mise à zero des bit de egde up/down */
       Reset_cde_exterieure();                                        /* Mise à zero des bit de commande exterieure (furtifs) */
       Partage->audit_tour_dls_per_sec++;                                   /* Gestion de l'audit nbr de tour DLS par seconde */
/******************************************** Gestion des 1000 tours DLS par seconde ******************************************/
       usleep(Partage->com_dls.temps_sched);
       sched_yield();
     }
    Decharger_plugins();                                                                      /* Dechargement des modules DLS */
    Close_zmq(Partage->com_dls.zmq_to_master);
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: DLS Down (%p)", __func__, pthread_self() );
    Partage->com_dls.TID = 0;                                                 /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
