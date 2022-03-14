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
  { gchar *ttech_id_1 = Json_get_string ( node1, "thread_tech_id" );
    gchar *ttech_id_2 = Json_get_string ( node2, "thread_tech_id" );
    gint result = strcasecmp ( ttech_id_1, ttech_id_2 );
    if (result) return(result);
    gchar *tacronyme_1 = Json_get_string ( node1, "thread_acronyme" );
    gchar *tacronyme_2 = Json_get_string ( node2, "thread_acronyme" );
    return( strcasecmp ( tacronyme_1, tacronyme_2 ) );
  }
/******************************************************************************************************************************/
/* MSRV_Comparer_clef_local: Compare deux clefs locales dans le mapping                                                       */
/* Entrée: néant                                                                                                              */
/******************************************************************************************************************************/
 static gint MSRV_Comparer_clef_local ( JsonNode *node1, JsonNode *node2 )
  { gchar *tech_id_1 = Json_get_string ( node1, "tech_id" );
    gchar *tech_id_2 = Json_get_string ( node2, "tech_id" );
    gint result = strcasecmp ( tech_id_1, tech_id_2 );
    if (result) return(result);
    gchar *acronyme_1 = Json_get_string ( node1, "acronyme" );
    gchar *acronyme_2 = Json_get_string ( node2, "acronyme" );
    return( strcasecmp ( acronyme_1, acronyme_2 ) );
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
     { json_node_unref ( Partage->Maps_root );
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
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Saving DLS_DATA", __func__ );
    Updater_confDB_AO();                                                    /* Sauvegarde des valeurs des Sorties Analogiques */
    Updater_confDB_DO();                                                            /* Sauvegarde des valeurs des Sorties TOR */
    Updater_confDB_CH();                                                                  /* Sauvegarde des compteurs Horaire */
    Updater_confDB_CI();                                                              /* Sauvegarde des compteurs d'impulsion */
    Updater_confDB_AI();                                                              /* Sauvegarde des compteurs d'impulsion */
    Updater_confDB_Registre();                                                                    /* Sauvegarde des registres */
    Updater_confDB_MSG();                                                              /* Sauvegarde des valeurs des messages */
    Updater_confDB_MONO();                                             /* Sauvegarde des valeurs des bistables et monostables */
    Updater_confDB_BI();                                             /* Sauvegarde des valeurs des bistables et monostables */
  }
/******************************************************************************************************************************/
/* Handle_zmq_common: Analyse et reagi à un message ZMQ a destination du MSRV ou du SLAVE                                     */
/* Entrée: le message                                                                                                         */
/* Sortie: FALSE si n'a pas été pris en charge                                                                                */
/******************************************************************************************************************************/
 static gboolean Handle_zmq_common ( JsonNode *request, gchar *zmq_tag, gchar *zmq_src_tech_id, gchar *zmq_dst_tech_id )
  {     if ( !strcasecmp( zmq_tag, "PROCESS_RELOAD") &&
              Json_has_member ( request, "uuid" )
            )
     { return ( Process_reload_by_uuid ( Json_get_string ( request, "uuid" ) ) ); }
    else if ( !strcasecmp( zmq_tag, "PROCESS_DEBUG") &&
              Json_has_member ( request, "uuid" ) && Json_has_member ( request, "debug" )
            )
     { return ( Process_set_debug ( Json_get_string ( request, "uuid" ), Json_get_bool ( request, "debug" ) ) ); }

    if ( strcasecmp ( zmq_dst_tech_id, g_get_host_name() ) ) return(FALSE);                               /* Si pas pour nous */

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Handle_zmq_message_for_master: Analyse et reagi à un message ZMQ a destination du MSRV                                     */
/* Entrée: le message                                                                                                         */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Handle_zmq_for_master ( JsonNode *request )
  { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
    gchar *zmq_src_tech_id = Json_get_string ( request, "zmq_src_tech_id" );
    gchar *zmq_dst_tech_id = Json_get_string ( request, "zmq_dst_tech_id" );

    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: receive '%s' from '%s' to '%s'",
              __func__, zmq_tag, zmq_src_tech_id, zmq_dst_tech_id );

         if ( !strcasecmp( zmq_tag, "SET_WATCHDOG") )
     { if (! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
              Json_has_member ( request, "consigne" ) ) )
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: SET_WATCHDOG: wrong parameters from '%s'", __func__, zmq_src_tech_id );
          return(TRUE);                                                              /* Traité en erreur, mais traité qd meme */
        }

       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                 "%s: SET_WATCHDOG from '%s' to '%s': '%s:%s'=%d", __func__,
                 zmq_src_tech_id, zmq_dst_tech_id,
                 Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ),
                 Json_get_int ( request, "consigne" ) );
       Dls_data_set_WATCHDOG ( NULL, Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ), NULL,
                               Json_get_int ( request, "consigne" ) );
       return(TRUE);                                                                                                /* Traité */
     }
