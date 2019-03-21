/******************************************************************************************************************************/
/* Watchdogd/Snips/Snips.c  Gestion des messages Snips de Watchdog 2.0                                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    14.03.2019 19:48:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Archive.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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
    g_snprintf( Cfg_snips.snips_host, sizeof(Cfg_snips.snips_host), "localhost", valeur );

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
 static void Snips_traiter_commande_vocale ( gchar *texte )
  { struct CMD_TYPE_MNEMO_BASE *mnemo;
    gchar chaine[160];
    struct DB *db;

    if ( ! Recuperer_mnemo_baseDB_by_event_text ( &db, NOM_THREAD, texte ) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Error searching Database for '%s'", __func__, texte );
       return;
     }
          
    if ( db->nbr_result == 0 )                                                              /* Si pas d'enregistrement trouvé */
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: No match found for '%s'", __func__, texte );
       Libere_DB_SQL ( &db );
     }
    else if (db->nbr_result > 1)
     { g_snprintf(chaine, sizeof(chaine), "Too many events found for '%s'", texte );             /* Envoi de l'erreur si trop */
       Libere_DB_SQL ( &db );
     }
    else
     { while ( (mnemo = Recuperer_mnemo_baseDB_suite( &db )) != NULL)
        { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Match found for '%s' Type %d Num %d - %s", __func__,
                    texte, mnemo->type, mnemo->num, mnemo->libelle );

          if (Config.instance_is_master==TRUE)                                                          /* si l'instance est Maitre */
           { switch( mnemo->type )
              { case MNEMO_MONOSTABLE:
                     Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Mise à un du bit M%03d %s:%s", __func__,
                               mnemo->num, mnemo->dls_tech_id, mnemo->acronyme );
                     if (mnemo->num != -1) Envoyer_commande_dls ( mnemo->num );
                                      else Envoyer_commande_dls_data ( mnemo->dls_tech_id, mnemo->acronyme );
                     break;
                default:
                     Info_new( Config.log, Config.log_msrv, LOG_ERR,
                               "%s: Error, type of mnemo not handled", __func__);
              }
           }
          else /* Envoi au master via thread HTTP */
           { if (mnemo->type == MNEMO_MONOSTABLE)
              { struct ZMQ_SET_BIT bit;
                bit.type = mnemo->type;
                bit.num = mnemo->num;
                g_snprintf( bit.dls_tech_id, sizeof(bit.dls_tech_id), "%s", mnemo->dls_tech_id );
                g_snprintf( bit.acronyme, sizeof(bit.acronyme), "%s", mnemo->acronyme );
                /*Send_zmq_with_tag ( Cfg_snips.zmq_to_master, NULL, NOM_THREAD, "*", "msrv", "SET_BIT",
                                    &bit, sizeof(struct ZMQ_SET_BIT) );*/
              }
             else Info_new( Config.log, Config.log_msrv, LOG_ERR,
                           "%s: Error, type of mnemo not handled", __func__ );
           }
          g_free(mnemo);
        }
     }
  }
/******************************************************************************************************************************/
/* Snips_message_CB: appeller par la librairie snips lorsque qu'un evenement Vocal est reconnu                                */
/* Entrée : L'évènement                                                                                                       */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Snips_message_CB(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message)
  { const gchar *intent, *targetAction, *targetObject[3], *targetRoom[3];
    gint nbrObject = 0, nbrRoom = 0;
    gint nbr_slots, i, j;
    JsonArray *slotArray;
    JsonObject *object;
    JsonNode *Query;

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

    if (nbr_slots==1)
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: Only one slot", __func__ );
       json_node_unref (Query);
       return;
     }
    
    for( i=0;i<nbr_slots; i++)
     { const gchar *slotValue, *slotName;
       JsonObject *slot = json_array_get_object_element ( slotArray, i );
       slotValue = json_object_get_string_member( json_object_get_object_member ( slot, "value" ), "value" );
       slotName = json_object_get_string_member( slot, "slotName" );
       Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_DEBUG,
                 "%s: slot %d/%d trouvé: %s - %s", __func__, i+1, nbr_slots, slotName, slotValue );
       if (!strcmp(slotName,"targetAction")) targetAction=slotValue;
       if (nbrRoom<3   && !strcmp(slotName,"targetRoom"))   targetRoom  [nbrRoom++]   = slotValue;
       if (nbrObject<3 && !strcmp(slotName,"targetObject")) targetObject[nbrObject++] = slotValue;
     }

    for ( i=0; i<nbrObject; i++ )
     { for ( j=0; j<nbrRoom; j++ )
        { gchar result[64];
          g_snprintf ( result, sizeof(result), "%s,%s,%s,%s", intent, (targetAction ? targetAction : ""),
                       (targetObject[i] ? targetObject[i] : ""), (targetRoom[j] ? targetRoom[j] : "") );
          Snips_traiter_commande_vocale ( result );
        }
     }
    json_node_unref (Query);
    Cfg_snips.nbr_msg_recu++;
 	}
