/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_running.c        Gestion des responses Admin RUNNING au serveur watchdog                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         mer. 17 nov. 2010 20:00:45 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_running.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 
 #include <glib.h>
 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Admin_running: Appellée lorsque l'admin envoie une commande en mode run dans la ligne de commande                          */
/* Entrée: La response responsee et la ligne de commande, et le buffer de sortie                                            */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_running ( gchar *response, gchar *ligne )
  { struct LIBRAIRIE *lib;
    GSList *liste;
    gchar commande[128], chaine[256];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'running'" );
       response = Admin_write ( response, "  ident                 - ID du serveur Watchdog" );
       response = Admin_write ( response, "  ping                  - Ping Watchdog" );
       response = Admin_write ( response, "  audit                 - Audit bit/s" );
       response = Admin_write ( response, "  dbcfg $thread         - Manage Threads Parameters in Database" );
       response = Admin_write ( response, "  clear_histo           - Clear Histo DB" );
       response = Admin_write ( response, "  get                   - Sous-menu de lecture des bits internes" );
       response = Admin_write ( response, "  set                   - Sous-menu d'affectation des bits internes" );
       response = Admin_write ( response, "  modbus                - Sous-menu de gestion des equipements MODBUS" );
       response = Admin_write ( response, "  dls                   - D.L.S. Status" );
       response = Admin_write ( response, "  user                  - Manage Watchdog Users" );
       response = Admin_write ( response, "  update_schemaDB       - Update Database Schema" );
       response = Admin_write ( response, "  reload_confDB         - Reload Database conf for internals" );
       response = Admin_write ( response, "  log_level $loglevel   - Set Log Level (debug, info, notice, warning, error)" );
       response = Admin_write ( response, "  debug $switch         - Toggle debug Switch (list, all, none, dls, archive, db, msrv or library name)" );

       liste = Partage->com_msrv.Librairies;                                             /* Parcours de toutes les librairies */
       while(liste)
        { lib = (struct LIBRAIRIE *)liste->data;
          memset ( chaine, ' ', sizeof(chaine) );
          memcpy ( chaine + 2, lib->admin_prompt, strlen(lib->admin_prompt) );
          memcpy ( chaine + 24, "-", 1 );
          memcpy ( chaine + 26, lib->admin_help, strlen(lib->admin_help) );
          memcpy ( chaine + 26 + strlen(lib->admin_help), "", 2 );
          response = Admin_write ( response, chaine );
          liste = liste->next;
        }

       response = Admin_write ( response, "  help                  - This help" );
     } else
    if ( ! strcmp ( commande, "ident" ) )
     { gchar date[128], nom[128];
       struct tm *temps;
       gint num;
       temps = localtime( (time_t *)&Partage->start_time );
       if (temps) { strftime( date, sizeof(date), "%F %T", temps ); }
       else       { g_snprintf( date, sizeof(date), "Erreur" ); }
       gethostname( nom, sizeof(nom) );
       g_snprintf( chaine, sizeof(chaine),
                   " | - Watchdogd %s Instance '%s' started on '%s' (PID=%d)\n"
                   " | - Version (%s) running on %s\n"
                   " | - run as '%s' (uid=%d)\n"
                   " | - home = '%s'",
                   (Config.instance_is_master ? "Master" : "Slave"), g_get_host_name(), date, getpid(),
                   VERSION, nom, 
                   Config.run_as, getuid(),
                   Config.home );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "dbcfg" ) )
     { gchar chaine[128];
       if (sscanf ( ligne, "%s %s", commande, chaine ) != 2) return(response);
       return(Admin_dbcfg( response, chaine ));
     } else
    if ( ! strcmp ( commande, "clear_histo" ) )
     { Clear_histoDB ();                                                                   /* Clear de la table histo au boot */
       g_snprintf( chaine, sizeof(chaine), " | - HistoDB cleared" );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "update_schemaDB" ) )
     { Update_database_schema ();
       g_snprintf( chaine, sizeof(chaine), " | - Update Schema done" );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "reload_confDB" ) )
     { Charger_config_bit_interne ();                         /* Chargement des configurations des bits internes depuis la DB */
       g_snprintf( chaine, sizeof(chaine), " | - Reload done" );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "audit" ) )
     { gint num;

       g_snprintf( chaine, sizeof(chaine), " -- Audit de performance -- " );
       response = Admin_write ( response, chaine );

       g_snprintf( chaine, sizeof(chaine), " | - Bit/s                : %d", Partage->audit_bit_interne_per_sec_hold );
       response = Admin_write ( response, chaine );

       g_snprintf( chaine, sizeof(chaine), " | - Tour/s               : %d", Partage->audit_tour_dls_per_sec_hold );
       response = Admin_write ( response, chaine );

       g_snprintf( chaine, sizeof(chaine), " | - Archive to Proceed   : %d", Partage->com_arch.taille_arch );
       response = Admin_write ( response, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_slist_length( Partage->com_msrv.liste_i );                  /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " | - Distribution des I   : reste %d", num );
       response = Admin_write ( response, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_slist_length( Partage->com_msrv.liste_msg );                /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " | - Distribution des Msg : reste %d", num );
       response = Admin_write ( response, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );                /* Parcours de la liste a traiter */
       num = g_slist_length( Partage->com_msrv.liste_msg_repeat );                    /* liste des repeat */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " | - MSgs en REPEAT       : reste %d", num );
       response = Admin_write ( response, chaine );

     } else
    if ( ! strcmp ( commande, "log_level" ) )
     { gchar debug[128], chaine [128];
       if (sscanf ( ligne, "%s %s", commande, debug ) != 2) return(response);
       g_snprintf( chaine, sizeof(chaine), " | - Log level set to %s", debug );
       if ( ! strcmp ( debug, "debug"    ) )
        { Info_change_log_level ( Config.log, LOG_DEBUG   ); }
       else if ( ! strcmp ( debug, "info"  ) )
        { Info_change_log_level ( Config.log, LOG_INFO    ); }
       else if ( ! strcmp ( debug, "notice"  ) )
        { Info_change_log_level ( Config.log, LOG_NOTICE  ); }
       else if ( ! strcmp ( debug, "warning" ) )
        { Info_change_log_level ( Config.log, LOG_WARNING ); }
       else if ( ! strcmp ( debug, "error"   ) )
        { Info_change_log_level ( Config.log, LOG_ERR     ); }
       else g_snprintf( chaine, sizeof(chaine),
                       " | - -- Unknown log level %s. Valid level are : debug, info, notice, warning, error",
                        debug );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "debug" ) )
     { gchar debug[128];

       if (sscanf ( ligne, "%s %s", commande, debug ) != 2) return(response);
       g_snprintf( chaine, sizeof(chaine),
                       " | - Log level is %d", Config.log->log_level );
       response = Admin_write ( response, chaine );

       if ( ! strcmp ( debug, "all"       ) )
        { Config.log_msrv = TRUE;
          Config.log_db   = TRUE;
          Config.log_dls  = TRUE;
          Config.log_arch = TRUE;
          liste = Partage->com_msrv.Librairies;                                          /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             lib->Thread_debug = TRUE;
             g_snprintf( chaine, sizeof(chaine), " | - Debug enabled for library %s (%s)",
                         lib->admin_prompt, lib->nom_fichier );
             response = Admin_write ( response, chaine );
             liste = liste->next;
           }
        } else
       if ( ! strcmp ( debug, "none"      ) )
        { Config.log_msrv = FALSE;
          Config.log_db   = FALSE;
          Config.log_dls  = FALSE;
          Config.log_arch = FALSE;
          liste = Partage->com_msrv.Librairies;                                          /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             lib->Thread_debug = FALSE;
             g_snprintf( chaine, sizeof(chaine), " | - Debug disabled for library %s (%s)",
                         lib->admin_prompt, lib->nom_fichier );
             response = Admin_write ( response, chaine );
             liste = liste->next;
           }
        } else
       if ( ! strcmp ( debug, "list"      ) )
        { liste = Partage->com_msrv.Librairies;                                          /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             g_snprintf( chaine, sizeof(chaine), " | - Debug is %s for library %s (%s)",
                         (lib->Thread_debug ? " enabled" : "disabled"),
                         lib->admin_prompt, lib->nom_fichier );
             response = Admin_write ( response, chaine );
             liste = liste->next;
           }
          g_snprintf( chaine, sizeof(chaine), " | - Debug is %s for db",   (Config.log_db ? " enabled" : "disabled") );
          response = Admin_write ( response, chaine );
          g_snprintf( chaine, sizeof(chaine), " | - Debug is %s for dls",  (Config.log_dls ? " enabled" : "disabled") );
          response = Admin_write ( response, chaine );
          g_snprintf( chaine, sizeof(chaine), " | - Debug is %s for archive", (Config.log_arch ? " enabled" : "disabled") );
          response = Admin_write ( response, chaine );
          g_snprintf( chaine, sizeof(chaine), " | - Debug is %s for msrv", (Config.log_msrv ? " enabled" : "disabled") );
          response = Admin_write ( response, chaine );
        } else
       if ( ! strcmp ( debug, "db"   ) )
        { if (Config.log_db == TRUE) Config.log_db = FALSE;
          else Config.log_db = TRUE;
          g_snprintf( chaine, sizeof(chaine), " | - Debug is now %s for db", (Config.log_db ? " enabled" : "disabled") );
          response = Admin_write ( response, chaine );
        } else
       if ( ! strcmp ( debug, "dls"   ) )
        { if (Config.log_dls == TRUE) Config.log_dls = FALSE;
          else Config.log_dls = TRUE;
          g_snprintf( chaine, sizeof(chaine), " | - Debug is now %s for dls", (Config.log_dls ? " enabled" : "disabled") );
          response = Admin_write ( response, chaine );
        } else
       if ( ! strcmp ( debug, "archive"   ) )
        { if (Config.log_arch == TRUE) Config.log_arch = FALSE;
          else Config.log_arch = TRUE;
          g_snprintf( chaine, sizeof(chaine), " | - Debug is now %s for archive", (Config.log_arch ? " enabled" : "disabled") );
          response = Admin_write ( response, chaine );
        } else
       if ( ! strcmp ( debug, "msrv"   ) )
        { if (Config.log_msrv == TRUE) Config.log_msrv = FALSE;
          else Config.log_msrv = TRUE;
          g_snprintf( chaine, sizeof(chaine), " | - Debug is now %s for msrv", (Config.log_msrv ? " enabled" : "disabled") );
          response = Admin_write ( response, chaine );
        }
       else
        { liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             if ( ! strcmp ( debug, lib->admin_prompt ) )
              { if (lib->Thread_debug == TRUE)
                 { lib->Thread_debug = FALSE;
                   g_snprintf( chaine, sizeof(chaine), " | - Debug disabled for library %s (%s)",
                               lib->admin_prompt, lib->nom_fichier );
                 }
                else
                 { lib->Thread_debug = TRUE;
                   g_snprintf( chaine, sizeof(chaine), " | - Debug enabled for library %s (%s)",
                               lib->admin_prompt, lib->nom_fichier );
                 } 
                response = Admin_write ( response, chaine );
                break;
              }
             liste = liste->next;
           }
          if ( liste == NULL )                                       /* Si l'on a pas trouve de librairie */
           { g_snprintf( chaine, sizeof(chaine), " | - Unknown debug switch" );
             response = Admin_write ( response, chaine );
           }
        }
     } else
    if ( ! strcmp ( commande, "ping" ) )
     { response = Admin_write ( response, " | - Pong !" );
     } else
    if ( ! strcmp ( commande, "nocde" ) )
     { g_snprintf( chaine, sizeof(chaine), "" );
       response = Admin_write ( response, chaine );
     } else
     { g_snprintf( chaine, sizeof(chaine), " | - Unknown command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
   return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
