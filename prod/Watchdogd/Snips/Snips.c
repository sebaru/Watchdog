/******************************************************************************************************************************/
/* Watchdogd/Snips/Snips.c  Gestion des messages Snips de Watchdog 2.0                                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    14.03.2019 19:48:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Archive.c
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

 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/wait.h>
 #include <fcntl.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Snips.h"
 struct SNIPS_CONFIG Cfg_snips;
/******************************************************************************************************************************/
/* Snips_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                  */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gboolean Snips_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_snips.lib->Thread_debug = FALSE;                                                       /* Settings default parameters */
    Cfg_snips.enable            = FALSE;
    g_snprintf( Cfg_snips.snips_host, sizeof(Cfg_snips.snips_host), "localhost" );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_INFO, "%s: '%s' = %s", __func__, nom, valeur ); /* Print Config */
            if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_snips.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_snips.lib->Thread_debug = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "snips_host" ) )
        { g_snprintf( Cfg_snips.snips_host, sizeof(Cfg_snips.snips_host), "%s", valeur ); }
       else
        { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE,
                   "%s: Unknown Parameter '%s'(='%s') in Database", __func__, nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Snips_traiter_commande_vocale: appeller pour faire correspondre une action a une demande vocale                            */
/* Entrée : L'évènement                                                                                                       */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Snips_traiter_question_vocale ( const gchar *intent, const gchar *ai, const gchar *object, const gchar *room )
  { gchar texte[80], insert[32];

    g_snprintf ( texte, sizeof(texte), "%s,%s", intent, ai );
    if (object)
     { g_snprintf ( insert, sizeof(insert), ",%s", object );
       g_strlcat( texte, insert, sizeof(texte) );
     }
    if (room)
     { g_snprintf ( insert, sizeof(insert), ",%s", room );
       g_strlcat( texte, insert, sizeof(texte) );
     }
    Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: Sending %s", __func__, texte );
    Send_zmq_with_tag ( Cfg_snips.zmq_to_master, NULL, NOM_THREAD, "*", "msrv", "SNIPS_QUESTION", texte, strlen(texte)+1 );
  }
/******************************************************************************************************************************/
/* Snips_traiter_commande_vocale: appeller pour faire correspondre une action a une demande vocale                            */
/* Entrée : L'évènement                                                                                                       */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Snips_traiter_commande_vocale ( const gchar *intent, const gchar *verbe, const gchar *object, const gchar *room )
  { struct DB *db;
    gchar texte[80], insert[32];

    g_snprintf ( texte, sizeof(texte), "%s,%s", intent, verbe );
    if (object)
     { g_snprintf ( insert, sizeof(insert), ",%s", object );
       g_strlcat( texte, insert, sizeof(texte) );
     }
    if (room)
     { g_snprintf ( insert, sizeof(insert), ",%s", room );
       g_strlcat( texte, insert, sizeof(texte) );
     }

    Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: Searching for %s", __func__, texte );
    if ( ! Recuperer_mnemos_DI_by_text ( &db, NOM_THREAD, texte ) )
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_ERR, "%s: Error searching Database for '%s'", __func__, texte ); }
    else while ( Recuperer_mnemos_DI_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *libelle = db->row[3], *src_text = db->row[2];
       Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_INFO, "%s: Match found '%s' '%s:%s' - %s", __func__,
                 src_text, tech_id, acro, libelle );

       if (Config.instance_is_master==TRUE)                                                       /* si l'instance est Maitre */
        { Envoyer_commande_dls_data ( tech_id, acro ); }
       else /* Envoi au master via thread HTTP */
        { JsonBuilder *builder;
          gchar *result;
          gsize taille;
          builder = Json_create ();
          json_builder_begin_object ( builder );
          Json_add_string ( builder, "tech_id", tech_id );
          Json_add_string ( builder, "acronyme", acro );
          Json_add_bool   ( builder, "etat", TRUE );
          json_builder_end_object ( builder );
          result = Json_get_buf ( builder, &taille );
          Send_zmq_with_tag ( Cfg_snips.zmq_to_master, NULL, NOM_THREAD, "*", "msrv", "SET_CDE", result, taille );
          g_free(result);
        }
     }
  }
