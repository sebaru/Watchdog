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
 #include <locale.h>
 #include <pwd.h>
 #include <gcrypt.h>                                                      /* Pour assurer le multithreading avec IMSG et HTTP */
 #include <curl/curl.h>                                                          /* Pour le multithreading avec Master et SMS */

 #include "watchdogd.h"

 struct CONFIG Config;                                       /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 struct PARTAGE *Partage;                                                        /* Accès aux données partagées des processes */

/******************************************************************************************************************************/
/* Traitement_signaux: Gestion des signaux de controle du systeme                                                             */
/* Entrée: numero du signal à gerer                                                                                           */
/******************************************************************************************************************************/
 static void Traitement_signaux( int num )
  { static gpointer dls_wait, dls_tour_per_sec, dls_bit_per_sec;
    char chaine[50];
    if (num == SIGALRM)
     { Partage->top++;
       if (Partage->com_msrv.Thread_run != TRUE) return;

       if (!Partage->top)                                             /* Si on passe par zero, on le dit (DEBUG interference) */
        { Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Timer: Partage->top = 0 !!", __func__ ); }
       if (!(Partage->top%10))                                                                  /* Cligno toutes les secondes */
        { Partage->audit_bit_interne_per_sec_hold += Partage->audit_bit_interne_per_sec;
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
/* Charger_config_bit_interne: Chargement des configs bit interne depuis la base de données                                   */
/* Entrée: néant                                                                                                              */
/******************************************************************************************************************************/
 void Charger_config_bit_interne( void )
  { if (Config.instance_is_master == FALSE) return;                                /* Seul le master sauvegarde les compteurs */
    Charger_confDB_Registre();
    /*Charger_confDB_AO(NULL, NULL);*/
    Charger_confDB_AI(NULL, NULL);
    Charger_confDB_MSG();
    Charger_confDB_BOOL();
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
    Updater_confDB_CH();                                                                  /* Sauvegarde des compteurs Horaire */
    Updater_confDB_CI();                                                              /* Sauvegarde des compteurs d'impulsion */
    Updater_confDB_AI();                                                              /* Sauvegarde des compteurs d'impulsion */
    Updater_confDB_Registre();                                                                    /* Sauvegarde des registres */
    Updater_confDB_MSG();                                                              /* Sauvegarde des valeurs des messages */
    Updater_confDB_BOOL();                                             /* Sauvegarde des valeurs des bistables et monostables */
  }
/******************************************************************************************************************************/
/* Handle_zmq_message_for_master: Analyse et reagi à un message ZMQ a destination du MSRV                   */
/* Entrée: le message                                       */
/* Sortie: rien                                             */
/******************************************************************************************************************************/
 static void Handle_zmq_message_for_master ( struct ZMQ_TARGET *event, gchar *payload )
  { if ( !strcmp(event->tag,"SET_AI") )
     { JsonNode *query;
       query = Json_get_from_string ( payload );
       if (!query)
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: requete non Json", __func__ ); return; }
       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                 "%s: receive SET_AI from %s/%s to %s/%s : '%s:%s'=%f (range=%d)", __func__,
                 event->src_instance, event->src_thread, event->dst_instance, event->dst_thread,
                 Json_get_string ( query, "tech_id" ), Json_get_string ( query, "acronyme" ),
                 Json_get_float ( query, "valeur" ), Json_get_bool ( query, "in_range" ) );
       Dls_data_set_AI ( Json_get_string ( query, "tech_id" ), Json_get_string ( query, "acronyme" ), NULL,
                         Json_get_float ( query, "valeur" ), Json_get_bool ( query, "in_range" ) );
       json_node_unref (query);
     }
    else if ( !strcmp(event->tag,"SET_CDE") )
     { JsonNode *query;
       query = Json_get_from_string ( payload );
       if (!query)
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: requete non Json", __func__ ); return; }

       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                 "%s: receive SET_CDE=1 from %s/%s to %s/%s : bit techid %s acronyme %s", __func__,
                 event->src_instance, event->src_thread, event->dst_instance, event->dst_thread,
                 Json_get_string ( query, "tech_id" ), Json_get_string ( query, "acronyme" ) );
       Envoyer_commande_dls_data ( Json_get_string ( query, "tech_id" ), Json_get_string ( query, "acronyme" ) );
       json_node_unref (query);
     }
    else if ( !strcmp(event->tag,"SET_DI") )
     { JsonNode *query;
       query = Json_get_from_string ( payload );
       if (!query)
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: requete non Json", __func__ ); return; }

       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                 "%s: receive SET_DI from %s/%s to %s/%s : '%s:%s'=%d", __func__,
                 event->src_instance, event->src_thread, event->dst_instance, event->dst_thread,
                 Json_get_string ( query, "tech_id" ), Json_get_string ( query, "acronyme" ), Json_get_bool ( query, "etat" ) );
       Dls_data_set_DI ( NULL, Json_get_string ( query, "tech_id" ), Json_get_string ( query, "acronyme" ),
                         NULL, Json_get_bool ( query, "etat" ) );
       json_node_unref (query);
     }
    else if ( !strcmp(event->tag, "ping") )
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: receive PING from %s/%s to %s/%s",
                 __func__, event->src_instance, event->src_thread, event->dst_instance, event->dst_thread );
     }
    else if ( !strcmp(event->tag, "SLAVE_STOP") )
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: SLAVE '%s' stopped !", __func__, event->src_instance); }
    else if ( !strcmp(event->tag, "SLAVE_START") )
     { struct DLS_AO *ao;
       GSList *liste;
       Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: SLAVE '%s' started. Sending AO !", __func__, event->src_instance);
       liste = Partage->Dls_data_AO;
       while (liste)
        { ao = (struct DLS_AO *)Partage->com_msrv.Liste_AO->data;            /* Recuperation du numero de a */
          JsonBuilder *builder = Json_create ();
          if (builder)
           { gsize taille_buf;
             Dls_AO_to_json( builder, ao );
             gchar *buf = Json_get_buf ( builder, &taille_buf );
              if(buf)
               { Send_zmq_with_tag ( Partage->com_msrv.zmq_to_bus,   NULL, "msrv", event->src_instance, "*", "SET_AO", buf, taille_buf );
                 Send_zmq_with_tag ( Partage->com_msrv.zmq_to_slave, NULL, "msrv", event->src_instance, "*", "SET_AO", buf, taille_buf );
                 g_free(buf);
               }
           }
          liste = g_slist_next(liste);
        }
     }
    else if ( !strcmp(event->tag, "SNIPS_QUESTION") )
     { struct DB *db;
       Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: receive SNIPS_QUESTION from %s/%s to %s/%s : '%s'",
                 __func__, event->src_instance, event->src_thread, event->dst_instance, event->dst_thread, payload );

       if ( ! Recuperer_mnemos_AI_by_map_question_vocale ( &db, (gchar *)payload ) )
        { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Error searching Database for '%s'", __func__, payload ); }
       else while ( Recuperer_mnemos_AI_suite( &db ) )
        { gchar *tech_id = db->row[0], *acronyme = db->row[1], *libelle = db->row[2];
          gchar *map_question_vocale = db->row[3], *map_reponse_vocale = db->row[4];
          gchar *result_string;
          gpointer ai_p=NULL;
          Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Match found '%s' '%s:%s' - %s - %s", __func__,
                    map_question_vocale, tech_id, acronyme, libelle, map_reponse_vocale );
          result_string = Dls_dyn_string ( map_reponse_vocale, MNEMO_ENTREE_ANA, tech_id, acronyme, &ai_p );
          Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Sending %s:audio:play_google:'%s'", __func__,
                    event->src_instance, result_string );
          Send_zmq_with_tag ( Partage->com_msrv.zmq_to_bus, NULL, "msrv", event->src_instance,
                              "audio", "play_google", result_string, strlen(result_string)+1 );
          Send_zmq_with_tag ( Partage->com_msrv.zmq_to_slave, NULL, "msrv", event->src_instance,
                              "audio", "play_google", result_string, strlen(result_string)+1 );
          g_free(result_string);
        }

       if ( ! Recuperer_mnemos_R_by_map_question_vocale ( &db, (gchar *)payload ) )
        { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Error searching Database for '%s'", __func__, payload ); }
       else while ( Recuperer_mnemos_R_suite( &db ) )
        { gchar *tech_id = db->row[0], *acronyme = db->row[1], *libelle = db->row[2];
          gchar *map_question_vocale = db->row[3], *map_reponse_vocale = db->row[4];
          gchar *result_string;
          gpointer reg_p=NULL;
          Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Match found '%s' '%s:%s' - %s - %s", __func__,
                    map_question_vocale, tech_id, acronyme, libelle, map_reponse_vocale );
          result_string = Dls_dyn_string ( map_reponse_vocale, MNEMO_REGISTRE, tech_id, acronyme, &reg_p );
          Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Sending %s:audio:play_google:'%s'", __func__,
                    event->src_instance, result_string );
          Send_zmq_with_tag ( Partage->com_msrv.zmq_to_bus, NULL, "msrv", event->src_instance,
                              "audio", "play_google", result_string, strlen(result_string)+1 );
          Send_zmq_with_tag ( Partage->com_msrv.zmq_to_slave, NULL, "msrv", event->src_instance,
                              "audio", "play_google", result_string, strlen(result_string)+1 );
          g_free(result_string);
        }
     }
    else if ( !strcmp(event->tag, "sudo") )
     { gchar chaine[80];
       Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: receive SUDO from %s/%s to %s/%s/%s",
           __func__, event->src_instance, event->src_thread, event->dst_instance, event->dst_thread, payload );
       g_snprintf( chaine, sizeof(chaine), "sudo -n %s", (gchar *)payload );
       system(chaine);
     }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: receive UNKNOWN from %s/%s to %s/%s/%s",
           __func__, event->src_instance, event->src_thread, event->dst_instance, event->dst_thread, payload );
     }
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

