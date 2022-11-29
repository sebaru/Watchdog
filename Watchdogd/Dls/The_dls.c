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
/* Envoyer_commande_dls_data: Gestion des envois de commande DLS via dls_data                                                 */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Envoyer_commande_dls_data ( gchar *tech_id, gchar *acronyme )
  { struct DLS_DI *di= Dls_data_lookup_DI ( tech_id, acronyme );
    if (!di) return;

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
       Dls_data_set_DI ( NULL, di, TRUE );                                                       /* Mise a un du bit d'entrée */
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
       Dls_data_set_DI ( NULL, di, FALSE );                                                    /* Mise a zero du bit d'entrée */
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
 static void Dls_set_all_bool ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { GSList *liste;
    liste = plugin->Dls_data_MONO;
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

    liste = plugin->Dls_data_BI;
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
/* Dls_data_set_bus : Envoi un message sur le bus système                                                                     */
/* Entrée : l'acronyme, le owner dls, un pointeur de raccourci, et les paramètres du message                                  */
/******************************************************************************************************************************/
 void Dls_data_set_bus ( gchar *tech_id, gchar *acronyme, gpointer *bus_p,
                         gchar *target_tech_id, gchar *json_parametre )
  { JsonNode *RootNode = Json_get_from_string ( json_parametre );
    if (RootNode)
     { Http_Send_to_slaves ( target_tech_id, RootNode );
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
/* Http_Dls_get_syn_vars: ajoute un objet dans le tableau des syn_vars pour l'enoyer au client                                */
/* Entrées: le buuilder Json et la connexion Websocket                                                                        */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_syn_vars_to_json ( gpointer user_data, struct DLS_SYN *dls_syn )
  { JsonArray *array = user_data;
    JsonNode *element = Json_node_create ();
    Json_node_add_int  ( element, "syn_id", dls_syn->syn_id );
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
 static void Dls_run_plugin ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { struct timeval tv_avant, tv_apres;
    gboolean bit_comm_module = TRUE;

#ifdef bouh
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

    Dls_data_set_MONO ( &plugin->vars, plugin->vars.dls_memsa_ok,
                        bit_comm_module &&
                        !( Dls_data_get_MONO( plugin->vars.dls_memsa_defaut ) ||
                           Dls_data_get_MONO( plugin->vars.dls_memsa_defaut_fixe ) ||
                           Dls_data_get_MONO( plugin->vars.dls_memsa_alarme ) ||
                           Dls_data_get_MONO( plugin->vars.dls_memsa_alarme_fixe )
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
    Dls_data_set_MESSAGE ( &plugin->vars, plugin->vars.bit_msg_comm_ok, FALSE,  bit_comm_module );
    Dls_data_set_MESSAGE ( &plugin->vars, plugin->vars.bit_msg_comm_hs, FALSE, !bit_comm_module );
#endif
    if (!(plugin->enable && plugin->go)) return;                      /* si plugin a l'arret, on considère que la comm est OK */
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
    Prendre_heure();                                                     /* On initialise les variables de gestion de l'heure */

    Dls_Importer_plugins();                                                    /* Chargement des modules dls avec compilation */

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: Wait 20sec to let threads get I/Os", __func__ );
    wait=20;
    while( Partage->com_dls.Thread_run == TRUE && wait )                                     /* On tourne tant que necessaire */
     { sleep(1); wait--; }        /* attente 20 secondes pour initialisation des bit internes et collection des infos modules */

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: Starting", __func__ );

    last_top_2sec = last_top_1sec = last_top_2hz = last_top_5hz = last_top_1min = last_top_10min = Partage->top;
    while(Partage->com_dls.Thread_run == TRUE)                                               /* On tourne tant que necessaire */
     {
/******************************************************************************************************************************/
       if (Partage->top-last_top_5hz>=2)                                                           /* Toutes les 1/5 secondes */
        { Dls_data_set_MONO ( NULL, Partage->com_dls.dls_top_5hz, TRUE );
          Dls_data_set_BI   ( NULL, Partage->com_dls.dls_flipflop_5hz,
                             !Dls_data_get_BI ( Partage->com_dls.dls_flipflop_5hz) );
          last_top_5hz = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_2hz>=5)                                                           /* Toutes les 1/2 secondes */
        { Dls_data_set_MONO ( NULL, Partage->com_dls.dls_top_2hz, TRUE );
          Dls_data_set_BI   ( NULL, Partage->com_dls.dls_flipflop_2hz,
                             !Dls_data_get_BI ( Partage->com_dls.dls_flipflop_2hz) );
          last_top_2hz = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_1sec>=10)                                                             /* Toutes les secondes */
        { Dls_data_set_MONO ( NULL, Partage->com_dls.dls_top_1sec, TRUE );
          Dls_data_set_BI   ( NULL, Partage->com_dls.dls_flipflop_1sec,
                             !Dls_data_get_BI ( Partage->com_dls.dls_flipflop_1sec) );
          last_top_1sec = Partage->top;

          Partage->audit_bit_interne_per_sec_hold += Partage->audit_bit_interne_per_sec;
          Partage->audit_bit_interne_per_sec_hold = Partage->audit_bit_interne_per_sec_hold >> 1;
          Partage->audit_bit_interne_per_sec = 0;                                                               /* historique */
          Dls_data_set_AI ( NULL, Partage->com_dls.dls_bit_per_sec, (gdouble)Partage->audit_bit_interne_per_sec_hold, TRUE );

          Partage->audit_tour_dls_per_sec_hold += Partage->audit_tour_dls_per_sec;
          Partage->audit_tour_dls_per_sec_hold = Partage->audit_tour_dls_per_sec_hold >> 1;
          Partage->audit_tour_dls_per_sec = 0;
          Dls_data_set_AI ( NULL, Partage->com_dls.dls_tour_per_sec, (gdouble)Partage->audit_tour_dls_per_sec_hold, TRUE );
          if (Partage->audit_tour_dls_per_sec_hold > 100)                                           /* Moyennage tour DLS/sec */
           { Partage->com_dls.temps_sched += 50; }
          else if (Partage->audit_tour_dls_per_sec_hold < 80)
           { if (Partage->com_dls.temps_sched) Partage->com_dls.temps_sched -= 10; }
          Dls_data_set_AI ( NULL, Partage->com_dls.dls_wait, (gdouble)Partage->com_dls.temps_sched, TRUE );        /* historique */
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_2sec>=20)                                                           /* Toutes les 2 secondes */
        { Dls_data_set_BI ( NULL, Partage->com_dls.dls_flipflop_2sec,
                           !Dls_data_get_BI ( Partage->com_dls.dls_flipflop_2sec) );
          last_top_2sec = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_5sec>=50)                                                           /* Toutes les 5 secondes */
        { Dls_data_set_MONO ( NULL, Partage->com_dls.dls_top_5sec, TRUE );
          last_top_5sec = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_10sec>=100)                                                        /* Toutes les 10 secondes */
        { Dls_data_set_MONO ( NULL, Partage->com_dls.dls_top_10sec, TRUE );
          last_top_10sec = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_1min>=600)                                                             /* Toutes les minutes */
        { Dls_data_set_MONO ( NULL, Partage->com_dls.dls_top_1min, TRUE );
          Dls_data_set_AI ( NULL, Partage->com_dls.dls_nbr_msg_queue, (gdouble)g_slist_length(Partage->com_msrv.liste_msg), TRUE );
          Dls_data_set_AI ( NULL, Partage->com_dls.dls_nbr_visuel_queue, (gdouble)g_slist_length(Partage->com_msrv.liste_visuel), TRUE );
          Prendre_heure ();                                                /* Mise à jour des variables de gestion de l'heure */
#warning add check horloge
          last_top_1min = Partage->top;
        }
/******************************************************************************************************************************/
       if (Partage->top-last_top_10min>=6000)                                                        /* Toutes les 10 minutes */
        { pthread_t TID;
          pthread_create ( &TID, NULL, (void *)Dls_Export_Data_to_API, NULL );
          pthread_detach ( TID );

          last_top_10min = Partage->top;
        }

       Set_edge();                                                                     /* Mise à zero des bit de egde up/down */
       Set_cde_exterieure();                                            /* Mise à un des bit de commande exterieure (furtifs) */

       Partage->top_cdg_plugin_dls = 0;                                                         /* On reset le cdg plugin DLS */
       Dls_foreach_plugins ( NULL, Dls_run_plugin );
       Dls_foreach_plugins ( NULL, Dls_run_archivage );

       Partage->com_dls.Top_check_horaire = FALSE;                         /* Cotrole horaire effectué un fois par minute max */
       Reset_edge();                                                                   /* Mise à zero des bit de egde up/down */
       Reset_cde_exterieure();                                        /* Mise à zero des bit de commande exterieure (furtifs) */
       Dls_foreach_plugins ( NULL, Dls_set_all_bool );/* Positionne les booleans (mono/bi) selon la valeur calculé par les modules */
       Dls_data_clear_HORLOGE();
       Partage->audit_tour_dls_per_sec++;                                   /* Gestion de l'audit nbr de tour DLS par seconde */
/******************************************** Gestion des 1000 tours DLS par seconde ******************************************/
       usleep(Partage->com_dls.temps_sched);
       sched_yield();
     }
    Dls_Decharger_plugins();                                                                  /* Dechargement des modules DLS */
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: DLS Down (%p)", __func__, pthread_self() );
    Partage->com_dls.TID = 0;                                                 /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
