/******************************************************************************************************************************/
/* Watchdogd/Watchdogd.c        Démarrage/Arret du systeme Watchdog, gestion des connexions clientes                          */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           mar 14 fév 2006 15:56:40 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Watchdogd.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien LEFEVRE
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

 #include <sys/prctl.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdio.h>  /* Pour printf */
 #include <stdlib.h> /* Pour exit */
 #include <fcntl.h>
 #include <signal.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <sys/file.h>
 #include <sys/types.h>
 #include <grp.h>
 #include <popt.h>
 #include <pthread.h>
 #include <pwd.h>
 #include <systemd/sd-login.h>

 #include "watchdogd.h"

 struct CONFIG Config;                                       /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 struct PARTAGE *Partage;                                                        /* Accès aux données partagées des processes */

/******************************************************************************************************************************/
/* Traitement_signaux: Gestion des signaux de controle du systeme                                                             */
/* Entrée: numero du signal à gerer                                                                                           */
/******************************************************************************************************************************/
 static void Traitement_signaux( int num )
  { char chaine[50];
    if (num == SIGALRM)
     { Partage->top++;
       if (Partage->com_msrv.Thread_run != TRUE) return;

       if (!Partage->top)                                             /* Si on passe par zero, on le dit (DEBUG interference) */
        { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Timer: Partage->top = 0 !!", __func__ ); }

       Partage->top_cdg_plugin_dls++;                                                            /* Chien de garde plugin DLS */
       if (Partage->top_cdg_plugin_dls>200)                                         /* Si pas de réponse D.L.S en 20 secondes */
        { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: CDG plugin DLS !!", __func__ );
          Partage->top_cdg_plugin_dls = 0;
        }
       return;
     }

    prctl(PR_GET_NAME, chaine, 0, 0, 0 );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: handled by %s", __func__, chaine );

    switch (num)
     { case SIGQUIT:
       case SIGINT:  Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGINT" );
                     Partage->com_msrv.Thread_run = FALSE;                       /* On demande l'arret de la boucle programme */
                     break;
       case SIGTERM: Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGTERM" );
                     Partage->com_msrv.Thread_run = FALSE;                       /* On demande l'arret de la boucle programme */
                     break;
       case SIGABRT: Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGABRT" );
                     break;
       case SIGCHLD: Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGCHLD" );
                     break;
       case SIGPIPE: Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGPIPE" ); break;
       case SIGBUS:  Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGBUS" ); break;
       case SIGIO:   Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGIO" ); break;
       case SIGUSR1: Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGUSR1" );
                     break;
       case SIGUSR2: Info_new( Config.log, Config.log_msrv, LOG_INFO, "Recu SIGUSR2" );
                     break;
       default: Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Recu signal %d", num ); break;
     }
  }
/******************************************************************************************************************************/
/* MSRV_Comparer_clef_thread: Compare deux clef thread dans le mapping                                                        */
/* Entrée: néant                                                                                                              */
/******************************************************************************************************************************/
 static gint MSRV_Comparer_clef_thread ( JsonNode *node1, JsonNode *node2 )
  { if (!node1) return(-1);
    if (!node2) return(1);
    gchar *ttech_id_1 = Json_get_string ( node1, "thread_tech_id" );
    gchar *ttech_id_2 = Json_get_string ( node2, "thread_tech_id" );
    if (!ttech_id_1) { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: ttech_id1 is NULL", __func__ ); return(-1); }
    if (!ttech_id_2) { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: ttech_id2 is NULL", __func__ ); return(1); }
    gint result = strcasecmp ( ttech_id_1, ttech_id_2 );
    if (result) return(result);
    gchar *tacronyme_1 = Json_get_string ( node1, "thread_acronyme" );
    gchar *tacronyme_2 = Json_get_string ( node2, "thread_acronyme" );
    if (!tacronyme_1) { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: tacronyme1 is NULL", __func__ ); return(-1); }
    if (!tacronyme_2) { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: tacronyme2 is NULL", __func__ ); return(1); }
    return( strcasecmp ( tacronyme_1, tacronyme_2 ) );
  }
/******************************************************************************************************************************/
/* MSRV_Comparer_clef_local: Compare deux clefs locales dans le mapping                                                       */
/* Entrée: néant                                                                                                              */
/******************************************************************************************************************************/
 static gint MSRV_Comparer_clef_local ( JsonNode *node1, JsonNode *node2 )
  { if (!node1) return(-1);
    if (!node2) return(1);
    gchar *tech_id_1 = Json_get_string ( node1, "tech_id" );
    gchar *tech_id_2 = Json_get_string ( node2, "tech_id" );
    if (!tech_id_1) { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: tech_id1 is NULL", __func__ ); return(-1); }
    if (!tech_id_2) { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: tech_id2 is NULL", __func__ ); return(1); }
    gint result = strcasecmp ( tech_id_1, tech_id_2 );
    if (result) return(result);
    gchar *acronyme_1 = Json_get_string ( node1, "acronyme" );
    gchar *acronyme_2 = Json_get_string ( node2, "acronyme" );
    if (!acronyme_1) { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: acronyme1 is NULL", __func__ ); return(-1); }
    if (!acronyme_2) { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: acronyme2 is NULL", __func__ ); return(1); }
    return( strcasecmp ( acronyme_1, acronyme_2 ) );
  }
/******************************************************************************************************************************/
/* MSRV_Map_to_thread: Met à jour à buffer json en mappant l'equivalent thread d'un bit interne local                         */
/* Entrée: FALSE si pas trouvé                                                                                                */
/******************************************************************************************************************************/
 gboolean MSRV_Map_to_thread ( JsonNode *key )
  { JsonNode *map = g_tree_lookup ( Partage->Maps_to_thread, key );
    if (map && Json_has_member ( map, "thread_tech_id" ) && Json_has_member ( map, "thread_acronyme" ) )
     { Json_node_add_string ( key, "thread_tech_id",  Json_get_string ( map, "thread_tech_id" ) );
       Json_node_add_string ( key, "thread_acronyme", Json_get_string ( map, "thread_acronyme" ) );
       return(TRUE);
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* MSRV_Map_to_thread: Met à jour à buffer json en mappant l'equivalent thread d'un bit interne local                         */
/* Entrée: FALSE si pas trouvé                                                                                                */
/******************************************************************************************************************************/
 gboolean MSRV_Map_from_thread ( JsonNode *key )
  { JsonNode *map = g_tree_lookup ( Partage->Maps_from_thread, key );
    if (map && Json_has_member ( map, "tech_id" ) && Json_has_member ( map, "acronyme" ) )
     { Json_node_add_string ( key, "tech_id",  Json_get_string ( map, "tech_id" ) );
       Json_node_add_string ( key, "acronyme", Json_get_string ( map, "acronyme" ) );
       return(TRUE);
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* MSRV_Remap: Charge les données de mapping en mémoire                                                                       */
/* Entrée: néant                                                                                                              */
/******************************************************************************************************************************/
 void MSRV_Remap( void )
  { pthread_mutex_lock( &Partage->com_msrv.synchro );
    if (Partage->Maps_from_thread)
     { g_tree_destroy ( Partage->Maps_from_thread );
       Partage->Maps_from_thread = NULL;
     }
    if (Partage->Maps_to_thread)
     { g_tree_destroy ( Partage->Maps_to_thread );
       Partage->Maps_to_thread = NULL;
     }
    if (Partage->Maps_root)
     { Json_node_unref ( Partage->Maps_root );
       Partage->Maps_root = NULL;
     }

    Partage->Maps_from_thread = g_tree_new ( (GCompareFunc)MSRV_Comparer_clef_thread );
    Partage->Maps_to_thread   = g_tree_new ( (GCompareFunc)MSRV_Comparer_clef_local );

    Partage->Maps_root = Json_node_create ();
    if (Partage->Maps_root)
     { SQL_Select_to_json_node ( Partage->Maps_root, "mappings",
                                 "SELECT * FROM mappings "
                                 "WHERE tech_id IS NOT NULL AND acronyme IS NOT NULL" );
       GList *Results = json_array_get_elements ( Json_get_array ( Partage->Maps_root, "mappings" ) );
       GList *results = Results;
       while(results)
        { JsonNode *element = results->data;
          g_tree_insert ( Partage->Maps_from_thread, element, element );
          g_tree_insert ( Partage->Maps_to_thread, element, element );
          results = g_list_next(results);
        }
       g_list_free(Results);
     }
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
  }
/******************************************************************************************************************************/
/* Charger_config_bit_interne: Chargement des configs bit interne depuis la base de données                                   */
/* Entrée: néant                                                                                                              */
/******************************************************************************************************************************/
 void Charger_config_bit_interne( void )
  { if (Config.instance_is_master == FALSE) return;                                /* Seul le master sauvegarde les compteurs */
    Charger_confDB_Registre(NULL);
    /*Charger_confDB_AO(NULL, NULL);*/
    Charger_confDB_AI(NULL, NULL);
    Charger_confDB_MSG();
    Charger_confDB_MONO();
    Charger_confDB_BI();
  }
/******************************************************************************************************************************/
/* Save_dls_data_to_DB : Envoie les infos DLS_DATA à la base de données pour sauvegarde !                                     */
/* Entrée : Néant                                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Save_dls_data_to_DB ( void )
  { if (Config.instance_is_master == FALSE) return;                                /* Seul le master sauvegarde les compteurs */
    gint top = Partage->top;
    Updater_confDB_AO();                                                    /* Sauvegarde des valeurs des Sorties Analogiques */
    Updater_confDB_DO();                                                            /* Sauvegarde des valeurs des Sorties TOR */
    Updater_confDB_CH();                                                                  /* Sauvegarde des compteurs Horaire */
    Updater_confDB_CI();                                                              /* Sauvegarde des compteurs d'impulsion */
    Updater_confDB_AI();                                                              /* Sauvegarde des compteurs d'impulsion */
    Updater_confDB_Registre();                                                                    /* Sauvegarde des registres */
    Updater_confDB_MSG();                                                              /* Sauvegarde des valeurs des messages */
    Updater_confDB_MONO();                                             /* Sauvegarde des valeurs des bistables et monostables */
    Updater_confDB_BI();                                               /* Sauvegarde des valeurs des bistables et monostables */
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Saved DLS_DATA in %04.1fs", __func__, (Partage->top-top)/10.0 );
    return;
  }
/******************************************************************************************************************************/
/* MSRV_handle_API_messages: Traite les messages recue de l'API                                                               */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MSRV_handle_API_messages ( void )
  {
    if (!Partage->com_msrv.API_ws_messages) return;

    pthread_mutex_lock ( &Partage->com_msrv.synchro );
    JsonNode *request = Partage->com_msrv.API_ws_messages->data;
    Partage->com_msrv.API_ws_messages = g_slist_remove ( Partage->com_msrv.API_ws_messages, request );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );

    gchar *api_tag = Json_get_string ( request, "api_tag" );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: receive api_tag '%s' !", __func__, api_tag );

         if ( !strcasecmp( api_tag, "RESET") )
     { Partage->com_msrv.Thread_run = FALSE;
       Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: RESET: Stopping in progress", __func__ );
     }
    else if ( !strcasecmp( api_tag, "UPGRADE") )
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: UPGRADE: Upgrading in progress", __func__ );
       gint pid = fork();
       if (pid<0)
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s_Fils: UPGRADE: erreur Fork", __func__ ); }
       else if (!pid)
        { system("cd SRC; ./autogen.sh; sudo make install; " );
          Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s_Fils: UPGRADE: done. Restarting.", __func__ );
          system("sudo killall Watchdogd" );
          exit(0);
        }
     }
    else if ( !strcasecmp( api_tag, "THREAD_START") ) { Thread_Start_one_thread ( NULL, 0, request, NULL ); }
    else if ( !strcasecmp( api_tag, "THREAD_STOP") )  { Thread_Stop_one_thread ( request ); }
    else if ( !strcasecmp( api_tag, "THREAD_SEND") )  { Thread_Push_API_message ( request ); }
    else if ( !strcasecmp( api_tag, "AGENT_SET") )
     { if ( !( Json_has_member ( request, "log_bus" ) && Json_has_member ( request, "log_level" ) &&
               Json_has_member ( request, "log_msrv" ) && Json_has_member ( request, "headless" )
             )
          )
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: AGENT_SET: wrong parameters", __func__ );
          goto end;
        }
       Config.log_bus    = Json_get_bool ( request, "log_bus" );
       Config.log_msrv   = Json_get_bool ( request, "log_msrv" );
       gboolean headless = Json_get_bool ( request, "headless" );
       Info_change_log_level ( Config.log, Json_get_int ( request, "log_level" ) );
       Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: AGENT_SET: log_msrv=%d, bus=%d, log_level=%d, headless=%d", __func__,
                 Config.log_msrv, Config.log_bus, Json_get_int ( request, "log_level" ), headless );
       if (Config.headless != headless)
        { Partage->com_msrv.Thread_run = FALSE;
          Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: AGENT_SET: headless has changed, rebooting", __func__ );
        }
     }
end:
    Json_node_unref(request);
  }
/******************************************************************************************************************************/
/* MSRV_on_API_message_CB: Appelé par libsoup lorsque l'on recoit un message sur la websocket connectée à l'API               */
/* Entrée: les parametres de la libsoup                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void MSRV_on_API_message_CB ( SoupWebsocketConnection *connexion, gint type, GBytes *message_brut, gpointer user_data )
  { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: WebSocket Message received !", __func__ );
    gsize taille;

    JsonNode *request = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!request)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: WebSocket Message Dropped (not JSON) !", __func__ );
       return;
     }

    if (!Json_has_member ( request, "api_tag" ))
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: WebSocket Message Dropped (no 'api_tag') !", __func__ );
       Json_node_unref(request);
       return;
     }

    pthread_mutex_lock ( &Partage->com_msrv.synchro );                                       /* Ajout dans la liste a traiter */
    Partage->com_msrv.API_ws_messages = g_slist_append ( Partage->com_msrv.API_ws_messages, request );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/******************************************************************************************************************************/
/* MSRV_ws_on_master_close_CB: Traite une deconnexion sur la websocket MSRV                                                   */
/* Entrée: les données fournies par la librairie libsoup                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void MSRV_on_API_close_CB ( SoupWebsocketConnection *connexion, gpointer user_data )
  { g_object_unref(Partage->com_msrv.API_websocket);
    Partage->com_msrv.API_websocket = NULL;
    Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: WebSocket Close. Reboot Needed!", __func__ );
    /* Partage->com_msrv.Thread_run = FALSE; */
  }
 static void MSRV_on_API_error_CB ( SoupWebsocketConnection *self, GError *error, gpointer user_data)
  { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: WebSocket Error received %p!", __func__, self );
  }
/******************************************************************************************************************************/
/* MSRV_ws_on_API_connected: Termine la creation de la connexion websocket API MSRV et raccorde le signal handler             */
/* Entrée: les variables traditionnelles de libsous                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void MSRV_on_API_connected ( GObject *source_object, GAsyncResult *res, gpointer user_data )
  { GError *error = NULL;
    Partage->com_msrv.API_websocket = soup_session_websocket_connect_finish ( Partage->com_msrv.API_session, res, &error );
    if (!Partage->com_msrv.API_websocket)                                                    /* No limit on incoming packet ! */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: WebSocket error: %s.", __func__, error->message );
       g_error_free (error);
       return;
     }
    g_object_set ( G_OBJECT(Partage->com_msrv.API_websocket), "max-incoming-payload-size", G_GINT64_CONSTANT(256000), NULL );
    g_object_set ( G_OBJECT(Partage->com_msrv.API_websocket), "keepalive-interval", G_GINT64_CONSTANT(30), NULL );
    g_signal_connect ( Partage->com_msrv.API_websocket, "message", G_CALLBACK(MSRV_on_API_message_CB), NULL );
    g_signal_connect ( Partage->com_msrv.API_websocket, "closed",  G_CALLBACK(MSRV_on_API_close_CB), NULL );
    g_signal_connect ( Partage->com_msrv.API_websocket, "error",   G_CALLBACK(MSRV_on_API_error_CB), NULL );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: WebSocket to API connected", __func__ );
  }
/******************************************************************************************************************************/
/* MSRV_ws_init: appelé pour démarrer le websocket vers l'API                                                                 */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void MSRV_ws_init ( void )
  { static gchar *protocols[] = { "live-agent", NULL };
    gchar chaine[256];
    g_snprintf(chaine, sizeof(chaine), "wss://%s/websocket", Json_get_string ( Config.config, "api_url" ) );
    SoupMessage *query = soup_message_new ( "GET", chaine );
    Http_Add_Agent_signature ( query, NULL, 0 );

    GCancellable *cancel = g_cancellable_new();
    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Starting WebSocket connect to %s", __func__, chaine );
    soup_session_websocket_connect_async ( Partage->com_msrv.API_session, query,
                                           NULL, protocols, cancel, MSRV_on_API_connected, NULL );
    g_object_unref(query);
    g_object_unref(cancel);
  }
/******************************************************************************************************************************/
/* MSRV_ws_end: appelé pour stopper la websocket vers l'API                                                                   */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void MSRV_ws_end ( void )
  { if ( Partage->com_msrv.API_websocket &&
         soup_websocket_connection_get_state ( Partage->com_msrv.API_websocket ) == SOUP_WEBSOCKET_STATE_OPEN )
     { soup_websocket_connection_close ( Partage->com_msrv.API_websocket, 0, "Thanks, Bye !" ); }
    Partage->com_msrv.API_websocket = NULL;
    if (Partage->com_msrv.API_ws_messages)
     { g_slist_foreach ( Partage->com_msrv.API_ws_messages, (GFunc) json_node_unref, NULL );
       g_slist_free ( Partage->com_msrv.API_ws_messages );
       Partage->com_msrv.API_ws_messages = NULL;
     }
  }
/******************************************************************************************************************************/
/* Boucle_pere: boucle de controle du pere de tous les serveurs                                                               */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void *Boucle_pere_master ( void )
  { gint cpt_5_minutes, cpt_1_minute;

    prctl(PR_SET_NAME, "W-MASTER", 0, 0, 0 );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Debut boucle sans fin", __func__ );
    Partage->com_msrv.Thread_run = TRUE;                                             /* On dit au maitre que le thread tourne */

    gchar *requete = SQL_Read_from_file ( "base_icones.sql" );                                    /* Load DB icons at startup */
    if (!requete)
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Icons DB Error.", __func__ ); }
    else if (!SQL_Writes ( requete ))
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Icons DB SQL Error.", __func__ ); }
    else Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Icons DB Loaded.", __func__ );
    if (requete) g_free(requete);

/***************************************** Active l'API ***********************************************************************/
    if (!Demarrer_http())                                                                                   /* Démarrage HTTP */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Pb HTTP", __func__ ); }
/***************************************** Demarrage des threads builtin et librairies ****************************************/
    if (Config.single)                                                                             /* Si demarrage des thread */
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: NOT starting threads (single mode=true)", __func__ ); }
    else
     { if (Config.installed)
        { if (!Demarrer_arch())                                                                /* Demarrage gestion Archivage */
           { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Pb ARCH", __func__ ); }

          if (!Demarrer_dls())                                                                            /* Démarrage D.L.S. */
           { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Pb DLS", __func__ ); }

          if (!Demarrer_api_sync())                                                                     /* Démarrage API_SYNC */
           { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Pb API_SYNC", __func__ ); }

          Charger_librairies();                                               /* Chargement de toutes les librairies Watchdog */
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: NOT starting threads (Instance is not installed)", __func__ ); }
     }

/***************************************** WebSocket Connect to API ************************************************************/
    MSRV_ws_init();

/***************************************** Charge le mapping des bits internes ************************************************/
    MSRV_Remap();

/***************************************** Debut de la boucle sans fin ********************************************************/
    cpt_5_minutes = Partage->top + 3000;
    cpt_1_minute  = Partage->top + 600;

    sleep(1);
    while(Partage->com_msrv.Thread_run == TRUE)                                           /* On tourne tant que l'on a besoin */
     { Gerer_arrive_Axxx_dls();                                           /* Distribution des changements d'etats sorties TOR */
       MSRV_handle_API_messages();

       if (cpt_5_minutes < Partage->top)                                                    /* Update DB toutes les 5 minutes */
        { pthread_attr_t attr;                                                 /* Attribut de mutex pour parametrer le module */
          pthread_t TID;
          pthread_attr_init(&attr);                                                 /* Initialisation des attributs du thread */
          pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);                       /* On le laisse joinable au boot */
          pthread_create( &TID, &attr, (void *)Save_dls_data_to_DB, NULL );
          cpt_5_minutes += 3000;                                                           /* Sauvegarde toutes les 5 minutes */
        }

       if (cpt_1_minute < Partage->top)                                                       /* Update DB toutes les minutes */
        { static gpointer bit_io_comm = NULL;
          Http_Send_ping_to_slaves();
          Dls_data_set_WATCHDOG ( NULL, g_get_host_name(), "IO_COMM", &bit_io_comm, 900 );
          Print_SQL_status();                                                             /* Print SQL status for debugging ! */
          Activer_horlogeDB();
          if (Partage->com_msrv.API_websocket == NULL) MSRV_ws_init();                 /* Si websocket closed, try to restart */
          cpt_1_minute += 600;                                                               /* Sauvegarde toutes les minutes */
        }

       usleep(1000);
       sched_yield();
     }

/*********************************** Terminaison: Deconnexion DB et kill des serveurs *****************************************/
    Save_dls_data_to_DB();                                                                 /* Dernière sauvegarde avant arret */

    MSRV_ws_end();
    Decharger_librairies();                                                   /* Déchargement de toutes les librairies filles */
    Stopper_fils();                                                                        /* Arret de tous les fils watchdog */

    if (Partage->Maps_from_thread) g_tree_destroy ( Partage->Maps_from_thread );
    if (Partage->Maps_to_thread) g_tree_destroy ( Partage->Maps_to_thread );
    if (Partage->Maps_root) Json_node_unref ( Partage->Maps_root );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: fin boucle sans fin", __func__ );
    pthread_exit( NULL );
  }
/******************************************************************************************************************************/
/* Boucle_pere: boucle de controle du pere de tous les serveurs                                                               */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void *Boucle_pere_slave ( void )
  { gint cpt_5_minutes = 0, cpt_1_minute = 0;
    gchar chaine[128];

    prctl(PR_SET_NAME, "W-SLAVE", 0, 0, 0 );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Debut boucle sans fin", __func__ );

    g_snprintf(chaine, sizeof(chaine), "Gestion de l'instance slave %s", g_get_host_name());
    if (Dls_auto_create_plugin( g_get_host_name(), chaine ) == FALSE)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: %s: DLS Create ERROR\n", __func__, g_get_host_name ); }

    g_snprintf(chaine, sizeof(chaine), "Statut de la communication avec le slave %s", g_get_host_name() );
    Mnemo_auto_create_WATCHDOG ( FALSE, g_get_host_name(), "IO_COMM", chaine );

/***************************************** Active l'API ***********************************************************************/
    if (!Demarrer_http())                                                                                   /* Démarrage HTTP */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Pb HTTP", __func__ ); }
/***************************************** Demarrage des threads builtin et librairies ****************************************/
    if (Config.single)                                                                             /* Si demarrage des thread */
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: NOT starting threads (single mode=true)", __func__ ); }
    else
     { if (Config.installed)
        { Charger_librairies(); }                                             /* Chargement de toutes les librairies Watchdog */
       else
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: NOT starting threads (Instance is not installed)", __func__ ); }
     }

/***************************************** WebSocket Connect to API ************************************************************/
    MSRV_ws_init();

/***************************************** Debut de la boucle sans fin ********************************************************/
    sleep(1);
    Partage->com_msrv.Thread_run = TRUE;                                             /* On dit au maitre que le thread tourne */
/*    Http_Post_to_local_BUS ( module, "SLAVE_START", NULL );*/

    while(Partage->com_msrv.Thread_run == TRUE)                                           /* On tourne tant que l'on a besoin */
     { MSRV_handle_API_messages();
       if (cpt_5_minutes < Partage->top)                                                    /* Update DB toutes les 5 minutes */
        { cpt_5_minutes += 3000;                                                           /* Sauvegarde toutes les 5 minutes */
        }

       if (cpt_1_minute < Partage->top)                                                       /* Update DB toutes les minutes */
        { /*soup_websocket_connection_send_text ( Partage->com_msrv.API_websocket, "ping" );*/
          /*if(body)
           { Json_node_add_string ( body, "zmq_tag", "SET_WATCHDOG" );
             Json_node_add_string ( body, "tech_id",  g_get_host_name() );
             Json_node_add_string ( body, "acronyme", "IO_COMM" );
             Json_node_add_int    ( body, "consigne", 900 );
             Zmq_Send_json_node ( Partage->com_msrv.zmq_to_master, g_get_host_name(), Config.master_hostname, body );
             Json_node_unref(body);
           }*/
          if (Partage->com_msrv.API_websocket == NULL) MSRV_ws_init();                 /* Si websocket closed, try to restart */
          Print_SQL_status();                                                             /* Print SQL status for debugging ! */
          cpt_1_minute += 600;                                                               /* Sauvegarde toutes les minutes */
        }

       /*if (Partage->com_msrv.last_master_ping + 1200 < Partage->top)
        { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Master is not responding. Restart Slave in 10s.", __func__ );
          Partage->com_msrv.Thread_run = FALSE;
          sleep(10);
        }*/
       usleep(1000);
       sched_yield();
     }

/*********************************** Terminaison: Deconnexion DB et kill des serveurs *****************************************/
/*    Http_Post_to_local_BUS ( module, "SLAVE_STOP", NULL );*/

    MSRV_ws_end();
    Decharger_librairies();                                                   /* Déchargement de toutes les librairies filles */
    Stopper_fils();                                                                        /* Arret de tous les fils watchdog */

/********************************* Dechargement des zones de bits internes dynamiques *****************************************/
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: fin boucle sans fin", __func__ );
    pthread_exit( NULL );
  }
/******************************************************************************************************************************/
/* Lire_ligne_commande: Parse la ligne de commande pour d'eventuels parametres                                                */
/* Entrée: argc, argv                                                                                                         */
/* Sortie: -1 si erreur, 0 si ok                                                                                              */
/******************************************************************************************************************************/
 static void Lire_ligne_commande( int argc, char *argv[] )
  { gint help = 0, log_level = -1, single = 0, version = 0;
    struct poptOption Options[]=
     { { "version",    'v', POPT_ARG_NONE,
         &version,          0, "Display Version Number", NULL },
       { "debug",      'd', POPT_ARG_INT,
         &log_level,      0, "Debug level", "LEVEL" },
       { "help",       'h', POPT_ARG_NONE,
         &help,             0, "Help", NULL },
       { "single",     's', POPT_ARG_NONE,
         &single,           0, "Don't start thread", NULL },
       POPT_TABLEEND
     };
    poptContext context;
    int rc;

    context = poptGetContext( NULL, argc, (const char **)argv, Options, POPT_CONTEXT_ARG_OPTS );
    while ( (rc = poptGetNextOpt( context )) != -1)                                          /* Parse de la ligne de commande */
     { switch (rc)
        { case POPT_ERROR_BADOPT: printf( "Option %s is unknown\n", poptBadOption(context, 0) );
                                  help=1; break;
          default: printf("Parsing error\n");
        }
     }

    if (help)                                                                                 /* Affichage de l'aide en ligne */
     { poptPrintHelp(context, stdout, 0);
       poptFreeContext(context);
       exit(EXIT_OK);
     }
    poptFreeContext( context );                                                                         /* Liberation memoire */

    if (version)                                                                            /* Affichage du numéro de version */
     { printf(" Watchdogd - Version %s\n", WTD_VERSION );
       exit(EXIT_OK);
     }

    if (single)          Config.single      = TRUE;                                            /* Demarrage en mode single ?? */
    if (log_level!=-1)   Config.log_level   = log_level;
    fflush(0);
  }
/******************************************************************************************************************************/
/* Drop_privileges: Passe sous un autre user que root                                                                         */
/* Entrée: néant                                                                                                              */
/* Sortie: EXIT si erreur                                                                                                     */
/******************************************************************************************************************************/
 static gboolean Drop_privileges( void )
  { struct passwd *pwd;

    if (getuid())
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                "%s: Error, running user is not 'root' Could not drop privileges.", __func__, getuid(), strerror(errno) );
       return(FALSE);
     }

    if (Config.headless)
     { pwd = getpwnam ( "watchdog" );
       if (!pwd)
        { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                    "%s: 'watchdog' user not found while Headless, creating.", __func__ );
          system("useradd -m -c 'WatchdogServer' watchdog" );
        }
       pwd = getpwnam ( "watchdog" );
       if (!pwd)
        { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                   "%s: Creation of user 'watchdog' failed (%s). Stopping.", __func__, strerror(errno) );
          return(FALSE);
        }
     }
    else /* When not headless */
     { gchar *session;
       uid_t active_session;
       if (sd_seat_get_active(	"seat0",	&session,	&active_session) < 0)
        { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                    "%s: seat_get_active failed (%s). Waiting 5s.", __func__, strerror (errno) );
          return(FALSE);
        }
       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                "%s: session found = '%s' for user '%d'", __func__, session, active_session );
       pwd = getpwuid ( active_session );
       if (!pwd)
        { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                   "%s: Error when searching seat user. Stopping.", __func__ );
          return(FALSE);
        }
     }
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Target User '%s' (uid %d) found.\n", __func__, pwd->pw_name, pwd->pw_uid );

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
             "%s: Dropping from root to '%s' (%d).\n", __func__, pwd->pw_name, pwd->pw_uid );

    /* setting groups */
    gid_t *grp_list = NULL;
    gint nbr_group = 0;
    struct group *grp;

    grp = getgrnam("audio");
    if (grp)
     { nbr_group++;
       grp_list = g_try_realloc ( grp_list, sizeof(gid_t) * nbr_group );
       grp_list[nbr_group-1] = grp->gr_gid;
     }

    grp = getgrnam("dialout");
    if (grp)
     { nbr_group++;
       grp_list = g_try_realloc ( grp_list, sizeof(gid_t) * nbr_group );
       grp_list[nbr_group-1] = grp->gr_gid;
     }

    grp = getgrnam("gpio");
    if (grp)
     { nbr_group++;
       grp_list = g_try_realloc ( grp_list, sizeof(gid_t) * nbr_group );
       grp_list[nbr_group-1] = grp->gr_gid;
     }

    if (nbr_group)
     { if (setgroups ( nbr_group, grp_list )==-1)
        { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Error, cannot SetGroups for user '%s' (%s)\n",
                    __func__, pwd->pw_name, strerror(errno) );
          g_free(grp_list);
          return(FALSE);
        }
       g_free(grp_list);
     }

    if (setregid ( pwd->pw_gid, pwd->pw_gid )==-1)                                                  /* On drop les privilèges */
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Error, cannot setREgid for user '%s' (%s)\n",
                 __func__, pwd->pw_name, strerror(errno) );
       return(FALSE);
     }

    if (setreuid ( pwd->pw_uid, pwd->pw_uid )==-1)                                                  /* On drop les privilèges */
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Error, cannot setREuid for user '%s' (%s)\n",
                 __func__, pwd->pw_name, strerror(errno) );
       return(FALSE);
     }

    if ( Config.headless )
         { g_snprintf(Config.home, sizeof(Config.home), "%s", pwd->pw_dir ); }
    else { g_snprintf(Config.home, sizeof(Config.home), "%s/.watchdog", pwd->pw_dir ); }
    mkdir (Config.home, 0);

    if (Config.instance_is_master)
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), "%s/Dls", Config.home );
       mkdir ( chaine, S_IRUSR | S_IWUSR | S_IXUSR );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Created Dls '%s' directory'", __func__, chaine );
     }

    if (chdir(Config.home))                                                             /* Positionnement à la racine du home */
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Chdir %s failed\n", __func__, Config.home ); exit(EXIT_ERREUR); }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Chdir %s successfull. PID=%d\n", __func__, Config.home, getpid() ); }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du serveur watchdog                                                                              */
/* Entrée: argc, argv                                                                                                         */
/* Sortie: -1 si erreur, 0 si ok                                                                                              */
/******************************************************************************************************************************/
 int main ( int argc, char *argv[], char *envp[] )
  { struct itimerval timer;
    struct sigaction sig;
    gchar strpid[12];
    gint fd_lock;
    pthread_t TID;
    gint error_code = EXIT_OK;

    umask(022);                                                                              /* Masque de creation de fichier */

    Lire_config();                                                     /* Lecture sur le fichier /etc/abls-habitat-agent.conf */
    Config.log = Info_init( "Watchdogd", Config.log_level );                                           /* Init msgs d'erreurs */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Start %s", WTD_VERSION );

    Partage = Shm_init();                                                            /* Initialisation de la mémoire partagée */
    if (!Partage)
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "Shared memory failed to allocate" );
       error_code = EXIT_FAILURE;
       goto first_stage_end;
     }

    if ( Config.installed == FALSE )                                                    /* Si le fichier de conf n'existe pas */
     { Partage->com_msrv.Thread_run = TRUE;                                          /* On dit au maitre que le thread tourne */
       if (!Demarrer_http())                                                                                /* Démarrage HTTP */
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Pb HTTP", __func__ );
          sleep(5);
          error_code = EXIT_FAILURE;
          goto second_stage_end;
        }
       while(Partage->com_msrv.Thread_run == TRUE) sleep(1);                              /* On tourne tant que l'on a besoin */
       Stopper_fils();
       Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Agent %s Installed", WTD_VERSION );
       goto second_stage_end;
     }

