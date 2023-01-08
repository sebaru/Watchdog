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
        { Info_new( __func__, Config.log_msrv, LOG_INFO, "Timer: Partage->top = 0 !!" ); }

       Partage->top_cdg_plugin_dls++;                                                            /* Chien de garde plugin DLS */
       if (Partage->top_cdg_plugin_dls>200)                                         /* Si pas de réponse D.L.S en 20 secondes */
        { Info_new( __func__, Config.log_msrv, LOG_INFO, "CDG plugin DLS !!" );
          Partage->top_cdg_plugin_dls = 0;
        }
       return;
     }

    prctl(PR_GET_NAME, chaine, 0, 0, 0 );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "handled by %s", chaine );

    switch (num)
     { case SIGQUIT:
       case SIGINT:  Info_new( __func__, Config.log_msrv, LOG_INFO, "Recu SIGINT" );
                     Partage->com_msrv.Thread_run = FALSE;                       /* On demande l'arret de la boucle programme */
                     break;
       case SIGTERM: Info_new( __func__, Config.log_msrv, LOG_INFO, "Recu SIGTERM" );
                     Partage->com_msrv.Thread_run = FALSE;                       /* On demande l'arret de la boucle programme */
                     break;
       case SIGABRT: Info_new( __func__, Config.log_msrv, LOG_INFO, "Recu SIGABRT" );
                     break;
       case SIGCHLD: Info_new( __func__, Config.log_msrv, LOG_INFO, "Recu SIGCHLD" );
                     break;
       case SIGPIPE: Info_new( __func__, Config.log_msrv, LOG_INFO, "Recu SIGPIPE" ); break;
       case SIGBUS:  Info_new( __func__, Config.log_msrv, LOG_INFO, "Recu SIGBUS" ); break;
       case SIGIO:   Info_new( __func__, Config.log_msrv, LOG_INFO, "Recu SIGIO" ); break;
       case SIGUSR1: Info_new( __func__, Config.log_msrv, LOG_INFO, "Recu SIGUSR1" );
                     break;
       case SIGUSR2: Info_new( __func__, Config.log_msrv, LOG_INFO, "Recu SIGUSR2" );
                     break;
       default: Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Recu signal %d", num ); break;
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
    if (!ttech_id_1) { Info_new( __func__, Config.log_msrv, LOG_ERR, "ttech_id1 is NULL" ); return(-1); }
    if (!ttech_id_2) { Info_new( __func__, Config.log_msrv, LOG_ERR, "ttech_id2 is NULL" ); return(1); }
    gint result = strcasecmp ( ttech_id_1, ttech_id_2 );
    if (result) return(result);
    gchar *tacronyme_1 = Json_get_string ( node1, "thread_acronyme" );
    gchar *tacronyme_2 = Json_get_string ( node2, "thread_acronyme" );
    if (!tacronyme_1) { Info_new( __func__, Config.log_msrv, LOG_ERR, "tacronyme1 is NULL" ); return(-1); }
    if (!tacronyme_2) { Info_new( __func__, Config.log_msrv, LOG_ERR, "tacronyme2 is NULL" ); return(1); }
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
    if (!tech_id_1) { Info_new( __func__, Config.log_msrv, LOG_ERR, "tech_id1 is NULL" ); return(-1); }
    if (!tech_id_2) { Info_new( __func__, Config.log_msrv, LOG_ERR, "tech_id2 is NULL" ); return(1); }
    gint result = strcasecmp ( tech_id_1, tech_id_2 );
    if (result) return(result);
    gchar *acronyme_1 = Json_get_string ( node1, "acronyme" );
    gchar *acronyme_2 = Json_get_string ( node2, "acronyme" );
    if (!acronyme_1) { Info_new( __func__, Config.log_msrv, LOG_ERR, "acronyme1 is NULL" ); return(-1); }
    if (!acronyme_2) { Info_new( __func__, Config.log_msrv, LOG_ERR, "acronyme2 is NULL" ); return(1); }
    return( strcasecmp ( acronyme_1, acronyme_2 ) );
  }
