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

/******************************************************************************************************************************/
/* Http_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                   */
/* Entrée: le pointeur sur la PROCESS                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gboolean Dls_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Creer_configDB ( "dls", "debug", "false" );                                                /* Settings default parameters */
    Partage->com_dls.Thread_debug   = FALSE;                                                   /* Settings default parameters */

    if ( ! Recuperer_configDB( &db, "dls" ) )                                               /* Connexion a la base de données */
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     {      if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Partage->com_dls.Thread_debug = TRUE;  }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Dls_get_top_alerte: Remonte la valeur du plus haut bit d'alerte dans l'arbre DLS                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: TRUE ou FALSe                                                                                                      */
/******************************************************************************************************************************/
 gboolean Dls_get_top_alerte ( void )
  { return( Partage->com_dls.Dls_syns->bit_alerte ); }
/******************************************************************************************************************************/
/* Dls_get_top_alerte_fugitive: Remonte la valeur du plus haut bit d'alerte fugitive dans l'arbre DLS                         */
/* Entrée: Rien                                                                                                               */
/* Sortie: TRUE ou FALSe                                                                                                      */
/******************************************************************************************************************************/
 gboolean Dls_get_top_alerte_fugitive ( void )
  { return( Partage->com_dls.Dls_syns->bit_alerte_fugitive ); }
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
 static void ST_local( struct DLS_TO_PLUGIN *vars, struct DLS_TEMPO *tempo, int etat )
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
       Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_TEMPO '%s:%s'=%d, WAIT_FOR_DELAI_ON", __func__,
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_ON && etat == 0)
     { tempo->status = DLS_TEMPO_NOT_COUNTING;
       Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_TEMPO '%s:%s'=%d, NOT_COUNTING", __func__,
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_ON && tempo->date_on <= Partage->top)
     { tempo->status = DLS_TEMPO_WAIT_FOR_MIN_ON;
       tempo->state = TRUE;
       Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_MIN_ON", __func__,
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MIN_ON && etat == 0 &&
        Partage->top < tempo->date_on + tempo->min_on )
     { if (Partage->top+tempo->delai_off <= tempo->date_on + tempo->min_on)
            { tempo->date_off = tempo->date_on+tempo->min_on; }
       else { tempo->date_off = Partage->top+tempo->delai_off; }
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
       Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_DELAI_OFF", __func__,
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MIN_ON && etat == 0 &&
        tempo->date_on + tempo->min_on <= Partage->top )
     { tempo->date_off = Partage->top+tempo->delai_off;
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
       Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_DELAI_OFF", __func__,
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MIN_ON && etat == 1 &&
        tempo->date_on + tempo->min_on <= Partage->top )
     { tempo->status = DLS_TEMPO_WAIT_FOR_MAX_ON;
       Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_DELAI_MAX_ON", __func__,
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MAX_ON && etat == 0 )
     { if (tempo->max_on)
            { if (Partage->top+tempo->delai_off < tempo->date_on+tempo->max_on)
                   { tempo->date_off = Partage->top + tempo->delai_off; }
              else { tempo->date_off = tempo->date_on+tempo->max_on; }
            }
       else { tempo->date_off = Partage->top+tempo->delai_off; }
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
       Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_DELAI_OFF", __func__,
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_MAX_ON && etat == 1 && tempo->max_on &&
        tempo->date_on + tempo->max_on <= Partage->top )
     { tempo->date_off = tempo->date_on+tempo->max_on;
       tempo->status = DLS_TEMPO_WAIT_FOR_DELAI_OFF;
       Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_DELAI_OFF", __func__,
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_DELAI_OFF && tempo->date_off <= Partage->top )
     { tempo->date_on = tempo->date_off = 0;
       tempo->status = DLS_TEMPO_WAIT_FOR_COND_OFF;
       tempo->state = FALSE;
       Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_TEMPO '%s:%s'=%d WAIT_FOR_COND_OFF", __func__,
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }

    if (tempo->status == DLS_TEMPO_WAIT_FOR_COND_OFF && etat == 0 )
     { tempo->status = DLS_TEMPO_NOT_COUNTING;
       Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_TEMPO '%s:%s'=%d NOT_COUNTING", __func__,
                 (vars ? vars->num_ligne : -1), tempo->tech_id, tempo->acronyme, tempo->state );
     }
  }
/******************************************************************************************************************************/
/* Envoyer_commande_dls_data: Gestion des envois de commande DLS via dls_data                                                 */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Envoyer_commande_dls_data ( gchar *tech_id, gchar *acronyme )
  { gpointer di=NULL;
    Dls_data_get_DI ( tech_id, acronyme, &di );
    if (!di) { Dls_data_set_DI ( NULL, tech_id, acronyme, &di, TRUE ); }

    pthread_mutex_lock( &Partage->com_dls.synchro_data );
    Partage->com_dls.Set_Dls_Data = g_slist_append ( Partage->com_dls.Set_Dls_Data, di );
    pthread_mutex_unlock( &Partage->com_dls.synchro_data );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Mise a un du bit DI '%s:%s' demandée", __func__, tech_id, acronyme );
  }
/******************************************************************************************************************************/
/* Set_cde_exterieure: Mise à un des bits de commande exterieure                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Set_cde_exterieure ( void )
  { pthread_mutex_lock( &Partage->com_dls.synchro_data );
    while( Partage->com_dls.Set_Dls_Data )                                                  /* A-t-on une entrée a allumer ?? */
     { struct DLS_DI *di = Partage->com_dls.Set_Dls_Data->data;
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Mise a 1 du bit DI %s:%s",
                 __func__, di->tech_id, di->acronyme );
       Partage->com_dls.Set_Dls_Data = g_slist_remove ( Partage->com_dls.Set_Dls_Data, di );
       Partage->com_dls.Reset_Dls_Data = g_slist_append ( Partage->com_dls.Reset_Dls_Data, di );
       Dls_data_set_DI ( NULL, NULL, NULL, (gpointer *)&di, TRUE );                              /* Mise a un du bit d'entrée */
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro_data );
  }
/******************************************************************************************************************************/
/* Reset_cde_exterieure: Mise à zero des bits de commande exterieure                                                          */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Reset_cde_exterieure ( void )
  { pthread_mutex_lock( &Partage->com_dls.synchro_data );
    while( Partage->com_dls.Reset_Dls_Data )                                            /* A-t-on un monostable a éteindre ?? */
     { struct DLS_DI *di = Partage->com_dls.Reset_Dls_Data->data;
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Mise a 0 du bit DI %s:%s",
                 __func__, di->tech_id, di->acronyme );
       Partage->com_dls.Reset_Dls_Data = g_slist_remove ( Partage->com_dls.Reset_Dls_Data, di );
       Dls_data_set_DI ( NULL, NULL, NULL, (gpointer *)&di, FALSE );                             /* Mise a un du bit d'entrée */
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro_data );
  }
