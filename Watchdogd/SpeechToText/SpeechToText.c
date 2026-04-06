/******************************************************************************************************************************/
/* Watchdogd/SpeechToText/SpeechToText.c  Gestion de la reconnaissance vocale Watchdog                                        */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                29.05.2025 19:49:37 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * SpeechToText.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
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

 #include <stdio.h>
 #include <sys/prctl.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <locale.h>

/*********************************************************** Headers **********************************************************/
 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "SpeechToText.h"

/******************************************************************************************************************************/
/* Deconnecter: Deconnexion du ups                                                                                            */
/* Entrée: un id                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Deconnecter_UPS ( struct THREAD *module )
  { struct UPS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *host    = Json_get_string ( module->config, "host" );

    if (vars->started == TRUE)
     { upscli_disconnect( &vars->upsconn );
       vars->started = FALSE;
     }

    MQTT_Send_AI ( module, vars->Load, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Realpower, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Battery_charge, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Input_voltage, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Battery_runtime, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Battery_voltage, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Input_hz, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Output_current, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Output_hz, 0.0, FALSE );
    MQTT_Send_AI ( module, vars->Output_voltage, 0.0, FALSE );

    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s disconnected (host='%s')", thread_tech_id, host );
    Thread_send_comm_to_master ( module, FALSE );
  }
/******************************************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                                               */
/* Entrée: une nom et un password                                                                                             */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                                              */
/******************************************************************************************************************************/
 static gboolean Connecter_ups ( struct THREAD *module )
  { struct UPS_VARS *vars = module->vars;
    gchar buffer[80];
    int connexion;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *host           = Json_get_string ( module->config, "host" );
    gchar *name           = Json_get_string ( module->config, "name" );
    gchar *admin_username = Json_get_string ( module->config, "admin_username" );
    gchar *admin_password = Json_get_string ( module->config, "admin_password" );

    if ( (connexion = upscli_connect( &vars->upsconn, host, UPS_PORT_TCP, UPSCLI_CONN_TRYSSL)) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "%s: connexion refused by ups (host '%s' -> %s)", thread_tech_id, host,
                 (char *)upscli_strerror(&vars->upsconn) );
       return(FALSE);
     }

    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s connected (host='%s')", thread_tech_id, host );
/********************************************************* UPSDESC ************************************************************/
    g_snprintf( buffer, sizeof(buffer), "GET UPSDESC %s\n", name );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "%s: Sending GET UPSDESC failed (%s)", thread_tech_id,
                (char *)upscli_strerror(&vars->upsconn) );
     }
    else
     { if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                   "%s: Reading GET UPSDESC failed (%s)", thread_tech_id,
                   (char *)upscli_strerror(&vars->upsconn) );
        }
       else
        { gchar description[128];
          g_snprintf( description, sizeof(description), "%s", buffer + strlen(name) + 10 );
          description [ strlen(description) - 1 ] = 0; /* supprime les " du début/fin */
          Info_new( __func__, module->Thread_debug, LOG_DEBUG, "%s: Reading GET UPSDESC %s", thread_tech_id, description );
        }
     }
/**************************************************** USERNAME ****************************************************************/
    g_snprintf( buffer, sizeof(buffer), "USERNAME %s\n", admin_username );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "%s: Sending USERNAME failed %s", thread_tech_id,
                (char *)upscli_strerror(&vars->upsconn) );
     }
    else
     { if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                   "%s: Reading USERNAME failed %s", thread_tech_id,
                   (char *)upscli_strerror(&vars->upsconn) );
        }
       else
        { Info_new( __func__, module->Thread_debug, LOG_DEBUG,
                   "%s: Reading USERNAME %s", thread_tech_id, buffer );
        }
     }

