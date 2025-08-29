/******************************************************************************************************************************/
/* Watchdogd/Audio/Audio.c  Gestion des messages audio de Watchdog 2.0                                                        */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                     sam. 09 nov. 2013 13:49:53 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Archive.c
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

 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/wait.h>
 #include <fcntl.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Audio.h"

/******************************************************************************************************************************/
/* Jouer_google_speech : Joue un texte avec google_speech et attend la fin de la diffusion                                    */
/* Entrée : le message à jouer                                                                                                */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Jouer_google_speech ( struct THREAD *module, gchar *audio_libelle )
  { gint pid;

    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "Send '%s'", audio_libelle );
    gchar *language = Json_get_string ( module->config, "language" );
    gchar *device   = Json_get_string ( module->config, "device" );
    Info_new( __func__, module->Thread_debug, LOG_INFO,
              "Running Wtd_play_google.sh %s %s %s", language, audio_libelle, device );

    pid = fork();
    if (pid<0)
     { Info_new( __func__, module->Thread_debug, LOG_ERR,
                 "'%s' fork failed pid=%d (%s)", audio_libelle, pid, strerror(errno) );
       Thread_send_comm_to_master ( module, FALSE );
       return;
     }
    else if (!pid)
     { execlp( "Wtd_play_google.sh", "Wtd_play_google", language, audio_libelle, device, NULL );
       _exit(0);
     }

    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "'%s' waiting to finish pid=%d", audio_libelle, pid );
    waitpid(pid, NULL, 0 );

    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Wtd_play_google %s '%s' %s finished pid=%d",
              language, audio_libelle, device, pid );
    Thread_send_comm_to_master ( module, TRUE );
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct AUDIO_VARS) );
    struct AUDIO_VARS *vars = module->vars;

    /*gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );*/

    gint volume  = Json_get_int ( module->config, "volume" );
    gchar chaine[256];
    g_snprintf( chaine, sizeof(chaine), "wpctl set-volume @DEFAULT_AUDIO_SINK@ %d%%", volume );
    system(chaine);
    Info_new( __func__, module->Thread_debug, LOG_NOTICE, "Volume set to %d", volume );

    GList *Audio_zones = json_array_get_elements ( Json_get_array ( module->config, "audio_zones" ) );
    GList *audio_zones = Audio_zones;
    while(audio_zones)
     { JsonNode *element = audio_zones->data;
       gchar *audio_zone_name = Json_get_string ( element, "audio_zone_name" );
       Info_new( __func__, module->Thread_debug, LOG_NOTICE, "Listening to AudioZone '%s'", audio_zone_name );
       MQTT_Subscribe ( module->MQTT_session, "AUDIO_ZONE/%s", audio_zone_name );
       audio_zones = g_list_next(audio_zones);
     }
    g_list_free(Audio_zones);

    Jouer_google_speech( module, "Module audio démarré !" );
    vars->diffusion_enabled = TRUE;                                                     /* A l'init, la diffusion est activée */
    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/******************************************************************************************************************************/
       if (!module->MQTT_messages) continue;
       pthread_mutex_lock ( &module->synchro );
       JsonNode *request = module->MQTT_messages->data;
       module->MQTT_messages = g_slist_remove ( module->MQTT_messages, request );
       pthread_mutex_unlock ( &module->synchro );
       if ( Json_has_member ( request, "token_lvl0" ) )
        { gchar *token_lvl0 = Json_get_string ( request, "token_lvl0" );
          if (!strcasecmp ( token_lvl0, "AUDIO_ZONE" ) &&
              Json_has_member ( request, "token_lvl1" ) && Json_has_member ( request, "audio_libelle" )
             )
           { gchar *audio_zone_name = Json_get_string ( request, "token_lvl1" );
             gchar *audio_libelle   = Json_get_string ( request, "audio_libelle" );
             Info_new( __func__, module->Thread_debug, LOG_INFO, "Saying '%s' on audio_zone '%s'", audio_libelle, audio_zone_name );
             if (vars->last_audio + AUDIO_JINGLE < Partage->top)                                  /* Si Pas de message depuis xx */
              { Jouer_google_speech( module, "Attention"); }                                           /* On balance le jingle ! */
             vars->last_audio = Partage->top;

             Jouer_google_speech( module, audio_libelle );                                                   /* Jouer le libelle */
           }
          else if (!strcasecmp ( token_lvl0, "SET_TEST" ) )
           { Info_new( __func__, module->Thread_debug, LOG_INFO, "Saying 'test'" );
             Jouer_google_speech( module, "Ceci est un test" );
           }
        }
       Json_node_unref ( request );
     }
    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
