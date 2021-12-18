/******************************************************************************************************************************/
/* Watchdogd/Audio/Audio.c  Gestion des messages audio de Watchdog 2.0                                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         sam. 09 nov. 2013 13:49:53 CET */
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
 #include "Audio.h"

/******************************************************************************************************************************/
/* Audio_Creer_DB : Creation de la table du thread                                                                            */
/* Entrée: le pointeur sur la PROCESS                                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Audio_Creer_DB ( struct PROCESS *lib )
  { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'.", __func__, lib->database_version );

    if (lib->database_version==0)
     { SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s` ("
                       "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                       "`date_create` datetime NOT NULL DEFAULT NOW(),"
                       "`uuid` varchar(37) COLLATE utf8_unicode_ci NOT NULL,"
                       "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                       "`language` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                       "`device` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                       "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                       "FOREIGN KEY (`uuid`) REFERENCES `processes` (`uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                       ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", lib->name );
       goto end;
     }

end:
    Process_set_database_version ( lib, 1 );
  }
/******************************************************************************************************************************/
/* Jouer_wav: Jouer un fichier wav dont le nom est en paramètre                                                               */
/* Entrée : le nom du fichier wav                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 static gboolean Jouer_wav_by_file ( struct SUBPROCESS *module, gchar *texte )
  { gint fd_cible, pid;
    gchar fichier[80];

    g_snprintf( fichier, sizeof(fichier), "Son/%s.wav", texte );
    fd_cible = open ( fichier, O_RDONLY, 0 );
    if (fd_cible < 0 && Config.instance_is_master == FALSE)
     { gchar chaine[80];
       g_snprintf(chaine, sizeof(chaine), "wget https://%s:5560/audio/%s.wav -O %s", Config.master_host, texte, fichier );
       Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                 "%s: '%s' not found trying down from master '%s'", __func__, fichier, chaine );
       system(chaine);
       fd_cible = open ( fichier, O_RDONLY, 0 );
       if (fd_cible < 0)
        { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                    "%s: '%s' not found (even after download)", __func__, fichier );
          return(FALSE);
        }
     }
    else if (fd_cible < 0 && Config.instance_is_master == TRUE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s' not found", __func__, fichier );
       return(FALSE);
     }
    else close (fd_cible);

    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: Envoi d'un wav %s", __func__, fichier );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: APLAY '%s' fork failed pid=%d", __func__, fichier, pid );
       return(FALSE);
     }
    else if (!pid)
     { execlp( "aplay", "aplay", "--device", Json_get_string ( module->config, "device" ), fichier, NULL );
       Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: APLAY '%s' exec failed pid=%d", __func__, fichier, pid );
       _exit(0);
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
                "%s: APLAY '%s' waiting to finish pid=%d", __func__, fichier, pid );
       waitpid(pid, NULL, 0 );
     }
    Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: APLAY '%s' finished pid=%d", __func__, fichier, pid );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Jouer_google_speech : Joue un texte avec google_speech et attend la fin de la diffusion                                    */
/* Entrée : le message à jouer                                                                                                */
/* Sortie : True si OK, False sinon                                                                                           */
/******************************************************************************************************************************/
 gboolean Jouer_google_speech ( struct SUBPROCESS *module, gchar *audio_libelle )
  { gint pid;

    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: Send '%s'", __func__, audio_libelle );
    pid = fork();
    if (pid<0)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                 "%s: '%s' fork failed pid=%d (%s)", __func__, audio_libelle, pid, strerror(errno) );
       return(FALSE);
     }
    else if (!pid)
     { execlp( "Wtd_play_google.sh", "Wtd_play_google", Json_get_string ( module->config, "language" ), audio_libelle,
                                                        Json_get_string ( module->config, "device" ), NULL );
       Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                "%s: '%s' exec failed pid=%d (%s)", __func__, audio_libelle, pid, strerror( errno ) );
       _exit(0);
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
                "%s: '%s' waiting to finish pid=%d", __func__, audio_libelle, pid );
       waitpid(pid, NULL, 0 );
     }
    Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: Wtd_play_google %s '%s' %s finished pid=%d", __func__,
              Json_get_string ( module->config, "language" ), audio_libelle,
              Json_get_string ( module->config, "device" ), pid );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Run_subprocess: Prend en charge un des sous process du thread                                                              */