/******************************************************* PASSWORD *************************************************************/
    g_snprintf( buffer, sizeof(buffer), "PASSWORD %s\n", admin_password );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "%s: Sending PASSWORD failed %s", thread_tech_id,
                (char *)upscli_strerror(&vars->upsconn) );
     }
    else
     { if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                   "%s: Reading PASSWORD failed %s", thread_tech_id,
                   (char *)upscli_strerror(&vars->upsconn) );
        }
       else
        { Info_new( __func__, module->Thread_debug, LOG_DEBUG,
                   "%s: Reading PASSWORD %s", thread_tech_id, buffer );
        }
     }

    vars->date_next_connexion = 0;
    vars->started = TRUE;
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s connected (host='%s')", thread_tech_id, host );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* SpeechToText_set_instcmd: Envoi d'une instant commande à l'ups                                                             */
/* Entrée : l'ups, le nom de la commande                                                                                      */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                                          */
/******************************************************************************************************************************/
 static void SpeechToText_set_instcmd ( struct THREAD *module, gchar *nom_cmd )
  { struct UPS_VARS *vars = module->vars;
    gchar buffer[80];

    if (vars->started != TRUE) return;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *name    = Json_get_string ( module->config, "name" );

    g_snprintf( buffer, sizeof(buffer), "INSTCMD %s %s\n", name, nom_cmd );
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Sending '%s'", thread_tech_id, buffer );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "%s: Sending INSTCMD failed with error '%s' for '%s'", thread_tech_id,
                 (char *)upscli_strerror(&vars->upsconn), buffer );
       Deconnecter_UPS ( module );
       return;
     }

    if ( upscli_readline( &vars->upsconn, buffer, sizeof(buffer) ) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                "%s: Reading INSTCMD result failed (%s) error %s", thread_tech_id,
                 nom_cmd, (char *)upscli_strerror(&vars->upsconn) );
       Deconnecter_UPS ( module );
       return;
     }
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "%s: Sending '%s' OK", thread_tech_id, nom_cmd );
  }
/******************************************************************************************************************************/
/* SpeechToText_get_var: Recupere une valeur de la variable en parametre                                                      */
/* Entrée : l'ups, le nom de variable, la variable a renseigner                                                               */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                                          */
/******************************************************************************************************************************/
 static gchar *SpeechToText_get_var ( struct THREAD *module, gchar *nom_var )
  { struct UPS_VARS *vars = module->vars;
    static gchar buffer[80];
    gint retour_read;
    if (!vars->started) return(NULL);

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *name           = Json_get_string ( module->config, "name" );

    g_snprintf( buffer, sizeof(buffer), "GET VAR %s %s\n", name, nom_var );
    if ( upscli_sendline( &vars->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING, "%s: Sending GET VAR failed (%s) error=%s", thread_tech_id,
                 buffer, (char *)upscli_strerror(&vars->upsconn) );
       Deconnecter_UPS ( module );
       return(NULL);
     }

    retour_read = upscli_readline( &vars->upsconn, buffer, sizeof(buffer) );
    Info_new( __func__, module->Thread_debug, LOG_DEBUG,
             "%s: Reading GET VAR %s ReadLine result = %d, upscli_upserror = %d, buffer = %s", thread_tech_id,
              nom_var, retour_read, upscli_upserror(&vars->upsconn), buffer );
    if ( retour_read == -1 )
     { Info_new( __func__, module->Thread_debug, LOG_WARNING, "%s: Reading GET VAR result failed (%s) error=%s", thread_tech_id,
                 nom_var, (char *)upscli_strerror(&vars->upsconn) );
       Deconnecter_UPS ( module );
       return(NULL);
     }

    if ( ! strncmp ( buffer, "VAR", 3 ) )                                    /* si Réponse numérique de la part du UPS daemon */
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG,
                "%s: Reading GET VAR %s OK = %s", thread_tech_id, nom_var, buffer );
       return(buffer + 6 + strlen(name) + strlen(nom_var));
     }

    if ( ! strncmp ( buffer, "ERR", 3 ) )                                            /* Detection des erreurs type DATA-STALE */
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                "%s: Reading GET VAR %s ERROR = %s", thread_tech_id, nom_var, buffer );
     }

    return(NULL);                                                  /* VAR NOT SUPPORTED / DRIVER NOT CONNECTED are not errors */
  }