/************************************************* Init libsoup session *******************************************************/
    Partage->com_msrv.API_session = soup_session_new_with_options( "idle_timeout", 0, "timeout", 60, "ssl-strict", TRUE,
                                                                   "user-agent", "Abls-habitat Agent", NULL );

/************************************************* Test Connexion to Global API ***********************************************/
    JsonNode *API = Http_Get_from_global_API ( "status", NULL );
    if (API)
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Connected with API %s", __func__, Json_get_string ( API, "version" ) );
       Json_node_unref ( API );
     }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connection to Global API FAILED. Sleep 5s and stopping.", __func__ );
       sleep(5);
       error_code = EXIT_FAILURE;
       goto second_stage_end;
     }
/************************************************* Tell Global API thread is UP ***********************************************/
    JsonNode *RootNode = Json_node_create();
    if (RootNode)
     { Json_node_add_int    ( RootNode, "start_time", time(NULL) );
       Json_node_add_string ( RootNode, "agent_hostname", g_get_host_name() );
       Json_node_add_string ( RootNode, "version", WTD_VERSION );
       Json_node_add_string ( RootNode, "install_time", Json_get_string ( Config.config, "install_time" ) );

       JsonNode *api_result = Http_Post_to_global_API ( "/run/agent/start", RootNode );
       Json_node_unref ( RootNode );
       if (api_result && Json_get_int ( api_result, "api_status" ) == SOUP_STATUS_OK)
         { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: API Request for AGENT START OK.", __func__ ); }
        else
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: API Request for AGENT START failed. Sleep 5s and stopping.", __func__ );
          sleep(5);
          error_code = EXIT_FAILURE;
          goto second_stage_end;
        }

       Config.headless           = Json_get_bool ( api_result, "headless" );
       Config.log_bus            = Json_get_bool ( api_result, "log_bus" );
       Config.log_msrv           = Json_get_bool ( api_result, "log_msrv" );
       Config.instance_is_master = Json_get_bool ( api_result, "is_master" );
       gchar *master_hostname    = Json_get_string ( api_result, "master_hostname" );
       if (master_hostname) g_snprintf( Config.master_hostname, sizeof(Config.master_hostname), "%s", master_hostname );
                       else g_snprintf( Config.master_hostname, sizeof(Config.master_hostname), "nomasterhost" );

       Info_change_log_level ( Config.log, Json_get_int ( api_result, "log_level" ) );
       Json_node_unref ( api_result );
     }
