/******************************************************************************************************************************/
/* Watchdogd/Audio/Audio.c  Gestion des messages audio de Watchdog 2.0                                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         sam. 09 nov. 2013 13:49:53 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Archive.c
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
/* Jouer_wav: Jouer un fichier wav dont le nom est en paramètre                                                               */
/* Entrée : le nom du fichier wav                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static gboolean Jouer_wav_by_file ( struct THREAD *module, gchar *texte )
  { gint fd_cible, pid;
    gchar fichier[80];

    gchar *device   = Json_get_string ( module->config, "device" );

    g_snprintf( fichier, sizeof(fichier), "Son/%s.wav", texte );
    fd_cible = open ( fichier, O_RDONLY, 0 );
    if (fd_cible < 0 && Config.instance_is_master == FALSE)
     { gchar chaine[80];
       g_snprintf(chaine, sizeof(chaine), "wget https://%s:5560/audio/%s.wav -O %s", Config.master_hostname, texte, fichier );
       Info_new( __func__, module->Thread_debug, LOG_WARNING,
                 "'%s' not found trying down from master '%s'", fichier, chaine );
       system(chaine);
       fd_cible = open ( fichier, O_RDONLY, 0 );
       if (fd_cible < 0)
        { Info_new( __func__, module->Thread_debug, LOG_ERR,
                    "'%s' not found (even after download)", fichier );
          return(FALSE);
        }
     }
    else if (fd_cible < 0 && Config.instance_is_master == TRUE)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "'%s' not found", fichier );
       return(FALSE);
     }
    else close (fd_cible);

    Info_new( __func__, module->Thread_debug, LOG_INFO, "Envoi d'un wav %s", fichier );
    pid = fork();
    if (pid<0)
     { Info_new( __func__, module->Thread_debug, LOG_ERR, "APLAY '%s' fork failed pid=%d", fichier, pid );
       return(FALSE);
     }
    else if (!pid)
     { execlp( "aplay", "aplay", "--device", device, fichier, NULL );
       Info_new( __func__, module->Thread_debug, LOG_ERR, "APLAY '%s' exec failed pid=%d", fichier, pid );
       _exit(0);
     }
    else
     { Info_new( __func__, module->Thread_debug, LOG_DEBUG,
                "APLAY '%s' waiting to finish pid=%d", fichier, pid );
       waitpid(pid, NULL, 0 );
     }
    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "APLAY '%s' finished pid=%d", fichier, pid );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Jouer_google_speech : Joue un texte avec google_speech et attend la fin de la diffusion                                    */
/* Entrée : le message à jouer                                                                                                */
/* Sortie : True si OK, False sinon                                                                                           */
/******************************************************************************************************************************/
 gboolean Jouer_google_speech ( struct THREAD *module, gchar *audio_libelle )
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
       return(FALSE);
     }
    else if (!pid)
     { execlp( "Wtd_play_google.sh", "Wtd_play_google", language, audio_libelle, device, NULL );
       _exit(0);
     }

    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "'%s' waiting to finish pid=%d", audio_libelle, pid );
    waitpid(pid, NULL, 0 );

    Info_new( __func__, module->Thread_debug, LOG_DEBUG, "Wtd_play_google %s '%s' %s finished pid=%d",
              language, audio_libelle, device, pid );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Run_thread: Prend en charge un des sous thread de l'agent                                                                  */
