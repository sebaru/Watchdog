/**********************************************************************************************************/
/* Watchdogd/Imsg/admin_imsg.c        Gestion des connexions Admin IMSG au serveur watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                  sam. 28 juil. 2012 16:35:09 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_imsg.c
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
 
 #include "watchdogd.h"
 #include "Imsg.h"

/**********************************************************************************************************/
/* Admin_imsg_reload: Demande le rechargement des conf IMSG                                               */
/* Entrée: le connexion                                                                                   */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_imsg_reload ( struct CONNEXION *connexion )
  { if (Cfg_imsg.lib->Thread_run == FALSE)
     { Admin_write ( connexion, " Thread IMSG is not running\n" );
       return;
     }
    
    Cfg_imsg.reload = TRUE;
    Admin_write ( connexion, " IMSG Reloading in progress\n" );
    while (Cfg_imsg.reload) sched_yield();
    Admin_write ( connexion, " IMSG Reloading done\n" );
  }
/**********************************************************************************************************/
/* Admin_print_contact : Affiche le parametre sur la console d'admin CLI                                  */
/* Entrée: La connexion connexion ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_print_imsg ( struct CONNEXION *connexion, struct IMSGDB *imsg )
  { gchar chaine[512];

    g_snprintf( chaine, sizeof(chaine),
              " [%03d]%12s -> user_enable       = %d\n"
              "   |               -> imsg_enable       = %d\n"
              "   |               -> imsg_jabberid     = %s\n"
              "   |               -> imsg_allow_cde    = %d\n"
              "   |               -> imsg_availability = %d\n"
              "   |----------------> %s\n",
                imsg->user_id, imsg->user_name, imsg->user_enable, imsg->user_imsg_enable, imsg->user_jabberid,
                imsg->user_allow_cde, imsg->user_available, imsg->user_comment
              );

    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_imsg_list : L'utilisateur admin lance la commande "list" en mode imsg                            */
/* Entrée: La connexion connexion ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_imsg_list ( struct CONNEXION *connexion )
  { struct IMSGDB *imsg;
    gchar chaine[256];
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " -- Liste des Contacts IMSG\n" );
    Admin_write ( connexion, chaine );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_WARNING,
                "Admin_imsg_list: Database Connection Failed" );
       return;
     }

/************************************* Chargement des informations en bases *******************************/
    if ( ! Recuperer_imsgDB( db ) )
     { Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_WARNING,
                "Admin_imsg_list: Recuperer_imsg Failed" );
       return;
     }

    while ( (imsg = Recuperer_imsgDB_suite( db )) != NULL)
     { Admin_print_imsg ( connexion, imsg ); }

    Libere_DB_SQL( &db );
  }
/**********************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                 */
/* Entrée: Le connexion d'admin, la ligne a traiter                                                       */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "send" ) )
     { gchar to[256];
       sscanf ( ligne, "%s %s", commande, to );                      /* Découpage de la ligne de commande */
       Imsg_Envoi_message_to ( to, ligne + strlen (to) + 6 );
       g_snprintf( chaine, sizeof(chaine), " Message '%s' send to %s\n", ligne + strlen (to) + 6, to );
       Admin_write ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_imsg_list ( connexion ); }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_imsg_reload ( connexion ); }
    else if ( ! strcmp ( commande, "status" ) )
     { gchar chaine[128];
       if (Cfg_imsg.connection)
        { LmConnectionState state;
          state =  lm_connection_get_state ( Cfg_imsg.connection );
          switch (state)
           { case LM_CONNECTION_STATE_CLOSED:
                  g_snprintf( chaine, sizeof(chaine), " Connexion closed.\n"); break;
             case LM_CONNECTION_STATE_OPENING:
                  g_snprintf( chaine, sizeof(chaine), " Connexion is opening.\n"); break;
             case LM_CONNECTION_STATE_OPEN:
                  g_snprintf( chaine, sizeof(chaine), " Connexion opened.\n"); break;
             case LM_CONNECTION_STATE_AUTHENTICATING:
                  g_snprintf( chaine, sizeof(chaine), " Connexion is authenticating.\n"); break;
             case LM_CONNECTION_STATE_AUTHENTICATED:
                  g_snprintf( chaine, sizeof(chaine), " Connexion authenticated (OK).\n"); break;
             default:
                  g_snprintf( chaine, sizeof(chaine), " Connexion Status Unknown.\n"); break;
           }
          Admin_write ( connexion, chaine );
          if (Cfg_imsg.date_retente)
           { g_snprintf( chaine, sizeof(chaine), " Re-trying in %03ds.\n",
                         (Cfg_imsg.date_retente - Partage->top)/10);
             Admin_write ( connexion, chaine );
           }
        }
       else 
        { g_snprintf( chaine, sizeof(chaine), " No connexion ... strange ! \n" );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "presence" ) )
     { g_snprintf( Cfg_imsg.new_status, sizeof(Cfg_imsg.new_status), "%s", ligne + 14 );
       Cfg_imsg.set_status = TRUE;
       g_snprintf( chaine, sizeof(chaine), " Presence Status changed to %s! \n", Cfg_imsg.new_status );
       Admin_write ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { if (Admin_dbcfg_thread ( connexion, NOM_THREAD, ligne+6 ) == TRUE)   /* Si changement de parametre */
        { gboolean retour;
          retour = Imsg_Lire_config();
          g_snprintf( chaine, sizeof(chaine), " Reloading Thread Parameters from Database -> %s\n",
                      (retour ? "Success" : "Failed") );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'IMSG'\n" );
       Admin_write ( connexion, "  dbcfg ...                              - Get/Set Database Parameters\n" );
       Admin_write ( connexion, "  send user@domain/resource message      - Send a message to user\n" );
       Admin_write ( connexion, "  reload                                 - Reload configuration from Database\n" );
       Admin_write ( connexion, "  list                                   - List contact and availability\n" );
       Admin_write ( connexion, "  presence new_status                    - Change Presence to 'new_status'\n" );
       Admin_write ( connexion, "  status                                 - See connexion status\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown IMSG command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