/******************************************************************************************************************************/
/* MSRV_Map_to_thread: Met à jour à buffer json en mappant l'equivalent thread d'un bit interne local                         */
/* Entrée: FALSE si pas trouvé                                                                                                */
/******************************************************************************************************************************/
 gboolean MSRV_Map_to_thread ( JsonNode *key )
  { if (!Partage->Maps_to_thread)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "%s: Mapping is not done. Could not map '%s:%s' to thread.",
                 __func__, Json_get_string ( key, "tech_id" ), Json_get_string ( key, "acronyme" ) );
       return(FALSE);
     }
    JsonNode *map = g_tree_lookup ( Partage->Maps_to_thread, key );
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
  { if (!Partage->Maps_from_thread)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "%s: Mapping is not done. Could not map '%s:%s' from thread.",
                 __func__, Json_get_string ( key, "thread_tech_id" ), Json_get_string ( key, "thread_acronyme" ) );
       return(FALSE);
     }
    JsonNode *map = g_tree_lookup ( Partage->Maps_from_thread, key );
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

    Partage->Maps_root = Http_Post_to_global_API ( "/run/mapping/list", NULL );
    if (Partage->Maps_root && Json_get_int ( Partage->Maps_root, "api_status" ) == SOUP_STATUS_OK)
     { GList *Results = json_array_get_elements ( Json_get_array ( Partage->Maps_root, "mappings" ) );
       GList *results = Results;
       while(results)
        { JsonNode *element = results->data;
          g_tree_insert ( Partage->Maps_from_thread, element, element );
          g_tree_insert ( Partage->Maps_to_thread, element, element );
          results = g_list_next(results);
        }
       g_list_free(Results);
     } else { Json_node_unref ( Partage->Maps_root ); Partage->Maps_root = NULL; }
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
  }
