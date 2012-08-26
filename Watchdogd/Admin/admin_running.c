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
       Write_admin ( client->connexion, "  tell message_num      - Envoi d'un message audio _num_\n" );
       Write_admin ( client->connexion, "  msgs message          - Envoi d'un message a tous les clients\n" );
       Write_admin ( client->connexion, "  setrootpasswd         - Set the Watchdog root password\n" );
       Write_admin ( client->connexion, "  modbus                - Sous-menu de gestion des equipements MODBUS\n" );
       Write_admin ( client->connexion, "  rs485                 - Sous-menu de gestion des equipements RS485\n" );
       Write_admin ( client->connexion, "  onduleur              - Sous-menu de gestion des equipements ONDULEUR\n" );
       Write_admin ( client->connexion, "  sms                   - Sous-menu d'envoi de SMS\n" );
       Write_admin ( client->connexion, "  dls                   - D.L.S. Status\n" );
       Write_admin ( client->connexion, "  debug debug_to_switch - Switch Debug Mode (switch are : list, all, none, or library name)\n" );

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
    if ( ! strcmp ( commande, "tell" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Ajouter_audio ( num );
       g_snprintf( chaine, sizeof(chaine), " Message id %d sent\n", num );
       Write_admin ( client->connexion, chaine );
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
       num = g_list_length( Partage->com_msrv.liste_i );                   /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " Distribution des I      : reste %d\n", num );
       Write_admin ( client->connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_list_length( Partage->com_msrv.liste_msg_off );             /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " Distribution des Msg OFF: reste %d\n", num );
       Write_admin ( client->connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_list_length( Partage->com_msrv.liste_msg_on );              /* Recuperation du numero de i */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), " Distribution des Msg ON : reste %d\n", num );
       Write_admin ( client->connexion, chaine );

       pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = g_list_length( Partage->com_msrv.liste_msg_repeat );                     /* liste des repeat */
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       g_snprintf( chaine, sizeof(chaine), "          MSgs en REPEAT : reste %d\n", num );
       Write_admin ( client->connexion, chaine );
       
     } else
    if ( ! strcmp ( commande, "debug" ) )
     { gchar debug[128];

       sscanf ( ligne, "%s %s", commande, debug );

       if ( ! strcmp ( debug, "all"       ) )
        { Info_change_debug ( Config.log, ~0 );
          liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             lib->Thread_debug = TRUE;
             g_snprintf( chaine, sizeof(chaine), "  -> Debug enabled for library %s (%s)\n",
                         lib->admin_prompt, lib->nom_fichier );
             Write_admin ( client->connexion, chaine );
             liste = liste->next;
           }
        } else
       if ( ! strcmp ( debug, "none"      ) )
        { Info_change_debug ( Config.log,  0 );
          liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             lib->Thread_debug = FALSE;
             g_snprintf( chaine, sizeof(chaine), "  -> Debug disabled for library %s (%s)\n",
                         lib->admin_prompt, lib->nom_fichier );
             Write_admin ( client->connexion, chaine );
             liste = liste->next;
           }
        } else
       if ( ! strcmp ( debug, "list"      ) )
        { liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             g_snprintf( chaine, sizeof(chaine), "  -> Debug is %s for library %s (%s)\n",
                         (lib->Thread_debug ? " enabled" : "disabled"),
                         lib->admin_prompt, lib->nom_fichier );
             Write_admin ( client->connexion, chaine );
             liste = liste->next;
           }
        } else
       if ( ! strcmp ( debug, "INFO"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_SIGNAUX   ); } else
       if ( ! strcmp ( debug, "NOTICE"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_SIGNAUX   ); } else
       if ( ! strcmp ( debug, "WARNING"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_SIGNAUX   ); } else
       if ( ! strcmp ( debug, "ERR"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_SIGNAUX   ); } else
       if ( ! strcmp ( debug, "CRIT"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_SIGNAUX   ); } else
       if ( ! strcmp ( debug, "EMERG"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_SIGNAUX   ); } else
       if ( ! strcmp ( debug, "signaux"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_SIGNAUX   ); } else
       if ( ! strcmp ( debug, "db"        ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_DB        ); } else
       if ( ! strcmp ( debug, "config"    ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_CONFIG    ); } else
       if ( ! strcmp ( debug, "user"      ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_USER      ); } else
       if ( ! strcmp ( debug, "crypto"    ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_CRYPTO    ); } else
       if ( ! strcmp ( debug, "info"      ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_INFO      ); } else
       if ( ! strcmp ( debug, "serveur"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_SERVEUR   ); } else
       if ( ! strcmp ( debug, "cdg"       ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_CDG       ); } else
       if ( ! strcmp ( debug, "network"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_NETWORK   ); } else
       if ( ! strcmp ( debug, "arch"   ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_ARCHIVE   ); } else
       if ( ! strcmp ( debug, "connexion" ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_CONNEXION ); } else
       if ( ! strcmp ( debug, "dls"       ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_DLS       ); } else
       if ( ! strcmp ( debug, "modbus"    ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_MODBUS    ); } else
       if ( ! strcmp ( debug, "admin"     ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_ADMIN     ); } else
       if ( ! strcmp ( debug, "onduleur"  ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_ONDULEUR  ); } else
       if ( ! strcmp ( debug, "sms"       ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_SMS       ); } else
       if ( ! strcmp ( debug, "audio"     ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_AUDIO     ); } else
       if ( ! strcmp ( debug, "camera"    ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_CAMERA    ); } else
       if ( ! strcmp ( debug, "courbe"    ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_COURBE    ); } else
       if ( ! strcmp ( debug, "tellstick" ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_TELLSTICK ); } else
       if ( ! strcmp ( debug, "lirc"      ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_LIRC      ); } else
       if ( ! strcmp ( debug, "asterisk"  ) )
        { Info_change_debug ( Config.log, Config.debug_level ^= DEBUG_ASTERISK  ); }
       else
        { liste = Partage->com_msrv.Librairies;                      /* Parcours de toutes les librairies */
          while(liste)
           { lib = (struct LIBRAIRIE *)liste->data;
             if ( ! strcmp ( debug, lib->admin_prompt ) )
              { if (lib->Thread_debug == TRUE)
                 { lib->Thread_debug = FALSE;
                   g_snprintf( chaine, sizeof(chaine), "  -> Debug disabled for library %s (%s)\n",
                               lib->admin_prompt, lib->nom_fichier );
                 }
                else
                 { lib->Thread_debug = TRUE;
                   g_snprintf( chaine, sizeof(chaine), "  -> Debug enabled for library %s (%s)\n",
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
       g_snprintf( chaine, sizeof(chaine), " Debug_level is now %d\n", Config.log->debug_level );
       Write_admin ( client->connexion, chaine );
       Config.debug_level = Config.log->debug_level;  /* Sauvegarde pour persistence (export des données) */
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