/************************************ Positionne une valeur d'une Entrée Analogique *******************************************/
    else if ( !strcasecmp( zmq_tag, "SET_AI") )
     { if (! (Json_has_member ( request, "thread_tech_id" ) && Json_has_member ( request, "thread_acronyme" ) &&
              Json_has_member ( request, "valeur" ) && Json_has_member ( request, "in_range" ) &&
              Json_has_member ( request, "libelle" ) && Json_has_member ( request, "unite" )
             )
          )
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: SET_AI: wrong parameters from '%s'", __func__, zmq_src_tech_id );
          return(TRUE);                                                              /* Traité en erreur, mais traité qd meme */
        }

       gchar *thread_tech_id  = Json_get_string ( request, "thread_tech_id" );
       gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
       gchar *tech_id         = thread_tech_id;
       gchar *acronyme        = thread_acronyme;

       JsonNode *map = g_tree_lookup ( Partage->Maps_from_thread, request );
       if (map)
        { tech_id  = Json_get_string ( map, "tech_id" );
          acronyme = Json_get_string ( map, "acronyme" );
        }
       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                 "%s: SET_AI from '%s' to '%s': '%s:%s/'%s:%s'=%f %s (range=%d)", __func__,
                 zmq_src_tech_id, zmq_dst_tech_id, thread_tech_id, thread_acronyme, tech_id, acronyme,
                 Json_get_double ( request, "valeur" ), Json_get_string ( request, "unite" ), Json_get_bool ( request, "in_range" ) );
       struct DLS_AI *ai = NULL;
       Dls_data_set_AI ( tech_id, acronyme, (gpointer)&ai,
                         Json_get_double ( request, "valeur" ), Json_get_bool ( request, "in_range" ) );
       if (Json_get_bool ( request, "first_send" ) == TRUE )
        { g_snprintf ( ai->libelle, sizeof(ai->libelle), "%s", Json_get_string ( request, "libelle" ) );
          g_snprintf ( ai->unite,   sizeof(ai->unite),   "%s", Json_get_string ( request, "unite" ) );
          ai->archivage = Json_get_int ( request, "archivage" );
          gchar *libelle = Normaliser_chaine ( ai->libelle );
          SQL_Write_new ( "INSERT INTO mappings SET classe='AI', "
                          "thread_tech_id = '%s', thread_acronyme = '%s', tech_id = '%s', acronyme = '%s', libelle='%s' "
                          "ON DUPLICATE KEY UPDATE classe=VALUE(classe), libelle=VALUE(libelle) ",
                          thread_tech_id, thread_acronyme, tech_id, acronyme, libelle );
          g_free(libelle);
        }
       return(TRUE);                                                                                                /* Traité */
     }
/************************************ Réaction sur SET_CDE ********************************************************************/
    else if ( !strcasecmp( zmq_tag, "SET_CDE") )
     { if (! (Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) ) )
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: SET_CDE: wrong parameters from '%s'", __func__, zmq_src_tech_id );
          return(TRUE);                                                              /* Traité en erreur, mais traité qd meme */
        }
       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                 "%s: SET_CDE from '%s' to '%s': '%s:%s'=1", __func__,
                 zmq_src_tech_id, zmq_dst_tech_id,
                 Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ) );
       Envoyer_commande_dls_data ( Json_get_string ( request, "tech_id" ), Json_get_string ( request, "acronyme" ) );
       return(TRUE);                                                                                                /* Traité */
     }