/******************************************************************************************************************************/
/* Lire_ligne_commande: Parse la ligne de commande pour d'eventuels parametres                                                */
/* Entrée: argc, argv                                                                                                         */
/* Sortie: -1 si erreur, 0 si ok                                                                                              */
/******************************************************************************************************************************/
 static void Lire_ligne_commande( int argc, char *argv[] )
  { gint help = 0, log_level = -1, single = 0, version = 0, link = 0;
    gchar *api_url = NULL, *domain_uuid = NULL, *domain_secret = NULL, *agent_uuid_src = NULL;
    struct poptOption Options[]=
     { { "version",        'v', POPT_ARG_NONE,
         &version,          0, "Display Version Number", NULL },
       { "debug",          'd', POPT_ARG_INT,
         &log_level,      0, "Debug level", "LEVEL" },
       { "help",           'h', POPT_ARG_NONE,
         &help,             0, "Help", NULL },
       { "single",         's', POPT_ARG_NONE,
         &single,           0, "Don't start thread", NULL },
       { "link",           'l', POPT_ARG_NONE,
         &link,             0, "Link to API", NULL },
       { "api-url",        'A', POPT_ARG_STRING,
         &api_url,          0, "API Url (default is api.abls-habitat.fr)", NULL },
       { "domain-uuid",    'D', POPT_ARG_STRING,
         &domain_uuid,      0, "Domain to link to (mandatory)", NULL },
       { "domain-secret",  'S', POPT_ARG_STRING,
         &domain_secret,    0, "Domain secret (mandatory)", NULL },
       { "agent-uuid",     'U', POPT_ARG_STRING,
         &agent_uuid_src,   0, "Agent UUID (default is to create a new one)", NULL },
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

/*--------------------------------------------------------- Creation du fichier de config ------------------------------------*/
    if (link)
     { if (getuid()!=0)
        { printf(" You should be root to link to API\n" );
          exit(EXIT_OK);
        }

       if ( !domain_uuid )
        { printf(" You need 'domain_uuid' to link to API\n" );
          exit(EXIT_OK);
        }

       if ( !domain_secret )
        { printf(" You need 'domain_secret' to link to API\n" );
          exit(EXIT_OK);
        }

       if ( !api_url ) api_url = "https://api.abls-habitat.fr";
       if ( g_str_has_prefix ( api_url, "https://" ) ) api_url+=8;
       if ( g_str_has_prefix ( api_url, "http://"  ) ) api_url+=7;
       if ( g_str_has_prefix ( api_url, "wss://"   ) ) api_url+=6;
       if ( g_str_has_prefix ( api_url, "ws://"    ) ) api_url+=5;
       if (strlen(api_url)==0) api_url = "api.abls-habitat.fr";

       gchar agent_uuid[37];
       if ( agent_uuid_src ) g_snprintf( agent_uuid, sizeof(agent_uuid), "%s", agent_uuid_src );
       else UUID_New ( agent_uuid );
/******************************************* Création fichier de config *******************************************************/
       JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { Json_node_add_string( RootNode, "domain_uuid", domain_uuid );
          Json_node_add_string( RootNode, "domain_secret", domain_secret );
          Json_node_add_string( RootNode, "agent_uuid", agent_uuid );
          Json_node_add_string( RootNode, "api_url", api_url );
          Json_node_add_string( RootNode, "product", "agent" );
          Json_node_add_string( RootNode, "vendor", "abls-habitat.fr" );
          time_t t = time(NULL);
          struct tm *temps = localtime( &t );
          if (temps)
           { gchar date[64];
             strftime( date, sizeof(date), "%F %T", temps );
             Json_node_add_string( RootNode, "install_time", date );
           }
          Json_write_to_file ( "/etc/abls-habitat-agent.conf", RootNode );
          Json_node_unref(RootNode);
          printf(" Config file created, you can restart.\n" );
        }
       else { printf ("Writing config failed: Memory Error.\n" ); }
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
     { Info_new( __func__, Config.log_msrv, LOG_CRIT,
                "Error, running user is not 'root' Could not drop privileges.", getuid(), strerror(errno) );
       return(FALSE);
     }

    if (Config.headless)
     { pwd = getpwnam ( "watchdog" );
       if (!pwd)
        { Info_new( __func__, Config.log_msrv, LOG_CRIT,
                    "'watchdog' user not found while Headless, creating." );
          system("useradd -m -c 'WatchdogServer' watchdog" );
        }
       pwd = getpwnam ( "watchdog" );
       if (!pwd)
        { Info_new( __func__, Config.log_msrv, LOG_CRIT,
                   "Creation of user 'watchdog' failed (%s). Stopping.", strerror(errno) );
          return(FALSE);
        }
       system("loginctl enable-linger watchdog");                       /* Enable lingering for dbus and pipewire for example */
     }
    else /* When not headless */
     { gchar *session;
       uid_t active_session;
       if (sd_seat_get_active(	"seat0",	&session,	&active_session) < 0)
        { Info_new( __func__, Config.log_msrv, LOG_CRIT,
                    "seat_get_active failed (%s). Waiting 5s.", strerror (errno) );
          return(FALSE);
        }
       Info_new( __func__, Config.log_msrv, LOG_INFO,
                "session found = '%s' for user '%d'", session, active_session );
       pwd = getpwuid ( active_session );
       if (!pwd)
        { Info_new( __func__, Config.log_msrv, LOG_CRIT,
                   "Error when searching seat user. Stopping." );
          return(FALSE);
        }
     }
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Target User '%s' (uid %d) found.\n", pwd->pw_name, pwd->pw_uid );

    Info_new( __func__, Config.log_msrv, LOG_NOTICE,
             "Dropping from root to '%s' (%d).\n", pwd->pw_name, pwd->pw_uid );

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
        { Info_new( __func__, Config.log_msrv, LOG_CRIT, "%s: Error, cannot SetGroups for user '%s' (%s)\n",
                    __func__, pwd->pw_name, strerror(errno) );
          g_free(grp_list);
          return(FALSE);
        }
       g_free(grp_list);
     }

    if (setregid ( pwd->pw_gid, pwd->pw_gid )==-1)                                                  /* On drop les privilèges */
     { Info_new( __func__, Config.log_msrv, LOG_CRIT, "%s: Error, cannot setREgid for user '%s' (%s)\n",
                 __func__, pwd->pw_name, strerror(errno) );
       return(FALSE);
     }

    if (setreuid ( pwd->pw_uid, pwd->pw_uid )==-1)                                                  /* On drop les privilèges */
     { Info_new( __func__, Config.log_msrv, LOG_CRIT, "%s: Error, cannot setREuid for user '%s' (%s)\n",
                 __func__, pwd->pw_name, strerror(errno) );
       return(FALSE);
     }

    if ( Config.headless )
         { g_snprintf(Config.home, sizeof(Config.home), "%s", pwd->pw_dir ); }
    else { g_snprintf(Config.home, sizeof(Config.home), "%s/.watchdog", pwd->pw_dir ); }
    mkdir (Config.home, S_IRUSR | S_IWUSR | S_IXUSR );

    if (Config.instance_is_master)
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), "%s/Dls", Config.home );
       mkdir ( chaine, S_IRUSR | S_IWUSR | S_IXUSR );
       Info_new( __func__, Config.log_msrv, LOG_INFO, "Created Dls '%s' directory'", chaine );
     }

    if (chdir(Config.home))                                                             /* Positionnement à la racine du home */
     { Info_new( __func__, Config.log_msrv, LOG_CRIT, "Chdir %s failed\n", Config.home ); exit(EXIT_ERREUR); }
    else
     { Info_new( __func__, Config.log_msrv, LOG_INFO, "Chdir %s successfull. PID=%d\n", Config.home, getpid() ); }
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
    gint error_code = EXIT_OK;

    prctl(PR_SET_NAME, "W-INIT", 0, 0, 0 );
    umask(022);                                                                              /* Masque de creation de fichier */

    Lire_config();                                                     /* Lecture sur le fichier /etc/abls-habitat-agent.conf */
    Info_init( LOG_INFO );                                               /* Init msgs d'erreurs, par défaut, en mode LOG_INFO */
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Start %s, branche '%s'", WTD_VERSION, WTD_BRANCHE );
    Lire_ligne_commande( argc, argv );                                            /* Lecture du fichier conf et des arguments */

    Partage = Shm_init();                                                            /* Initialisation de la mémoire partagée */
    if (!Partage)
     { Info_new( __func__, Config.log_msrv, LOG_CRIT, "Shared memory failed to allocate" );
       error_code = EXIT_FAILURE;
       goto first_stage_end;
     }

    if ( Config.installed == FALSE )                                                    /* Si le fichier de conf n'existe pas */
     { Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Agent %s is not installed. Please run with --link.", WTD_VERSION );
       goto second_stage_end;
     }