/******************************************************************************************************************************/
/* Snips_connect_CB: appeller par la librairie snips lors d'un evenement de connect                                           */
/* Entrée : L'évènement                                                                                                       */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static void Snips_connect_CB(struct mosquitto *mosq, void *userdata, int result)
  {	if(!result)
     {	/* Subscribe to broker information topics on successful connect. */
		     mosquitto_subscribe(mosq, NULL, "hermes/intent/#", 2);
       Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: Connect OK. subscribing...", __func__ );
	    }
    else
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: Connect Error/ Result=%d.", __func__, result ); }
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
    memset( &Cfg_snips, 0, sizeof(Cfg_snips) );                                     /* Mise a zero de la structure de travail */
    Cfg_snips.lib = lib;                                           /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_snips.lib->TID = pthread_self();                                                    /* Sauvegarde du TID pour le pere */
    Snips_Lire_config ();                                                   /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE,
              "%s: Demarrage %s . . . TID = %p", __func__, VERSION, pthread_self() );
    Cfg_snips.lib->Thread_run = TRUE;                                                                   /* Le thread tourne ! */

    g_snprintf( Cfg_snips.lib->admin_prompt, sizeof(Cfg_snips.lib->admin_prompt), "snips" );
    g_snprintf( Cfg_snips.lib->admin_help,   sizeof(Cfg_snips.lib->admin_help),   "Manage Snips system" );

    if (lib->Thread_boot_start && !Cfg_snips.enable)
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE,
                "%s: Thread is not enabled in config. Shutting Down %p", __func__, pthread_self() );
       lib->Thread_boot_start = FALSE;
       goto end;
     }

    zmq_from_bus = Connect_zmq ( ZMQ_SUB, "listen-to-bus", "inproc", ZMQUEUE_LOCAL_BUS, 0 );

	   mosquitto_lib_init();
 	  mosq = mosquitto_new(NULL, TRUE, NULL);
	   if(!mosq)
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE,
                "%s: Unable to create Mosquitto MQTT. Shutting Down %p", __func__, pthread_self() );
       lib->Thread_boot_start = FALSE;
       goto end;
     }
   	mosquitto_log_callback_set(mosq, Snips_log_CB);
	   mosquitto_connect_callback_set(mosq, Snips_connect_CB);
	   mosquitto_message_callback_set(mosq, Snips_message_CB);
	   mosquitto_subscribe_callback_set(mosq, Snips_subscribe_CB);

   	if( mosquitto_connect(mosq, Cfg_snips.snips_host, 1883, 60) )
     { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE,
                "%s: Unable to connect to MQTT local Queue. Shutting Down %p", __func__, pthread_self() );
       lib->Thread_boot_start = FALSE;
       goto end;
     }

    while(Cfg_snips.lib->Thread_run == TRUE)                                                 /* On tourne tant que necessaire */
     { struct ZMQ_TARGET *event;
       gchar buffer[256];
       void *payload;

       if (Cfg_snips.lib->Thread_reload)                                                             /* On a recu reload ?? */
        { Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: SIGUSR1", __func__ );
          Snips_Lire_config();
          Cfg_snips.lib->Thread_reload = FALSE;
        }

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
       usleep(1000);
     }
    Close_zmq ( zmq_from_bus );
end:
   	mosquitto_destroy(mosq);
   	mosquitto_lib_cleanup();
    Info_new( Config.log, Cfg_snips.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    Cfg_snips.lib->Thread_run = FALSE;                                                          /* Le thread ne tourne plus ! */
    Cfg_snips.lib->TID = 0;                                                   /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