/* Entrée: la structure SUBPROCESS associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_subprocess ( struct SUBPROCESS *module )
  { SubProcess_init ( module, sizeof(struct AUDIO_VARS) );
    struct AUDIO_VARS *vars = module->vars;

    gchar *tech_id = Json_get_string ( module->config, "tech_id" );

    if (Dls_auto_create_plugin( tech_id, "Gestion de l'audio diffusion" ) == FALSE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", __func__, tech_id ); }

    Mnemo_auto_create_DI ( FALSE, tech_id, "P_ALL", "Profil Audio: All Hps Enabled" );
    Mnemo_auto_create_DI ( FALSE, tech_id, "P_NONE", "Profil audio: All Hps disabled" );

    gboolean retour = Jouer_google_speech( module, "Instance démarrée !" );
    SubProcess_send_comm_to_master_new ( module, retour );

    while(module->lib->Thread_run == TRUE && module->lib->Thread_reload == FALSE)            /* On tourne tant que necessaire */
     { usleep(100000);
       sched_yield();

       SubProcess_send_comm_to_master_new ( module, module->comm_status );         /* Périodiquement envoie la comm au master */
/******************************************************************************************************************************/
       JsonNode *request;
       while ( (request = SubProcess_Listen_to_master_new ( module ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "DLS_HISTO" ) && Json_has_member ( request, "alive" ) &&
               Json_has_member ( request, "tech_id" ) && Json_has_member ( request, "acronyme" ) &&
               Json_has_member ( request, "audio_profil" ) && Json_has_member ( request, "audio_libelle" ) &&
               Json_get_bool ( request, "alive" ) == TRUE &&
               strcasecmp( Json_get_string( request, "audio_profil" ), "P_NONE" ) )
           { gchar *tech_id       = Json_get_string ( request, "tech_id" );
             gchar *acronyme      = Json_get_string ( request, "acronyme" );
             gchar *audio_profil  = Json_get_string ( request, "audio_profil" );
             gchar *audio_libelle = Json_get_string ( request, "audio_libelle" );
             gchar *libelle       = Json_get_string ( request, "libelle" );
             gint typologie = Json_get_int ( request, "typologie" );

             Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG,
                       "%s: Recu message '%s:%s' (audio_profil=%s)", __func__, tech_id, acronyme, audio_profil );

             if ( vars->diffusion_enabled == FALSE &&
                  ! (typologie == MSG_ALERTE || typologie == MSG_DANGER)
                )                                                               /* Bit positionné quand arret diffusion audio */
              { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                          "%s : Envoi audio inhibé. Dropping '%s:%s'", __func__, tech_id, acronyme );
              }
             else
              { /* { Envoyer_commande_dls_data( "AUDIO", audio_profil ); }                /* Pos. du profil audio via interne */

                if (vars->last_audio + AUDIO_JINGLE < Partage->top)                            /* Si Pas de message depuis xx */
                 { Jouer_wav_by_file( module, "jingle"); }                                          /* On balance le jingle ! */
                vars->last_audio = Partage->top;

                if (strlen(audio_libelle))                  /* Si audio_libelle, le jouer, sinon jouer le libelle tout court) */
                 { gboolean retour = Jouer_google_speech( module, audio_libelle );
                   SubProcess_send_comm_to_master_new ( module, retour );
                 }
                else
                 { gboolean retour = Jouer_google_speech( module, libelle );
                   SubProcess_send_comm_to_master_new ( module, retour );
                 }
                /* { Envoyer_commande_dls_data( "AUDIO", "P_NONE" ); }                       /* Bit de fin d'emission message */
              }
           }
          else if ( !strcasecmp( zmq_tag, "DISABLE" ) )
           { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s : Diffusion disabled by master", __func__ );
             vars->diffusion_enabled = FALSE;
           }
          else if ( !strcasecmp( zmq_tag, "ENABLE" ) )
           { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s : Diffusion enabled by master", __func__ );
             vars->diffusion_enabled = TRUE;
           }
          else if ( !strcasecmp( zmq_tag, "TEST" ) )
           { Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s : Test de diffusion", __func__ );
             Jouer_google_speech( module, "Ceci est un test de diffusion audio" );
           }
          json_node_unref ( request );
        }
     }

    SubProcess_end(module);
  }
/******************************************************************************************************************************/
/* Run_process: Run du Process                                                                                                */
/* Entrée: la structure PROCESS associée                                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_process ( struct PROCESS *lib )
  {
reload:
    Audio_Creer_DB ( lib );                                                                    /* Création de la DB du thread */
    Thread_init ( "audio", "USER", lib, WTD_VERSION, "Manage Audio System" );

    lib->config = Json_node_create();
    if(lib->config) SQL_Select_to_json_node ( lib->config, "subprocess", "SELECT * FROM %s WHERE uuid='%s'", lib->name, lib->uuid );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: %d subprocess to load", __func__, Json_get_int ( lib->config, "nbr_subprocess" ) );

    Json_node_foreach_array_element ( lib->config, "subprocess", Process_Load_one_subprocess, lib );   /* Chargement des modules */
    while( lib->Thread_run == TRUE && lib->Thread_reload == FALSE) sleep(1);                 /* On tourne tant que necessaire */
    Process_Unload_all_subprocess ( lib );

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }

    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
