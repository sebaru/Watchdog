/******************************************************************************************************************************/
/* Watchdogd/admin_master.c        Gestion des responses Admin du thread "Master" de watchdog                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        sam. 02 févr. 2013 14:04:58 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
 
 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include "watchdogd.h"
 #include "Satellite.h"

/******************************************************************************************************************************/
/* Admin_Satellite_Mode_vers_string: convertir le statut numérique en chaine de caractere                                     */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
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
/******************************************************************************************************************************/
/* Admin_Satellite_status: Affiche le statut du satellite                                                                     */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_Satellite_status ( gchar *response )
  { gchar chaine[128];
    gfloat next;
    next = (Cfg_satellite.date_next_retry - Partage->top)/10.0;
    g_snprintf( chaine, sizeof(chaine), " -- Satellite Status -- %s (%02d)",
                Admin_Satellite_Mode_vers_string( Cfg_satellite.Mode ),
                Cfg_satellite.Mode );
    response = Admin_write ( response, chaine );
    if (Cfg_satellite.Mode == SAT_RETRY_CONNECT)
     { g_snprintf( chaine, sizeof(chaine), " | - Retry in %02.1fs", next );
       response = Admin_write ( response, chaine );
     }

    g_snprintf( chaine, sizeof(chaine), " | - master = %s:%d - %s",
                Cfg_satellite.master_host, Cfg_satellite.master_port,
                (Cfg_satellite.master_certif ? Nom_certif( Cfg_satellite.master_certif ) : "Unknown") );
    response = Admin_write ( response, chaine );
    g_snprintf( chaine, sizeof(chaine), " | - issuer = %s",
                (Cfg_satellite.master_certif ? Nom_certif_signataire( Cfg_satellite.master_certif ) : "Unknown") );
    response = Admin_write ( response, chaine );
    g_snprintf( chaine, sizeof(chaine), " | - local master = cert %s, key %s, ca %s",
                Cfg_satellite.ssl_file_cert, Cfg_satellite.ssl_file_key, Cfg_satellite.ssl_file_ca );
    response = Admin_write ( response, chaine );

    pthread_mutex_lock ( &Cfg_satellite.lib->synchro );
    g_snprintf( chaine, sizeof(chaine), " | - processing %03d Events",
                g_slist_length ( Cfg_satellite.liste_Events ) );
    pthread_mutex_unlock ( &Cfg_satellite.lib->synchro );
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_master: Gere une commande 'admin masterdepuis une response admin                                                    */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 gchar *Admin_command ( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'Satellite'" );
       response = Admin_write ( response, "  dbcfg ...          - Get/Set Database Parameters" );
       response = Admin_write ( response, "  status             - Show connection status" );
       response = Admin_write ( response, "  reload             - Reload config from Database" );
       response = Admin_write ( response, "  help               - This help" );
     } else
    if ( ! strcmp ( commande, "dbcfg" ) )                     /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { gboolean retour;
       response =  Admin_dbcfg_thread ( response, NOM_THREAD, ligne+6 );                        /* Si changement de parametre */
       retour = Satellite_Lire_config();
       g_snprintf( chaine, sizeof(chaine), " Reloading Thread Parameters from Database -> %s", (retour ? "Success" : "Failed") );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "status" ) )
     { response = Admin_Satellite_status ( response );
     } else
    if ( ! strcmp ( commande, "reload" ) )                                    /* Rechargement de la configuration en Database */
     { gboolean retour;
       retour = Satellite_Lire_config();
       g_snprintf( chaine, sizeof(chaine), " Reloading Satellite Parameters -> %s",
                   (retour ? "Success" : "Failed") );
       response = Admin_write ( response, chaine );
     } else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
