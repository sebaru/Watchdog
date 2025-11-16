/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls.c  Gestion et execution des plugins DLS Watchdgo 2.0                                                 */
/* Projet Abls-Habitat version 4.6       Gestion d'habitat                                   mar. 06 juil. 2010 18:31:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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
 #include <sys/resource.h>
 #include <semaphore.h>
 #include <locale.h>
 #include <math.h>

 #include "watchdogd.h"

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
/* Set_cde_exterieure: Mise à un des bits de commande exterieure                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Set_cde_exterieure ( void )
  { while( Partage->com_dls.Set_Dls_Data )                                                  /* A-t-on une entrée a allumer ?? */
     { struct DLS_DI *di = Partage->com_dls.Set_Dls_Data->data;
       Info_new( __func__, Config.log_dls, LOG_NOTICE, "%s: Mise a 1 du bit DI %s:%s",
                 __func__, di->tech_id, di->acronyme );
       Partage->com_dls.Set_Dls_Data = g_slist_remove ( Partage->com_dls.Set_Dls_Data, di );
       Partage->com_dls.Reset_Dls_Data = g_slist_append ( Partage->com_dls.Reset_Dls_Data, di );
       Dls_data_set_DI ( di, TRUE );                                                             /* Mise a un du bit d'entrée */
     }
  }