/************************************ Réaction sur SET_DI *********************************************************************/
    else if ( !strcasecmp( zmq_tag, "SET_DI") )
     { if (! (Json_has_member ( request, "thread_tech_id" ) && Json_has_member ( request, "thread_acronyme" ) &&
              Json_has_member ( request, "etat" )&& Json_has_member ( request, "libelle" )
             )
          )
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: SET_DI: wrong parameters from '%s'", __func__, zmq_src_tech_id );
          return(TRUE);                                                              /* Traité en erreur, mais traité qd meme */
        }

       gchar *thread_tech_id  = Json_get_string ( request, "thread_tech_id" );
       gchar *thread_acronyme = Json_get_string ( request, "thread_acronyme" );
       gchar *tech_id         = thread_tech_id;
       gchar *acronyme        = thread_acronyme;

       JsonNode *map = g_tree_lookup ( Partage->Maps_from_thread, request );
       if (map)
        { tech_id  = Json_get_string ( map, "tech_id" );
          acronyme = Json_get_string ( map, "acronyme" );
        }
       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                 "%s: SET_DI from '%s' to '%s': '%s:%s/'%s:%s'=%d", __func__,
                 zmq_src_tech_id, zmq_dst_tech_id, thread_tech_id, thread_acronyme, tech_id, acronyme,
                 Json_get_bool ( request, "etat" ) );
       struct DLS_DI *di = NULL;
       Dls_data_set_DI ( NULL, tech_id, acronyme, (gpointer)&di, Json_get_bool ( request, "etat" ) );
       if (Json_get_bool ( request, "first_send" ) == TRUE )
        { g_snprintf ( di->libelle, sizeof(di->libelle), "%s", Json_get_string ( request, "libelle" ) );
          gchar *libelle = Normaliser_chaine ( di->libelle );
          SQL_Write_new ( "INSERT INTO mappings SET classe='DI', "
                          "thread_tech_id = '%s', thread_acronyme = '%s', tech_id = '%s', acronyme = '%s', libelle='%s' "
                          "ON DUPLICATE KEY UPDATE classe=VALUE(classe), libelle=VALUE(libelle) ",
                          thread_tech_id, thread_acronyme, tech_id, acronyme, libelle );
          g_free(libelle);
        }
       return(TRUE);                                                                                                /* Traité */
     }
    else if ( !strcasecmp( zmq_tag, "SET_SYN_VARS") )
     { Http_ws_send_to_all( request );
       return(TRUE);                                                                                                /* Traité */
     }
    else if ( !strcasecmp( zmq_tag, "SLAVE_STOP") )
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: SLAVE '%s' stopped !", __func__, zmq_src_tech_id );
       return(TRUE);                                                                                                /* Traité */
     }
    else if ( !strcasecmp( zmq_tag, "SLAVE_START") )
     { struct DLS_AO *ao;
       GSList *liste;
       Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: SLAVE '%s' started. Sending AO !", __func__, zmq_src_tech_id );
       liste = Partage->Dls_data_AO;
       while (liste)
        { ao = (struct DLS_AO *)Partage->com_msrv.Liste_AO->data;            /* Recuperation du numero de a */
          JsonNode *RootNode = Json_node_create ();
          if (RootNode)
           { Dls_AO_to_json( RootNode, ao );
             Json_node_add_string ( RootNode, "zmq_tag", "SET_AO" );
             Zmq_Send_json_node ( Partage->com_msrv.zmq_to_slave, g_get_host_name(), "*", RootNode );
             json_node_unref(RootNode);
           }
          liste = g_slist_next(liste);
        }
       return(TRUE);                                                                                                /* Traité */
     }

    return ( Handle_zmq_common ( request, zmq_tag, zmq_src_tech_id, zmq_dst_tech_id ) );
  }
/******************************************************************************************************************************/
/* Handle_zmq_message_for_master: Analyse et reagi à un message ZMQ a destination du MSRV                                     */
/* Entrée: le message                                                                                                         */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gboolean Handle_zmq_for_slave ( JsonNode *request )
  { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
    gchar *zmq_src_tech_id   = Json_get_string ( request, "zmq_src_tech_id" );
    gchar *zmq_dst_tech_id   = Json_get_string ( request, "zmq_dst_tech_id" );

    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: receive '%s' from '%s' to '%s'",
              __func__, zmq_tag, zmq_src_tech_id, zmq_dst_tech_id );

         if ( !strcasecmp( zmq_tag, "PING") )
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: receive PING from '%s'", __func__, zmq_src_tech_id );
       Partage->com_msrv.last_master_ping = Partage->top;
       return(TRUE);                                                                                                /* Traité */
     }
    return ( Handle_zmq_common ( request, zmq_tag, zmq_src_tech_id, zmq_dst_tech_id ) );
  }
