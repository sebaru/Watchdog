/******************************************************************************************************************************/
/* Watchdogd/Dls/The_dls.c  Gestion et execution des plugins DLS Watchdgo 2.0                                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       mar. 06 juil. 2010 18:31:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
 void Dls_data_set_bus ( gchar *tech_id, gchar *acronyme, gpointer *bus_p,
                         gchar *target_tech_id, gchar *json_parametre )
  { JsonNode *RootNode = Json_get_from_string ( json_parametre );
    if (RootNode)
     { gchar topic[256];
       g_snprintf( topic, sizeof(topic), "thread/%s", target_tech_id );
       MQTT_Send_to_topic ( Partage->com_msrv.MQTT_local_session, topic, "DLS", RootNode );
       Json_node_unref(RootNode);
     }
  }
/******************************************************************************************************************************/
/* Dls_data_get_reg: Remonte l'etat d'un registre                                                                             */
/* Sortie : TRUE sur le regean est UP                                                                                         */
/******************************************************************************************************************************/
 void Dls_PID_reset ( gchar *input_tech_id, gchar *input_acronyme, gpointer *r_input )
  {
#ifdef bouh
    Dls_data_get_REGISTRE ( input_tech_id, input_acronyme, r_input );
    if ( ! (r_input) ) return;

    struct DLS_REGISTRE *input = *r_input;
    if ( ! (input) ) return;

    input->pid_somme_erreurs = 0.0;
    input->pid_prev_erreur   = 0.0;
#endif
  }
/******************************************************************************************************************************/
/* Dls_PID: Gestion du PID                                                                                                    */
/* Sortie : TRUE sur le regean est UP                                                                                         */
/******************************************************************************************************************************/
 gint Dls_get_top ( void )
  { return (Partage->top); }
#ifdef bouh
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
#endif
/******************************************************************************************************************************/
/* Dls_run_dls_tree: Fait tourner les DLS synoptique en parametre + les sous DLS                                              */
/* Entrée : le Dls_tree correspondant                                                                                         */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_run_plugin ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { struct timeval tv_avant, tv_apres;
    if (!plugin->handle) return;                                                 /* si plugin non chargé, on ne l'éxecute pas */

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
       JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { Dls_MONO_to_json ( RootNode, plugin->vars.dls_comm );
          pthread_mutex_lock ( &Partage->abonnements_synchro );
          Partage->abonnements = g_slist_append ( Partage->abonnements, RootNode );
          pthread_mutex_unlock ( &Partage->abonnements_synchro );
        }
     }

    Dls_data_set_MONO ( &plugin->vars, plugin->vars.dls_memsa_ok,
                        bit_comm_module &&
                        !( Dls_data_get_MONO( plugin->vars.dls_memsa_defaut ) ||
                           Dls_data_get_MONO( plugin->vars.dls_memsa_defaut_fixe ) ||
                           Dls_data_get_MONO( plugin->vars.dls_memsa_alarme ) ||
                           Dls_data_get_MONO( plugin->vars.dls_memsa_alarme_fixe )
                         )

                      );

    Dls_data_set_MONO ( &plugin->vars, plugin->vars.dls_memssp_ok,
                        !( Dls_data_get_MONO( plugin->vars.dls_memssp_derangement ) ||
                           Dls_data_get_MONO( plugin->vars.dls_memssp_derangement_fixe ) ||
                           Dls_data_get_MONO( plugin->vars.dls_memssp_danger ) ||
                           Dls_data_get_MONO( plugin->vars.dls_memssp_danger_fixe )
                         )
                      );

/*----------------------------------------------- Mise a jour des messages de comm -------------------------------------------*/
    Dls_data_set_MESSAGE ( &plugin->vars, plugin->vars.dls_msg_comm_ok,  bit_comm_module );
    Dls_data_set_MESSAGE ( &plugin->vars, plugin->vars.dls_msg_comm_hs, !bit_comm_module );

    if (!plugin->enable) return;                                            /* si plugin a l'arret, on n'éxécute pas non plus */
    if (!plugin->go)     return;                                          /* si pas de fonction GO, on n'éxécute pas non plus */
