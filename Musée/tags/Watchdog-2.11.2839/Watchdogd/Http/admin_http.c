/**********************************************************************************************************/
/* Watchdogd/HttpMobile/admin_httpmobile.c  Gestion des connexions Admin du thrd "HttpMobile" de watchdog */
/* Projet WatchDog version 2.0       Gestion d'habitat                   mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_httpmobile.c
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
 #include "Http.h"

/**********************************************************************************************************/
/* Admin_http_status: Print le statut du thread HTTP                                                      */
/* Entrée: la connexion pour sortiee client et la ligne de commande                                       */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_http_status ( struct CONNEXION *connexion )
  { gchar chaine[128];
    g_snprintf( chaine, sizeof(chaine), " | HTTP Server : port %d %s with SSL=%d\n",
                Cfg_http.tcp_port, (Cfg_http.ws_context ? "Running" : "Stopped" ), Cfg_http.ssl_enable );
    Admin_write ( connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), " | ssl_cert_filepath = %s\n", Cfg_http.ssl_cert_filepath );
    Admin_write ( connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), " | ssl_file_key      = %s\n", Cfg_http.ssl_private_key_filepath );
    Admin_write ( connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), " | ssl_file_ca       = %s\n", Cfg_http.ssl_ca_filepath );
    Admin_write ( connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), " | ssl_cipher        = %s\n", Cfg_http.ssl_cipher_list );
    Admin_write ( connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), " -\n");
         
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_http_list: List les sessions actives du thread HTTP                                              */
/* Entrée: la connexion pour sortiee client et la ligne de commande                                       */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_http_list ( struct CONNEXION *connexion )
  { struct HTTP_SESSION *session = NULL;
    gchar chaine[128];
    GSList *liste;
#ifdef bouh
    pthread_mutex_lock( &Cfg_http.lib->synchro );                        /* Ajout dans la liste a traiter */
    liste = Cfg_http.Liste_sessions;
    while ( liste )
     { session = (struct HTTP_SESSION *)liste->data;
       g_snprintf( chaine, sizeof(chaine), " | ----------- ID %s\n", session->sid );
       Admin_write( connexion, chaine );
       g_snprintf( chaine, sizeof(chaine), " | - type     = %03d\n", session->type );
       Admin_write( connexion, chaine );
       g_snprintf( chaine, sizeof(chaine), " | - username = %s\n", (session->util ? session->util->nom : "Unknown") );
       Admin_write( connexion, chaine );
       g_snprintf( chaine, sizeof(chaine), " | - last_top = %.1fs\n", (Partage->top - session->last_top)/10.0 );
       Admin_write( connexion, chaine );
       liste = liste->next;
     }
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
#endif
    Admin_write( connexion, " -\n" );
  }

/******************************************************************************************************************************/
/* Admin_command: Gere une commande liée au thread HTTP depuis une connexion admin                                            */
/* Entrée: le client et la ligne de commande                                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Admin_command ( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "list" ) )
     { Admin_http_list ( connexion ); }
    else if ( ! strcmp ( commande, "status" ) )
     { Admin_http_status ( connexion ); }
    else if ( ! strcmp ( commande, "dbcfg" ) )                /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { if (Admin_dbcfg_thread ( connexion, NOM_THREAD, ligne+6 ) == TRUE)                       /* Si changement de parametre */
        { gboolean retour;
          retour = Http_Lire_config();
          g_snprintf( chaine, sizeof(chaine), " Reloading Thread Parameters from Database -> %s\n",
                      (retour ? "Success" : "Failed") );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'UPS'\n" );
       Admin_write ( connexion, "  dbcfg ...                              - Get/Set Database Parameters\n" );
       Admin_write ( connexion, "  status                                 - Get Status of HTTP Thread\n");
       Admin_write ( connexion, "  list                                   - Get Sessions list\n");
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
