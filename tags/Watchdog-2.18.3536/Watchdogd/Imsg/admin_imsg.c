/**********************************************************************************************************/
/* Watchdogd/Imsg/admin_imsg.c        Gestion des responses Admin IMSG au serveur watchdog               */
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
/* Entrée: le response                                                                                   */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static gchar *Admin_imsg_reload ( gchar *response )
  { if (Cfg_imsg.lib->Thread_run == FALSE)
     { response = Admin_write ( response, " Thread IMSG is not running\n" );
       return(response);
     }
    
    Cfg_imsg.reload = TRUE;
    while (Cfg_imsg.reload) sched_yield();
    response = Admin_write ( response, " IMSG Reloading done\n" );
    return(response);
  }
/**********************************************************************************************************/
/* Admin_print_contact : Affiche le parametre sur la console d'admin CLI                                  */
/* Entrée: La response response ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static gchar *Admin_print_imsg ( gchar *response, struct IMSGDB *imsg )
  { gchar chaine[512];

    g_snprintf( chaine, sizeof(chaine),
              " [%03d]%12s -> user_enable       = %d\n"
              "   |               -> imsg_enable       = %d\n"
              "   |               -> imsg_jabberid     = %s\n"
              "   |               -> imsg_allow_cde    = %d\n"
              "   |               -> imsg_availability = %d\n"
              "   |----------------> %s",
                imsg->user_id, imsg->user_name, imsg->user_enable, imsg->user_imsg_enable, imsg->user_jabberid,
                imsg->user_allow_cde, imsg->user_available, imsg->user_comment
              );

    response = Admin_write ( response, chaine );
    return(response);
  }
/**********************************************************************************************************/
/* Admin_imsg_list : L'utilisateur admin lance la commande "list" en mode imsg                            */
/* Entrée: La response response ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static gchar *Admin_imsg_list ( gchar *response )
  { struct IMSGDB *imsg;
    gchar chaine[256];
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " -- Liste des Contacts IMSG\n" );
    response = Admin_write ( response, chaine );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_WARNING,
                "Admin_imsg_list: Database Connection Failed" );
       return(response);
     }

/************************************* Chargement des informations en bases *******************************/
    if ( ! Recuperer_imsgDB( db ) )
     { Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_imsg.lib->Thread_debug, LOG_WARNING,
                "Admin_imsg_list: Recuperer_imsg Failed" );
       return(response);
     }

    while ( (imsg = Recuperer_imsgDB_suite( db )) != NULL)
     { Admin_print_imsg ( response, imsg ); }

    Libere_DB_SQL( &db );
    return(response);
  }
/**********************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                 */
/* Entrée: Le response d'admin, la ligne a traiter                                                       */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 gchar *Admin_command( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "send" ) )
     { gchar to[256];
       sscanf ( ligne, "%s %s", commande, to );                      /* Découpage de la ligne de commande */
       Imsg_Envoi_message_to ( to, ligne + strlen (to) + 6 );
       g_snprintf( chaine, sizeof(chaine), " Message '%s' send to %s\n", ligne + strlen (to) + 6, to );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { response = Admin_imsg_list ( response ); }
    else if ( ! strcmp ( commande, "reload" ) )
     { response = Admin_imsg_reload ( response ); }
    else if ( ! strcmp ( commande, "status" ) )
     { gchar chaine[128];
       if (Cfg_imsg.connection)
        { LmConnectionState state;
          state =  lm_connection_get_state ( Cfg_imsg.connection );
          switch (state)
           { case LM_CONNECTION_STATE_CLOSED:
                  g_snprintf( chaine, sizeof(chaine), " response closed.\n"); break;
             case LM_CONNECTION_STATE_OPENING:
                  g_snprintf( chaine, sizeof(chaine), " response is opening.\n"); break;
             case LM_CONNECTION_STATE_OPEN:
                  g_snprintf( chaine, sizeof(chaine), " response opened.\n"); break;
             case LM_CONNECTION_STATE_AUTHENTICATING:
                  g_snprintf( chaine, sizeof(chaine), " response is authenticating.\n"); break;
             case LM_CONNECTION_STATE_AUTHENTICATED:
                  g_snprintf( chaine, sizeof(chaine), " response authenticated (OK).\n"); break;
             default:
                  g_snprintf( chaine, sizeof(chaine), " response Status Unknown.\n"); break;
           }
          response = Admin_write ( response, chaine );
          if (Cfg_imsg.date_retente)
           { g_snprintf( chaine, sizeof(chaine), " Re-trying in %03ds.\n",
                         (Cfg_imsg.date_retente - Partage->top)/10);
             response = Admin_write ( response, chaine );
           }
        }
       else 
        { g_snprintf( chaine, sizeof(chaine), " No response ... strange ! \n" );
          response = Admin_write ( response, chaine );
        }
     }
    else if ( ! strcmp ( commande, "presence" ) )
     { g_snprintf( Cfg_imsg.new_status, sizeof(Cfg_imsg.new_status), "%s", ligne + 14 );
       Cfg_imsg.set_status = TRUE;
       g_snprintf( chaine, sizeof(chaine), " Presence Status changed to %s! \n", Cfg_imsg.new_status );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { gboolean retour;
       response =  Admin_dbcfg_thread ( response, NOM_THREAD, ligne+6 );                        /* Si changement de parametre */
       retour = Imsg_Lire_config();
       g_snprintf( chaine, sizeof(chaine), " Reloading Thread Parameters from Database -> %s", (retour ? "Success" : "Failed") );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'IMSG'\n" );
       response = Admin_write ( response, "  dbcfg ...                              - Get/Set Database Parameters\n" );
       response = Admin_write ( response, "  send user@domain/resource message      - Send a message to user\n" );
       response = Admin_write ( response, "  reload                                 - Reload configuration from Database\n" );
       response = Admin_write ( response, "  list                                   - List contact and availability\n" );
       response = Admin_write ( response, "  presence new_status                    - Change Presence to 'new_status'\n" );
       response = Admin_write ( response, "  status                                 - See response status\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown IMSG command : %s\n", ligne );
       response = Admin_write ( response, chaine );
     }
   return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