/******************************************************************************************************************************/
/* Boucle_pere: boucle de controle du pere de tous les serveurs                                                               */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void *Boucle_pere_master ( void )
  { gint cpt_5_minutes, cpt_1_minute;
    struct ZMQUEUE *zmq_from_slave, *zmq_from_bus;

    prctl(PR_SET_NAME, "W-MASTER", 0, 0, 0 );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Debut boucle sans fin", __func__ );

    gchar *requete = SQL_Read_from_file ( "base_icones.sql" );                                    /* Load DB icons at startup */
    if (!requete)
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Icons DB Error.", __func__ ); }
    else if (!SQL_Writes ( requete ))
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Icons DB SQL Error.", __func__ ); }
    else Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Icons DB Loaded.", __func__ );
    if (requete) g_free(requete);

/************************************************* Socket ZMQ interne *********************************************************/
    Partage->com_msrv.zmq_to_bus = Zmq_Bind ( ZMQ_PUB, "pub-to-bus", "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    zmq_from_bus                 = Zmq_Bind ( ZMQ_PULL, "listen-to-bus", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

/***************************************** Socket pour une instance master ****************************************************/
    Partage->com_msrv.zmq_to_slave = Zmq_Bind ( ZMQ_PUB, "pub-to-slave", "tcp", "*", 5555 );
    zmq_from_slave                 = Zmq_Bind ( ZMQ_PULL, "listen-to-slave", "tcp", "*", 5556 );

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

          Charger_librairies();                                               /* Chargement de toutes les librairies Watchdog */
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: NOT starting threads (Instance is not installed)", __func__ ); }
     }


/***************************************** Charge le mapping des bits internes ************************************************/
    MSRV_Remap();

/***************************************** Debut de la boucle sans fin ********************************************************/
    cpt_5_minutes = Partage->top + 3000;
    cpt_1_minute  = Partage->top + 600;

    sleep(1);
    Partage->com_msrv.Thread_run = TRUE;                                             /* On dit au maitre que le thread tourne */
    while(Partage->com_msrv.Thread_run == TRUE)                                           /* On tourne tant que l'on a besoin */
     { gchar buffer[2048];
       JsonNode *request;

       Gerer_arrive_MSGxxx_dls();                                 /* Redistrib des messages DLS vers les clients + Historique */
       Gerer_arrive_Ixxx_dls();                                                 /* Distribution des changements d'etats motif */
       Gerer_arrive_Axxx_dls();                                           /* Distribution des changements d'etats sorties TOR */

       request = Recv_zmq_with_json( zmq_from_slave, g_get_host_name(), (gchar *)&buffer, sizeof(buffer) );
       if (request)
        { Handle_zmq_for_master( request );
          json_node_unref ( request );
        }

       request = Recv_zmq_with_json( zmq_from_bus, NULL, (gchar *)&buffer, sizeof(buffer) );
       if (request)
        { gint taille = strlen(buffer);
          if (!Handle_zmq_for_master( request ))                   /* Gère d'abord le message avant de l'envoyer au bus local */
           { Zmq_Send_as_raw ( Partage->com_msrv.zmq_to_bus, buffer, taille );                        /* on envoi aux threads */
             Zmq_Send_as_raw ( Partage->com_msrv.zmq_to_slave, buffer, taille );                       /* on envoi aux slaves */
           }
          json_node_unref ( request );
        }

       if (cpt_5_minutes < Partage->top)                                                    /* Update DB toutes les 5 minutes */
        { Save_dls_data_to_DB();
          cpt_5_minutes += 3000;                                                           /* Sauvegarde toutes les 5 minutes */
        }

       if (cpt_1_minute < Partage->top)                                                       /* Update DB toutes les minutes */
        { static gpointer bit_io_comm = NULL;
          JsonNode *RootNode = Json_node_create();
          if (RootNode)
           { Json_node_add_string ( RootNode, "zmq_tag", "PING" );
             Zmq_Send_json_node ( Partage->com_msrv.zmq_to_slave, g_get_host_name(), Config.master_hostname, RootNode );
             json_node_unref(RootNode);
           }
          Dls_data_set_WATCHDOG ( NULL, g_get_host_name(), "IO_COMM", &bit_io_comm, 900 );
          Print_SQL_status();                                                             /* Print SQL status for debugging ! */
          Activer_horlogeDB();
          cpt_1_minute += 600;                                                               /* Sauvegarde toutes les minutes */
        }

       usleep(1000);
       sched_yield();
     }

/*********************************** Terminaison: Deconnexion DB et kill des serveurs *****************************************/
    if (!Config.installed) { sleep(2); }              /* Laisse le temps au thread HTTP de repondre OK au client avant reboot */
    Save_dls_data_to_DB();                                                                 /* Dernière sauvegarde avant arret */

    Decharger_librairies();                                                   /* Déchargement de toutes les librairies filles */
    Stopper_fils();                                                                        /* Arret de tous les fils watchdog */
    Zmq_Close ( Partage->com_msrv.zmq_to_bus );
    Zmq_Close ( zmq_from_bus );
    Zmq_Close ( Partage->com_msrv.zmq_to_slave );
    Zmq_Close ( zmq_from_slave );

    if (Partage->Maps_from_thread) g_tree_destroy ( Partage->Maps_from_thread );
    if (Partage->Maps_to_thread) g_tree_destroy ( Partage->Maps_to_thread );
    if (Partage->Maps_root) json_node_unref ( Partage->Maps_root );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: fin boucle sans fin", __func__ );
    pthread_exit( NULL );
  }