/************************************************* Socket ZMQ interne *********************************************************/
    Partage->com_msrv.zmq_msg = Bind_zmq ( ZMQ_PUB, "pub-int-msgs", "inproc", ZMQUEUE_LIVE_MSGS, 0 );
    Partage->com_msrv.zmq_motif = Bind_zmq ( ZMQ_PUB, "pub-int-motifs", "inproc", ZMQUEUE_LIVE_MOTIFS, 0 );
    Partage->com_msrv.zmq_to_bus = Bind_zmq ( ZMQ_PUB, "pub-to-bus", "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    zmq_from_bus = Bind_zmq ( ZMQ_SUB, "listen-to-bus", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

/***************************************** Socket pour une instance master ****************************************************/
    Partage->com_msrv.zmq_to_slave = Bind_zmq ( ZMQ_PUB, "pub-to-slave", "tcp", "*", 5555 );
    zmq_from_slave = Bind_zmq ( ZMQ_SUB, "listen-to-slave", "tcp", "*", 5556 );

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

    if (!Config.installed) Charger_librairie_par_prompt ("http");/* Charge uniquement le module HTTP si instance pas installée*/

/***************************************** Debut de la boucle sans fin ********************************************************/
    cpt_5_minutes = Partage->top + 3000;
    cpt_1_minute  = Partage->top + 600;

    sleep(1);
    Partage->com_msrv.Thread_run = TRUE;                                             /* On dit au maitre que le thread tourne */
    while(Partage->com_msrv.Thread_run == TRUE)                                           /* On tourne tant que l'on a besoin */
     { struct ZMQ_TARGET *event;
       gchar buffer[2048];
       void *payload;
       gint byte;

       Gerer_arrive_MSGxxx_dls();                                 /* Redistrib des messages DLS vers les clients + Historique */
       Gerer_arrive_Ixxx_dls();                                                 /* Distribution des changements d'etats motif */
       Gerer_arrive_Axxx_dls();                                           /* Distribution des changements d'etats sorties TOR */

       if ( (byte=Recv_zmq_with_tag( zmq_from_slave, "msrv", &buffer, sizeof(buffer)-1, &event, &payload )) > 0 )
        { Handle_zmq_message_for_master( event, payload ); }

       if ( (byte=Recv_zmq_with_tag( zmq_from_bus, NULL, &buffer, sizeof(buffer)-1, &event, &payload )) > 0 )
        { if (!strcmp(event->dst_thread, "msrv"))
           { Handle_zmq_message_for_master( event, payload ); }
          else
           { Send_zmq ( Partage->com_msrv.zmq_to_bus, buffer, byte );                           /* Sinon on envoi aux threads */
             Send_zmq ( Partage->com_msrv.zmq_to_slave, buffer, byte );                           /* Sinon on envoi aux slave */
           }
        }

       if (cpt_5_minutes < Partage->top)                                                    /* Update DB toutes les 5 minutes */
        { Send_zmq_with_tag ( Partage->com_msrv.zmq_to_slave, NULL, "msrv", "*", "msrv", "ping", NULL, 0 );
          Save_dls_data_to_DB();
          cpt_5_minutes += 3000;                                                           /* Sauvegarde toutes les 5 minutes */
        }

       if (cpt_1_minute < Partage->top)                                                       /* Update DB toutes les minutes */
        { Print_SQL_status();                                                             /* Print SQL status for debugging ! */
          Activer_horlogeDB();
          cpt_1_minute += 600;                                                               /* Sauvegarde toutes les minutes */
        }

       usleep(1000);
       sched_yield();
     }

/*********************************** Terminaison: Deconnexion DB et kill des serveurs *****************************************/
    Save_dls_data_to_DB();                                                                 /* Dernière sauvegarde avant arret */
    Decharger_librairies();                                                   /* Déchargement de toutes les librairies filles */
    Stopper_fils();                                                                        /* Arret de tous les fils watchdog */
    Close_zmq ( Partage->com_msrv.zmq_msg );
    Close_zmq ( Partage->com_msrv.zmq_motif );
    Close_zmq ( Partage->com_msrv.zmq_to_bus );
    Close_zmq ( zmq_from_bus );
    Close_zmq ( Partage->com_msrv.zmq_to_slave );
    Close_zmq ( zmq_from_slave );

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: fin boucle sans fin", __func__ );
    pthread_exit( NULL );
  }