/******************************************************************************************************************************/
/* Snips_message_CB: appeller par la librairie snips lorsque qu'un evenement Vocal est reconnu                                */
/* Entrée : L'évènement                                                                                                       */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Snips_message_CB(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
  { const gchar *targetVerbe, *targetObject, *targetRoom, *targetAI, *targetMode;
    const gchar *slotValue, *slotName;
    const gchar *intent;
    JsonArray *slotArray;
    JsonObject *object;
    JsonNode *Query;
    gint nbr_slots, i;

	   if(!message->payloadlen)
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: Message recu: %s - NoPayload", __func__,
                 message->topic );
       return;
     }

    Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_DEBUG, "%s: Message recu: %s - %s", __func__,
              message->topic, message->payload );
    Query = json_from_string ( message->payload, NULL );
    if (!Query)
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_ERR, "%s: requete non Json", __func__ );
       return;                                                                                                 /* Bad Request */
     }

    object = json_node_get_object (Query);
    if (!object)
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_ERR, "%s: Object non trouvé", __func__ );
       json_node_unref (Query);
       return;                                                                                                 /* Bad Request */
     }

    intent = json_object_get_string_member( json_object_get_object_member ( object, "intent" ), "intentName" );
    if (!intent)
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_ERR, "%s: intent non trouvé", __func__ );
       json_node_unref (Query);
       return;                                                                                                 /* Bad Request */
     }

    intent = g_strrstr ( intent, ":" ) + 1;
    Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: Message recu from topic %s intent %s",
              __func__, message->topic, intent );

    slotArray = json_object_get_array_member ( object, "slots" );
    if (!slotArray)
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: 0 slot array", __func__ );
       json_node_unref (Query);
       return;
     }

    nbr_slots = json_array_get_length (slotArray);
    if (nbr_slots==0)
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: 0 slots", __func__ );
       json_node_unref (Query);
       return;
     }

    if (nbr_slots>3)
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: %d slots -> Too many slots", __func__, nbr_slots );
       json_node_unref (Query);
       return;
     }

    targetAI = targetVerbe = targetObject = targetRoom = NULL;
    for( i=0;i<nbr_slots; i++)
     { JsonObject *slot = json_array_get_object_element ( slotArray, i );
       slotValue = json_object_get_string_member( json_object_get_object_member ( slot, "value" ), "value" );
       slotName = json_object_get_string_member( slot, "slotName" );
       Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_DEBUG,
                 "%s: slot %d/%d trouvé: %s - %s", __func__, i+1, nbr_slots, slotName, slotValue );
       if (!strcmp(slotName,"targetAI")) targetAI = slotValue;
       else if (!strcmp(slotName,"targetVerbe")) targetVerbe = slotValue;
       else if (!strcmp(slotName,"targetObject")) targetObject = slotValue;
       else if (!strcmp(slotName,"targetRoom"))   targetRoom   = slotValue;
       else if (!strcmp(slotName,"targetMode"))   targetMode   = slotValue;
       else Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_WARNING,
                     "%s: slot unknown: %s - %s", __func__, slotName, slotValue );
     }
    if (!strcmp(intent,"ACTION"))
     { Snips_traiter_commande_vocale ( intent, targetVerbe, targetObject, targetRoom ); }
    if (!strcmp(intent,"MODE"))
     { Snips_traiter_commande_vocale ( intent, targetMode, NULL, NULL ); }
    else if (!strcmp(intent,"QUESTION"))
     { Snips_traiter_question_vocale ( intent, targetAI, targetObject, targetRoom ); }
    json_node_unref (Query);
    Cfg_snips.nbr_msg_recu++;
 	}
/******************************************************************************************************************************/
/* Snips_connect_CB: appeller par la librairie snips lors d'un evenement de connect                                           */
/* Entrée : L'évènement                                                                                                       */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Snips_connect_CB(struct mosquitto *mosq, void *userdata, int texte)
  {	if(!texte)
     {	/* Subscribe to broker information topics on successful connect. */
		     mosquitto_subscribe(mosq, NULL, "hermes/intent/#", 2);
       Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: Connect OK. subscribing...", __func__ );
	    }
    else
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: Connect Error/ Result=%d.", __func__, texte ); }
  }