/************************************************* Init libsoup session *******************************************************/
    Partage->com_msrv.API_session = soup_session_new_with_options( "idle_timeout", 0, "timeout", 10, "ssl-strict", TRUE,
                                                                   "user-agent", "Abls-habitat Agent", NULL );

/************************************************* Test Connexion to Global API ***********************************************/
    JsonNode *API = Http_Get_from_global_API ( "status", NULL );
    if (API)
     { Info_new( __func__, Config.log_msrv, LOG_INFO, "Connected with API %s", Json_get_string ( API, "version" ) );
       Json_node_unref ( API );
     }
    else
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Connection to Global API FAILED. Sleep 5s and stopping." );
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
       Json_node_add_string ( RootNode, "branche", WTD_BRANCHE );
       Json_node_add_string ( RootNode, "install_time", Json_get_string ( Config.config, "install_time" ) );

       JsonNode *api_result = Http_Post_to_global_API ( "/run/agent/start", RootNode );
       Json_node_unref ( RootNode );
       if (api_result && Json_get_int ( api_result, "api_status" ) == SOUP_STATUS_OK)
        { Info_new( __func__, Config.log_msrv, LOG_INFO, "API Request for AGENT START OK." ); }
       else
        { Info_new( __func__, Config.log_msrv, LOG_ERR, "API Request for AGENT START failed. Sleep 5s and stopping." );
          Json_node_unref ( api_result );
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

       Info_change_log_level ( Json_get_int ( api_result, "log_level" ) );
       Json_node_unref ( api_result );
     }
/******************************************************* Drop privileges ******************************************************/
    if (!Drop_privileges()) { sleep(5); goto second_stage_end; }

    Print_config();

    fd_lock = open( VERROU_SERVEUR, O_RDWR | O_CREAT | O_SYNC, 0640 );              /* Verification de l'unicité du processus */
    if (fd_lock<0)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Lock file creation failed: %s/%s.", Config.home, VERROU_SERVEUR );
       error_code = EXIT_FAILURE;
       goto second_stage_end;
     }
    if (flock( fd_lock, LOCK_EX | LOCK_NB )<0)                                         /* Creation d'un verrou sur le fichier */
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Cannot lock %s/%s. Probably another daemon is running : %s.",
                 Config.home, VERROU_SERVEUR, strerror(errno) );
       error_code = EXIT_FAILURE;
       goto third_stage_end;
     }
    fcntl(fd_lock, F_SETFD, FD_CLOEXEC );                                                           /* Set close on exec flag */
    g_snprintf( strpid, sizeof(strpid), "%d\n", getpid() );                                /* Enregistrement du pid au cas ou */
    if (write( fd_lock, strpid, strlen(strpid) )<0)
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Cannot write PID on %s/%s (%s).",
                 Config.home, VERROU_SERVEUR, strerror(errno) );
       error_code = EXIT_FAILURE;
       goto third_stage_end;
     }

    time ( &Partage->start_time );
    pthread_mutex_init( &Partage->com_msrv.synchro, NULL );                            /* Initialisation des mutex de synchro */
    pthread_mutex_init( &Partage->com_http.synchro, NULL );
    pthread_mutex_init( &Partage->com_dls.synchro, NULL );
    pthread_mutex_init( &Partage->com_dls.synchro_data, NULL );
    pthread_mutex_init( &Partage->archive_liste_sync, NULL );
    pthread_mutex_init( &Partage->com_db.synchro, NULL );