/******************************************************************************************************************************/
/* Interroger_ups: Interrogation d'un ups                                                                                     */
/* Entrée: identifiants des ups                                                                                               */
/* Sortie: TRUE si pas de probleme, FALSE sinon                                                                               */
/******************************************************************************************************************************/
 static void Interroger_ups( struct THREAD *module )
  { struct UPS_VARS *vars = module->vars;
    gchar *reponse;

    if ( (reponse = SpeechToText_get_var ( module, "ups.load" )) != NULL )
     { MQTT_Send_AI ( module, vars->Load, atof(reponse+1), TRUE ); }

    if ( (reponse = SpeechToText_get_var ( module, "ups.realpower" )) != NULL )
     { MQTT_Send_AI ( module, vars->Realpower, atof(reponse+1), TRUE ); }

    if ( (reponse = SpeechToText_get_var ( module, "battery.charge" )) != NULL )
     { MQTT_Send_AI ( module, vars->Battery_charge, atof(reponse+1), TRUE ); }

    if ( (reponse = SpeechToText_get_var ( module, "input.voltage" )) != NULL )
     { MQTT_Send_AI ( module, vars->Input_voltage, atof(reponse+1), TRUE ); }

    if ( (reponse = SpeechToText_get_var ( module, "battery.runtime" )) != NULL )
     { MQTT_Send_AI ( module, vars->Battery_runtime, atof(reponse+1), TRUE ); }

    if ( (reponse = SpeechToText_get_var ( module, "battery.voltage" )) != NULL )
     { MQTT_Send_AI ( module, vars->Battery_voltage, atof(reponse+1), TRUE ); }

    if ( (reponse = SpeechToText_get_var ( module, "input.frequency" )) != NULL )
     { MQTT_Send_AI ( module, vars->Input_hz, atof(reponse+1), TRUE ); }

    if ( (reponse = SpeechToText_get_var ( module, "output.current" )) != NULL )
     { MQTT_Send_AI ( module, vars->Output_current, atof(reponse+1), TRUE ); }

    if ( (reponse = SpeechToText_get_var ( module, "output.frequency" )) != NULL )
     { MQTT_Send_AI ( module, vars->Output_hz, atof(reponse+1), TRUE ); }

    if ( (reponse = SpeechToText_get_var ( module, "output.voltage" )) != NULL )
     { MQTT_Send_AI ( module, vars->Output_voltage, atof(reponse+1), TRUE ); }

/*---------------------------------------------- Récupération des entrées TOR de l'UPS ---------------------------------------*/
    if ( (reponse = SpeechToText_get_var ( module, "outlet.1.status" )) != NULL )
     { MQTT_Send_DI ( module, vars->Outlet_1_status, !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = SpeechToText_get_var ( module, "outlet.2.status" )) != NULL )
     { MQTT_Send_DI ( module, vars->Outlet_2_status, !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = SpeechToText_get_var ( module, "ups.status" )) != NULL )
     { MQTT_Send_DI ( module, vars->Ups_online,       (g_strrstr(reponse, "OL")?TRUE:FALSE) );
       MQTT_Send_DI ( module, vars->Ups_charging,     (g_strrstr(reponse, "DISCHRG")?FALSE:TRUE) );
       MQTT_Send_DI ( module, vars->Ups_on_batt,      (g_strrstr(reponse, "OB")?TRUE:FALSE) );
       MQTT_Send_DI ( module, vars->Ups_replace_batt, (g_strrstr(reponse, "RB")?TRUE:FALSE) );
       MQTT_Send_DI ( module, vars->Ups_alarm,        (g_strrstr(reponse, "ALARM")?TRUE:FALSE) );
       Thread_send_comm_to_master ( module, TRUE );
     }
    else Thread_send_comm_to_master ( module, FALSE );
  }
