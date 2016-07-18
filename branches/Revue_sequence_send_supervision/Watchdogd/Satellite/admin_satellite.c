/**********************************************************************************************************/
/* Watchdogd/admin_master.c        Gestion des connexions Admin du thread "Master" de watchdog            */
/* Projet WatchDog version 2.0       Gestion d'habitat                    sam. 02 févr. 2013 14:04:58 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_master.c
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
 #include "Satellite.h"

/**********************************************************************************************************/
/* Admin_Satellite_Mode_vers_string: convertir le statut numérique en chaine de caractere                 */
/* Entrée: le mode a convertir                                                                            */
/* Sortie: la chaine de caractere                                                                         */
/**********************************************************************************************************/
 static gchar *Admin_Satellite_Mode_vers_string ( gint mode )
  { switch ( mode )
     { case SAT_DISCONNECTED          : return ("DISCONNECTED");
       case SAT_RETRY_CONNECT         : return ("RETRYING");
       case SAT_ATTENTE_INTERNAL      : return ("WAIT INTERNAL");
       case SAT_ATTENTE_CONNEXION_SSL : return ("WAIT SSL");
       case SAT_CONNECTED             : return ("CONNECTED");
     }
    return("Unknown");
  };
/**********************************************************************************************************/
/* Admin_Satellite_status: Affiche le statut du satellite                                                 */
/* Entrée: le connexion                                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_Satellite_status ( struct CONNEXION *connexion )
  { gchar chaine[128];
    gfloat next;
    next = (Cfg_satellite.date_next_retry - Partage->top)/10.0;
    g_snprintf( chaine, sizeof(chaine), " -- Satellite Status -- %s (%02d)\n",
                Admin_Satellite_Mode_vers_string( Cfg_satellite.Mode ),
                Cfg_satellite.Mode );
    Admin_write( connexion, chaine );
    if (Cfg_satellite.Mode == SAT_RETRY_CONNECT)
     { g_snprintf( chaine, sizeof(chaine), " | - Retry in %02.1fs\n", next );
       Admin_write( connexion, chaine );
     }

    g_snprintf( chaine, sizeof(chaine), " | - master = %s:%d - %s\n",
                Cfg_satellite.master_host, Cfg_satellite.master_port,
                (Cfg_satellite.master_certif ? Nom_certif( Cfg_satellite.master_certif ) : "Unknown") );
    Admin_write( connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), " | - issuer = %s\n",
                (Cfg_satellite.master_certif ? Nom_certif_signataire( Cfg_satellite.master_certif ) : "Unknown") );
    Admin_write( connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), " | - local master = cert %s, key %s, ca %s\n",
                Cfg_satellite.ssl_file_cert, Cfg_satellite.ssl_file_key, Cfg_satellite.ssl_file_ca );
    Admin_write( connexion, chaine );

    pthread_mutex_lock ( &Cfg_satellite.lib->synchro );
    g_snprintf( chaine, sizeof(chaine), " | - processing %03d Events\n",
                g_slist_length ( Cfg_satellite.liste_Events ) );
    pthread_mutex_unlock ( &Cfg_satellite.lib->synchro );
    Admin_write( connexion, chaine );
    Admin_write( connexion, " -\n" );
  }
/**********************************************************************************************************/
/* Admin_master: Gere une commande 'admin masterdepuis une connexion admin                                */
/* Entrée: le connexion et la ligne de commande                                                           */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command ( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'Satellite'\n" );
       Admin_write ( connexion, "  dbcfg ...          - Get/Set Database Parameters\n" );
       Admin_write ( connexion, "  status             - Show connection status\n" );
       Admin_write ( connexion, "  reload             - Reload config from Database\n" );
       Admin_write ( connexion, "  help               - This help\n" );
     } else
    if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { if (Admin_dbcfg_thread ( connexion, NOM_THREAD, ligne+6 ) == TRUE)
        { gboolean retour;
          retour = Satellite_Lire_config();
          g_snprintf( chaine, sizeof(chaine), " Reloading Thread Parameters from Database -> %s\n",
                      (retour ? "Success" : "Failed") );
          Admin_write ( connexion, chaine );
        }
     } else
    if ( ! strcmp ( commande, "status" ) )
     { Admin_Satellite_status ( connexion );
     } else
    if ( ! strcmp ( commande, "reload" ) )                /* Rechargement de la configuration en Database */
     { gboolean retour;
       retour = Satellite_Lire_config();
       g_snprintf( chaine, sizeof(chaine), " Reloading Satellite Parameters -> %s\n",
                   (retour ? "Success" : "Failed") );
       Admin_write ( connexion, chaine );
     } else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }

  }
/*--------------------------------------------------------------------------------------------------------*/