/******************************************************************************************************************************/
/* Reset_cde_exterieure: Mise à zero des bits de commande exterieure                                                          */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Reset_cde_exterieure ( void )
  { while( Partage->com_dls.Reset_Dls_Data )                                            /* A-t-on un monostable a éteindre ?? */
     { struct DLS_DI *di = Partage->com_dls.Reset_Dls_Data->data;
       Info_new( __func__, Config.log_dls, LOG_DEBUG, "%s: Mise a 0 du bit DI %s:%s",
                 __func__, di->tech_id, di->acronyme );
       Partage->com_dls.Reset_Dls_Data = g_slist_remove ( Partage->com_dls.Reset_Dls_Data, di );
       Dls_data_set_DI ( di, FALSE );                                                          /* Mise a zero du bit d'entrée */
     }
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
    while( Partage->com_dls.Set_Dls_BI_Edge_up )                                         /* A-t-on un boolean up a allumer ?? */
     { struct DLS_BI *bi = Partage->com_dls.Set_Dls_BI_Edge_up->data;
       Partage->com_dls.Set_Dls_BI_Edge_up   = g_slist_remove  ( Partage->com_dls.Set_Dls_BI_Edge_up, bi );
       Partage->com_dls.Reset_Dls_BI_Edge_up = g_slist_prepend ( Partage->com_dls.Reset_Dls_BI_Edge_up, bi );
       bi->edge_up = TRUE;
     }
    while( Partage->com_dls.Set_Dls_BI_Edge_down )                                     /* A-t-on un boolean down a allumer ?? */
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
/* Dls_data_set_bus : Envoi un message sur le bus système                                                                     */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et les paramètres du message                                  */
/******************************************************************************************************************************/
 void Dls_data_set_bus ( struct DLS_TO_PLUGIN *vars, gchar *thread_tech_id, gchar *commande )
  { JsonNode *RootNode = Json_node_create ();
    if (RootNode)
     { Json_node_add_string ( RootNode, "commande", commande );
       MQTT_Send_to_topic ( Partage->MQTT_local_session, RootNode, FALSE, "SET_BUS/%s", thread_tech_id );
       Json_node_unref(RootNode);
     }
  }
/******************************************************************************************************************************/
/* Dls_PID_reset: Reset les données calculées du PID                                                                          */
/* Sortie : les bits somme et prev sont mis à 0                                                                               */
/******************************************************************************************************************************/
 void Dls_PID_reset ( struct DLS_TO_PLUGIN *vars, struct DLS_REGISTRE *input )
  { if (!input) return;

    input->pid_somme_erreurs = 0.0;
    input->pid_prev_erreur   = 0.0;
  }
/******************************************************************************************************************************/
/* Dls_get_top: Récupètre la valeur de l'horloge                                                                              */
/* Sortie : le top horloge                                                                                                    */
/******************************************************************************************************************************/
 gint Dls_get_top ( void )
  { return (Partage->top); }
/******************************************************************************************************************************/
/* Dls_PID: Gestion du PID                                                                                                    */
/* Sortie : TRUE sur le regean est UP                                                                                         */
/******************************************************************************************************************************/
 void Dls_PID ( struct DLS_TO_PLUGIN *vars, struct DLS_REGISTRE *input, struct DLS_REGISTRE *consigne,
                struct DLS_REGISTRE *kp,struct DLS_REGISTRE *ki, struct DLS_REGISTRE *kd,
                struct DLS_REGISTRE *outputmin, struct DLS_REGISTRE *outputmax, struct DLS_REGISTRE *output
              )
  { if ( ! (input && consigne && kp && ki && kd && outputmin && outputmax && output ) ) return;

    gdouble erreur           = consigne->valeur - input->valeur;
    input->pid_somme_erreurs+= erreur;                                /* possibilité de débordement si trop long a stabiliser */
    gdouble variation_erreur = erreur - input->pid_prev_erreur;
    gdouble result = kp->valeur * erreur + ki->valeur * input->pid_somme_erreurs + kd->valeur * variation_erreur;
    input->pid_prev_erreur = erreur;

         if (result > outputmax->valeur ) result = outputmax->valeur;
    else if (result < outputmin->valeur ) result = outputmin->valeur;
    Dls_data_set_REGISTRE ( vars, output, result );
  }
/******************************************************************************************************************************/
/* Dls_sync_all_output: Envoi une synchronisation globale de toutes les sorties DO et AO                                      */
/* Entrée : le Dls_tree correspondant                                                                                         */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 void Dls_sync_all_output ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { if (!plugin->handle) return;                                                 /* si plugin non chargé, on ne l'éxecute pas */
    GSList *liste = plugin->Dls_data_DO;
    while ( liste )                                                   /* Calcul de la COMM du DLS a partir de ses dependances */
     { struct DLS_DO *bit = liste->data;
       JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { Dls_DO_to_json ( RootNode, bit );
          pthread_rwlock_wrlock ( &Partage->Liste_DO_synchro );
          Partage->Liste_DO = g_slist_append ( Partage->Liste_DO, RootNode );
          pthread_rwlock_unlock ( &Partage->Liste_DO_synchro );
        }
       else Info_new( __func__, Config.log_msrv, LOG_ERR, "JSon RootNode creation failed" );
       liste = g_slist_next ( liste );
     }

    liste = plugin->Dls_data_AO;
    while ( liste )                                                   /* Calcul de la COMM du DLS a partir de ses dependances */
     { struct DLS_AO *bit = liste->data;
       JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { Dls_AO_to_json ( RootNode, bit );
          pthread_rwlock_wrlock ( &Partage->Liste_AO_synchro );
          Partage->Liste_AO = g_slist_append ( Partage->Liste_AO, RootNode );
          pthread_rwlock_unlock ( &Partage->Liste_AO_synchro );
        }
       else Info_new( __func__, Config.log_msrv, LOG_ERR, "JSon RootNode creation failed" );
       liste = g_slist_next ( liste );
     }
  }
/******************************************************************************************************************************/
/* Dls_run_dls_tree: Fait tourner les DLS synoptique en parametre + les sous DLS                                              */
/* Entrée : le Dls_tree correspondant                                                                                         */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_run_plugin ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { struct timeval tv_avant, tv_apres;
    if (!plugin->handle) return;                                                 /* si plugin non chargé, on ne l'éxecute pas */
    plugin->vars.debug = plugin->debug_time > Partage->top;                             /* Recopie du champ de debug temporel */

/*--------------------------------------------- Calcul des bits internals ----------------------------------------------------*/
    gboolean bit_comm_module = TRUE;
    GSList *liste = plugin->Arbre_Comm;
    while ( liste )                                                   /* Calcul de la COMM du DLS a partir de ses dependances */
     { struct DLS_WATCHDOG *bit = liste->data;
       bit_comm_module &= Dls_data_get_WATCHDOG( bit );
       liste = g_slist_next ( liste );
     }

    if ( Dls_data_get_MONO ( plugin->vars.dls_comm ) != bit_comm_module )                    /* Envoi à l'API si il y a écart */
     { Dls_data_set_MONO ( &plugin->vars, plugin->vars.dls_comm, bit_comm_module );
       Dls_MONO_export_to_API ( plugin->vars.dls_comm );
     }

/*-------------------------------------------------- Calcul du MEMSA_OK ------------------------------------------------------*/
    gboolean new_memsa_ok = bit_comm_module && !( Dls_data_get_MONO( plugin->vars.dls_memsa_defaut ) ||
                                                  Dls_data_get_MONO( plugin->vars.dls_memsa_defaut_fixe ) ||
                                                  Dls_data_get_MONO( plugin->vars.dls_memsa_alarme ) ||
                                                  Dls_data_get_MONO( plugin->vars.dls_memsa_alarme_fixe )
                                                );
    Dls_data_set_MONO ( &plugin->vars, plugin->vars.dls_memsa_ok, new_memsa_ok );

/*-------------------------------------------------- Calcul du MEMSSP_OK -----------------------------------------------------*/
    Dls_data_set_MONO ( &plugin->vars, plugin->vars.dls_memssp_ok,
                        !( Dls_data_get_MONO( plugin->vars.dls_memssp_derangement ) ||
                           Dls_data_get_MONO( plugin->vars.dls_memssp_derangement_fixe ) ||
                           Dls_data_get_MONO( plugin->vars.dls_memssp_danger ) ||
                           Dls_data_get_MONO( plugin->vars.dls_memssp_danger_fixe )
                         )
                      );

/*----------------------------------------------- Mise a jour des messages de comm -------------------------------------------*/
   if (bit_comm_module) Dls_data_set_MESSAGE ( &plugin->vars, plugin->vars.dls_msg_comm_ok );
                   else Dls_data_set_MESSAGE ( &plugin->vars, plugin->vars.dls_msg_comm_hs );

/*----------------------------------------------- Lancement du plugin --------------------------------------------------------*/
    gettimeofday( &tv_avant, NULL );
    if (plugin->enable && plugin->go)                                                  /* Si plugin enabled ET fonction go ok */
     { if(plugin->vars.resetted && plugin->init_visuels)
        { Info_new( __func__, Config.log_dls, LOG_INFO, "Send '_START' to '%s', and Init_visuel", plugin->tech_id );
          plugin->init_visuels(&plugin->vars);
        }
       plugin->go( &plugin->vars );                                                                     /* On appel le plugin */
     }
    gettimeofday( &tv_apres, NULL );
    plugin->conso+=Chrono( &tv_avant, &tv_apres );
    Dls_data_MESSAGE_apply ( plugin );                                             /* Application des nouveaux etats messages */
    Dls_data_VISUEL_apply ( plugin );
    plugin->vars.resetted = FALSE;
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du DLS                                                                                           */
/******************************************************************************************************************************/
 void Run_dls ( void )
  { gint next_top_10sec, next_top_5sec, next_top_2sec, next_top_1sec, next_top_2hz, next_top_5hz, next_top_1min, next_top_10min;

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    prctl(PR_SET_NAME, "W-DLS", 0, 0, 0 );
    Info_new( __func__, Config.log_dls, LOG_NOTICE, "Demarrage . . . TID = %p", pthread_self() );
    Partage->com_dls.Thread_run = TRUE;                                                                 /* Le thread tourne ! */
    Prendre_heure();                                                     /* On initialise les variables de gestion de l'heure */

    next_top_5hz   = next_top_2hz  = Partage->top;                                                       /* Init des next top */
    next_top_1sec  = next_top_2sec = next_top_5sec = Partage->top + 10;
    next_top_1min  = Partage->top + 600;
    next_top_10min = Partage->top + 6000;
    while(Partage->com_dls.Thread_run == TRUE)                                               /* On tourne tant que necessaire */
     { pthread_mutex_lock( &Partage->com_dls.synchro );                               /* Zone de protection des bits internes */
/******************************************************************************************************************************/
       if (Partage->top>=next_top_5hz)                                                             /* Toutes les 1/5 secondes */
        { next_top_5hz = Partage->top + 2;
          Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_5hz, TRUE );
          Dls_data_set_BI   ( NULL, Partage->com_dls.sys_flipflop_5hz,
                             !Dls_data_get_BI ( Partage->com_dls.sys_flipflop_5hz) );
        }
/******************************************************************************************************************************/
       if (Partage->top>=next_top_2hz)                                                             /* Toutes les 1/2 secondes */
         {next_top_2hz = Partage->top + 5;
          Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_2hz, TRUE );
          Dls_data_set_BI   ( NULL, Partage->com_dls.sys_flipflop_2hz,
                             !Dls_data_get_BI ( Partage->com_dls.sys_flipflop_2hz) );
        }
/******************************************************************************************************************************/
       if (Partage->top>=next_top_1sec)                                                                /* Toutes les secondes */
        { next_top_1sec = Partage->top + 10;
          Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_1sec, TRUE );
          Dls_data_set_BI   ( NULL, Partage->com_dls.sys_flipflop_1sec,
                             !Dls_data_get_BI ( Partage->com_dls.sys_flipflop_1sec) );

          Partage->audit_bit_interne_per_sec_hold += Partage->audit_bit_interne_per_sec;
          Partage->audit_bit_interne_per_sec_hold = Partage->audit_bit_interne_per_sec_hold >> 1;
          Partage->audit_bit_interne_per_sec = 0;                                                               /* historique */
          Dls_data_set_AI ( Partage->com_dls.sys_bit_per_sec, (gdouble)Partage->audit_bit_interne_per_sec_hold, TRUE );

          Partage->audit_tour_dls_per_sec_hold += Partage->audit_tour_dls_per_sec;
          Partage->audit_tour_dls_per_sec_hold = Partage->audit_tour_dls_per_sec_hold >> 1;
          Partage->audit_tour_dls_per_sec = 0;
          Dls_data_set_AI ( Partage->com_dls.sys_tour_per_sec, (gdouble)Partage->audit_tour_dls_per_sec_hold, TRUE );
          if (Partage->audit_tour_dls_per_sec_hold > 100)                                           /* Moyennage tour DLS/sec */
           { Partage->com_dls.temps_sched += 50; }
          else if (Partage->audit_tour_dls_per_sec_hold < 80)
           { if (Partage->com_dls.temps_sched) Partage->com_dls.temps_sched -= 10; }
          Dls_data_set_AI ( Partage->com_dls.sys_dls_wait, (gdouble)Partage->com_dls.temps_sched, TRUE ); /* historique */
        }
/******************************************************************************************************************************/
       if (Partage->top>=next_top_2sec)                                                              /* Toutes les 2 secondes */
        { next_top_2sec = Partage->top+20;
          Dls_data_set_BI ( NULL, Partage->com_dls.sys_flipflop_2sec,
                           !Dls_data_get_BI ( Partage->com_dls.sys_flipflop_2sec) );
        }
/******************************************************************************************************************************/
       if (Partage->top>=next_top_5sec)                                                              /* Toutes les 5 secondes */
        { next_top_5sec = Partage->top + 50;
          Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_5sec, TRUE );
          Dls_foreach_plugins ( NULL, Dls_run_archivage );                        /* Archivage au mieux toutes les 5 secondes */
        }