/******************************************************************************************************************************/
/* Boucle_pere: boucle de controle du pere de tous les serveurs                                                               */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void *Boucle_pere_slave ( void )
  { struct ZMQUEUE *zmq_from_master, *zmq_from_bus;
    gint cpt_5_minutes = 0, cpt_1_minute = 0;
    gchar chaine[128];

    prctl(PR_SET_NAME, "W-SLAVE", 0, 0, 0 );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Debut boucle sans fin", __func__ );

    g_snprintf(chaine, sizeof(chaine), "Gestion de l'instance slave %s", g_get_host_name());
    if (Dls_auto_create_plugin( g_get_host_name(), chaine ) == FALSE)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: %s: DLS Create ERROR\n", __func__, g_get_host_name ); }

    g_snprintf(chaine, sizeof(chaine), "Statut de la communication avec le slave %s", g_get_host_name() );
    Mnemo_auto_create_WATCHDOG ( FALSE, g_get_host_name(), "IO_COMM", chaine );

/************************************************* Socket ZMQ interne *********************************************************/
    Partage->com_msrv.zmq_to_bus = Zmq_Bind ( ZMQ_PUB, "pub-to-bus",    "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    zmq_from_bus                 = Zmq_Bind ( ZMQ_PULL, "listen-to-bus", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

/***************************************** Socket de subscription au master ***************************************************/
    Partage->com_msrv.zmq_to_master = Zmq_Connect ( ZMQ_PUSH, "pub-to-master", "tcp", Config.master_hostname, 5556 );
    zmq_from_master                 = Zmq_Connect ( ZMQ_SUB, "listen-to-master", "tcp", Config.master_hostname, 5555 );

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

/***************************************** Debut de la boucle sans fin ********************************************************/
    sleep(1);
    Partage->com_msrv.Thread_run = TRUE;                                             /* On dit au maitre que le thread tourne */
    JsonNode *RootNode = Json_node_create ();
    if (RootNode)
     { Json_node_add_string ( RootNode, "zmq_tag", "SLAVE_START" );
       Zmq_Send_json_node ( Partage->com_msrv.zmq_to_master, g_get_host_name(), Config.master_hostname, RootNode );
       json_node_unref ( RootNode );
     }
    while(Partage->com_msrv.Thread_run == TRUE)                                           /* On tourne tant que l'on a besoin */
     { gchar buffer[2048];
       JsonNode *request;
       gint byte;

       request = Recv_zmq_with_json( zmq_from_master, NULL, (gchar *)&buffer, sizeof(buffer) );
       if (request)
        { if (!Handle_zmq_for_slave( request ))                    /* Gère d'abord le message avant de l'envoyer au bus local */
           { Zmq_Send_as_raw ( Partage->com_msrv.zmq_to_bus, buffer, strlen(buffer) ); }        /* Sinon on envoi aux threads */
          json_node_unref ( request );
        }
                                                /* Si reception depuis un thread, report vers le master et les autres threads */
       if ( (byte=Recv_zmq( zmq_from_bus, &buffer, sizeof(buffer) )) > 0 )
        { /*Zmq_Send_as_raw ( Partage->com_msrv.zmq_to_bus, buffer, byte );*/
          Zmq_Send_as_raw ( Partage->com_msrv.zmq_to_master, buffer, byte );
        }

       if (cpt_5_minutes < Partage->top)                                                    /* Update DB toutes les 5 minutes */
        { cpt_5_minutes += 3000;                                                           /* Sauvegarde toutes les 5 minutes */
        }

       if (cpt_1_minute < Partage->top)                                                       /* Update DB toutes les minutes */
        { JsonNode *body = Json_node_create ();
          if(body)
           { Json_node_add_string ( body, "zmq_tag", "SET_WATCHDOG" );
             Json_node_add_string ( body, "tech_id",  g_get_host_name() );
             Json_node_add_string ( body, "acronyme", "IO_COMM" );
             Json_node_add_int    ( body, "consigne", 900 );
             Zmq_Send_json_node ( Partage->com_msrv.zmq_to_master, g_get_host_name(), Config.master_hostname, body );
             json_node_unref(body);
           }
          Print_SQL_status();                                                             /* Print SQL status for debugging ! */
          cpt_1_minute += 600;                                                               /* Sauvegarde toutes les minutes */
        }

       if (Partage->com_msrv.last_master_ping + 1200 < Partage->top)
        { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Master is not responding. Restart Slave in 10s.", __func__ );
          Partage->com_msrv.Thread_run = FALSE;
          sleep(10);
        }
       usleep(1000);
       sched_yield();
     }

/*********************************** Terminaison: Deconnexion DB et kill des serveurs *****************************************/
    RootNode = Json_node_create ();
    if (RootNode)
     { Json_node_add_string ( RootNode, "zmq_tag", "SLAVE_STOP" );
       Zmq_Send_json_node ( Partage->com_msrv.zmq_to_master, g_get_host_name(), Config.master_hostname, RootNode );
       Json_node_add_string ( RootNode, "zmq_tag", "SET_WATCHDOG" );
       Json_node_add_string ( RootNode, "tech_id",  g_get_host_name() );
       Json_node_add_string ( RootNode, "acronyme", "IO_COMM" );
       Json_node_add_int    ( RootNode, "consigne", 0 );
       Zmq_Send_json_node ( Partage->com_msrv.zmq_to_master, g_get_host_name(), Config.master_hostname, RootNode );
       json_node_unref ( RootNode );
     }

    Decharger_librairies();                                                   /* Déchargement de toutes les librairies filles */
    Stopper_fils();                                                                        /* Arret de tous les fils watchdog */
    Zmq_Close ( Partage->com_msrv.zmq_to_bus );
    Zmq_Close ( zmq_from_bus );
    Zmq_Close ( Partage->com_msrv.zmq_to_master );
    Zmq_Close ( zmq_from_master );

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
 static void Drop_privileges( void )
  { struct passwd *pwd, *old;

    pwd = getpwnam ( Config.run_as );
    if (!pwd)
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                "%s: Error, target user '%s' not found in /etc/passwd (%s).. Could not set run_as user\n", __func__, Config.run_as, strerror(errno) );
       exit(EXIT_ERREUR);
     }
    else Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: User '%s' (uid %d) found.\n", __func__, Config.run_as, pwd->pw_uid);

    old = getpwuid ( getuid() );
    if (!old)
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT,
                "%s: Error, actual user '%d' not found in /etc/passwd (%s).. Could not set run_as user\n", __func__, getuid(), strerror(errno) );
       exit(EXIT_ERREUR);
     }

    if (old->pw_uid != pwd->pw_uid)                                                      /* Besoin de changer d'utilisateur ? */
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                "%s: From '%s' (%d) To '%s' (%d).\n", __func__, old->pw_name, old->pw_uid, pwd->pw_name, pwd->pw_uid );

       if (initgroups ( Config.run_as, pwd->pw_gid )==-1)                                           /* On drop les privilèges */
        { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Error, cannot Initgroups for user '%s' (%s)\n",
                    __func__, Config.run_as, strerror(errno) );
          exit(EXIT_ERREUR);
        }

       if (setregid ( pwd->pw_gid, pwd->pw_gid )==-1)                                                              /* On drop les privilèges */
        { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Error, cannot setREgid for user '%s' (%s)\n",
                    __func__, Config.run_as, strerror(errno) );
          exit(EXIT_ERREUR);
        }

       if (setreuid ( pwd->pw_uid, pwd->pw_uid )==-1)                                                              /* On drop les privilèges */
        { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Error, cannot setREuid for user '%s' (%s)\n",
                    __func__, Config.run_as, strerror(errno) );
          exit(EXIT_ERREUR);
        }
     }
    if ( Config.use_subdir )
         { g_snprintf(Config.home, sizeof(Config.home), "%s/.watchdog", pwd->pw_dir ); }
    else { g_snprintf(Config.home, sizeof(Config.home), "%s", pwd->pw_dir ); }
    mkdir (Config.home, 0);

    if (chdir(Config.home))                                                             /* Positionnement à la racine du home */
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Chdir %s failed\n", __func__, Config.home ); exit(EXIT_ERREUR); }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Chdir %s successfull. PID=%d\n", __func__, Config.home, getpid() ); }
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

    umask(022);                                                                              /* Masque de creation de fichier */

    Lire_config();                                                  /* Lecture sur le fichier /etc/fr-abls-habitat.agent.conf */
    Config.log = Info_init( "Watchdogd", Config.log_level );                                           /* Init msgs d'erreurs */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Start %s", WTD_VERSION );

