/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_sms.c        Gestion des responses Admin SET au serveur watchdog                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        sam. 19 mai 2012 11:03:52 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_sms.c
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
 
 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include "watchdogd.h"
 #include "Sms.h"

/******************************************************************************************************************************/
/* Admin_sms_reload: Demande le rechargement des conf SMS                                                                     */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_sms_reload ( gchar *response )
  { if (Cfg_sms.lib->Thread_run == FALSE)
     { response = Admin_write ( response, " Thread SMS is not running" );
       return(response);
     }
    
    Cfg_sms.lib->Thread_sigusr1 = TRUE;
    while (Cfg_sms.lib->Thread_sigusr1) sched_yield();
    response = Admin_write ( response, " SMS Reload done" );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_print_sms : Affiche le parametre sur la console d'admin CLI                                                          */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_print_sms ( gchar *response, struct SMSDB *sms )
  { gchar chaine[256];

    g_snprintf( chaine, sizeof(chaine),
              " | ---------------------------------\n"
              " | [%03d]%12s -> user_enable   = %d\n"
              " |               -> sms_enable    = %d\n"
              " |               -> sms_phone     = %s\n"
              " |               -> sms_allow_cde = %d\n"
              " |----------------> %s",
                sms->user_id, sms->user_name, sms->user_enable, sms->user_sms_enable, sms->user_sms_phone,
                sms->user_sms_allow_cde, sms->user_comment
              );
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_sms_list : L'utilisateur admin lance la commande "list" en mode sms                                                  */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_sms_list ( gchar *response )
  { struct SMSDB *sms;
    gchar chaine[80];
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " -- Liste des contacts SMS" );
    response = Admin_write ( response, chaine );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING, "%s: Database Connection Failed", __func__ );
       return(response);
     }

/********************************************* Chargement des informations en bases *******************************************/
    if ( ! Sms_Recuperer_smsDB( db ) )
     { Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING, "%s: Recuperer_sms Failed", __func__ );
       return(response);
     }

    while ( (sms = Sms_Recuperer_smsDB_suite( db )) != NULL)
     { response = Admin_print_sms ( response, sms ); }

    Libere_DB_SQL( &db );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_sms: Gere une commande 'admin sms' depuis une response admin                                                         */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 gchar *Sms_Admin_response ( gchar *ligne )
  { gchar commande[128], chaine[128];
    gchar *response = NULL;

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  | -- Watchdog ADMIN -- Help du mode 'SMS'" );
       response = Admin_write ( response, "  | - reload                - Reload contacts from Database" );
       response = Admin_write ( response, "  | - sms smsbox message    - Send 'message' via smsbox" );
       response = Admin_write ( response, "  | - sms gsm    message    - Send 'message' via gsm" );
       response = Admin_write ( response, "  | - list                  - Liste les contacts SMS" );
       response = Admin_write ( response, "  | - help                  - This help" );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { response = Admin_sms_list ( response );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { response = Admin_sms_reload ( response ); }
    else if ( ! strcmp ( commande, "gsm" ) )
     { gchar message[80];
       sscanf ( ligne, "%s %s", commande, message );                                     /* Découpage de la ligne de commande */
       Envoyer_sms_gsm_text ( ligne + 4 );                   /* On envoie le reste de la liste, pas seulement le mot suivant. */
       g_snprintf( chaine, sizeof(chaine), " | - Sms sent\n" );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "smsbox" ) )
     { gchar message[80];
       sscanf ( ligne, "%s %s", commande, message );                                     /* Découpage de la ligne de commande */
       Envoyer_sms_smsbox_text ( ligne + 7 );                /* On envoie le reste de la liste, pas seulement le mot suivant. */
       g_snprintf( chaine, sizeof(chaine), " | - Sms sent\n" );
       response = Admin_write ( response, chaine );
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " | - Unknown command : %s\n", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