/******************************************************************************************************************************/
       if (Partage->top>=next_top_10sec)                                                            /* Toutes les 10 secondes */
        { next_top_10sec = Partage->top + 100;
          Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_10sec, TRUE );
          Dls_data_set_BI ( NULL, Partage->com_dls.sys_mqtt_connected, Partage->MQTT_connected );
        }
/******************************************************************************************************************************/
       if (Partage->top>=next_top_1min)                                                                 /* Toutes les minutes */
        { next_top_1min = Partage->top + 600;
          Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_1min, TRUE );
          struct rusage conso;
          getrusage ( RUSAGE_SELF, &conso );
          Dls_data_set_AI ( Partage->com_dls.sys_maxrss, (gdouble)conso.ru_maxrss, TRUE );
          Dls_data_set_AI ( Partage->com_dls.sys_log_per_min, 1.0*Info_reset_nbr_log(), TRUE );
          Prendre_heure ();                                                /* Mise à jour des variables de gestion de l'heure */
          Dls_data_activer_horloge();
        }
/******************************************************************************************************************************/
       if (Partage->top>=next_top_10min)                                                             /* Toutes les 10 minutes */
        { next_top_10min = Partage->top + 6000;
        }

       Set_edge();                                                                    /* Mise à zero des bits de egde up/down */
       Set_cde_exterieure();                                           /* Mise à un des bits de commande exterieure (furtifs) */

       Partage->top_cdg_plugin_dls = 0;                                                         /* On reset le cdg plugin DLS */

       Dls_foreach_plugins ( NULL, Dls_run_plugin );                                                  /* Run all plugin D.L.S */

       Partage->com_dls.Top_check_horaire = FALSE;                        /* Controle horaire effectué un fois par minute max */
       Reset_edge();                                                                   /* Mise à zero des bit de egde up/down */
       Reset_cde_exterieure();                                        /* Mise à zero des bit de commande exterieure (furtifs) */

       Dls_data_clear_HORLOGE();
       Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_5hz,   FALSE );                     /* RaZ des Mono du plugin 'SYS' */
       Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_2hz,   FALSE );
       Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_1sec,  FALSE );
       Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_5sec,  FALSE );
       Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_10sec, FALSE );
       Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_1min,  FALSE );

       pthread_mutex_unlock( &Partage->com_dls.synchro );                      /* Fin de Zone de protection des bits internes */
