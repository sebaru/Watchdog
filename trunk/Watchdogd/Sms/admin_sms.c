/**********************************************************************************************************/
/* Watchdogd/Admin/admin_sms.c        Gestion des connexions Admin SET au serveur watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                    sam. 19 mai 2012 11:03:52 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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
 
 #include <unistd.h>                                                                  /* Pour gethostname */
 #include "watchdogd.h"
 #include "Sms.h"

/**********************************************************************************************************/
/* Admin_sms_reload: Demande le rechargement des conf SMS                                                 */
/* Entrée: le connexion                                                                                   */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_sms_reload ( struct CONNEXION *connexion )
  { if (Cfg_sms.lib->Thread_run == FALSE)
     { Admin_write ( connexion, " Thread SMS is not running\n" );
       return;
     }
    
    Cfg_sms.reload = TRUE;
    Admin_write ( connexion, " SMS Reloading in progress\n" );
    while (Cfg_sms.reload) sched_yield();
    Admin_write ( connexion, " SMS Reloading done\n" );
  }
/**********************************************************************************************************/
/* Admin_print_sms : Affiche le parametre sur la console d'admin CLI                                      */
/* Entrée: La connexion connexion ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_print_sms ( struct CONNEXION *connexion, struct SMSDB *sms )
  { gchar chaine[256];

    g_snprintf( chaine, sizeof(chaine),
              " [%03d]%12s -> user_enable   = %d\n"
              "   |               -> sms_enable    = %d\n"
              "   |               -> sms_phone     = %s\n"
              "   |               -> sms_allow_cde = %d\n"
              "   |----------------> %s\n",
                sms->user_id, sms->user_name, sms->user_enable, sms->user_sms_enable, sms->user_sms_phone,
                sms->user_sms_allow_cde, sms->user_comment
              );
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_sms_list : L'utilisateur admin lance la commande "list" en mode sms                              */
/* Entrée: La connexion connexion ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_sms_list ( struct CONNEXION *connexion )
  { struct SMSDB *sms;
    gchar chaine[80];
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " -- Liste des contacts SMS\n" );
    Admin_write ( connexion, chaine );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                "Admin_sms_list: Database Connection Failed" );
       return;
     }

/************************************* Chargement des informations en bases *******************************/
    if ( ! Sms_Recuperer_smsDB( db ) )
     { Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_sms.lib->Thread_debug, LOG_WARNING,
                "Admin_sms_list: Recuperer_sms Failed" );
       return;
     }

    while ( (sms = Sms_Recuperer_smsDB_suite( db )) != NULL)
     { Admin_print_sms ( connexion, sms ); }

    Libere_DB_SQL( &db );
  }
/**********************************************************************************************************/
/* Admin_sms: Gere une commande 'admin sms' depuis une connexion admin                                    */
/* Entrée: le connexion et la ligne de commande                                                           */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command ( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'SMS'\n" );
       Admin_write ( connexion, "  dbcfg ...             - Get/Set Database Parameters\n" );
       Admin_write ( connexion, "  reload                - Reload contacts from Database\n" );
       Admin_write ( connexion, "  sms smsbox message    - Send 'message' via smsbox\n" );
       Admin_write ( connexion, "  sms gsm    message    - Send 'message' via gsm\n" );
       Admin_write ( connexion, "  list                  - Liste les contacts SMS\n" );
       Admin_write ( connexion, "  help                  - This help\n" );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_sms_list ( connexion );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_sms_reload ( connexion ); }
    else if ( ! strcmp ( commande, "gsm" ) )
     { gchar message[80];
       sscanf ( ligne, "%s %s", commande, message );                 /* Découpage de la ligne de commande */
       Envoyer_sms_gsm_text ( ligne + 4 ); /* On envoie le reste de la liste, pas seulement le mot suivant. */
       g_snprintf( chaine, sizeof(chaine), " Sms sent\n" );
       Admin_write ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "smsbox" ) )
     { gchar message[80];
       sscanf ( ligne, "%s %s", commande, message );                 /* Découpage de la ligne de commande */
       Envoyer_sms_smsbox_text ( ligne + 7 ); /* On envoie le reste de la liste, pas seulement le mot suivant. */
       g_snprintf( chaine, sizeof(chaine), " Sms sent\n" );
       Admin_write ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { if (Admin_dbcfg_thread ( connexion, NOM_THREAD, ligne+6 ) == TRUE)   /* Si changement de parametre */
        { gboolean retour;
          retour = Sms_Lire_config();
          g_snprintf( chaine, sizeof(chaine), " Reloading Sms Thread Parameters from Database -> %s\n",
                      (retour ? "Success" : "Failed") );
          Admin_write ( connexion, chaine );
        }
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
