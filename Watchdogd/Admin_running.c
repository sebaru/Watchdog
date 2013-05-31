/**********************************************************************************************************/
/* Watchdogd/Admin/admin_running.c        Gestion des connexions Admin RUNNING au serveur watchdog        */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 17 nov. 2010 20:00:45 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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
 #include <unistd.h>                                                                  /* Pour gethostname */
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Admin_running: Appellée lorsque l'admin envoie une commande en mode run dans la ligne de commande      */
/* Entrée: La connexion connexione et la ligne de commande, et le buffer de sortie                           */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_running ( struct CONNEXION *connexion, gchar *ligne )
  { struct LIBRAIRIE *lib;
    GSList *liste;
    gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'running'\n" );
       Admin_write ( connexion, "  ident                 - ID du serveur Watchdog\n" );
       Admin_write ( connexion, "  ping                  - Ping Watchdog\n" );
       Admin_write ( connexion, "  audit                 - Audit bit/s\n" );
       Admin_write ( connexion, "  clear_histo           - Clear Histo DB\n" );
       Admin_write ( connexion, "  get                   - Sous-menu de lecture des bits internes\n" );
       Admin_write ( connexion, "  set                   - Sous-menu d'affectation des bits internes\n" );
       Admin_write ( connexion, "  setrootpasswd         - Set the Watchdog root password\n" );
       Admin_write ( connexion, "  modbus                - Sous-menu de gestion des equipements MODBUS\n" );
       Admin_write ( connexion, "  dls                   - D.L.S. Status\n" );
       Admin_write ( connexion, "  log_level loglevel    - Set Log Level (debug, info, notice, warning, error)\n" );
       Admin_write ( connexion, "  log switch            - Switch log (list, all, none, dls, arch, db, msrv or library name)\n" );

       liste = Partage->com_msrv.Librairies;                           /* Parcours de toutes les librairies */
       while(liste)
        { lib = (struct LIBRAIRIE *)liste->data;
          memset ( chaine, ' ', sizeof(chaine) );
          memcpy ( chaine + 2, lib->admin_prompt, strlen(lib->admin_prompt) );
          memcpy ( chaine + 24, "-", 1 );
          memcpy ( chaine + 26, lib->admin_help, strlen(lib->admin_help) );
          memcpy ( chaine + 26 + strlen(lib->admin_help), "\n", 2 );
          Admin_write ( connexion, chaine );
          liste = liste->next;
        }

       Admin_write ( connexion, "  help                  - This help\n" );
     } else
    if ( ! strcmp ( commande, "ident" ) )
     { char nom[128];
       gethostname( nom, sizeof(nom) );
       g_snprintf( chaine, sizeof(chaine), " Watchdogd %s on %s\n", VERSION, nom );
       Admin_write ( connexion, chaine );
     } else
    if ( ! strcmp ( commande, "setrootpasswd" ) )
     { struct CMD_TYPE_UTILISATEUR util;
       gchar password[80];
       struct DB *db;

       sscanf ( ligne, "%s %s", commande, password );                /* Découpage de la ligne de commande */

       db = Init_DB_SQL();       
       if (!db)
        { g_snprintf( chaine, sizeof(chaine), " Unable to connect to Database\n" );
          Admin_write ( connexion, chaine );
        }
       else
        { util.id = 0;
          g_snprintf( util.nom, sizeof(util.nom), "root" );
          g_snprintf( util.commentaire, sizeof(util.commentaire), "Watchdog root user" );
          util.cansetpass = TRUE;
          util.setpassnow = TRUE;
          g_snprintf( util.code_en_clair, sizeof(util.code_en_clair), "%s", password );
          util.actif = TRUE;
          util.expire = FALSE;
          util.changepass = FALSE;
          memset ( &util.gids, 0, sizeof(util.gids) );
          if( Modifier_utilisateurDB( Config.crypto_key, &util ) )
           { g_snprintf( chaine, sizeof(chaine), " Password set\n" ); }
          else
           { g_snprintf( chaine, sizeof(chaine), " Error while setting password\n" ); }
          Admin_write ( connexion, chaine );
          Libere_DB_SQL( &db );
        }
     } else
    if ( ! strcmp ( commande, "clear_histo" ) )
     { Clear_histoDB ();                                            /* Clear de la table histo au boot */
       g_snprintf( chaine, sizeof(chaine), " HistoDB cleared\n" );
          Admin_write ( connexion, chaine );
     } else
    if ( ! strcmp ( commande, "audit" ) )
     { gint num;
       g_snprintf( chaine, sizeof(chaine), " -- Audit de performance\n" );
       Admin_write ( connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Bit/s        : %d\n", Partage->audit_bit_interne_per_sec_hold );
       Admin_write ( connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Tour/s       : %d\n", Partage->audit_tour_dls_per_sec_hold );
       Admin_write ( connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_slist_length( Partage->com_msrv.liste_i );                  /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " Distribution des I      : reste %d\n", num );
       Admin_write ( connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_slist_length( Partage->com_msrv.liste_msg );                /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " Distribution des Msg: reste %d\n", num );
       Admin_write ( connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );                /* Parcours de la liste a traiter */
       num = g_slist_length( Partage->com_msrv.liste_msg_repeat );                    /* liste des repeat */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), "          MSgs en REPEAT : reste %d\n", num );
       Admin_write ( connexion, chaine );
     } else
    if ( ! strcmp ( commande, "log_level" ) )
     { gchar debug[128], chaine [80];
       sscanf ( ligne, "%s %s", commande, debug );
       g_snprintf( chaine, sizeof(chaine), " Log level set to %s\n", debug );
       if ( ! strcmp ( debug, "debug"    ) )
        { Info_change_log_level ( Config.log, LOG_DEBUG   ); } else
       if ( ! strcmp ( debug, "info"  ) )
        { Info_change_log_level ( Config.log, LOG_INFO    ); } else
       if ( ! strcmp ( debug, "notice"  ) )
        { Info_change_log_level ( Config.log, LOG_NOTICE  ); } else
       if ( ! strcmp ( debug, "warning" ) )
        { Info_change_log_level ( Config.log, LOG_WARNING ); } else
       if ( ! strcmp ( debug, "error"   ) )
        { Info_change_log_level ( Config.log, LOG_ERR     ); }
       else g_snprintf( chaine, sizeof(chaine), " -- Unknown log level %s\n", debug );
       Admin_write ( connexion, chaine );
     } else
    if ( ! strcmp ( commande, "log" ) )
     { gchar debug[128];

       sscanf ( ligne, "%s %s", commande, debug );

       if ( ! strcmp ( debug, "all"       ) )
        { Config.log_msrv = TRUE;
          Config.log_db   = TRUE;
          Config.log_dls  = TRUE;
          Config.log_arch = TRUE;
          liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             lib->Thread_debug = TRUE;
             g_snprintf( chaine, sizeof(chaine), "  -> Log enabled for library %s (%s)\n",
                         lib->admin_prompt, lib->nom_fichier );
             Admin_write ( connexion, chaine );
             liste = liste->next;
           }
        } else
       if ( ! strcmp ( debug, "none"      ) )
        { Config.log_msrv = FALSE;
          Config.log_db   = FALSE;
          Config.log_dls  = FALSE;
          Config.log_arch = FALSE;
          liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             lib->Thread_debug = FALSE;
             g_snprintf( chaine, sizeof(chaine), "  -> Log disabled for library %s (%s)\n",
                         lib->admin_prompt, lib->nom_fichier );
             Admin_write ( connexion, chaine );
             liste = liste->next;
           }
        } else
       if ( ! strcmp ( debug, "list"      ) )
        { liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             g_snprintf( chaine, sizeof(chaine), "  -> Log is %s for library %s (%s)\n",
                         (lib->Thread_debug ? " enabled" : "disabled"),
                         lib->admin_prompt, lib->nom_fichier );
             Admin_write ( connexion, chaine );
             liste = liste->next;
           }
          g_snprintf( chaine, sizeof(chaine), "  -> Log is %s for db\n",
                      (Config.log_db ? " enabled" : "disabled") );
          Admin_write ( connexion, chaine );
          g_snprintf( chaine, sizeof(chaine), "  -> Log is %s for dls\n",
                      (Config.log_dls ? " enabled" : "disabled") );
          Admin_write ( connexion, chaine );
          g_snprintf( chaine, sizeof(chaine), "  -> Log is %s for arch\n",
                      (Config.log_arch ? " enabled" : "disabled") );
          Admin_write ( connexion, chaine );
          g_snprintf( chaine, sizeof(chaine), "  -> Log is %s for msrv\n",
                      (Config.log_msrv ? " enabled" : "disabled") );
          Admin_write ( connexion, chaine );
        } else
       if ( ! strcmp ( debug, "db"   ) )
        { if (Config.log_db == TRUE) Config.log_db = FALSE;
          else Config.log_db = TRUE;
          g_snprintf( chaine, sizeof(chaine), "  -> Log is now %s for db\n",
                      (Config.log_db ? " enabled" : "disabled") );
          Admin_write ( connexion, chaine );
        }
       if ( ! strcmp ( debug, "dls"   ) )
        { if (Config.log_dls == TRUE) Config.log_dls = FALSE;
          else Config.log_dls = TRUE;
          g_snprintf( chaine, sizeof(chaine), "  -> Log is now %s for dls\n",
                      (Config.log_dls ? " enabled" : "disabled") );
          Admin_write ( connexion, chaine );
        }
       if ( ! strcmp ( debug, "arch"   ) )
        { if (Config.log_arch == TRUE) Config.log_arch = FALSE;
          else Config.log_arch = TRUE;
          g_snprintf( chaine, sizeof(chaine), "  -> Log is now %s for arch\n",
                      (Config.log_arch ? " enabled" : "disabled") );
          Admin_write ( connexion, chaine );
        }
       if ( ! strcmp ( debug, "msrv"   ) )
        { if (Config.log_msrv == TRUE) Config.log_msrv = FALSE;
          else Config.log_msrv = TRUE;
          g_snprintf( chaine, sizeof(chaine), "  -> Log is now %s for msrv\n",
                      (Config.log_msrv ? " enabled" : "disabled") );
          Admin_write ( connexion, chaine );
        }
       else
        { liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             if ( ! strcmp ( debug, lib->admin_prompt ) )
              { if (lib->Thread_debug == TRUE)
                 { lib->Thread_debug = FALSE;
                   g_snprintf( chaine, sizeof(chaine), "  -> Log disabled for library %s (%s)\n",
                               lib->admin_prompt, lib->nom_fichier );
                 }
                else
                 { lib->Thread_debug = TRUE;
                   g_snprintf( chaine, sizeof(chaine), "  -> Log enabled for library %s (%s)\n",
                               lib->admin_prompt, lib->nom_fichier );
                 } 
                Admin_write ( connexion, chaine );
                break;
              }
             liste = liste->next;
           }
          if ( liste == NULL )                                       /* Si l'on a pas trouve de librairie */
           { g_snprintf( chaine, sizeof(chaine), " -- Unknown debug switch\n" );
             Admin_write ( connexion, chaine );
           }
        }
     } else
    if ( ! strcmp ( commande, "ping" ) )
     { Admin_write ( connexion, " Pong !\n" );
     } else
    if ( ! strcmp ( commande, "nocde" ) )
     { g_snprintf( chaine, sizeof(chaine), "\n" );
       Admin_write ( connexion, chaine );
     } else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