/******************************************** Gestion des 1000 tours DLS par seconde ******************************************/
       Partage->audit_tour_dls_per_sec++;                                   /* Gestion de l'audit nbr de tour DLS par seconde */
       usleep(Partage->com_dls.temps_sched);
       sched_yield();
     }

    g_slist_free ( Partage->com_dls.Set_Dls_Data );
    g_slist_free ( Partage->com_dls.Reset_Dls_Data );
    g_slist_free ( Partage->com_dls.Set_Dls_DI_Edge_up );
    g_slist_free ( Partage->com_dls.Set_Dls_DI_Edge_down );
    g_slist_free ( Partage->com_dls.Reset_Dls_DI_Edge_up );
    g_slist_free ( Partage->com_dls.Reset_Dls_DI_Edge_down );
    g_slist_free ( Partage->com_dls.Set_Dls_MONO_Edge_up );
    g_slist_free ( Partage->com_dls.Set_Dls_MONO_Edge_down );
    g_slist_free ( Partage->com_dls.Reset_Dls_MONO_Edge_up );
    g_slist_free ( Partage->com_dls.Reset_Dls_MONO_Edge_down );
    g_slist_free ( Partage->com_dls.Set_Dls_BI_Edge_up );
    g_slist_free ( Partage->com_dls.Set_Dls_BI_Edge_down );
    g_slist_free ( Partage->com_dls.Reset_Dls_BI_Edge_up );
    g_slist_free ( Partage->com_dls.Reset_Dls_BI_Edge_down );

    Info_new( __func__, Config.log_dls, LOG_NOTICE, "DLS Down (%p)", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