/********************************************************* Create instance UUID ***********************************************/
    if (!Json_has_member ( Config.config, "instance_uuid" ))    /* si jamais lancé, on ajoute un instance_uuid dans la config */
     { gchar instance_uuid[37];
       UUID_New ( instance_uuid );
       Json_node_add_string ( Config.config, "instance_uuid", instance_uuid );
       Json_write_to_file ( "/etc/fr-abls-habitat-agent.conf", Config.config );
     }

/************************************************* Test Connexion to Global API ***********************************************/
    JsonNode *API = Http_Get_from_global_API ( "status", NULL );
    if (API)
     { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Connected with API %s", __func__, Json_get_string ( API, "version" ) );
       json_node_unref ( API );
     }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connection to Global API FAILED. Sleep 5s and stopping.", __func__ );
       sleep(5);
       exit(-1);
     }
/************************************************* Tell Global API thread is UP ***********************************************/
    JsonNode *RootNode = Json_node_create();
    if (RootNode)
     { Json_node_add_int    ( RootNode, "start_time", time(NULL) );
       Json_node_add_string ( RootNode, "hostname", g_get_host_name() );
       Json_node_add_string ( RootNode, "version", WTD_VERSION );
       Json_node_add_string ( RootNode, "install_time", Json_get_string ( Config.config, "install_time" ) );
       JsonNode *api_result = Http_Post_to_global_API ( "instance", "START", RootNode );
       if (api_result) json_node_unref ( api_result );
       json_node_unref ( RootNode );
     }