/******************************************************* Drop privileges ******************************************************/
    if (!Drop_privileges()) { sleep(5); goto second_stage_end; }

    Lire_ligne_commande( argc, argv );                                            /* Lecture du fichier conf et des arguments */
    Print_config();

    fd_lock = open( VERROU_SERVEUR, O_RDWR | O_CREAT | O_SYNC, 0640 );              /* Verification de l'unicité du processus */
    if (fd_lock<0)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Lock file creation failed: %s/%s.", __func__, Config.home, VERROU_SERVEUR );
       error_code = EXIT_FAILURE;
       goto second_stage_end;
     }
    if (flock( fd_lock, LOCK_EX | LOCK_NB )<0)                                         /* Creation d'un verrou sur le fichier */
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Cannot lock %s/%s. Probably another daemon is running : %s.", __func__,
                 Config.home, VERROU_SERVEUR, strerror(errno) );
       error_code = EXIT_FAILURE;
       goto third_stage_end;
     }
    fcntl(fd_lock, F_SETFD, FD_CLOEXEC );                                                           /* Set close on exec flag */
    g_snprintf( strpid, sizeof(strpid), "%d\n", getpid() );                                /* Enregistrement du pid au cas ou */
    if (write( fd_lock, strpid, strlen(strpid) )<0)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Cannot write PID on %s/%s (%s).", __func__,
                 Config.home, VERROU_SERVEUR, strerror(errno) );
       error_code = EXIT_FAILURE;
       goto third_stage_end;
     }

    pthread_mutexattr_t attr;                                                       /* Initialisation des mutex de synchro */
    time ( &Partage->start_time );
    pthread_mutexattr_init( &attr );
    pthread_mutexattr_setpshared( &attr, PTHREAD_PROCESS_SHARED );
    pthread_mutex_init( &Partage->com_msrv.synchro, &attr );
    pthread_mutex_init( &Partage->com_http.synchro, &attr );
    pthread_mutex_init( &Partage->com_dls.synchro, &attr );
    pthread_mutex_init( &Partage->com_dls.synchro_traduction, &attr );
    pthread_mutex_init( &Partage->com_dls.synchro_data, &attr );
    pthread_mutex_init( &Partage->com_arch.synchro, &attr );
    pthread_mutex_init( &Partage->com_db.synchro, &attr );