/*----------------------------------------------- Lancement du plugin --------------------------------------------------------*/
    if(plugin->vars.resetted && plugin->init_visuels)
     { Info_new( __func__, Config.log_dls, LOG_INFO, "Send '_START' to '%s', and Init_visuel", plugin->tech_id );
       plugin->init_visuels(&plugin->vars);
     }
    gettimeofday( &tv_avant, NULL );
    plugin->go( &plugin->vars );                                                                        /* On appel le plugin */
    gettimeofday( &tv_apres, NULL );
    plugin->conso+=Chrono( &tv_avant, &tv_apres );
    plugin->vars.resetted = FALSE;
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du DLS                                                                                           */
/******************************************************************************************************************************/
 void Run_dls ( void )
  { gint last_top_10sec, last_top_5sec, last_top_2sec, last_top_1sec, last_top_2hz, last_top_5hz, last_top_1min, last_top_10min;

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    prctl(PR_SET_NAME, "W-DLS", 0, 0, 0 );
    Info_new( __func__, Config.log_dls, LOG_NOTICE, "Demarrage . . . TID = %p", pthread_self() );
    Partage->com_dls.Thread_run = TRUE;                                                                 /* Le thread tourne ! */
    Prendre_heure();                                                     /* On initialise les variables de gestion de l'heure */

    last_top_2sec = last_top_1sec = last_top_2hz = last_top_5hz = last_top_1min = last_top_10min = Partage->top;
    while(Partage->com_dls.Thread_run == TRUE)                                               /* On tourne tant que necessaire */
     { pthread_mutex_lock( &Partage->com_dls.synchro );                               /* Zone de protection des bits internes */
/******************************************************************************************************************************/
       if (Partage->top-last_top_5hz>=2)                                                           /* Toutes les 1/5 secondes */
        { Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_5hz, TRUE );
          Dls_data_set_BI   ( NULL, Partage->com_dls.sys_flipflop_5hz,
                             !Dls_data_get_BI ( Partage->com_dls.sys_flipflop_5hz) );
          last_top_5hz = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_2hz>=5)                                                           /* Toutes les 1/2 secondes */
        { Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_2hz, TRUE );
          Dls_data_set_BI   ( NULL, Partage->com_dls.sys_flipflop_2hz,
                             !Dls_data_get_BI ( Partage->com_dls.sys_flipflop_2hz) );
          last_top_2hz = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_1sec>=10)                                                             /* Toutes les secondes */
        { Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_1sec, TRUE );
          Dls_data_set_BI   ( NULL, Partage->com_dls.sys_flipflop_1sec,
                             !Dls_data_get_BI ( Partage->com_dls.sys_flipflop_1sec) );
          last_top_1sec = Partage->top;

          Partage->audit_bit_interne_per_sec_hold += Partage->audit_bit_interne_per_sec;
          Partage->audit_bit_interne_per_sec_hold = Partage->audit_bit_interne_per_sec_hold >> 1;
          Partage->audit_bit_interne_per_sec = 0;                                                               /* historique */
          Dls_data_set_AI ( NULL, Partage->com_dls.sys_bit_per_sec, (gdouble)Partage->audit_bit_interne_per_sec_hold, TRUE );

          Partage->audit_tour_dls_per_sec_hold += Partage->audit_tour_dls_per_sec;
          Partage->audit_tour_dls_per_sec_hold = Partage->audit_tour_dls_per_sec_hold >> 1;
          Partage->audit_tour_dls_per_sec = 0;
          Dls_data_set_AI ( NULL, Partage->com_dls.sys_tour_per_sec, (gdouble)Partage->audit_tour_dls_per_sec_hold, TRUE );
          if (Partage->audit_tour_dls_per_sec_hold > 100)                                           /* Moyennage tour DLS/sec */
           { Partage->com_dls.temps_sched += 50; }
          else if (Partage->audit_tour_dls_per_sec_hold < 80)
           { if (Partage->com_dls.temps_sched) Partage->com_dls.temps_sched -= 10; }
          Dls_data_set_AI ( NULL, Partage->com_dls.sys_dls_wait, (gdouble)Partage->com_dls.temps_sched, TRUE ); /* historique */
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_2sec>=20)                                                           /* Toutes les 2 secondes */
        { Dls_data_set_BI ( NULL, Partage->com_dls.sys_flipflop_2sec,
                           !Dls_data_get_BI ( Partage->com_dls.sys_flipflop_2sec) );
          last_top_2sec = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_5sec>=50)                                                           /* Toutes les 5 secondes */
        { Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_5sec, TRUE );
          Dls_data_set_AI ( NULL, Partage->com_dls.sys_nbr_archive_queue, 1.0*Partage->archive_liste_taille, TRUE );
          Dls_foreach_plugins ( NULL, Dls_run_archivage );                        /* Archivage au mieux toutes les 5 secondes */
          last_top_5sec = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_10sec>=100)                                                        /* Toutes les 10 secondes */
        { Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_10sec, TRUE );
          Dls_data_set_BI ( NULL, Partage->com_dls.sys_mqtt_connected, Partage->com_msrv.MQTT_connected );
          last_top_10sec = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_1min>=600)                                                             /* Toutes les minutes */
        { Dls_data_set_MONO ( NULL, Partage->com_dls.sys_top_1min, TRUE );
          struct rusage conso;
          getrusage ( RUSAGE_SELF, &conso );
          Dls_data_set_AI ( NULL, Partage->com_dls.sys_maxrss, (gdouble)conso.ru_maxrss, TRUE );
          Prendre_heure ();                                                /* Mise à jour des variables de gestion de l'heure */
          Dls_data_activer_horloge();
          last_top_1min = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_10min>=6000)                                                        /* Toutes les 10 minutes */
        { last_top_10min = Partage->top;
        }

       Set_edge();                                                                     /* Mise à zero des bit de egde up/down */
       Set_cde_exterieure();                                            /* Mise à un des bit de commande exterieure (furtifs) */

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