/************************************************* Get instance parameters ****************************************************/
    JsonNode *api_result = Http_Post_to_global_API ( "instance", "GET_CONFIG", NULL );
    if (api_result)
     { gchar *run_as = Json_get_string ( api_result, "run_as" );
       if (run_as && strlen(run_as))
            g_snprintf( Config.run_as, sizeof(Config.run_as), "%s", run_as );
       else g_snprintf( Config.run_as, sizeof(Config.run_as), "watchdog" );

       Config.log_db             = Json_get_bool ( api_result, "log_db" );
       Config.log_bus            = Json_get_bool ( api_result, "log_bus" );
       Config.log_zmq            = Json_get_bool ( api_result, "log_zmq" );
       Config.log_trad           = Json_get_bool ( api_result, "log_trad" );
       Config.log_msrv           = Json_get_bool ( api_result, "log_msrv" );
       if (Json_has_member ( api_result, "is_master") )
            Config.instance_is_master = Json_get_bool ( api_result, "is_master" );
       else Config.instance_is_master = TRUE;
       Config.use_subdir         = Json_get_bool ( api_result, "use_subdir" );
       gchar *master_hostname    = Json_get_string ( api_result, "master_hostname" );
       if (master_hostname) g_snprintf( Config.master_hostname, sizeof(Config.master_hostname), "%s", master_hostname );
                       else g_snprintf( Config.master_hostname, sizeof(Config.master_hostname), "nomasterhost" );

       Info_change_log_level ( Config.log, Json_get_int ( api_result, "log_level" ) );
       json_node_unref ( api_result );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: API config loaded.", __func__ );
     }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Cannot get API config. Sleep 5s and stopping.", __func__ );
       sleep(5);
       exit(-1);
     }

