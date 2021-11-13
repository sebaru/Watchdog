/******************************************************************************************************************************/
/* Watchdogd/Imsgp/admin_imsg.c        Gestion des responses Admin IMSG au serveur watchdog                                   */
/* Projet WatchDog version 2.0        Gestion d'habitat                                                   25.02.2018 17:36:21 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

/******************************************************************************************************************************/
/* Admin_imsgp_reload: Demande le rechargement des conf IMSG                                                                  */
/* Entrée: le response                                                                                                        */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gchar *Admin_imsgp_reload ( gchar *response )
  { Cfg_imsgp.lib->Thread_sigusr1 = TRUE;
    response = Admin_write ( response, " | - IMSG Reloading done\n" );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_print_contact : Affiche le parametre sur la console d'admin CLI                                                      */
/* Entrée: La response response ADMIN                                                                                         */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                                           */
/******************************************************************************************************************************/
 static gchar *Admin_print_imsgp ( gchar *response, struct IMSGPDB *imsgp )
  { gchar chaine[512];

    g_snprintf( chaine, sizeof(chaine),
              " | - [%03d]%12s -> user_enable       = %d\n"
              " |               -> imsgp_enable       = %d\n"
              " |               -> imsgp_jabberid     = %s\n"
              " |               -> imsgp_allow_cde    = %d\n"
              " |               -> imsgp_availability = %d\n"
              " | ---------------> %s",
                imsgp->user_id, imsgp->user_name, imsgp->user_enable, imsgp->user_imsg_enable, imsgp->user_jabberid,
                imsgp->user_allow_cde, imsgp->user_available, imsgp->user_comment
              );

    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_imsgp_list : L'utilisateur admin lance la commande "list" en mode imsgp                                              */
/* Entrée: La response response ADMIN                                                                                         */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                                           */
/******************************************************************************************************************************/
 static gchar *Admin_imsgp_list ( gchar *response )
  { struct IMSGPDB *imsgp;
    struct DB *db;

    response = Admin_write ( response, " | -- Liste des Contacts IMSG" );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING, "%s: Database Connection Failed", __func__ );
       return(response);
     }

/******************************************** Chargement des informations en bases ********************************************/
    if ( ! Recuperer_imsgpDB( db ) )
     { Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_imsgp.lib->Thread_debug, LOG_WARNING, "%s: Recuperer_imsgp Failed", __func__ );
       return(response);
     }

    while ( (imsgp = Recuperer_imsgpDB_suite( db )) != NULL)
     { response = Admin_print_imsgp ( response, imsgp ); }

    Libere_DB_SQL( &db );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                                     */
/* Entrée: Le response d'admin, la ligne a traiter                                                                            */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_command( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "send" ) )
     { gchar to[256];
       sscanf ( ligne, "%s %s", commande, to );                      /* Découpage de la ligne de commande */
       Imsgp_Envoi_message_to ( to, ligne + strlen (to) + 6 );
       g_snprintf( chaine, sizeof(chaine), " | - Message '%s' sent to '%s'", ligne + strlen (to) + 6, to );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { response = Admin_imsgp_list ( response ); }
    else if ( ! strcmp ( commande, "reload" ) )
     { response = Admin_imsgp_reload ( response ); }
    else if ( ! strcmp ( commande, "status" ) )
     { gchar chaine[128];
     }
    else if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, " | -- Watchdog ADMIN -- Help du mode 'IMSG'" );
       response = Admin_write ( response, " | - send user@domain/resource message      - Send a message to user" );
       response = Admin_write ( response, " | - add_buddy user@domain                  - Ajoute un buddy dans la liste" );
       response = Admin_write ( response, " | - reload                                 - Reload configuration from Database" );
       response = Admin_write ( response, " | - list                                   - List contact and availability" );
/*       response = Admin_write ( response, "  presence new_status                    - Change Presence to 'new_status'" );
  */
       response = Admin_write ( response, " | - status                                 - See response status" );
     }
    else if ( ! strcmp ( commande, "add_buddy" ) )
				 { g_snprintf( chaine, sizeof(chaine), " | - Adding '%s'", ligne + 10 );
       response = Admin_write ( response, chaine );
       purple_account_add_buddy( Cfg_imsgp.account, purple_buddy_new	( Cfg_imsgp.account, ligne + 10, ligne + 10 ) );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown IMSGP command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
   return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