/******************************************************************************************************************************/
/* Boucle_pere: boucle de controle du pere de tous les serveurs                                                               */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void *Boucle_pere_slave ( void )
  { gint cpt_5_minutes, cpt_1_minute;
    struct ZMQUEUE *zmq_from_master, *zmq_from_bus;

    prctl(PR_SET_NAME, "W-SLAVE", 0, 0, 0 );
    Modifier_configDB ( "msrv", "thread_version", WTD_VERSION );

    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Debut boucle sans fin", __func__ );

/************************************************* Socket ZMQ interne *********************************************************/
    Partage->com_msrv.zmq_msg    = Bind_zmq ( ZMQ_PUB, "pub-int-msgs",  "inproc", ZMQUEUE_LIVE_MSGS, 0 );
    Partage->com_msrv.zmq_to_bus = Bind_zmq ( ZMQ_PUB, "pub-to-bus",    "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    zmq_from_bus                 = Bind_zmq ( ZMQ_SUB, "listen-to-bus", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

/***************************************** Socket de subscription au master ***************************************************/
    Partage->com_msrv.zmq_to_master = Connect_zmq ( ZMQ_PUB, "pub-to-master",    "tcp", Config.master_host, 5556 );
    if (!Partage->com_msrv.zmq_to_master) goto end;
    zmq_from_master                 = Connect_zmq ( ZMQ_SUB, "listen-to-master", "tcp", Config.master_host, 5555 );
    if (!zmq_from_master) goto end;

/***************************************** Demarrage des threads builtin et librairies ****************************************/
    if (Config.single == FALSE)                                                                    /* Si demarrage des thread */
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Arch Thread is administratively DOWN", __func__ );
       Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: D.L.S Thread is administratively DOWN", __func__ );

       Charger_librairies();                                                  /* Chargement de toutes les librairies Watchdog */
     }
    else
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: NOT starting threads (single mode=true)", __func__ ); }

/***************************************** Debut de la boucle sans fin ********************************************************/
    cpt_5_minutes = Partage->top + 3000;
    cpt_1_minute  = Partage->top + 600;

    sleep(1);
    Partage->com_msrv.Thread_run = TRUE;                                             /* On dit au maitre que le thread tourne */
    Send_zmq_with_tag ( Partage->com_msrv.zmq_to_master, NULL, "msrv", "*", "msrv", "SLAVE_START", NULL, 0 );
    while(Partage->com_msrv.Thread_run == TRUE)                                           /* On tourne tant que l'on a besoin */
     { struct ZMQ_TARGET *event;                                                    /* Instance is slave, listening to master */
       gchar buffer[2048];
       void *payload;
       gint byte;
       if ( (byte=Recv_zmq_with_tag( zmq_from_master, NULL, &buffer, sizeof(buffer), &event, &payload )) > 0 )
        { if (!strcmp(event->dst_thread,"msrv") && !strcmp(event->tag,"histo"))
           { if (Send_zmq( Partage->com_msrv.zmq_msg, payload, sizeof(struct CMD_TYPE_HISTO)) == -1)
              { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Send to ZMQ '%s' socket failed (%s)",
                          __func__, Partage->com_msrv.zmq_msg->name, zmq_strerror(errno) );
              }
           } else
          if (!strcmp(event->dst_thread,"msrv") && !strcmp(event->tag, "sudo") )
           { gchar chaine[80];
             Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: receive SUDO from %s/%s to %s/%s/%s",
                       __func__, event->src_instance, event->src_thread, event->dst_instance, event->dst_thread, payload );
             g_snprintf( chaine, sizeof(chaine), "sudo -n %s", (gchar *)payload );
             system(chaine);
           } else
          if ( !strcmp(event->tag, "ping") )
           { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: receive PING from %s/%s to %s/%s",
                       __func__, event->src_instance, event->src_thread, event->dst_instance, event->dst_thread );
           } else
           { Send_zmq( Partage->com_msrv.zmq_to_bus, buffer, byte ); }
        }
                                                /* Si reception depuis un thread, report vers le master et les autres threads */
       if ( (byte=Recv_zmq( zmq_from_bus, &buffer, sizeof(buffer) )) > 0 )
        { Send_zmq ( Partage->com_msrv.zmq_to_bus, buffer, byte );
          Send_zmq ( Partage->com_msrv.zmq_to_master, buffer, byte );
        }

       if (cpt_5_minutes < Partage->top)                                                    /* Update DB toutes les 5 minutes */
        { Send_zmq_with_tag ( Partage->com_msrv.zmq_to_master, NULL, "msrv", Config.master_host, "msrv", "ping", NULL, 0 );
          cpt_5_minutes += 3000;                                                           /* Sauvegarde toutes les 5 minutes */
        }

       if (cpt_1_minute < Partage->top)                                                       /* Update DB toutes les minutes */
        { Print_SQL_status();                                                             /* Print SQL status for debugging ! */
          cpt_1_minute += 600;                                                               /* Sauvegarde toutes les minutes */
        }

       usleep(1000);
       sched_yield();
     }