/********************************** Création des zones de bits internes dynamiques *****************************************/
    Partage->Dls_data_DI       = NULL;
    Partage->Dls_data_DO       = NULL;
    Partage->Dls_data_AI       = NULL;
    Partage->Dls_data_AO       = NULL;
    Partage->Dls_data_MONO     = NULL;
    Partage->Dls_data_BI       = NULL;
    Partage->Dls_data_REGISTRE = NULL;
    Partage->Dls_data_MSG      = NULL;
    Partage->Dls_data_CH       = NULL;
    Partage->Dls_data_CI       = NULL;
    Partage->Dls_data_TEMPO    = NULL;
    Partage->Dls_data_VISUEL   = NULL;
    Partage->Dls_data_WATCHDOG = NULL;

    sigfillset (&sig.sa_mask);                                                    /* Par défaut tous les signaux sont bloqués */
    pthread_sigmask( SIG_SETMASK, &sig.sa_mask, NULL );

    Update_database_schema();                                                       /* Update du schéma de Database si besoin */
    Charger_config_bit_interne ();                            /* Chargement des configurations des bits internes depuis la DB */

    if (Config.instance_is_master)
     { if ( pthread_create( &TID, NULL, (void *)Boucle_pere_master, NULL ) )
        { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                   "%s: Demarrage boucle sans fin pthread_create failed %s", __func__, strerror(errno) );
        }
     }
    else
     { if ( pthread_create( &TID, NULL, (void *)Boucle_pere_slave, NULL ) )
        { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                   "%s: Demarrage boucle sans fin pthread_create failed %s", __func__, strerror(errno) );
        }
     }
                                                         /********** Mise en place de la gestion des signaux ******************/
    sig.sa_handler = Traitement_signaux;                                            /* Gestionnaire de traitement des signaux */
    sig.sa_flags = SA_RESTART;                            /* Voir Linux mag de novembre 2002 pour le flag anti cut read/write */
    sigaction( SIGALRM, &sig, NULL );                                                                /* Reinitialisation soft */
    sigaction( SIGUSR1, &sig, NULL );                                                      /* Reinitialisation DLS uniquement */
    sigaction( SIGUSR2, &sig, NULL );                                                      /* Reinitialisation DLS uniquement */
    sigaction( SIGINT,  &sig, NULL );                                                                /* Reinitialisation soft */
    sigaction( SIGTERM, &sig, NULL );
    sigaction( SIGABRT, &sig, NULL );
    sigaction( SIGPIPE, &sig, NULL );                                                  /* Pour prevenir un segfault du client */
    sigfillset (&sig.sa_mask);                                                    /* Par défaut tous les signaux sont bloqués */
    sigdelset ( &sig.sa_mask, SIGALRM );
    sigdelset ( &sig.sa_mask, SIGUSR1 );
    sigdelset ( &sig.sa_mask, SIGUSR2 );
    sigdelset ( &sig.sa_mask, SIGINT  );
    sigdelset ( &sig.sa_mask, SIGTERM );
    sigdelset ( &sig.sa_mask, SIGABRT );
    sigdelset ( &sig.sa_mask, SIGPIPE );
    pthread_sigmask( SIG_SETMASK, &sig.sa_mask, NULL );

    timer.it_value.tv_sec = timer.it_interval.tv_sec = 0;                                       /* Tous les 100 millisecondes */
    timer.it_value.tv_usec = timer.it_interval.tv_usec = 100000;                                    /* = 10 fois par secondes */
    setitimer( ITIMER_REAL, &timer, NULL );                                                                /* Active le timer */

    pthread_join( TID, NULL );                                                          /* Attente fin de la boucle pere MSRV */