/******************************************************************************************************************************/
/* Modbus_SET_DO: Met a jour une sortie TOR en fonction du jsonnode en parametre                                              */
/* Entrée: le module et le buffer Josn                                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Ups_SET_DO ( struct THREAD *module, JsonNode *msg )
  { gchar *thread_tech_id      = Json_get_string ( module->config, "thread_tech_id" );
    gchar *msg_thread_tech_id  = Json_get_string ( msg, "thread_tech_id" );
    gchar *msg_thread_acronyme = Json_get_string ( msg, "thread_acronyme" );
    gchar *msg_tech_id         = Json_get_string ( msg, "tech_id" );
    gchar *msg_acronyme        = Json_get_string ( msg, "acronyme" );

    if (!msg_thread_tech_id)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': requete mal formée manque msg_thread_tech_id", thread_tech_id ); }
    else if (!msg_thread_acronyme)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': requete mal formée manque msg_thread_acronyme", thread_tech_id ); }
    else if (strcasecmp (msg_thread_tech_id, thread_tech_id))
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG, "'%s': Pas pour nous", thread_tech_id ); }
    else if (!Json_has_member ( msg, "etat" ))
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s': requete mal formée manque etat", thread_tech_id ); }
    else
     { gboolean etat = Json_get_bool ( msg, "etat" );
       pthread_mutex_lock ( &module->synchro );
       Info_new( __func__, module->Thread_debug, LOG_NOTICE, "'%s': SET_DO '%s:%s'/'%s:%s'=%d",
                 thread_tech_id, msg_thread_tech_id, msg_thread_acronyme, msg_tech_id, msg_acronyme, etat );
       if (etat)
        { if (!strcasecmp(msg_thread_acronyme, "LOAD_OFF"))        SpeechToText_set_instcmd ( module, "load.off" );
          if (!strcasecmp(msg_thread_acronyme, "LOAD_ON"))         SpeechToText_set_instcmd ( module, "load.on" );
          if (!strcasecmp(msg_thread_acronyme, "OUTLET_1_OFF"))    SpeechToText_set_instcmd ( module, "outlet.1.load.off" );
          if (!strcasecmp(msg_thread_acronyme, "OUTLET_1_ON"))     SpeechToText_set_instcmd ( module, "outlet.1.load.on" );
          if (!strcasecmp(msg_thread_acronyme, "OUTLET_2_OFF"))    SpeechToText_set_instcmd ( module, "outlet.2.load.off" );
          if (!strcasecmp(msg_thread_acronyme, "OUTLET_2_ON"))     SpeechToText_set_instcmd ( module, "outlet.2.load.on" );
          if (!strcasecmp(msg_thread_acronyme, "START_DEEP_BAT"))  SpeechToText_set_instcmd ( module, "test.battery.start.deep" );
          if (!strcasecmp(msg_thread_acronyme, "START_QUICK_BAT")) SpeechToText_set_instcmd ( module, "test.battery.start.quick" );
          if (!strcasecmp(msg_thread_acronyme, "STOP_TEST_BAT"))   SpeechToText_set_instcmd ( module, "test.battery.stop" );
        }
       pthread_mutex_unlock ( &module->synchro );
     }
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct UPS_VARS) );
    struct UPS_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if (vosk_init() != 0) {
        fprintf(stderr, "Échec de l'initialisation Vosk\n");
        return 1;
    }

    const char *model_path = "models/vosk-model-small-fr-0.22";
    VoskModel *model = vosk_model_new(model_path);
    if (!model) {
        fprintf(stderr, "Erreur chargement modèle\n");
        return 1;
    }

    VoskRecognizer *recognizer = vosk_recognizer_new(model, SAMPLE_RATE);
    vosk_recognizer_set_max_alternatives(recognizer, 0);

    // Initialiser capture ALSA
    snd_pcm_t *handle;
    snd_pcm_open(&handle, "default", SND_PCM_STREAM_CAPTURE, 0);
    snd_pcm_set_params(handle,
                      SND_PCM_FORMAT_S16_LE,
                      SND_PCM_ACCESS_RW_INTERLEAVED,
                      1, SAMPLE_RATE, 1, 500000);

    int16_t buffer[BUFFER_SIZE];
    printf("🟢 Dites \"jarvis\" pour tester la détection...\n");


    const char *access_key = "votre_access_key";
    const char *keyword_path = "jarvis_linux.ppn";

    pv_porcupine_t *porcupine;
    pv_status_t status = pv_porcupine_init(access_key, 1, &keyword_path, NULL, &porcupine);

    if (status != PV_STATUS_SUCCESS) {
        fprintf(stderr, "Porcupine init error: %d\n", status);
goto end;
    }

    // Configuration de PulseAudio
    static const pa_sample_spec ss = {
        .format = PA_SAMPLE_S16LE,
        .rate = 16000,
        .channels = 1
    };

    int error;
    pa_simple *s = pa_simple_new(NULL, "porcupine-example", PA_STREAM_RECORD, NULL, "record", &ss, NULL, NULL, &error);
    if (!s) {
        fprintf(stderr, "pa_simple_new() failed: %s\n", pa_strerror(error));
        return 1;
    }


    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/****************************************************** Ecoute de l'API *******************************************************/
       if (module->Test_request)
        { Json_node_unref ( module->Test_request );
          module->Test_request = NULL;
        }