/******************************************************************************************************************************/
/* Snips_subscribe_CB: appeller par la librairie snips lors d'un evenement de subscribe arrive                                */
/* Entrée : L'évènement                                                                                                       */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Snips_subscribe_CB(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos)
  {
    Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: Subscribe ok for '%d'.", __func__, mid );
  }
/******************************************************************************************************************************/
/* Snips_log_CB: appeller par la librairie snips lors d'un evenement de log                                                   */
/* Entrée : L'évènement                                                                                                       */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Snips_log_CB(struct mosquitto *mosq, void *userdata, int level, const char *str)
  { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_DEBUG, "%s: Level %d -> '%s'", __func__, level, str );
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du Thread Snips                                                                                  */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct ZMQUEUE *zmq_from_bus;
   	struct mosquitto *mosq = NULL;

    prctl(PR_SET_NAME, "W-SNIPS", 0, 0, 0 );
reload:
    memset( &Cfg_snips, 0, sizeof(Cfg_snips) );                                     /* Mise a zero de la structure de travail */
    Cfg_snips.lib = lib;                                           /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_snips.lib->TID = pthread_self();                                                    /* Sauvegarde du TID pour le pere */
    Snips_Lire_config ();                                                   /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE,
              "%s: Demarrage %s . . . TID = %p", __func__, VERSION, pthread_self() );
    Cfg_snips.lib->Thread_run = TRUE;                                                                   /* Le thread tourne ! */

    g_snprintf( Cfg_snips.lib->admin_prompt, sizeof(Cfg_snips.lib->admin_prompt), "snips" );
    g_snprintf( Cfg_snips.lib->admin_help,   sizeof(Cfg_snips.lib->admin_help),   "Manage Snips system" );

    if (!Cfg_snips.enable)
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE,
                "%s: Thread is not enabled in config. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }

    zmq_from_bus            = Connect_zmq ( ZMQ_SUB, "listen-to-bus", "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    Cfg_snips.zmq_to_master = Connect_zmq ( ZMQ_PUB, "pub-to-master", "inproc", ZMQUEUE_LOCAL_MASTER, 0 );

	   mosquitto_lib_init();
 	  mosq = mosquitto_new(NULL, TRUE, NULL);
	   if(!mosq)
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE,
                "%s: Unable to create Mosquitto MQTT. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }
   	mosquitto_log_callback_set(mosq, Snips_log_CB);
	   mosquitto_connect_callback_set(mosq, Snips_connect_CB);
	   mosquitto_message_callback_set(mosq, Snips_message_CB);
	   mosquitto_subscribe_callback_set(mosq, Snips_subscribe_CB);

   	if( mosquitto_connect(mosq, Cfg_snips.snips_host, 1883, 60) )
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE,
                "%s: Unable to connect to MQTT local Queue. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }

    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { struct ZMQ_TARGET *event;
       gchar buffer[256];
       void *payload;

       mosquitto_loop(mosq, 1000, 1);

       if (Recv_zmq_with_tag ( zmq_from_bus, NOM_THREAD, &buffer, sizeof(buffer), &event, &payload ) > 0) /* Reception d'un paquet master ? */
        { /*if ( !strcmp( event->tag, "play_snips" ) )
           { gchar snips[80];
             Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_DEBUG,
                      "%s : Reception d'un message PLAY SNIPS : %s", __func__, (gchar *)payload );
             Jouer_snips ( (gchar *)payload );
           } else
          if ( !strcmp( event->tag, "stop_snips" ) )
           { Stopper_snips(); }*/
        }
       if ( !Partage->top%3000) Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_INFO, "%s: Waiting for Intents", __func__ );
       usleep(1000);
     }
    Close_zmq ( zmq_from_bus );
    Close_zmq ( Cfg_snips.zmq_to_master );
end:
   	mosquitto_destroy(mosq);
   	mosquitto_lib_cleanup();
    Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    if (lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Cfg_snips.lib->Thread_run = FALSE;                                                          /* Le thread ne tourne plus ! */
    Cfg_snips.lib->TID = 0;                                                   /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