/********************************* Dechargement des zones de bits internes dynamiques *****************************************/

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique MONO", __func__ );
    g_slist_foreach (Partage->Dls_data_MONO, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_MONO);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique BI", __func__ );
    g_slist_foreach (Partage->Dls_data_BI, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_BI);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique DI", __func__ );
    g_slist_foreach (Partage->Dls_data_DI, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_DI);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique DO", __func__ );
    g_slist_foreach (Partage->Dls_data_DO, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_DO);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique AI", __func__ );
    g_slist_foreach (Partage->Dls_data_AI, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_AI);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique R", __func__ );
    g_slist_foreach (Partage->Dls_data_REGISTRE, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_REGISTRE);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique AO", __func__ );
    g_slist_foreach (Partage->Dls_data_AO, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_AO);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique MSG", __func__ );
    g_slist_foreach (Partage->Dls_data_MSG, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_MSG);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique TEMPO", __func__ );
    g_slist_foreach (Partage->Dls_data_TEMPO, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_TEMPO);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique CH", __func__ );
    g_slist_foreach (Partage->Dls_data_CH, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_CH);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique CI", __func__ );
    g_slist_foreach (Partage->Dls_data_CI, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_CI);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique VISUEL", __func__ );
    g_slist_foreach (Partage->Dls_data_VISUEL, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_VISUEL);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique WATCHDOG", __func__ );
    g_slist_foreach (Partage->Dls_data_WATCHDOG, (GFunc) g_free, NULL );
    g_slist_free (Partage->Dls_data_WATCHDOG);

    pthread_mutex_destroy( &Partage->com_msrv.synchro );
    pthread_mutex_destroy( &Partage->com_dls.synchro );
    pthread_mutex_destroy( &Partage->com_dls.synchro_traduction );
    pthread_mutex_destroy( &Partage->com_dls.synchro_data );
    pthread_mutex_destroy( &Partage->com_arch.synchro );
    pthread_mutex_destroy( &Partage->com_db.synchro );

    sigfillset (&sig.sa_mask);                                                    /* Par défaut tous les signaux sont bloqués */
    pthread_sigmask( SIG_SETMASK, &sig.sa_mask, NULL );

third_stage_end:
    close(fd_lock);                                           /* Fermeture du FileDescriptor correspondant au fichier de lock */

second_stage_end:
    soup_session_abort ( Partage->com_msrv.API_session );
    g_object_unref( Partage->com_msrv.API_session );
    Partage->com_msrv.API_session = NULL;
    Shm_stop( Partage );                                                                       /* Libération mémoire partagée */

first_stage_end:
    if (Config.config) Json_node_unref ( Config.config );
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Stopped", __func__ );
    return(error_code);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