/*********************************** Terminaison: Deconnexion DB et kill des serveurs *****************************************/
    Send_zmq_with_tag ( Partage->com_msrv.zmq_to_master, NULL, "msrv", "*", "msrv", "SLAVE_STOP", NULL, 0 );
end:
    Decharger_librairies();                                                   /* Déchargement de toutes les librairies filles */
    Stopper_fils();                                                                        /* Arret de tous les fils watchdog */
    Close_zmq ( Partage->com_msrv.zmq_msg );
    Close_zmq ( Partage->com_msrv.zmq_to_bus );
    Close_zmq ( zmq_from_bus );
    Close_zmq( Partage->com_msrv.zmq_to_master );
    Close_zmq( zmq_from_master );

/********************************* Dechargement des zones de bits internes dynamiques *****************************************/
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: fin boucle sans fin", __func__ );
    pthread_exit( NULL );
  }
/******************************************************************************************************************************/
/* Lire_ligne_commande: Parse la ligne de commande pour d'eventuels parametres                                                */
/* Entrée: argc, argv                                                                                                         */
/* Sortie: -1 si erreur, 0 si ok                                                                                              */
/******************************************************************************************************************************/
 static gboolean Lire_ligne_commande( int argc, char *argv[] )
  { gint help = 0, log_level = -1, fg = 0, single = 0, version = 0;
    struct poptOption Options[]=
     { { "foreground", 'f', POPT_ARG_NONE,
         &fg,               0, "Run in foreground", NULL },
       { "version",    'v', POPT_ARG_NONE,
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
    return(fg);
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

       if (setgid ( pwd->pw_gid )==-1)                                                              /* On drop les privilèges */
        { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Error, cannot setGID for user '%s' (%s)\n",
                    __func__, Config.run_as, strerror(errno) );
          exit(EXIT_ERREUR);
        }

       if (setuid ( pwd->pw_uid )==-1)                                                              /* On drop les privilèges */
        { Info_new( Config.log, Config.log_msrv, LOG_CRIT, "%s: Error, cannot setUID for user '%s' (%s)\n",
                    __func__, Config.run_as, strerror(errno) );
          exit(EXIT_ERREUR);
        }
     }
    g_snprintf(Config.home, sizeof(Config.home), "%s", pwd->pw_dir );
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
    gboolean fg;

    umask(022);                                                                              /* Masque de creation de fichier */

    Config.installed = Lire_config();                                      /* Lecture sur le fichier /etc/watchdogd.abls.conf */
    Config.log = Info_init( "Watchdogd", Config.log_level );                                           /* Init msgs d'erreurs */
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "Start v%s", WTD_VERSION );

    if (Config.installed) { Drop_privileges(); }

    fg = Lire_ligne_commande( argc, argv );                                       /* Lecture du fichier conf et des arguments */

    if (fg == FALSE)                                                                       /* On tourne en tant que daemon ?? */
     { gint pid;
       pid = fork();
       if (pid<0) { printf("Fork 1 failed\n"); exit(EXIT_ERREUR); }                                           /* On daemonize */
       if (pid>0) exit(EXIT_OK);                                                                           /* On kill le père */

       pid = fork();
       if (pid<0) { printf("Fork 2 failed\n"); exit(EXIT_ERREUR); }                         /* Evite des pb (linuxmag 44 p78) */
       if (pid>0) exit(EXIT_OK);                                                                           /* On kill le père */

       setsid();                                                                                 /* Indépendance du processus */
     }
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


    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    gcry_check_version(NULL);                                                        /* Initialisation de la librairie GCRYPT */
    curl_global_init (CURL_GLOBAL_ALL);                                                 /* Initialisation de la libraire CURL */
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
       pthread_mutex_init( &Partage->com_dls.synchro, &attr );
       pthread_mutex_init( &Partage->com_dls.synchro_traduction, &attr );
       pthread_mutex_init( &Partage->com_dls.synchro_data, &attr );
       pthread_mutex_init( &Partage->com_arch.synchro, &attr );
       pthread_mutex_init( &Partage->com_db.synchro, &attr );

