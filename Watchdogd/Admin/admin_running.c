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
/* Ecouter_admin: Ecoute ce que dis le client                                                             */
/* Entrée: le client                                                                                      */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_running ( struct CLIENT_ADMIN *client, gchar *ligne )
  { struct LIBRAIRIE *lib;
    GSList *liste;
    gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion, "  -- Watchdog ADMIN -- Help du mode 'running'\n" );
       Write_admin ( client->connexion, "  ident                 - ID du serveur Watchdog\n" );
       Write_admin ( client->connexion, "  ping                  - Ping Watchdog\n" );
       Write_admin ( client->connexion, "  audit                 - Audit bit/s\n" );
       Write_admin ( client->connexion, "  ssrv                  - SousServers Status\n" );
       Write_admin ( client->connexion, "  client                - Client Status\n" );
       Write_admin ( client->connexion, "  kick nom_machine      - Kick client nom@machine\n" );
       Write_admin ( client->connexion, "  clear_histo           - Clear Histo DB\n" );
       Write_admin ( client->connexion, "  get                   - Sous-menu de lecture des bits internes\n" );
       Write_admin ( client->connexion, "  set                   - Sous-menu d'affectation des bits internes\n" );
       Write_admin ( client->connexion, "  msgs message          - Envoi d'un message a tous les clients\n" );
       Write_admin ( client->connexion, "  setrootpasswd         - Set the Watchdog root password\n" );
       Write_admin ( client->connexion, "  modbus                - Sous-menu de gestion des equipements MODBUS\n" );
       Write_admin ( client->connexion, "  dls                   - D.L.S. Status\n" );
       Write_admin ( client->connexion, "  log_level loglevel    - Set Log Level (debug, info, notice, warning, error)\n");
       Write_admin ( client->connexion, "  log switch            - Switch log (list, all, none, process name or library name)\n" );

       liste = Partage->com_msrv.Librairies;                           /* Parcours de toutes les librairies */
       while(liste)
        { lib = (struct LIBRAIRIE *)liste->data;
          memset ( chaine, ' ', sizeof(chaine) );
          memcpy ( chaine + 2, lib->admin_prompt, strlen(lib->admin_prompt) );
          memcpy ( chaine + 24, "-", 1 );
          memcpy ( chaine + 26, lib->admin_help, strlen(lib->admin_help) );
          memcpy ( chaine + 26 + strlen(lib->admin_help), "\n", 2 );
          Write_admin ( client->connexion, chaine );
          liste = liste->next;
        }

       Write_admin ( client->connexion, "  help                  - This help\n" );
     } else
    if ( ! strcmp ( commande, "ident" ) )
     { char nom[128];
       gethostname( nom, sizeof(nom) );
       g_snprintf( chaine, sizeof(chaine), " Watchdogd %s on %s\n", VERSION, nom );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "ssrv" ) )
     { int i;

       g_snprintf( chaine, sizeof(chaine), " Jeton au SSRV %02d\n", Partage->jeton );
       Write_admin ( client->connexion, chaine );

       for (i=0; i<Config.max_serveur; i++)
        { g_snprintf( chaine, sizeof(chaine), " SSRV[%02d] -> %02d clients\n",
                      i, Partage->Sous_serveur[i].nb_client );
          Write_admin ( client->connexion, chaine );
        }
     } else
    if ( ! strcmp ( commande, "client" ) )
     { GList *liste;
       gint i;
        
       g_snprintf( chaine, sizeof(chaine), " -- Liste des clients connectés au serveur\n" );
       Write_admin ( client->connexion, chaine );
       for (i=0; i<Config.max_serveur; i++)
         { if (Partage->Sous_serveur[i].Thread_run == FALSE) continue;

           pthread_mutex_lock( &Partage->Sous_serveur[i].synchro );
           liste = Partage->Sous_serveur[i].Clients;
           while(liste)                                               /* Parcours de la liste des clients */
            { struct CLIENT *client_srv;
              client_srv = (struct CLIENT *)liste->data;

              g_snprintf( chaine, sizeof(chaine), " SSRV%02d - v%s %s@%s - mode %d defaut %d date %s",
                          i, client_srv->ident.version, client_srv->util->nom, client_srv->machine,
                          client_srv->mode, client_srv->defaut, ctime(&client_srv->date_connexion) );
              Write_admin ( client->connexion, chaine );                /* ctime ajoute un \n à la fin !! */

              liste = liste->next;
            }
           pthread_mutex_unlock( &Partage->Sous_serveur[i].synchro );
         }
     } else
    if ( ! strcmp ( commande, "msgs" ) )
     { GList *liste;
       gint i;

       g_snprintf( chaine, sizeof(chaine), " -- Liste des clients recevant le message\n" );
       Write_admin ( client->connexion, chaine );
       for (i=0; i<Config.max_serveur; i++)
         { if (Partage->Sous_serveur[i].Thread_run == FALSE) continue;
           liste = Partage->Sous_serveur[i].Clients;
           while(liste)                                               /* Parcours de la liste des clients */
            { struct CMD_GTK_MESSAGE erreur;
              struct CLIENT *client_wat;
              client_wat = (struct CLIENT *)liste->data;

              g_snprintf( erreur.message, sizeof(erreur.message), "AdminMSG : %s", ligne + 5 );
              Envoi_client( client_wat, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );

              g_snprintf( chaine, sizeof(chaine), " - %s@%s\n",
                          client_wat->util->nom, client_wat->machine );
              Write_admin ( client->connexion, chaine );
              liste = liste->next;
            }
         }
     } else
    if ( ! strcmp ( commande, "setrootpasswd" ) )
     { struct CMD_TYPE_UTILISATEUR util;
       gchar password[80];
       struct DB *db;

       sscanf ( ligne, "%s %s", commande, password );                /* Découpage de la ligne de commande */

       db = Init_DB_SQL( Config.log );
       if (!db)
        { g_snprintf( chaine, sizeof(chaine), " Unable to connect to Database\n" );
          Write_admin ( client->connexion, chaine );
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
          if( Modifier_utilisateurDB( Config.log, db, Config.crypto_key, &util ) )
           { g_snprintf( chaine, sizeof(chaine), " Password set\n" ); }
          else
           { g_snprintf( chaine, sizeof(chaine), " Error while setting password\n" ); }
          Write_admin ( client->connexion, chaine );
          Libere_DB_SQL( Config.log, &db );
        }
     } else
    if ( ! strcmp ( commande, "clear_histo" ) )
     { Clear_histoDB ();                                            /* Clear de la table histo au boot */
       g_snprintf( chaine, sizeof(chaine), " HistoDB cleared\n" );
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "audit" ) )
     { gint num;
       g_snprintf( chaine, sizeof(chaine), " -- Audit de performance\n" );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Partage->Top : %d\n", Partage->top );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Bit/s        : %d\n", Partage->audit_bit_interne_per_sec_hold );
       Write_admin ( client->connexion, chaine );

       g_snprintf( chaine, sizeof(chaine), " Tour/s       : %d\n", Partage->audit_tour_dls_per_sec_hold );
       Write_admin ( client->connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_slist_length( Partage->com_msrv.liste_i );                  /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " Distribution des I      : reste %d\n", num );
       Write_admin ( client->connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_slist_length( Partage->com_msrv.liste_msg_off );            /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " Distribution des Msg OFF: reste %d\n", num );
       Write_admin ( client->connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_slist_length( Partage->com_msrv.liste_msg_on );             /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " Distribution des Msg ON : reste %d\n", num );
       Write_admin ( client->connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );                /* Parcours de la liste a traiter */
       num = g_slist_length( Partage->com_msrv.liste_msg_repeat );                    /* liste des repeat */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), "          MSgs en REPEAT : reste %d\n", num );
       Write_admin ( client->connexion, chaine );
       
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
       Write_admin ( client->connexion, chaine );
     } else
    if ( ! strcmp ( commande, "log" ) )
     { gchar debug[128];

       sscanf ( ligne, "%s %s", commande, debug );

       if ( ! strcmp ( debug, "all"       ) )
        { Config.log_all = TRUE;
          Config.log_db  = TRUE;
          liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             lib->Thread_debug = TRUE;
             g_snprintf( chaine, sizeof(chaine), "  -> Log enabled for library %s (%s)\n",
                         lib->admin_prompt, lib->nom_fichier );
             Write_admin ( client->connexion, chaine );
             liste = liste->next;
           }
        } else
       if ( ! strcmp ( debug, "none"      ) )
        { Config.log_all = FALSE;
          Config.log_db  = FALSE;
          liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             lib->Thread_debug = FALSE;
             g_snprintf( chaine, sizeof(chaine), "  -> Log disabled for library %s (%s)\n",
                         lib->admin_prompt, lib->nom_fichier );
             Write_admin ( client->connexion, chaine );
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
             Write_admin ( client->connexion, chaine );
             liste = liste->next;
           }
          g_snprintf( chaine, sizeof(chaine), "  -> Log is %s for db\n",
                      (Config.log_db ? " enabled" : "disabled") );
          Write_admin ( client->connexion, chaine );
          g_snprintf( chaine, sizeof(chaine), "  -> Log is %s for all\n",
                      (Config.log_all ? " enabled" : "disabled") );
          Write_admin ( client->connexion, chaine );
        } else
       if ( ! strcmp ( debug, "db"   ) )
        { if (Config.log_db == TRUE) Config.log_db = FALSE;
          else Config.log_db = TRUE;
          g_snprintf( chaine, sizeof(chaine), "  -> Log is now %s for db\n",
                      (Config.log_db ? " enabled" : "disabled") );
          Write_admin ( client->connexion, chaine );        }
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
                Write_admin ( client->connexion, chaine );
                break;
              }
             liste = liste->next;
           }
          if ( liste == NULL )                                       /* Si l'on a pas trouve de librairie */
           { g_snprintf( chaine, sizeof(chaine), " -- Unknown debug switch\n" );
             Write_admin ( client->connexion, chaine );
           }
        }
     } else
    if ( ! strcmp ( commande, "ping" ) )
     { Write_admin ( client->connexion, " Pong !\n" );
     } else
    if ( ! strcmp ( commande, "nocde" ) )
     { g_snprintf( chaine, sizeof(chaine), "\n" );
       Write_admin ( client->connexion, chaine );
     } else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