/******************************************************************************************************************************/
/* Set_cde_exterieure: Mise à un des bits de commande exterieure                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Set_edge ( void )
  { while( Partage->com_dls.Set_Dls_MONO_Edge_up )                                       /* A-t-on un boolean up a allumer ?? */
     { struct DLS_MONO *mono = Partage->com_dls.Set_Dls_MONO_Edge_up->data;
       Partage->com_dls.Set_Dls_MONO_Edge_up   = g_slist_remove  ( Partage->com_dls.Set_Dls_MONO_Edge_up, mono );
       Partage->com_dls.Reset_Dls_MONO_Edge_up = g_slist_prepend ( Partage->com_dls.Reset_Dls_MONO_Edge_up, mono );
       mono->edge_up = TRUE;
     }
    while( Partage->com_dls.Set_Dls_MONO_Edge_down )                                   /* A-t-on un boolean down a allumer ?? */
     { struct DLS_MONO *mono = Partage->com_dls.Set_Dls_MONO_Edge_down->data;
       Partage->com_dls.Set_Dls_MONO_Edge_down   = g_slist_remove  ( Partage->com_dls.Set_Dls_MONO_Edge_down, mono );
       Partage->com_dls.Reset_Dls_MONO_Edge_down = g_slist_prepend ( Partage->com_dls.Reset_Dls_MONO_Edge_down, mono );
       mono->edge_down = TRUE;
     }
    while( Partage->com_dls.Set_Dls_BI_Edge_up )                                       /* A-t-on un boolean up a allumer ?? */
     { struct DLS_BI *bi = Partage->com_dls.Set_Dls_BI_Edge_up->data;
       Partage->com_dls.Set_Dls_BI_Edge_up   = g_slist_remove  ( Partage->com_dls.Set_Dls_BI_Edge_up, bi );
       Partage->com_dls.Reset_Dls_BI_Edge_up = g_slist_prepend ( Partage->com_dls.Reset_Dls_BI_Edge_up, bi );
       bi->edge_up = TRUE;
     }
    while( Partage->com_dls.Set_Dls_BI_Edge_down )                                   /* A-t-on un boolean down a allumer ?? */
     { struct DLS_BI *bi = Partage->com_dls.Set_Dls_BI_Edge_down->data;
       Partage->com_dls.Set_Dls_BI_Edge_down   = g_slist_remove  ( Partage->com_dls.Set_Dls_BI_Edge_down, bi );
       Partage->com_dls.Reset_Dls_BI_Edge_down = g_slist_prepend ( Partage->com_dls.Reset_Dls_BI_Edge_down, bi );
       bi->edge_down = TRUE;
     }
    while( Partage->com_dls.Set_Dls_DI_Edge_up )                                         /* A-t-on un boolean up a allumer ?? */
     { struct DLS_DI *di = Partage->com_dls.Set_Dls_DI_Edge_up->data;
       Partage->com_dls.Set_Dls_DI_Edge_up   = g_slist_remove  ( Partage->com_dls.Set_Dls_DI_Edge_up, di );
       Partage->com_dls.Reset_Dls_DI_Edge_up = g_slist_prepend ( Partage->com_dls.Reset_Dls_DI_Edge_up, di );
       di->edge_up = TRUE;
     }
    while( Partage->com_dls.Set_Dls_DI_Edge_down )                                     /* A-t-on un boolean down a allumer ?? */
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
  { while( Partage->com_dls.Reset_Dls_MONO_Edge_up )                                     /* A-t-on un boolean up a allumer ?? */
     { struct DLS_MONO *mono = Partage->com_dls.Reset_Dls_MONO_Edge_up->data;
       Partage->com_dls.Reset_Dls_MONO_Edge_up = g_slist_remove ( Partage->com_dls.Reset_Dls_MONO_Edge_up, mono );
       mono->edge_up = FALSE;
     }
    while( Partage->com_dls.Reset_Dls_MONO_Edge_down )                                 /* A-t-on un boolean down a allumer ?? */
     { struct DLS_MONO *mono = Partage->com_dls.Reset_Dls_MONO_Edge_down->data;
       Partage->com_dls.Reset_Dls_MONO_Edge_down = g_slist_remove ( Partage->com_dls.Reset_Dls_MONO_Edge_down, mono );
       mono->edge_down = FALSE;
     }
    while( Partage->com_dls.Reset_Dls_BI_Edge_up )                                       /* A-t-on un boolean up a allumer ?? */
     { struct DLS_BI *bi = Partage->com_dls.Reset_Dls_BI_Edge_up->data;
       Partage->com_dls.Reset_Dls_BI_Edge_up = g_slist_remove ( Partage->com_dls.Reset_Dls_BI_Edge_up, bi );
       bi->edge_up = FALSE;
     }
    while( Partage->com_dls.Reset_Dls_BI_Edge_down )                                   /* A-t-on un boolean down a allumer ?? */
     { struct DLS_BI *bi = Partage->com_dls.Reset_Dls_BI_Edge_down->data;
       Partage->com_dls.Reset_Dls_BI_Edge_down = g_slist_remove ( Partage->com_dls.Reset_Dls_BI_Edge_down, bi );
       bi->edge_down = FALSE;
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
/******************************************************************************************************************************/
/* Dls_set_all_bool : positionne les nouvelles valeur des bits internes                                                       */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Dls_set_all_bool ( void )
  { GSList *liste;
    liste = Partage->Dls_data_MONO;
    while ( liste )
     { struct DLS_MONO *mono = liste->data;
       if (mono->etat != mono->next_etat)
        { if (mono->next_etat == TRUE)
           { Partage->com_dls.Set_Dls_MONO_Edge_up   = g_slist_prepend ( Partage->com_dls.Set_Dls_MONO_Edge_up, mono ); }
          else
           { Partage->com_dls.Set_Dls_MONO_Edge_down = g_slist_prepend ( Partage->com_dls.Set_Dls_MONO_Edge_down, mono ); }
          Partage->audit_bit_interne_per_sec++;
          mono->etat = mono->next_etat;
        }
       mono->next_etat = FALSE;
       liste = g_slist_next(liste);
     }

    liste = Partage->Dls_data_BI;
    while ( liste )
     { struct DLS_BI *bi = liste->data;
       if (bi->etat != bi->next_etat)
        { if (bi->next_etat == TRUE)
           { Partage->com_dls.Set_Dls_BI_Edge_up   = g_slist_prepend ( Partage->com_dls.Set_Dls_BI_Edge_up, bi ); }
          else
           { Partage->com_dls.Set_Dls_BI_Edge_down = g_slist_prepend ( Partage->com_dls.Set_Dls_BI_Edge_down, bi ); }
          Partage->audit_bit_interne_per_sec++;
          bi->etat = bi->next_etat;
        }
       liste = g_slist_next(liste);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
/******************************************************************************************************************************/
/* Dls_data_set_WATCHDOG: Positionne un watchdog en fonction de la valeur en parametre                                        */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 void Dls_data_set_WATCHDOG ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *wtd_p, gint consigne )
  { struct DLS_WATCHDOG *wtd;

    if (!wtd_p || !*wtd_p)
     { GSList *liste;
       if ( !(acronyme && tech_id) ) return;
       liste = Partage->Dls_data_WATCHDOG;
       while (liste)
        { wtd = (struct DLS_WATCHDOG *)liste->data;
          if ( !strcasecmp ( wtd->acronyme, acronyme ) && !strcasecmp( wtd->tech_id, tech_id ) ) break;
          liste = g_slist_next(liste);
        }

       if (!liste)
        { wtd = g_try_malloc0 ( sizeof(struct DLS_WATCHDOG) );
          if (!wtd)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( wtd->acronyme, sizeof(wtd->acronyme), "%s", acronyme );
          g_snprintf( wtd->tech_id,  sizeof(wtd->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_WATCHDOG = g_slist_prepend ( Partage->Dls_data_WATCHDOG, wtd );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: adding DLS_WATCHDOG '%s:%s'", __func__, tech_id, acronyme );
        }
       if (wtd_p) *wtd_p = (gpointer)wtd;                                        /* Sauvegarde pour acceleration si besoin */
      }
    else wtd = (struct DLS_WATCHDOG *)*wtd_p;

    wtd->top = Partage->top + consigne;
    Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
              "%s: ligne %04d: Changing DLS_WATCHDOG '%s:%s'=%d", __func__,
              (vars ? vars->num_ligne : -1), wtd->tech_id, wtd->acronyme, consigne );
    Partage->audit_bit_interne_per_sec++;
  }
/******************************************************************************************************************************/
/* Dls_data_get_bool: Remonte l'etat d'un boolean                                                                             */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_WATCHDOG ( gchar *tech_id, gchar *acronyme, gpointer *wtd_p )
  { struct DLS_WATCHDOG *wtd;
    GSList *liste;
    if (wtd_p && *wtd_p)                                                           /* Si pointeur d'acceleration disponible */
     { wtd = (struct DLS_WATCHDOG *)*wtd_p;
       goto end;
     }
    if (!tech_id || !acronyme) return(FALSE);

    liste = Partage->Dls_data_WATCHDOG;
    while (liste)
     { wtd = (struct DLS_WATCHDOG *)liste->data;
       if ( !strcasecmp ( wtd->acronyme, acronyme ) && !strcasecmp( wtd->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste)                                                                  /* si n'existe pas, on le créé dans la liste */
     { Dls_data_set_WATCHDOG ( NULL, tech_id, acronyme, wtd_p, 0 );
       return(FALSE);
     }

    if (wtd_p) *wtd_p = (gpointer)wtd;                                           /* Sauvegarde pour acceleration si besoin */
end:
    return( Partage->top < wtd->top );
  }
/******************************************************************************************************************************/
/* Met à jour le message en parametre                                                                                         */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_BI *Dls_data_BI_lookup ( gchar *tech_id, gchar *acronyme )
  { struct DLS_BI *bi;
    if (!tech_id || !acronyme) return(FALSE);
    GSList *liste = Partage->Dls_data_BI;
    while (liste)                                                                               /* A la recherche du message. */
     { bi = (struct DLS_BI *)liste->data;
       if ( !strcasecmp( bi->tech_id, tech_id ) && !strcasecmp( bi->acronyme, acronyme ) ) return(bi);
       liste = g_slist_next(liste);
     }

    bi = g_try_malloc0 ( sizeof(struct DLS_BI) );
    if (!bi)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
       return(NULL);
     }
    g_snprintf( bi->acronyme, sizeof(bi->acronyme), "%s", acronyme );
    g_snprintf( bi->tech_id,  sizeof(bi->tech_id),  "%s", tech_id );
    pthread_mutex_lock( &Partage->com_dls.synchro_data );
    Partage->Dls_data_BI = g_slist_prepend ( Partage->Dls_data_BI, bi );
    pthread_mutex_unlock( &Partage->com_dls.synchro_data );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: adding DLS_BI '%s:%s'", __func__, tech_id, acronyme );
    return(bi);
  }
/******************************************************************************************************************************/
/* Met à jour le message en parametre                                                                                         */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_MONO *Dls_data_MONO_lookup ( gchar *tech_id, gchar *acronyme )
  { struct DLS_MONO *mono;
    if (!tech_id || !acronyme) return(FALSE);
    GSList *liste = Partage->Dls_data_MONO;
    while (liste)                                                                               /* A la recherche du message. */
     { mono = (struct DLS_MONO *)liste->data;
       if ( !strcasecmp( mono->tech_id, tech_id ) && !strcasecmp( mono->acronyme, acronyme ) ) return(mono);
       liste = g_slist_next(liste);
     }

    mono = g_try_malloc0 ( sizeof(struct DLS_MONO) );
    if (!mono)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
       return(NULL);
     }
    g_snprintf( mono->acronyme, sizeof(mono->acronyme), "%s", acronyme );
    g_snprintf( mono->tech_id,  sizeof(mono->tech_id),  "%s", tech_id );
    pthread_mutex_lock( &Partage->com_dls.synchro_data );
    Partage->Dls_data_MONO = g_slist_prepend ( Partage->Dls_data_MONO, mono );
    pthread_mutex_unlock( &Partage->com_dls.synchro_data );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: adding DLS_MONO '%s:%s'", __func__, tech_id, acronyme );
    return(mono);
  }
/******************************************************************************************************************************/
/* Dls_data_set_BI: Positionne un bistable                                                                                    */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 void Dls_data_set_BI ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *bi_p, gboolean valeur )
  { struct DLS_BI *bi;

    if (!bi_p || !*bi_p)
     { if ( !(acronyme && tech_id) ) return;
       bi = Dls_data_BI_lookup ( tech_id, acronyme );
       if (bi_p) *bi_p = (gpointer)bi;                                              /* Sauvegarde pour acceleration si besoin */
      }
    else bi = (struct DLS_BI *)*bi_p;

    if (bi->next_etat != valeur)
     { Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_BI '%s:%s'=%d up %d down %d", __func__,
                 (vars ? vars->num_ligne : -1), bi->tech_id, bi->acronyme, valeur, bi->edge_up, bi->edge_down );
       Partage->audit_bit_interne_per_sec++;
       bi->next_etat = valeur;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_set_MONO: Positionne un monostable                                                                                */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 void Dls_data_set_MONO ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *mono_p, gboolean valeur )
  { struct DLS_MONO *mono;

    if (!mono_p || !*mono_p)
     { if ( !(acronyme && tech_id) ) return;
       mono = Dls_data_MONO_lookup ( tech_id, acronyme );
       if (mono_p) *mono_p = (gpointer)mono;                                        /* Sauvegarde pour acceleration si besoin */
      }
    else mono = (struct DLS_MONO *)*mono_p;

    if (valeur == FALSE) { mono->etat = FALSE; }
    else
     { if (mono->etat == FALSE)
        { Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                    "%s: ligne %04d: Changing DLS_MONO '%s:%s'=1", __func__,
                    (vars ? vars->num_ligne : -1), mono->tech_id, mono->acronyme );
          Partage->audit_bit_interne_per_sec++;
        }
       mono->next_etat = TRUE;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_BI: Remonte l'etat d'un bistable                                                                             */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_BI ( gchar *tech_id, gchar *acronyme, gpointer *bi_p )
  { struct DLS_BI *bi;
    if (bi_p && *bi_p)                                                           /* Si pointeur d'acceleration disponible */
     { bi = (struct DLS_BI *)*bi_p;
       return( bi->etat );
     }

    bi = Dls_data_BI_lookup ( tech_id, acronyme );
    if (!bi) return(FALSE);
    if (bi_p) *bi_p = (gpointer)bi;                                              /* Sauvegarde pour acceleration si besoin */
    return( bi->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_get_bi_up: Remonte le front montant d'un boolean                                                                    */
/* Sortie : TRUE sur le boolean vient de passer à UP                                                                            */
/******************************************************************************************************************************/
 gboolean Dls_data_get_BI_up ( gchar *tech_id, gchar *acronyme, gpointer *bi_p )
  { struct DLS_BI *bi;
    if (bi_p && *bi_p)                                                               /* Si pointeur d'acceleration disponible */
     { bi = (struct DLS_BI *)*bi_p;
       return( bi->edge_up );
     }

    bi = Dls_data_BI_lookup ( tech_id, acronyme );
    if (!bi) return(FALSE);
    if (bi_p) *bi_p = (gpointer)bi;                                                 /* Sauvegarde pour acceleration si besoin */
    return( bi->edge_up );
  }
/******************************************************************************************************************************/
/* Dls_data_get_bi_down: Remonte le front descendant d'un boolean                                                               */
/* Sortie : TRUE sur le boolean vient de passer à DOWN                                                                          */
/******************************************************************************************************************************/
 gboolean Dls_data_get_BI_down ( gchar *tech_id, gchar *acronyme, gpointer *bi_p )
  { struct DLS_BI *bi;
    if (bi_p && *bi_p)                                                           /* Si pointeur d'acceleration disponible */
     { bi = (struct DLS_BI *)*bi_p;
       return( bi->edge_down );
     }

    bi = Dls_data_BI_lookup ( tech_id, acronyme );
    if (!bi) return(FALSE);
    if (bi_p) *bi_p = (gpointer)bi;                                                 /* Sauvegarde pour acceleration si besoin */
    return( bi->edge_down );
  }
/******************************************************************************************************************************/
/* Met à jour le groupe de messages en parametre                                                                              */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_BI_groupe ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *bi_p, gint groupe )
  { struct DLS_BI *bi;

    if (!bi_p || !*bi_p)
     { if ( !(acronyme && tech_id) ) return;
       bi = Dls_data_BI_lookup ( tech_id, acronyme );
       if (bi_p) *bi_p = (gpointer)bi;                                           /* Sauvegarde pour acceleration si besoin */
     }
    else bi = (struct DLS_BI *)*bi_p;

    GSList *liste = Partage->Dls_data_BI;
    while (liste)
     { struct DLS_BI *current_bi = liste->data;
       if ( current_bi != bi && current_bi->groupe == bi->groupe && !strcasecmp ( current_bi->tech_id, bi->tech_id ) )
        { Dls_data_set_BI ( vars, current_bi->tech_id, current_bi->acronyme, (gpointer)&current_bi, FALSE ); }
       liste = g_slist_next(liste);
     }
    Dls_data_set_BI ( vars, tech_id, acronyme, bi_p, TRUE );
  }
/******************************************************************************************************************************/
/* Dls_data_get_MONO: Remonte l'etat d'un monostable                                                                          */
/* Sortie : TRUE sur le boolean est UP                                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_MONO ( gchar *tech_id, gchar *acronyme, gpointer *mono_p )
  { struct DLS_MONO *mono;
    if (mono_p && *mono_p)                                                           /* Si pointeur d'acceleration disponible */
     { mono = (struct DLS_MONO *)*mono_p;
       return( mono->etat );
     }

    mono = Dls_data_MONO_lookup ( tech_id, acronyme );
    if (!mono) return(FALSE);
    if (mono_p) *mono_p = (gpointer)mono;                                           /* Sauvegarde pour acceleration si besoin */
    return( mono->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_get_mono_up: Remonte le front montant d'un boolean                                                                */
/* Sortie : TRUE sur le boolean vient de passer à UP                                                                          */
/******************************************************************************************************************************/
 gboolean Dls_data_get_MONO_up ( gchar *tech_id, gchar *acronyme, gpointer *mono_p )
  { struct DLS_MONO *mono;
    if (mono_p && *mono_p)                                                           /* Si pointeur d'acceleration disponible */
     { mono = (struct DLS_MONO *)*mono_p;
       return( mono->edge_up );
     }

    mono = Dls_data_MONO_lookup ( tech_id, acronyme );
    if (!mono) return(FALSE);
    if (mono_p) *mono_p = (gpointer)mono;                                           /* Sauvegarde pour acceleration si besoin */
    return( mono->edge_up );
  }
/******************************************************************************************************************************/
/* Dls_data_get_mono_down: Remonte le front descendant d'un boolean                                                           */
/* Sortie : TRUE sur le boolean vient de passer à DOWN                                                                        */
/******************************************************************************************************************************/
 gboolean Dls_data_get_MONO_down ( gchar *tech_id, gchar *acronyme, gpointer *mono_p )
  { struct DLS_MONO *mono;
    if (mono_p && *mono_p)                                                           /* Si pointeur d'acceleration disponible */
     { mono = (struct DLS_MONO *)*mono_p;
       return( mono->edge_down );
     }

    mono = Dls_data_MONO_lookup ( tech_id, acronyme );
    if (!mono) return(FALSE);
    if (mono_p) *mono_p = (gpointer)mono;                                           /* Sauvegarde pour acceleration si besoin */
    return( mono->edge_down );
  }
/******************************************************************************************************************************/
/* Dls_data_set_DI: Positionne une DigitalInput                                                                               */
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
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( di->acronyme, sizeof(di->acronyme), "%s", acronyme );
          g_snprintf( di->tech_id,  sizeof(di->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_DI = g_slist_prepend ( Partage->Dls_data_DI, di );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: adding DLS_DI '%s:%s'=%d", __func__, tech_id, acronyme, valeur );
        }
       if (di_p) *di_p = (gpointer)di;                                              /* Sauvegarde pour acceleration si besoin */
      }
    else di = (struct DLS_DI *)*di_p;

    if (di->etat != valeur)
     { Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG, "%s: Changing DLS_DI '%s:%s'=%d up %d down %d",
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
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( dout->acronyme, sizeof(dout->acronyme), "%s", acronyme );
          g_snprintf( dout->tech_id,  sizeof(dout->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_DO = g_slist_prepend ( Partage->Dls_data_DO, dout );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: adding DLS_DO '%s:%s'", __func__, tech_id, acronyme );
        }
       if (dout_p) *dout_p = (gpointer)dout;                                              /* Sauvegarde pour acceleration si besoin */
      }
    else dout = (struct DLS_DO *)*dout_p;

    if (dout->etat != etat)
     { Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_DO '%s:%s'=%d ", __func__,
                 (vars ? vars->num_ligne : -1), dout->tech_id, dout->acronyme, etat );

       pthread_mutex_lock( &Partage->com_msrv.synchro );
       Partage->com_msrv.Liste_DO = g_slist_prepend ( Partage->com_msrv.Liste_DO, dout );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       Partage->audit_bit_interne_per_sec++;
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
 void Dls_data_set_AI ( gchar *tech_id, gchar *acronyme, gpointer *ai_p, gdouble valeur, gboolean in_range )
  { struct DLS_AI *ai;

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
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( ai->acronyme, sizeof(ai->acronyme), "%s", acronyme );
          g_snprintf( ai->tech_id,  sizeof(ai->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_AI = g_slist_prepend ( Partage->Dls_data_AI, ai );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: adding AI '%s:%s'=%f", __func__, tech_id, acronyme, valeur );
        }
       if (ai_p) *ai_p = (gpointer)ai;                                              /* Sauvegarde pour acceleration si besoin */
      }
    else ai = (struct DLS_AI *)*ai_p;

    ai->valeur  = valeur;
    ai->inrange = in_range;
  }
/******************************************************************************************************************************/
/* Met à jour la sortie analogique à partir de sa valeur avant mise a l'echelle                                               */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_AO ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *ao_p, gdouble valeur )
  { struct DLS_AO *ao;

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
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( ao->acronyme, sizeof(ao->acronyme), "%s", acronyme );
          g_snprintf( ao->tech_id,  sizeof(ao->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_AO = g_slist_prepend ( Partage->Dls_data_AO, ao );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: adding AO '%s:%s'", __func__, tech_id, acronyme );
        }
       if (ao_p) *ao_p = (gpointer)ao;                                              /* Sauvegarde pour acceleration si besoin */
      }
    else ao = (struct DLS_AO *)*ao_p;

    ao->valeur = valeur;                                                            /* Archive au mieux toutes les 5 secondes */

    pthread_mutex_lock( &Partage->com_msrv.synchro );                                 /* Ajout dans la liste de msg a traiter */
    Partage->com_msrv.Liste_AO = g_slist_append( Partage->com_msrv.Liste_AO, ao );
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
              "%s: ligne %04d: Changing DLS_AO '%s:%s'=%f", __func__,
              (vars ? vars->num_ligne : -1), ao->tech_id, ao->acronyme, ao->valeur );

 /* Pensez a ajouter l'archivage */

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
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( cpt_imp->acronyme, sizeof(cpt_imp->acronyme), "%s", acronyme );
          g_snprintf( cpt_imp->tech_id,  sizeof(cpt_imp->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_CI = g_slist_prepend ( Partage->Dls_data_CI, cpt_imp );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: adding CI '%s:%s'", __func__, tech_id, acronyme );
          Charger_confDB_CI ( cpt_imp );                                   /* Chargement des valeurs en base pour ce compteur */
        }
       if (cpt_imp_p) *cpt_imp_p = (gpointer)cpt_imp;                   /* Sauvegarde pour acceleration si besoin */
      }
    else cpt_imp = (struct DLS_CI *)*cpt_imp_p;

    if (etat)
     { if (reset)                                                                       /* Le compteur doit-il etre resetté ? */
        { if (cpt_imp->valeur!=0)
           { cpt_imp->val_en_cours1 = 0;                                           /* Valeur transitoire pour gérer les ratio */
             cpt_imp->valeur = 0;                                                  /* Valeur transitoire pour gérer les ratio */
           }
        }
       else if ( cpt_imp->etat == FALSE )                                                                 /* Passage en actif */
        { cpt_imp->etat = TRUE;
          Partage->audit_bit_interne_per_sec++;
          cpt_imp->val_en_cours1++;
          if (cpt_imp->val_en_cours1>=ratio)
           { cpt_imp->valeur++;
             cpt_imp->val_en_cours1=0;                                                        /* RAZ de la valeur de calcul 1 */
             Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                       "%s: ligne %04d: Changing DLS_CI '%s:%s'=%d", __func__,
                       (vars ? vars->num_ligne : -1), cpt_imp->tech_id, cpt_imp->acronyme, cpt_imp->valeur );
           }
        }
     }
    else
     { if (reset==0) cpt_imp->etat = FALSE; }

    if ( cpt_imp->last_update + 10 <= Partage->top )                                                   /* Toutes les secondes */
     { memcpy( &cpt_imp->valeurs[0], &cpt_imp->valeurs[1], 59*sizeof(cpt_imp->valeurs[0]) );
       cpt_imp->valeurs[59] = cpt_imp->valeur;
       cpt_imp->imp_par_minute = cpt_imp->valeur - cpt_imp->valeurs[0];
     }

    if ( (cpt_imp->archivage == 1 && cpt_imp->last_arch + 50     <= Partage->top) ||
         (cpt_imp->archivage == 2 && cpt_imp->last_arch + 600    <= Partage->top) ||
         (cpt_imp->archivage == 3 && cpt_imp->last_arch + 36000  <= Partage->top) ||
         (cpt_imp->archivage == 4 && cpt_imp->last_arch + 864000 <= Partage->top)
       )
     { Ajouter_arch( cpt_imp->tech_id, cpt_imp->acronyme, cpt_imp->valeur*1.0 );                       /* Archivage si besoin */
       cpt_imp->last_arch = Partage->top;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_CI : Recupere la valeur de l'EA en parametre                                                                  */
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
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( cpt_h->acronyme, sizeof(cpt_h->acronyme), "%s", acronyme );
          g_snprintf( cpt_h->tech_id,  sizeof(cpt_h->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_CH = g_slist_prepend ( Partage->Dls_data_CH, cpt_h );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: adding CH '%s:%s'", __func__, tech_id, acronyme );
          Charger_confDB_CH ( cpt_h );                                     /* Chargement des valeurs en base pour ce compteur */
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
             Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                       "%s: ligne %04d: Changing DLS_CH '%s:%s'=%d", __func__,
                       (vars ? vars->num_ligne : -1), cpt_h->tech_id, cpt_h->acronyme, cpt_h->valeur );
             Partage->audit_bit_interne_per_sec++;
           }
          if (cpt_h->last_arch + 600 < Partage->top)
           { Ajouter_arch( cpt_h->tech_id, cpt_h->acronyme, 1.0*cpt_h->valeur );
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
 gdouble Dls_data_get_AI ( gchar *tech_id, gchar *acronyme, gpointer *ai_p )
  { struct DLS_AI *ai;
    GSList *liste;
    if (ai_p && *ai_p)                                                               /* Si pointeur d'acceleration disponible */
     { ai = (struct DLS_AI *)*ai_p;
       return( ai->valeur );
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
    return( ai->valeur );
  }
/******************************************************************************************************************************/
/* Dls_data_get_AI : Recupere la valeur de l'EA en parametre                                                                  */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 gdouble Dls_data_get_AO ( gchar *tech_id, gchar *acronyme, gpointer *ao_p )
  { struct DLS_AO *ao;
    GSList *liste;
    if (ao_p && *ao_p)                                                               /* Si pointeur d'acceleration disponible */
     { ao = (struct DLS_AO *)*ao_p;
       return( ao->valeur );
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
    return( ao->valeur );
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
 void Dls_data_set_tempo ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *tempo_p, gboolean etat,
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
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( tempo->acronyme, sizeof(tempo->acronyme), "%s", acronyme );
          g_snprintf( tempo->tech_id,  sizeof(tempo->tech_id),  "%s", tech_id );
          tempo->init = FALSE;
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_TEMPO = g_slist_prepend ( Partage->Dls_data_TEMPO, tempo );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: adding TEMPO '%s:%s'", __func__, tech_id, acronyme );
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
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Initializing TEMPO '%s:%s'", __func__, tech_id, acronyme );
     }
    ST_local ( vars, tempo, etat );                                                               /* Recopie dans la variable */
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
 void Dls_data_set_bus ( gchar *tech_id, gchar *acronyme, gpointer *bus_p,
                         gchar *target_tech_id, gchar *json_parametre )
  { JsonNode *RootNode = Json_get_from_string ( json_parametre );
    if (RootNode)
     { Zmq_Send_json_node ( Partage->com_dls.zmq_to_master, "DLS", target_tech_id, RootNode );
       json_node_unref(RootNode);
     }
  }
/******************************************************************************************************************************/
/* Met à jour le message en parametre                                                                                         */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_MESSAGES *Dls_data_MSG_lookup ( gchar *tech_id, gchar *acronyme )
  { struct DLS_MESSAGES *msg;
    GSList *liste = Partage->Dls_data_MSG;
    while (liste)                                                                               /* A la recherche du message. */
     { msg = (struct DLS_MESSAGES *)liste->data;
       if ( !strcasecmp( msg->tech_id, tech_id ) && !strcasecmp( msg->acronyme, acronyme ) ) return(msg);
       liste = g_slist_next(liste);
     }

    msg = g_try_malloc0 ( sizeof(struct DLS_MESSAGES) );
    if (!msg)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
       return(NULL);
     }
    g_snprintf( msg->acronyme, sizeof(msg->acronyme), "%s", acronyme );
    g_snprintf( msg->tech_id,  sizeof(msg->tech_id),  "%s", tech_id );
    pthread_mutex_lock( &Partage->com_dls.synchro_data );
    Partage->Dls_data_MSG = g_slist_prepend ( Partage->Dls_data_MSG, msg );
    pthread_mutex_unlock( &Partage->com_dls.synchro_data );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: adding DLS_MSG '%s:%s'", __func__, tech_id, acronyme );
    return(msg);
  }
/******************************************************************************************************************************/
/* Met à jour le message en parametre                                                                                         */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Dls_data_set_MSG_reel ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *msg_p,
                                     gboolean update, gboolean etat )
  { struct DLS_MESSAGES *msg;

    if (!msg_p || !*msg_p)
     { if ( !(acronyme && tech_id) ) return;
       msg = Dls_data_MSG_lookup ( tech_id, acronyme );
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
                      "%s: malloc Event failed. Memory error for Updating DLS_MSG'%s:%s'", __func__, msg->tech_id, msg->acronyme );
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
                      "%s: malloc Event failed. Memory error for Updating DLS_MSG'%s:%s'", __func__, msg->tech_id, msg->acronyme );
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
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: last_change trop tot for DLS_MSG '%s:%s' !", __func__,
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

          Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                    "%s: ligne %04d: Changing DLS_MSG '%s:%s'=%d", __func__,
                    (vars ? vars->num_ligne : -1), msg->tech_id, msg->acronyme, msg->etat );
          msg->changes++;
          msg->last_change = Partage->top;
          Partage->audit_bit_interne_per_sec++;
        }
     }
  }
/******************************************************************************************************************************/
/* Met à jour le message en parametre                                                                                         */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_MSG ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *msg_p, gboolean update, gboolean etat )
  { if (vars && Dls_data_get_MONO(NULL, NULL, &vars->bit_comm)==FALSE) etat = FALSE;
    Dls_data_set_MSG_reel( vars, tech_id, acronyme, msg_p, update, etat );
  }
/******************************************************************************************************************************/
/* Met à jour le groupe de messages en parametre                                                                              */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_MSG_groupe ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *msg_p, gint groupe )
  { struct DLS_MESSAGES *msg;
    if (!msg_p || !*msg_p)
     { if ( !(acronyme && tech_id) ) return;
       msg = Dls_data_MSG_lookup ( tech_id, acronyme );
       if (msg_p) *msg_p = (gpointer)msg;                                           /* Sauvegarde pour acceleration si besoin */
     }
    else msg = (struct DLS_MESSAGES *)*msg_p;

    GSList *liste = Partage->Dls_data_MSG;
    while (liste)
     { struct DLS_MESSAGES *current_msg = liste->data;
       if ( current_msg != msg && current_msg->groupe == msg->groupe && !strcasecmp ( current_msg->tech_id, msg->tech_id ) )
        { Dls_data_set_MSG_reel ( vars, current_msg->tech_id, current_msg->acronyme, (gpointer)&current_msg, FALSE, FALSE ); }
       liste = g_slist_next(liste);
     }
    if (vars && Dls_data_get_MONO(NULL, NULL, &vars->bit_comm)==FALSE) Dls_data_set_MSG_reel ( vars, tech_id, acronyme, msg_p, FALSE, FALSE );
    else Dls_data_set_MSG_reel ( vars, tech_id, acronyme, msg_p, FALSE, TRUE );
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
/* Dls_data_set_visuel : Gestion du positionnement des visuels en mode dynamique                                              */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et la valeur on ou off de la tempo                            */
/******************************************************************************************************************************/
 void Dls_data_set_VISUEL ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *visu_p,
                            gchar *mode, gchar *color, gboolean cligno, gchar *libelle )
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
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( visu->acronyme, sizeof(visu->acronyme), "%s", acronyme );
          g_snprintf( visu->tech_id,  sizeof(visu->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_VISUEL = g_slist_prepend ( Partage->Dls_data_VISUEL, visu );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: adding VISUEL '%s:%s'", __func__, tech_id, acronyme );
        }
       if (visu_p) *visu_p = (gpointer)visu;                                        /* Sauvegarde pour acceleration si besoin */
      }
    else visu = (struct DLS_VISUEL *)*visu_p;

    if (vars && Dls_data_get_MONO ( NULL, NULL, &vars->bit_comm )==FALSE)
     { mode = "hors_comm"; color = "darkgreen", cligno = TRUE;
     }

    if ( strcmp ( visu->mode, mode ) || strcmp( visu->color, color ) || visu->cligno != cligno )
     { if ( visu->last_change + 50 <= Partage->top )                                 /* Si pas de change depuis plus de 5 sec */
        { visu->changes = 0; }

       if ( visu->changes <= 10 )                                                          /* Si moins de 10 changes en 5 sec */
        { if ( visu->changes == 10 )                                                /* Est-ce le dernier change avant blocage */
           { g_snprintf( visu->mode,  sizeof(visu->mode),  "hors_comm" ); }
          else { g_snprintf( visu->mode,    sizeof(visu->mode), "%s", mode );/* Sinon on recopie ce qui est demandé par le plugin DLS */
                 g_snprintf( visu->color,   sizeof(visu->color), "%s", color );
                 g_snprintf( visu->libelle, sizeof(visu->libelle), "%s", libelle );
                 Convert_libelle_dynamique ( visu->tech_id, visu->libelle, sizeof(visu->libelle) );
                 visu->cligno  = cligno;
               }

          visu->last_change = Partage->top;                                                             /* Date de la photo ! */
          pthread_mutex_lock( &Partage->com_msrv.synchro );                             /* Ajout dans la liste de i a traiter */
          Partage->com_msrv.liste_visuel = g_slist_append( Partage->com_msrv.liste_visuel, visu );
          pthread_mutex_unlock( &Partage->com_msrv.synchro );
          Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                    "%s: ligne %04d: Changing DLS_VISUEL '%s:%s'-> mode %s color %s cligno %d libelle = %s", __func__,
                    (vars ? vars->num_ligne : -1), visu->tech_id, visu->acronyme, visu->mode, visu->color, visu->cligno, visu->libelle );
        }
       visu->changes++;                                                                                /* Un change de plus ! */
       Partage->audit_bit_interne_per_sec++;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_set_REGISTRE: Positionne un registre                                                                                     */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_REGISTRE ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *r_p, gdouble valeur )
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
                       "%s: Memory error for '%s:%s'", __func__, acronyme, tech_id );
             return;
           }
          g_snprintf( reg->acronyme, sizeof(reg->acronyme), "%s", acronyme );
          g_snprintf( reg->tech_id,  sizeof(reg->tech_id),  "%s", tech_id );
          pthread_mutex_lock( &Partage->com_dls.synchro_data );
          Partage->Dls_data_REGISTRE = g_slist_prepend ( Partage->Dls_data_REGISTRE, reg );
          pthread_mutex_unlock( &Partage->com_dls.synchro_data );
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO,
                    "%s: adding DLS_REGISTRE '%s:%s'", __func__, tech_id, acronyme );
        }
       if (r_p) *r_p = (gpointer)reg;                                               /* Sauvegarde pour acceleration si besoin */
      }
    else reg = (struct DLS_REGISTRE *)*r_p;

    if (valeur != reg->valeur)
     { reg->valeur = valeur;
       Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                 "%s: ligne %04d: Changing DLS_REGISTRE '%s:%s'=%f", __func__,
                 (vars ? vars->num_ligne : -1), reg->tech_id, reg->acronyme, reg->valeur );
       Partage->audit_bit_interne_per_sec++;
     }

    if ( (reg->archivage == 1 && reg->last_arch + 50     <= Partage->top) ||
         (reg->archivage == 2 && reg->last_arch + 600    <= Partage->top) ||
         (reg->archivage == 3 && reg->last_arch + 36000  <= Partage->top) ||
         (reg->archivage == 4 && reg->last_arch + 864000 <= Partage->top)
       )
     { Ajouter_arch( reg->tech_id, reg->acronyme, reg->valeur );                                       /* Archivage si besoin */
       reg->last_arch = Partage->top;
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_reg: Remonte l'etat d'un registre                                                                             */
/* Sortie : TRUE sur le regean est UP                                                                                         */
/******************************************************************************************************************************/
 gdouble Dls_data_get_REGISTRE ( gchar *tech_id, gchar *acronyme, gpointer *r_p )
  { struct DLS_REGISTRE *reg;
    GSList *liste;
    if (r_p && *r_p)                                                             /* Si pointeur d'acceleration disponible */
     { reg = (struct DLS_REGISTRE *)*r_p;
       return( reg->valeur );
     }
    if (!tech_id || !acronyme) return(0.0);

    liste = Partage->Dls_data_REGISTRE;
    while (liste)
     { reg = (struct DLS_REGISTRE *)liste->data;
       if ( !strcasecmp ( reg->acronyme, acronyme ) && !strcasecmp( reg->tech_id, tech_id ) ) break;
       liste = g_slist_next(liste);
     }

    if (!liste) return(0.0);
    if (r_p) *r_p = (gpointer)reg;                                              /* Sauvegarde pour acceleration si besoin */
    return( reg->valeur );
  }
/******************************************************************************************************************************/
/* Dls_data_get_reg: Remonte l'etat d'un registre                                                                             */
/* Sortie : TRUE sur le regean est UP                                                                                         */
/******************************************************************************************************************************/
 void Dls_PID_reset ( gchar *input_tech_id, gchar *input_acronyme, gpointer *r_input )
  { Dls_data_get_REGISTRE ( input_tech_id, input_acronyme, r_input );
    if ( ! (r_input) ) return;

    struct DLS_REGISTRE *input = *r_input;
    if ( ! (input) ) return;

    input->pid_somme_erreurs = 0.0;
    input->pid_prev_erreur   = 0.0;
  }
/******************************************************************************************************************************/
/* Dls_PID: Gestion du PID                                                                                                    */
/* Sortie : TRUE sur le regean est UP                                                                                         */
/******************************************************************************************************************************/
 gint Dls_get_top ( void )
  { return (Partage->top); }
/******************************************************************************************************************************/
/* Dls_PID: Gestion du PID                                                                                                    */
/* Sortie : TRUE sur le regean est UP                                                                                         */
/******************************************************************************************************************************/
 gdouble Dls_PID ( gchar *input_tech_id, gchar *input_acronyme, gpointer *r_input,
                   gchar *consigne_tech_id, gchar *consigne_acronyme, gpointer *r_consigne,
                   gchar *kp_tech_id, gchar *kp_acronyme, gpointer *r_kp,
                   gchar *ki_tech_id, gchar *ki_acronyme, gpointer *r_ki,
                   gchar *kd_tech_id, gchar *kd_acronyme, gpointer *r_kd,
                   gchar *outputmin_tech_id, gchar *outputmin_acronyme, gpointer *r_outputmin,
                   gchar *outputmax_tech_id, gchar *outputmax_acronyme, gpointer *r_outputmax
                 )
  { Dls_data_get_REGISTRE ( input_tech_id, input_acronyme, r_input );
    Dls_data_get_REGISTRE ( consigne_tech_id, consigne_acronyme, r_consigne );
    Dls_data_get_REGISTRE ( kp_tech_id, kp_acronyme, r_kp );
    Dls_data_get_REGISTRE ( kp_tech_id, ki_acronyme, r_ki );
    Dls_data_get_REGISTRE ( kp_tech_id, kd_acronyme, r_kd );
    Dls_data_get_REGISTRE ( outputmin_tech_id, outputmin_acronyme, r_outputmin );
    Dls_data_get_REGISTRE ( outputmax_tech_id, outputmax_acronyme, r_outputmax );
    if ( ! (r_input && r_consigne && r_kp && r_ki && r_kd && r_outputmin && r_outputmax) ) return(0.0);

    struct DLS_REGISTRE *input = *r_input;
    struct DLS_REGISTRE *consigne = *r_consigne;
    struct DLS_REGISTRE *kp = *r_kp;
    struct DLS_REGISTRE *ki = *r_ki;
    struct DLS_REGISTRE *kd = *r_kd;
    struct DLS_REGISTRE *outputmin = *r_outputmin;
    struct DLS_REGISTRE *outputmax = *r_outputmax;

    if ( ! (input && consigne && kp && ki && kd && outputmin && outputmax) ) return(0.0);

    gdouble erreur           = consigne->valeur - input->valeur;
    input->pid_somme_erreurs+= erreur;                                /* possibilité de débordement si trop long a stabiliser */
    gdouble variation_erreur = erreur - input->pid_prev_erreur;
    gdouble result = kp->valeur * erreur + ki->valeur * input->pid_somme_erreurs + kd->valeur * variation_erreur;
    input->pid_prev_erreur = erreur;

         if (result > outputmax->valeur ) result = outputmax->valeur;
    else if (result < outputmin->valeur ) result = outputmin->valeur;
    return(result);
  }
/******************************************************************************************************************************/
/* Http_Dls_get_syn_vars: ajoute un objet dans le tableau des syn_vars pour l'enoyer au client                                */
/* Entrées: le buuilder Json et la connexion Websocket                                                                        */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_syn_vars_to_json ( gpointer user_data, struct DLS_SYN *dls_syn )
  { JsonArray *array = user_data;
    JsonNode *element = Json_node_create ();
    Json_node_add_int  ( element, "id", dls_syn->syn_id );
    Json_node_add_bool ( element, "bit_comm", dls_syn->bit_comm );
    Json_node_add_bool ( element, "bit_defaut", dls_syn->bit_defaut );
    Json_node_add_bool ( element, "bit_defaut_fixe", dls_syn->bit_defaut_fixe );
    Json_node_add_bool ( element, "bit_alarme", dls_syn->bit_alarme );
    Json_node_add_bool ( element, "bit_alarme_fixe", dls_syn->bit_alarme_fixe );
    Json_node_add_bool ( element, "bit_veille_partielle", dls_syn->bit_veille_partielle );
    Json_node_add_bool ( element, "bit_veille_totale", dls_syn->bit_veille_totale );
    Json_node_add_bool ( element, "bit_alerte", dls_syn->bit_alerte );
    Json_node_add_bool ( element, "bit_alerte_fixe", dls_syn->bit_alerte_fixe );
    Json_node_add_bool ( element, "bit_alerte_fugitive", dls_syn->bit_alerte_fugitive );
    Json_node_add_bool ( element, "bit_derangement", dls_syn->bit_derangement );
    Json_node_add_bool ( element, "bit_derangement_fixe", dls_syn->bit_derangement_fixe );
    Json_node_add_bool ( element, "bit_danger", dls_syn->bit_danger );
    Json_node_add_bool ( element, "bit_danger_fixe", dls_syn->bit_danger_fixe );
    Json_array_add_element ( array, element );
  }
/******************************************************************************************************************************/
/* Dls_run_dls_tree: Fait tourner les DLS synoptique en parametre + les sous DLS                                              */
/* Entrée : le Dls_tree correspondant                                                                                         */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_run_syn ( gpointer user_data, struct DLS_SYN *dls_syn )
  { gboolean bit_comm, bit_defaut, bit_defaut_fixe, bit_alarme, bit_alarme_fixe;                                  /* Activité */
    gboolean bit_veille_partielle, bit_veille_totale, bit_alerte, bit_alerte_fixe;             /* Synthese Sécurité des Biens */
    gboolean bit_alerte_fugitive;
    gboolean bit_derangement, bit_derangement_fixe, bit_danger, bit_danger_fixe;           /* synthèse Sécurité des Personnes */
    GSList *liste;

    bit_defaut = bit_defaut_fixe = bit_alarme = bit_alarme_fixe = FALSE;
    bit_veille_partielle = FALSE;
    bit_comm = bit_veille_totale = TRUE;
    bit_alerte = bit_alerte_fixe = bit_alerte_fugitive = FALSE;
    bit_derangement = bit_derangement_fixe = bit_danger = bit_danger_fixe = FALSE;

    liste = dls_syn->Dls_plugins;
    while(liste)                                                                     /* On execute tous les modules un par un */
     { struct DLS_PLUGIN *plugin = liste->data;
/*----------------------------------------------- Calcul des synthèses -------------------------------------------------------*/
       bit_comm             &= Dls_data_get_MONO ( plugin->tech_id, "COMM", &plugin->vars.bit_comm );                                                /* Bit de synthese activite */
       bit_defaut           |= Dls_data_get_MONO ( plugin->tech_id, "MEMSA_DEFAUT", &plugin->vars.bit_defaut );
       bit_defaut_fixe      |= Dls_data_get_MONO ( plugin->tech_id, "MEMSA_DEFAUT_FIXE", &plugin->vars.bit_defaut_fixe );
       bit_alarme           |= Dls_data_get_MONO ( plugin->tech_id, "MEMSA_ALARME", &plugin->vars.bit_alarme );
       bit_alarme_fixe      |= Dls_data_get_MONO ( plugin->tech_id, "MEMSA_ALARME_FIXE", &plugin->vars.bit_alarme_fixe );

       bit_veille_partielle |= Dls_data_get_MONO ( plugin->tech_id, "MEMSSB_VEILLE", &plugin->vars.bit_veille );
       bit_veille_totale    &= Dls_data_get_MONO ( plugin->tech_id, "MEMSSB_VEILLE", &plugin->vars.bit_veille );
       bit_alerte           |= Dls_data_get_MONO ( plugin->tech_id, "MEMSSB_ALERTE", &plugin->vars.bit_alerte );
       bit_alerte_fixe      |= Dls_data_get_MONO ( plugin->tech_id, "MEMSSB_ALERTE_FIXE", &plugin->vars.bit_alerte_fixe );
       bit_alerte_fugitive  |= Dls_data_get_MONO ( plugin->tech_id, "MEMSSB_ALERTE_FUGITIVE", &plugin->vars.bit_alerte_fugitive );

       bit_derangement      |= Dls_data_get_MONO ( plugin->tech_id, "MEMSSP_DERANGEMENT", &plugin->vars.bit_derangement );
       bit_derangement_fixe |= Dls_data_get_MONO ( plugin->tech_id, "MEMSSP_DERANGEMENT_FIXE", &plugin->vars.bit_derangement_fixe );
       bit_danger           |= Dls_data_get_MONO ( plugin->tech_id, "MEMSSP_DANGER", &plugin->vars.bit_danger );
       bit_danger_fixe      |= Dls_data_get_MONO ( plugin->tech_id, "MEMSSP_DANGER_FIXE", &plugin->vars.bit_danger_fixe );
       liste = liste->next;
     }
    liste = dls_syn->Dls_sub_syns;
    while (liste)
     { struct DLS_SYN *sub_syn = liste->data;
       Dls_run_syn ( NULL, sub_syn );
       bit_comm             &= sub_syn->bit_comm;
       bit_defaut           |= sub_syn->bit_defaut;
       bit_defaut_fixe      |= sub_syn->bit_defaut_fixe;
       bit_alarme           |= sub_syn->bit_alarme;
       bit_alarme_fixe      |= sub_syn->bit_alarme_fixe;
       bit_veille_partielle |= sub_syn->bit_veille_partielle;
       bit_veille_totale    &= sub_syn->bit_veille_totale;
       bit_alerte           |= sub_syn->bit_alerte;
       bit_alerte_fixe      |= sub_syn->bit_alerte_fixe;
       bit_alerte_fugitive  |= sub_syn->bit_alerte_fugitive;
       bit_derangement      |= sub_syn->bit_derangement;
       bit_derangement_fixe |= sub_syn->bit_derangement_fixe;
       bit_danger           |= sub_syn->bit_danger;
       bit_danger_fixe      |= sub_syn->bit_danger_fixe;
       liste = liste->next;
     }

    if ( bit_comm             != dls_syn->bit_comm ||                                  /* Detection des changements */
         bit_defaut           != dls_syn->bit_defaut ||
         bit_defaut_fixe      != dls_syn->bit_defaut_fixe ||
         bit_alarme           != dls_syn->bit_alarme ||
         bit_alarme_fixe      != dls_syn->bit_alarme_fixe ||
         bit_veille_partielle != dls_syn->bit_veille_partielle ||
         bit_veille_totale    != dls_syn->bit_veille_totale ||
         bit_alerte           != dls_syn->bit_alerte ||
         bit_alerte_fixe      != dls_syn->bit_alerte_fixe ||
         bit_alerte_fugitive  != dls_syn->bit_alerte_fugitive ||
         bit_derangement      != dls_syn->bit_derangement ||
         bit_derangement_fixe != dls_syn->bit_derangement_fixe ||
         bit_danger           != dls_syn->bit_danger ||
         bit_danger_fixe      != dls_syn->bit_danger_fixe )
     { dls_syn->bit_comm             = bit_comm;                               /* Recopie et envoi aux threads SSRV */
       dls_syn->bit_defaut           = bit_defaut;
       dls_syn->bit_defaut_fixe      = bit_defaut_fixe;
       dls_syn->bit_alarme           = bit_alarme;
       dls_syn->bit_alarme_fixe      = bit_alarme_fixe;
       dls_syn->bit_veille_partielle = bit_veille_partielle;
       dls_syn->bit_veille_totale    = bit_veille_totale;
       dls_syn->bit_alerte           = bit_alerte;
       dls_syn->bit_alerte_fixe      = bit_alerte_fixe;
       dls_syn->bit_alerte_fugitive  = bit_alerte_fugitive;
       dls_syn->bit_derangement      = bit_derangement;
       dls_syn->bit_derangement_fixe = bit_derangement_fixe;
       dls_syn->bit_danger           = bit_danger;
       dls_syn->bit_danger_fixe      = bit_danger_fixe;
       JsonNode *RootNode = Json_node_create ();
       JsonArray *array   = Json_node_add_array ( RootNode, "syn_vars" );
       Dls_syn_vars_to_json ( array, dls_syn );
       Json_node_add_string ( RootNode, "zmq_tag", "SET_SYN_VARS" );
       Zmq_Send_json_node( Partage->com_dls.zmq_to_master, "DLS", "*", RootNode );
       json_node_unref (RootNode);
     }
 }
/******************************************************************************************************************************/
/* Dls_run_dls_tree: Fait tourner les DLS synoptique en parametre + les sous DLS                                              */
/* Entrée : le Dls_tree correspondant                                                                                         */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_run_plugin ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { struct timeval tv_avant, tv_apres;
    gboolean bit_comm_module = TRUE;

/*--------------------------------------------- Calcul des bits internals ----------------------------------------------------*/
    if (!plugin->is_thread)
     { GSList *liste = plugin->Arbre_Comm;
       while ( liste )
        { gpointer comm = liste->data;
          bit_comm_module &= Dls_data_get_MONO( NULL, NULL, &comm );
          liste = g_slist_next ( liste );
        }
     }
    else bit_comm_module = Dls_data_get_WATCHDOG ( plugin->tech_id, "IO_COMM", &plugin->vars.bit_io_comm );
    Dls_data_set_MONO ( &plugin->vars, plugin->tech_id, "COMM", &plugin->vars.bit_comm, bit_comm_module );

    Dls_data_set_MONO ( &plugin->vars, plugin->tech_id, "MEMSA_OK", &plugin->vars.bit_activite_ok,
                        bit_comm_module &&
                        !( Dls_data_get_MONO( plugin->tech_id, "MEMSA_DEFAUT", &plugin->vars.bit_defaut ) ||
                           Dls_data_get_MONO( plugin->tech_id, "MEMSA_DEFAUT_FIXE", &plugin->vars.bit_defaut_fixe ) ||
                           Dls_data_get_MONO( plugin->tech_id, "MEMSA_ALARME", &plugin->vars.bit_alarme ) ||
                           Dls_data_get_MONO( plugin->tech_id, "MEMSA_ALARME_FIXE", &plugin->vars.bit_alarme_fixe )
                         )

                      );

    Dls_data_set_MONO ( &plugin->vars, plugin->tech_id, "MEMSSP_OK", &plugin->vars.bit_secupers_ok,
                        !( Dls_data_get_MONO( plugin->tech_id, "MEMSSP_DERANGEMENT", &plugin->vars.bit_derangement ) ||
                           Dls_data_get_MONO( plugin->tech_id, "MEMSSP_DERANGEMENT_FIXE", &plugin->vars.bit_derangement_fixe ) ||
                           Dls_data_get_MONO( plugin->tech_id, "MEMSSP_DANGER", &plugin->vars.bit_danger ) ||
                           Dls_data_get_MONO( plugin->tech_id, "MEMSSP_DANGER_FIXE", &plugin->vars.bit_danger_fixe )
                         )
                      );

/*----------------------------------------------- Mise a jour des messages de comm -------------------------------------------*/
    Dls_data_set_MSG_reel ( &plugin->vars, plugin->tech_id, "MSG_COMM_OK", &plugin->vars.bit_msg_comm_ok, FALSE,  bit_comm_module );
    Dls_data_set_MSG_reel ( &plugin->vars, plugin->tech_id, "MSG_COMM_HS", &plugin->vars.bit_msg_comm_hs, FALSE, !bit_comm_module );

    if (!(plugin->on && plugin->go)) return;                          /* si plugin a l'arret, on considère que la comm est OK */
/*----------------------------------------------- Lancement du plugin --------------------------------------------------------*/
    gettimeofday( &tv_avant, NULL );
    plugin->go( &plugin->vars );                                                                     /* On appel le plugin */
    gettimeofday( &tv_apres, NULL );
    plugin->conso+=Chrono( &tv_avant, &tv_apres );
    plugin->vars.resetted = FALSE;
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du DLS                                                                                           */
/******************************************************************************************************************************/
 void Run_dls ( void )
  { gint last_top_10sec, last_top_5sec, last_top_2sec, last_top_1sec, last_top_2hz, last_top_5hz, last_top_1min, last_top_10min;
    gint wait;

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    prctl(PR_SET_NAME, "W-DLS", 0, 0, 0 );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );
    Partage->com_dls.Thread_run = TRUE;                                                                 /* Le thread tourne ! */
    Dls_Lire_config ();                                                     /* Lecture de la configuration logiciel du thread */
    Prendre_heure();                                                     /* On initialise les variables de gestion de l'heure */

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: Wait 10sec to let threads init", __func__ );
    wait=10;                    /* On laisse 10 secondes pour charger tous les threads et préparer les eventuels DLS associés */
    while( Partage->com_dls.Thread_run == TRUE && wait )                                     /* On tourne tant que necessaire */
     { sleep(1); wait--; }

    Dls_Charger_plugins(TRUE);                                                 /* Chargement des modules dls avec compilation */
    Dls_recalculer_arbre_comm();                                                        /* Calcul de l'arbre de communication */
    Dls_recalculer_arbre_dls_syn();

    Mnemo_auto_create_AI ( FALSE, "SYS", "DLS_BIT_PER_SEC", "nb bit par seconde", "bit par seconde" );
    Mnemo_auto_create_AI ( FALSE, "SYS", "DLS_WAIT", "delai d'attente DLS", "micro seconde" );
    Mnemo_auto_create_AI ( FALSE, "SYS", "DLS_TOUR_PER_SEC", "Nombre de tour dls par seconde", "tour par seconde" );
    Mnemo_auto_create_AI ( FALSE, "SYS", "TIME", "Represente l'heure/minute actuelles", "hh:mm" );
    Mnemo_auto_create_AI ( FALSE, "SYS", "NBR_MSG_QUEUE", "Nombre de messages dans la file de traitement", "messages" );
    Mnemo_auto_create_AI ( FALSE, "SYS", "NBR_VISUEL_QUEUE", "Nombre de visuels dans la file de traitement", "visuels" );
    Mnemo_auto_create_AI ( FALSE, "SYS", "NBR_LIGNE_DLS", "Nombre de lignes total de tous modules D.L.S", "lignes" );
    Mnemo_auto_create_MONO ( FALSE, "SYS", "TOP_1MIN", "Impulsion toutes les minutes" );
    Mnemo_auto_create_MONO ( FALSE, "SYS", "TOP_1SEC", "Impulsion toutes les secondes" );
    Mnemo_auto_create_MONO ( FALSE, "SYS", "TOP_5SEC", "Impulsion toutes les 5 secondes" );
    Mnemo_auto_create_MONO ( FALSE, "SYS", "TOP_10SEC", "Impulsion toutes les 10 secondes" );
    Mnemo_auto_create_MONO ( FALSE, "SYS", "TOP_2HZ", "Impulsion toutes les demi-secondes" );
    Mnemo_auto_create_MONO ( FALSE, "SYS", "TOP_5HZ", "Impulsion toutes les 1/5 secondes" );
    Mnemo_auto_create_BI ( FALSE, "SYS", "FLIPFLOP_2SEC", "Creneaux d'une durée de deux secondes", 0 );
    Mnemo_auto_create_BI ( FALSE, "SYS", "FLIPFLOP_1SEC", "Creneaux d'une durée d'une seconde", 0 );
    Mnemo_auto_create_BI ( FALSE, "SYS", "FLIPFLOP_2HZ",  "Creneaux d'une durée d'une demi seconde", 0 );
    Mnemo_auto_create_BI ( FALSE, "SYS", "FLIPFLOP_5HZ",  "Creneaux d'une durée d'un 5ième de seconde", 0 );

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: Wait 20sec to let threads get I/Os", __func__ );
    wait=20;
    while( Partage->com_dls.Thread_run == TRUE && wait )                                     /* On tourne tant que necessaire */
     { sleep(1); wait--; }        /* attente 20 secondes pour initialisation des bit internes et collection des infos modules */

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: Starting", __func__ );
    Partage->com_dls.zmq_to_master = Zmq_Connect ( ZMQ_PUB, "pub-to-master", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

    last_top_2sec = last_top_1sec = last_top_2hz = last_top_5hz = last_top_1min = last_top_10min = Partage->top;
    while(Partage->com_dls.Thread_run == TRUE)                                               /* On tourne tant que necessaire */
     { gpointer dls_top_10sec=NULL, dls_top_5sec=NULL, dls_top_1sec=NULL, dls_top_2hz=NULL, dls_top_5hz=NULL, dls_top_1min=NULL;
       gpointer dls_flipflop_1sec=NULL, dls_flipflop_2hz=NULL;
       gpointer dls_flipflop_2sec=NULL, dls_flipflop_5hz=NULL;
       gpointer dls_wait = NULL, dls_tour_per_sec = NULL, dls_bit_per_sec = NULL;
       gpointer dls_nbr_msg_queue = NULL, dls_nbr_visuel_queue = NULL;
       gpointer dls_nbr_ligne_dls = NULL;

       if (Partage->com_dls.Thread_reload || Partage->com_dls.Thread_reload_with_recompil)
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: RELOADING", __func__ );
          Dls_Lire_config();
          Dls_Decharger_plugins();
          Dls_Charger_plugins(Partage->com_dls.Thread_reload_with_recompil);
          Dls_recalculer_arbre_comm();                                                  /* Calcul de l'arbre de communication */
          Dls_recalculer_arbre_dls_syn();
          Partage->com_dls.Thread_reload               = FALSE;
          Partage->com_dls.Thread_reload_with_recompil = FALSE;
        }

/******************************************************************************************************************************/
       if (Partage->top-last_top_5hz>=2)                                                           /* Toutes les 1/5 secondes */
        { Dls_data_set_MONO ( NULL, "SYS", "TOP_5HZ", &dls_top_5hz, TRUE );
          Dls_data_set_BI   ( NULL, "SYS", "FLIPFLOP_5HZ", &dls_flipflop_5hz,
                             !Dls_data_get_BI ( "SYS", "FLIPFLOP_5HZ", &dls_flipflop_5hz) );
          last_top_5hz = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_2hz>=5)                                                           /* Toutes les 1/2 secondes */
        { Dls_data_set_MONO ( NULL, "SYS", "TOP_2HZ", &dls_top_2hz, TRUE );
          Dls_data_set_BI   ( NULL, "SYS", "FLIPFLOP_2HZ", &dls_flipflop_2hz,
                             !Dls_data_get_BI ( "SYS", "FLIPFLOP_2HZ", &dls_flipflop_2hz) );
          last_top_2hz = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_1sec>=10)                                                             /* Toutes les secondes */
        { Dls_data_set_MONO ( NULL, "SYS", "TOP_1SEC", &dls_top_1sec, TRUE );
          Dls_data_set_BI   ( NULL, "SYS", "FLIPFLOP_1SEC", &dls_flipflop_1sec,
                             !Dls_data_get_BI ( "SYS", "FLIPFLOP_1SEC", &dls_flipflop_1sec) );
          last_top_1sec = Partage->top;

          Partage->audit_bit_interne_per_sec_hold += Partage->audit_bit_interne_per_sec;
          Partage->audit_bit_interne_per_sec_hold = Partage->audit_bit_interne_per_sec_hold >> 1;
          Partage->audit_bit_interne_per_sec = 0;                                                               /* historique */
          Dls_data_set_AI ( "SYS", "DLS_BIT_PER_SEC", &dls_bit_per_sec, (gdouble)Partage->audit_bit_interne_per_sec_hold, TRUE );

          Partage->audit_tour_dls_per_sec_hold += Partage->audit_tour_dls_per_sec;
          Partage->audit_tour_dls_per_sec_hold = Partage->audit_tour_dls_per_sec_hold >> 1;
          Partage->audit_tour_dls_per_sec = 0;
          Dls_data_set_AI ( "SYS", "DLS_TOUR_PER_SEC", &dls_tour_per_sec, (gdouble)Partage->audit_tour_dls_per_sec_hold, TRUE );
          if (Partage->audit_tour_dls_per_sec_hold > 100)                                           /* Moyennage tour DLS/sec */
           { Partage->com_dls.temps_sched += 50; }
          else if (Partage->audit_tour_dls_per_sec_hold < 80)
           { if (Partage->com_dls.temps_sched) Partage->com_dls.temps_sched -= 10; }
          Dls_data_set_AI ( "SYS", "DLS_WAIT", &dls_wait, (gdouble)Partage->com_dls.temps_sched, TRUE );        /* historique */
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_2sec>=20)                                                           /* Toutes les 2 secondes */
        { Dls_data_set_BI ( NULL, "SYS", "FLIPFLOP_2SEC", &dls_flipflop_2sec,
                           !Dls_data_get_BI ( "SYS", "FLIPFLOP_2SEC", &dls_flipflop_2sec) );
          last_top_2sec = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_5sec>=50)                                                           /* Toutes les 5 secondes */
        { Dls_data_set_MONO ( NULL, "SYS", "TOP_5SEC", &dls_top_5sec, TRUE );
          GSList *liste = Partage->Dls_data_AI;
          while (liste)
           { struct DLS_AI *ai = liste->data;
			 if ( ai->archivage == 1 )
              { Ajouter_arch( ai->tech_id, ai->acronyme, (ai->inrange ? ai->valeur : 0.0) );          /* Archivage si besoin */
                ai->last_arch = Partage->top;
              }
             liste = g_slist_next(liste);
           }
          last_top_5sec = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_10sec>=100)                                                        /* Toutes les 10 secondes */
        { Dls_data_set_MONO ( NULL, "SYS", "TOP_10SEC", &dls_top_10sec, TRUE );
          last_top_10sec = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_1min>=600)                                                             /* Toutes les minutes */
        { Dls_data_set_MONO ( NULL, "SYS", "TOP_1MIN", &dls_top_1min, TRUE );
          Dls_data_set_AI ( "SYS", "NBR_MSG_QUEUE", &dls_nbr_msg_queue, (gdouble)g_slist_length(Partage->com_msrv.liste_msg), TRUE );
          Dls_data_set_AI ( "SYS", "NBR_VISUEL_QUEUE", &dls_nbr_visuel_queue, (gdouble)g_slist_length(Partage->com_msrv.liste_visuel), TRUE );
          Prendre_heure ();                                                /* Mise à jour des variables de gestion de l'heure */
          GSList *liste = Partage->Dls_data_AI;
          while (liste)
           { struct DLS_AI *ai = liste->data;
			 if ( ai->archivage == 2 )
              { Ajouter_arch( ai->tech_id, ai->acronyme, (ai->inrange ? ai->valeur : 0.0) );          /* Archivage si besoin */
                ai->last_arch = Partage->top;
              }
             liste = g_slist_next(liste);
           }
          last_top_1min = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_10min>=6000)                                                        /* Toutes les 10 minutes */
        { JsonNode *result = Json_node_create();
          if (result)
           { SQL_Select_to_json_node ( result, NULL, "SELECT SUM(nbr_ligne) AS nbr_ligne_total FROM dls" );
             Dls_data_set_AI ( "SYS", "NBR_LIGNE_DLS", &dls_nbr_ligne_dls, Json_get_int( result, "nbr_ligne_total" )*1.0, TRUE );
             json_node_unref(result);
           }
          GSList *liste = Partage->Dls_data_AI;
          while (liste)
           { struct DLS_AI *ai = liste->data;
			 if ( (ai->archivage == 3 && ai->last_arch + 36000  <= Partage->top) ||
                  (ai->archivage == 4 && ai->last_arch + 864000 <= Partage->top)
                )
              { Ajouter_arch( ai->tech_id, ai->acronyme, (ai->inrange ? ai->valeur : 0.0) );          /* Archivage si besoin */
                ai->last_arch = Partage->top;
              }
             liste = g_slist_next(liste);
           }
          last_top_10min = Partage->top;
        }

       Set_edge();                                                                     /* Mise à zero des bit de egde up/down */
       Set_cde_exterieure();                                            /* Mise à un des bit de commande exterieure (furtifs) */

       Partage->top_cdg_plugin_dls = 0;                                                         /* On reset le cdg plugin DLS */
       Dls_foreach_plugins ( NULL, Dls_run_plugin );
       Dls_foreach_syns    ( NULL, Dls_run_syn );

       Partage->com_dls.Top_check_horaire = FALSE;                         /* Cotrole horaire effectué un fois par minute max */
       Reset_edge();                                                                   /* Mise à zero des bit de egde up/down */
       Reset_cde_exterieure();                                        /* Mise à zero des bit de commande exterieure (furtifs) */
       Dls_set_all_bool ();                      /* Positionne les booleans (mono/bi) selon la valeur calculé par les modules */
       Partage->audit_tour_dls_per_sec++;                                   /* Gestion de l'audit nbr de tour DLS par seconde */
/******************************************** Gestion des 1000 tours DLS par seconde ******************************************/
       usleep(Partage->com_dls.temps_sched);
       sched_yield();
     }
    Dls_arbre_dls_syn_erase();
    Dls_Decharger_plugins();                                                                      /* Dechargement des modules DLS */
    Zmq_Close(Partage->com_dls.zmq_to_master);
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: DLS Down (%p)", __func__, pthread_self() );
    Partage->com_dls.TID = 0;                                                 /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