/* Entrée: la structure THREAD associée                                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_thread ( struct THREAD *module )
  { Thread_init ( module, sizeof(struct AUDIO_VARS) );
    struct AUDIO_VARS *vars = module->vars;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    vars->p_all  = Mnemo_create_thread_DI ( module, "P_ALL", "Profil Audio: All Hps Enabled" );
    vars->p_none = Mnemo_create_thread_DI ( module, "P_NONE", "Profil Audio: All Hps disabled" );
    gint volume  = Json_get_int ( module->config, "volume" );
    gchar chaine[256];
    g_snprintf( chaine, sizeof(chaine), "wpctl set-volume @DEFAULT_AUDIO_SINK@ %d%%", volume );
    system(chaine);

    gboolean retour = Jouer_google_speech( module, "Module audio démarré !" );
    Thread_send_comm_to_master ( module, retour );
    vars->diffusion_enabled = TRUE;                                                     /* A l'init, la diffusion est activée */
    while(module->Thread_run == TRUE)                                                        /* On tourne tant que necessaire */
     { Thread_loop ( module );                                            /* Loop sur thread pour mettre a jour la telemetrie */
/******************************************************************************************************************************/
       while ( module->MQTT_messages )
        { pthread_mutex_lock ( &module->synchro );
          JsonNode *request = module->MQTT_messages->data;
          module->MQTT_messages = g_slist_remove ( module->MQTT_messages, request );
          pthread_mutex_unlock ( &module->synchro );
          gchar *tag = Json_get_string ( request, "tag" );
          if ( !strcasecmp( tag, "DLS_HISTO" ) && Json_has_member ( request, "alive" ) &&
               Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
               Json_has_member ( request, "audio_profil" ) && Json_has_member ( request, "audio_libelle" ) &&
               Json_get_bool ( request, "alive" ) == TRUE &&
               strcasecmp( Json_get_string( request, "audio_profil" ), "P_NONE" ) )
           { gchar *tech_id        = Json_get_string ( request, "tech_id" );
             gchar *acronyme       = Json_get_string ( request, "acronyme" );
             gchar *audio_profil   = Json_get_string ( request, "audio_profil" );
             gchar *audio_libelle  = Json_get_string ( request, "audio_libelle" );
             gchar *libelle        = Json_get_string ( request, "libelle" );
             gint typologie        = Json_get_int ( request, "typologie" );

             Info_new( __func__, module->Thread_debug, LOG_DEBUG,
                       "Recu message '%s:%s' (audio_profil=%s)", tech_id, acronyme, audio_profil );

             if ( vars->diffusion_enabled == FALSE && ! (typologie == MSG_ALERTE || typologie == MSG_DANGER)
                )                                                               /* Bit positionné quand arret diffusion audio */
              { Info_new( __func__, module->Thread_debug, LOG_WARNING,
                          "Envoi audio inhibé. Dropping '%s:%s'", tech_id, acronyme );
              }
             else
              { MQTT_Send_DI_pulse ( module, thread_tech_id, "P_ALL" );           /* Pos. du profil audio via interne */

                if (vars->last_audio + AUDIO_JINGLE < Partage->top)                            /* Si Pas de message depuis xx */
                 { Jouer_wav_by_file( module, "jingle"); }                                          /* On balance le jingle ! */
                vars->last_audio = Partage->top;

                if (strlen(audio_libelle))                  /* Si audio_libelle, le jouer, sinon jouer le libelle tout court) */
                 { gboolean retour = Jouer_google_speech( module, audio_libelle );
                   Thread_send_comm_to_master ( module, retour );
                 }
                else
                 { gboolean retour = Jouer_google_speech( module, libelle );
                   Thread_send_comm_to_master ( module, retour );
                 }
                MQTT_Send_DI_pulse ( module, thread_tech_id, "P_NONE" );             /* Bit de fin d'emission message */
              }
           }
          else if ( !strcasecmp( tag, "DISABLE" ) )
           { Info_new( __func__, module->Thread_debug, LOG_NOTICE, "Diffusion disabled by master" );
             vars->diffusion_enabled = FALSE;
           }
          else if ( !strcasecmp( tag, "ENABLE" ) )
           { Info_new( __func__, module->Thread_debug, LOG_NOTICE, "Diffusion enabled by master" );
             vars->diffusion_enabled = TRUE;
           }
          else if ( !strcasecmp( tag, "TEST" ) )
           { Info_new( __func__, module->Thread_debug, LOG_NOTICE, "Test de diffusion" );
             Jouer_google_speech( module, "Ceci est un test de diffusion audio" );
           }
          Json_node_unref ( request );
        }
     }

    Thread_end(module);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