/************************************************** Check Database Access *****************************************************/
       if (Config.installed)
        { struct DB *db = Init_DB_SQL();
          if (db)
           { Libere_DB_SQL ( &db );
             Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Connection to DB OK.", __func__ );
           }
          else
           { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                       "%s: Connection to DB failed. Wait 5 sec and stopping.", __func__ );
             sleep(5);
             _exit(EXIT_FAILURE);
           }
        }

       gchar *use_subdir = Recuperer_configDB_by_nom ( "msrv", "use_subdir" );
       if (use_subdir)
        { if (!strcasecmp(use_subdir,"true"))
           { g_strlcat (Config.home, "/.watchdog", sizeof(Config.home));
             chdir(Config.home);
           }
          g_free(use_subdir);
        }

       gchar *is_master = Recuperer_configDB_by_nom ( "msrv", "instance_is_master" );
       if (is_master)
        { if (!strcasecmp(is_master,"true")) { Config.instance_is_master = TRUE; }
                                       else  { Config.instance_is_master = FALSE; }
          g_free(is_master);
        }

       gchar *master_host = Recuperer_configDB_by_nom ( "msrv", "master_host" );
       if (master_host)
        { g_snprintf( Config.master_host, sizeof(Config.master_host), "%s", master_host );
          g_free(master_host);
        }

       Print_config();