/************************************************* Gestion des signaux ********************************************************/
    sigfillset (&sig.sa_mask);                                                    /* Par défaut tous les signaux sont bloqués */
    pthread_sigmask( SIG_SETMASK, &sig.sa_mask, NULL );

/********************************************* Active les threads principaux **************************************************/
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Debut boucle sans fin" );
    Partage->com_msrv.Thread_run = TRUE;                                             /* On dit au maitre que le thread tourne */

    if (!Demarrer_http())                                                                                   /* Démarrage HTTP */
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Pb HTTP" ); }
    if (!Demarrer_api_sync())                                                                           /* Démarrage API_SYNC */
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Pb API_SYNC" ); }
    if (!Demarrer_arch_sync())                                                                         /* Démarrage ARCH_SYNC */
     { Info_new( __func__, Config.log_msrv, LOG_ERR, "Pb ARCH_SYNC" ); }
/***************************************** Demarrage des threads builtin et librairies ****************************************/
    if (Config.single)                                                                             /* Si demarrage des thread */
     { Info_new( __func__, Config.log_msrv, LOG_NOTICE, "NOT starting threads (single mode=true)" ); }
    else
     { if (!Config.installed)
        { Info_new( __func__, Config.log_msrv, LOG_ERR, "NOT starting threads (Instance is not installed)" ); }
       else
        { if (Config.instance_is_master)                                                                  /* Démarrage D.L.S. */
           { if (!Demarrer_dls()) Info_new( __func__, Config.log_msrv, LOG_ERR, "Pb DLS" );
             MSRV_Remap();                                                 /* Mappage des bits avant de charger les thread IO */
           }
          Charger_librairies();                                               /* Chargement de toutes les librairies Watchdog */
        }
     }

/*************************************** Mise en place de la gestion des signaux **********************************************/
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

    timer.it_value.tv_sec  = timer.it_interval.tv_sec  = 0;                                     /* Tous les 100 millisecondes */
    timer.it_value.tv_usec = timer.it_interval.tv_usec = 100000;                                    /* = 10 fois par secondes */
    setitimer( ITIMER_REAL, &timer, NULL );                                                                /* Active le timer */

/***************************************** Debut de la boucle sans fin ********************************************************/
    gint cpt_5_minutes = Partage->top + 3000;
    gint cpt_1_minute  = Partage->top + 600;

    sleep(1);
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Starting Main Thread" );

    if (Config.instance_is_master)
     { prctl(PR_SET_NAME, "W-MASTER", 0, 0, 0 );
       while(Partage->com_msrv.Thread_run == TRUE)                                        /* On tourne tant que l'on a besoin */
        { Gerer_arrive_Axxx_dls();                                        /* Distribution des changements d'etats sorties TOR */

          if (cpt_5_minutes < Partage->top)                                                 /* Update DB toutes les 5 minutes */
           {
             cpt_5_minutes += 3000;                                                        /* Sauvegarde toutes les 5 minutes */
           }

          if (cpt_1_minute < Partage->top)                                                    /* Update DB toutes les minutes */
           { Http_Send_ping_to_slaves();
             Print_SQL_status();                                                          /* Print SQL status for debugging ! */
             cpt_1_minute += 600;                                                            /* Sauvegarde toutes les minutes */
           }

          usleep(1000);
          sched_yield();
        }
     }
    else
     { prctl(PR_SET_NAME, "W-SLAVE", 0, 0, 0 );
       while(Partage->com_msrv.Thread_run == TRUE)                                        /* On tourne tant que l'on a besoin */
        { usleep(1000);
          sched_yield();
        }
     }
/*********************************** Terminaison: Deconnexion DB et kill des serveurs *****************************************/
    Info_new( __func__, Config.log_msrv, LOG_INFO, "fin boucle sans fin" );

    Decharger_librairies();                                                   /* Déchargement de toutes les librairies filles */
    Stopper_fils();                                                                        /* Arret de tous les fils watchdog */

    if (Partage->Maps_from_thread) g_tree_destroy ( Partage->Maps_from_thread );
    if (Partage->Maps_to_thread) g_tree_destroy ( Partage->Maps_to_thread );
    Json_node_unref ( Partage->Maps_root );

/************************************************* Dechargement des mutex *****************************************************/

    pthread_mutex_destroy( &Partage->com_msrv.synchro );
    pthread_mutex_destroy( &Partage->com_dls.synchro );
    pthread_mutex_destroy( &Partage->com_dls.synchro_data );
    pthread_mutex_destroy( &Partage->archive_liste_sync );
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
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Stopped" );
    return(error_code);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
