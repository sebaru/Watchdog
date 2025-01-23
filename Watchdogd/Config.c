/******************************************************************************************************************************/
/* Watchdogd/Config/Config.c        Lecture du fichier de configuration Watchdog                                              */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                     mer. 15 déc. 2010 13:30:12 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Config.c
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

 #include <stdio.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <string.h>
 #include <popt.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */

/******************************************************************************************************************************/
/* Lire_config : Lit la config Watchdog et remplit la structure mémoire                                                       */
/* Entrée: d'abord le fichier de config, puis l'environnement, puis les options en ligne de commandes                         */
/* Sortie: Mise à jour de la structure de configuration                                                                       */
/******************************************************************************************************************************/
 void Lire_config ( int argc, char *argv[] )
  { gchar *chaine;

    memset ( &Config, 0, sizeof(struct CONFIG) );
    g_snprintf( Config.master_hostname, sizeof(Config.master_hostname), "localhost" );
    g_snprintf( Config.librairie_dir,   sizeof(Config.librairie_dir),   "%s", DEFAUT_PROCESS_DIR   );
    Config.instance_is_master = TRUE;
    Config.log_level = LOG_INFO;

/********************************************** Read from config file first ***************************************************/
    Config.config = Json_read_from_file ( "/etc/abls-habitat-agent.conf" );
    if (!Config.config) Config.config = Json_node_create();

/********************************************** Then read from environ ********************************************************/
    chaine = getenv ( "ABLS_DOMAIN_UUID" );
    if (chaine) Json_node_add_string ( Config.config, "domain_uuid", chaine );

    chaine = getenv ( "ABLS_DOMAIN_SECRET" );
    if (chaine) Json_node_add_string ( Config.config, "domain_secret", chaine );

    chaine = getenv ( "ABLS_API_URL" );
    if (chaine)
     { if ( g_str_has_prefix ( chaine, "https://" ) ) chaine+=8;
       if ( g_str_has_prefix ( chaine, "http://"  ) ) chaine+=7;
       if ( g_str_has_prefix ( chaine, "wss://"   ) ) chaine+=6;
       if ( g_str_has_prefix ( chaine, "ws://"    ) ) chaine+=5;
       Json_node_add_string ( Config.config, "api_url", chaine );
     }

    chaine = getenv ( "ABLS_AGENT_UUID" );
    if (chaine) Json_node_add_string ( Config.config, "agent_uuid", chaine );

/********************************************** Finally from command line *****************************************************/
    gint help = 0, log_level = -1, single = 0, version = 0, save = 0;
    gchar *api_url = NULL, *domain_uuid = NULL, *domain_secret = NULL, *agent_uuid = NULL;
    struct poptOption Options[]=
     { { "version",        'v', POPT_ARG_NONE,   &version,       0, "Display Version Number", NULL },
       { "debug",          'd', POPT_ARG_INT,    &log_level,     0, "Set Debug level", "LEVEL" },
       { "help",           'h', POPT_ARG_NONE,   &help,          0, "Print help", NULL },
       { "single",         's', POPT_ARG_NONE,   &single,        0, "Don't start threads", NULL },
       { "save",           'l', POPT_ARG_NONE,   &save,          0, "Save config to file (erase previous config file)", NULL },
       { "api-url",        'A', POPT_ARG_STRING, &api_url,       0, "API Url (default is api.abls-habitat.fr)", "URL" },
       { "domain-uuid",    'D', POPT_ARG_STRING, &domain_uuid,   0, "Domain to link to", "UUID" },
       { "domain-secret",  'S', POPT_ARG_STRING, &domain_secret, 0, "Domain secret", "STRING" },
       { "agent-uuid",     'U', POPT_ARG_STRING, &agent_uuid,    0, "Agent UUID", "UUID" },
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
     { printf(" Abls-Agent (old-Watchdogd) - Version %s\n", WTD_VERSION );
       exit(EXIT_OK);
     }

    if ( domain_uuid )   Json_node_add_string ( Config.config, "domain_uuid",   domain_uuid );
    if ( domain_secret ) Json_node_add_string ( Config.config, "domain_secret", domain_secret );
    if ( agent_uuid )    Json_node_add_string ( Config.config, "agent_uuid",    agent_uuid );
    if ( api_url )
     { if ( g_str_has_prefix ( api_url, "https://" ) ) api_url+=8;
       if ( g_str_has_prefix ( api_url, "http://"  ) ) api_url+=7;
       if ( g_str_has_prefix ( api_url, "wss://"   ) ) api_url+=6;
       if ( g_str_has_prefix ( api_url, "ws://"    ) ) api_url+=5;
       Json_node_add_string ( Config.config, "api_url", api_url );
     }

/******************************************* Controle final *******************************************************************/
    if (!Json_has_member ( Config.config, "api_url" )) Json_node_add_string ( Config.config, "api_url", "api.abls-habitat.fr" );
    if (!Json_has_member ( Config.config, "domain_uuid" ))
     { printf(" Error: domain_uuid is missing. Add it in environ, config file or command line options\n" );
       exit ( EXIT_ERREUR );
     }
    if (!Json_has_member ( Config.config, "domain_secret" ))
     { printf(" Error: domain_secret is missing. Add it in environ, config file or command line options\n" );
       exit ( EXIT_ERREUR );
     }
    if (!Json_has_member ( Config.config, "agent_uuid" ))
     { printf(" Creating new agent_uuid\n" );
       gchar agent_uuid_new[37];
       UUID_New ( agent_uuid_new );
       Json_node_add_string ( Config.config, "agent_uuid", agent_uuid_new );
     }

/******************************************* Création fichier de config *******************************************************/
    if (save)
     { JsonNode *RootNode = Json_node_create ();
       if (RootNode)
        { Json_node_add_string( RootNode, "domain_uuid",   Json_get_string ( Config.config, "domain_uuid" ) );
          Json_node_add_string( RootNode, "domain_secret", Json_get_string ( Config.config, "domain_secret" ) );
          Json_node_add_string( RootNode, "agent_uuid",    Json_get_string ( Config.config, "agent_uuid" ) );
          Json_node_add_string( RootNode, "api_url",       Json_get_string ( Config.config, "api_url" ) );
          Json_node_add_string( RootNode, "product",       "agent" );
          Json_node_add_string( RootNode, "vendor",        "abls-habitat.fr" );
          Json_write_to_file ( "/etc/abls-habitat-agent.conf", RootNode );
          Json_node_unref(RootNode);
        }
       else { printf ("Writing config failed: Memory Error.\n" ); exit (EXIT_ERREUR); }
       printf(" Config file created, you can restart agent.\n" );
       exit(EXIT_OK);
     }

    if (single)          Config.single      = TRUE;                                            /* Demarrage en mode single ?? */
    if (log_level!=-1)   Config.log_level   = log_level;
    fflush(0);
  }
/******************************************************************************************************************************/
/* Print_config: Affichage (enfin log) la config actuelle en parametre                                                        */
/* Entrée: une config !! -> le champ log doit etre initialisé via la librairie Erreur                                         */
/******************************************************************************************************************************/
 void Print_config ( void )
  {
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config file                 %s", Config.config_file );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config single               %d", Config.single );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config headless             %d", Config.headless );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config log_level            %d", Config.log_level );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config log_bus              %d", Config.log_bus );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config log_msrv             %d", Config.log_msrv );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config home                 %s", Config.home );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config instance             %s", g_get_host_name() );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config instance is master   %d", Config.instance_is_master );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config master_hostname      %s", Config.master_hostname );
    Info_new( __func__, Config.log_msrv, LOG_INFO, "Config librairie_dir        %s", Config.librairie_dir );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