/************************************* Création des zones de bits internes dynamiques *****************************************/
       Partage->Dls_data_DI     = NULL;
       Partage->Dls_data_DO     = NULL;
       Partage->Dls_data_AI     = NULL;
       Partage->Dls_data_AO     = NULL;
       Partage->Dls_data_BOOL   = NULL;
       Partage->Dls_data_REGISTRE = NULL;
       Partage->Dls_data_MSG    = NULL;
       Partage->Dls_data_CH     = NULL;
       Partage->Dls_data_CI     = NULL;
       Partage->Dls_data_TEMPO  = NULL;
       Partage->Dls_data_VISUEL = NULL;

       sigfillset (&sig.sa_mask);                                                 /* Par défaut tous les signaux sont bloqués */
       pthread_sigmask( SIG_SETMASK, &sig.sa_mask, NULL );

       Update_database_schema();                                                    /* Update du schéma de Database si besoin */
       Charger_config_bit_interne ();                         /* Chargement des configurations des bits internes depuis la DB */
       Modifier_configDB ( "msrv", "thread_version", WTD_VERSION );                      /* Update du champs instance_version */

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
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Libération mémoire dynamique BOOL", __func__ );
       g_slist_foreach (Partage->Dls_data_BOOL, (GFunc) g_free, NULL );
       g_slist_free (Partage->Dls_data_BOOL);
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

       pthread_mutex_destroy( &Partage->com_msrv.synchro );
       pthread_mutex_destroy( &Partage->com_dls.synchro );
       pthread_mutex_destroy( &Partage->com_dls.synchro_traduction );
       pthread_mutex_destroy( &Partage->com_dls.synchro_data );
       pthread_mutex_destroy( &Partage->com_arch.synchro );
       pthread_mutex_destroy( &Partage->com_db.synchro );
     }

    sigfillset (&sig.sa_mask);                                                    /* Par défaut tous les signaux sont bloqués */
    pthread_sigmask( SIG_SETMASK, &sig.sa_mask, NULL );
    curl_global_cleanup();
    close(fd_lock);                                           /* Fermeture du FileDescriptor correspondant au fichier de lock */

    Shm_stop( Partage );                                                                       /* Libération mémoire partagée */

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Stopped", __func__ );
    return(EXIT_OK);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