/******************************************************* Drop privileges ******************************************************/
    if (Config.installed) { Drop_privileges(); }

    Lire_ligne_commande( argc, argv );                                            /* Lecture du fichier conf et des arguments */
    Print_config();
                                                                                      /* Verification de l'unicité du process */
    fd_lock = open( VERROU_SERVEUR, O_RDWR | O_CREAT | O_SYNC, 0640 );
    if (fd_lock<0)
     { printf( "Lock file creation failed: %s/%s\n", Config.home, VERROU_SERVEUR );
       exit(EXIT_ERREUR);
     }
    if (flock( fd_lock, LOCK_EX | LOCK_NB )<0)                                         /* Creation d'un verrou sur le fichier */
     { printf( "Cannot lock %s/%s. Probably another daemon is running : %s\n",
                Config.home, VERROU_SERVEUR, strerror(errno) );
       close(fd_lock);
       exit(EXIT_ERREUR);
     }
    fcntl(fd_lock, F_SETFD, FD_CLOEXEC );                                                           /* Set close on exec flag */
    g_snprintf( strpid, sizeof(strpid), "%d\n", getpid() );                                /* Enregistrement du pid au cas ou */
    if (write( fd_lock, strpid, strlen(strpid) )<0)
     { printf( "Cannot write PID on %s/%s (%s)\n", Config.home, VERROU_SERVEUR, strerror(errno) ); }

    Partage = NULL;                                                                                         /* Initialisation */
    Partage = Shm_init();                                                            /* Initialisation de la mémoire partagée */
    if (!Partage)
     { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "Shared memory failed to allocate" ); }
    else
     { pthread_mutexattr_t attr;                                                       /* Initialisation des mutex de synchro */
       memset( Partage, 0, sizeof(struct PARTAGE) );                                                 /* RAZ des bits internes */
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

/************************************* Création des zones de bits internes dynamiques *****************************************/
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

       sigfillset (&sig.sa_mask);                                                 /* Par défaut tous les signaux sont bloqués */
       pthread_sigmask( SIG_SETMASK, &sig.sa_mask, NULL );

       Update_database_schema();                                                    /* Update du schéma de Database si besoin */
       Charger_config_bit_interne ();                         /* Chargement des configurations des bits internes depuis la DB */

       Partage->zmq_ctx = zmq_ctx_new ();                                          /* Initialisation du context d'echange ZMQ */
       if (!Partage->zmq_ctx)
        { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Init ZMQ Context Failed (%s)", __func__, zmq_strerror(errno) ); }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Init ZMQ Context OK", __func__ ); }

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
       sig.sa_handler = Traitement_signaux;                                         /* Gestionnaire de traitement des signaux */
       sig.sa_flags = SA_RESTART;                         /* Voir Linux mag de novembre 2002 pour le flag anti cut read/write */
       sigaction( SIGALRM, &sig, NULL );                                                             /* Reinitialisation soft */
       sigaction( SIGUSR1, &sig, NULL );                                                   /* Reinitialisation DLS uniquement */
       sigaction( SIGUSR2, &sig, NULL );                                                   /* Reinitialisation DLS uniquement */
       sigaction( SIGINT,  &sig, NULL );                                                             /* Reinitialisation soft */
       sigaction( SIGTERM, &sig, NULL );
       sigaction( SIGABRT, &sig, NULL );
       sigaction( SIGPIPE, &sig, NULL );                                               /* Pour prevenir un segfault du client */
       sigfillset (&sig.sa_mask);                                                 /* Par défaut tous les signaux sont bloqués */
       sigdelset ( &sig.sa_mask, SIGALRM );
       sigdelset ( &sig.sa_mask, SIGUSR1 );
       sigdelset ( &sig.sa_mask, SIGUSR2 );
       sigdelset ( &sig.sa_mask, SIGINT  );
       sigdelset ( &sig.sa_mask, SIGTERM );
       sigdelset ( &sig.sa_mask, SIGABRT );
       sigdelset ( &sig.sa_mask, SIGPIPE );
       pthread_sigmask( SIG_SETMASK, &sig.sa_mask, NULL );

       timer.it_value.tv_sec = timer.it_interval.tv_sec = 0;                                    /* Tous les 100 millisecondes */
       timer.it_value.tv_usec = timer.it_interval.tv_usec = 100000;                                 /* = 10 fois par secondes */
       setitimer( ITIMER_REAL, &timer, NULL );                                                             /* Active le timer */

       pthread_join( TID, NULL );                                                       /* Attente fin de la boucle pere MSRV */
       zmq_ctx_term( Partage->zmq_ctx );
       zmq_ctx_destroy( Partage->zmq_ctx );
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
     }

    sigfillset (&sig.sa_mask);                                                    /* Par défaut tous les signaux sont bloqués */
    pthread_sigmask( SIG_SETMASK, &sig.sa_mask, NULL );
    close(fd_lock);                                           /* Fermeture du FileDescriptor correspondant au fichier de lock */

    if (Config.config) json_node_unref ( Config.config );
    Shm_stop( Partage );                                                                       /* Libération mémoire partagée */

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Stopped", __func__ );
    return(EXIT_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