/****************************************************** Ecoute du master ******************************************************/
       while ( module->MQTT_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->MQTT_messages->data;
          module->MQTT_messages = g_slist_remove ( module->MQTT_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          gchar *tag = Json_get_string ( request, "tag" );
          /*if ( !strcasecmp (tag, "SET_DO") ) Ups_SET_DO ( module, request );*/
          Json_node_unref ( request );
        }
/********************************************* Début de l'interrogation du ups ************************************************/
       if (pa_simple_read(s, pcm, sizeof(pcm), &error) < 0) {
            fprintf(stderr, "pa_simple_read() failed: %s\n", pa_strerror(error));
            break;
        }
        int keyword_index = -1;
        if (pv_porcupine_process(porcupine, frame, &keyword_index) == PV_STATUS_SUCCESS && keyword_index >= 0) {
            printf("✅ Mot-clé détecté ! Enregistrement en cours...\n");

            int num_samples = RECORD_SECONDS * SAMPLE_RATE;
            float *recorded = malloc(sizeof(float) * num_samples);
            int16_t buffer[FRAME_LENGTH];
            int pos = 0;
            while (pos < num_samples) {
                Pa_ReadStream(stream, buffer, FRAME_LENGTH);
                for (int i = 0; i < FRAME_LENGTH && pos < num_samples; i++)
                    recorded[pos++] = buffer[i] / 32768.0f;
            }

            /*whisper_transcribe(recorded, num_samples, "models/ggml-small.bin");*/
            free(recorded);
        }
     }

    pa_simple_free(s);
    pv_porcupine_delete(porcupine);
end:


#ifdef

    while (1) {
        snd_pcm_readi(handle, buffer, BUFFER_SIZE);

        if (vosk_recognizer_accept_waveform(recognizer, (const char *)buffer, sizeof(buffer))) {
            const char *result = vosk_recognizer_result(recognizer);
            if (strstr(result, "jarvis") != NULL) {
                printf("🟢 Mot-clé \"jarvis\" détecté !\n");
                break;
            }
        }
    }

    vosk_recognizer_free(recognizer);
    vosk_model_free(model);
    snd_pcm_close(handle)
#endif
    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
