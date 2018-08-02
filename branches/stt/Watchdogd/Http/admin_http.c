/******************************************************************************************************************************/
/* Watchdogd/HttpMobile/admin_http.c  Gestion des responses Admin du thread "Http" de watchdog                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                                       mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_http.c
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
 #include "Http.h"

/******************************************************************************************************************************/
/* Admin_http_status: Print le statut du thread HTTP                                                                          */
/* Entrée: la response pour sortiee client et la ligne de commande                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_http_status ( gchar *response )
  { gchar chaine[128];
    g_snprintf( chaine, sizeof(chaine), " | HTTP Server : port %d %s with SSL=%d",
                Cfg_http.tcp_port, (Cfg_http.ws_context ? "Running" : "Stopped" ), Cfg_http.ssl_enable );
    response = Admin_write ( response, chaine );
    g_snprintf( chaine, sizeof(chaine), " | ssl_cert_filepath = %s", Cfg_http.ssl_cert_filepath );
    response = Admin_write ( response, chaine );
    g_snprintf( chaine, sizeof(chaine), " | ssl_file_key      = %s", Cfg_http.ssl_private_key_filepath );
    response = Admin_write ( response, chaine );
    g_snprintf( chaine, sizeof(chaine), " | ssl_file_ca       = %s", Cfg_http.ssl_ca_filepath );
    response = Admin_write ( response, chaine );
    g_snprintf( chaine, sizeof(chaine), " | ssl_cipher        = %s", Cfg_http.ssl_cipher_list );
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_http_list: List les sessions actives du thread HTTP                                                                  */
/* Entrée: la response pour sortiee client et la ligne de commande                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_http_list ( gchar *response )
  { struct HTTP_SESSION *session = NULL;
    gchar chaine[256];
    GSList *liste;

    pthread_mutex_lock( &Cfg_http.lib->synchro );                                            /* Ajout dans la liste a traiter */
    liste = Cfg_http.Liste_sessions;
    while ( liste )
     { session = (struct HTTP_SESSION *)liste->data;
       g_snprintf( chaine, sizeof(chaine), " | ------------ ID = %s -------------", (session->util ? session->util->nom : "Unknown") );
       Admin_write( response, chaine );
       g_snprintf( chaine, sizeof(chaine), " | - session_id    = %s", session->sid );
       Admin_write( response, chaine );
       g_snprintf( chaine, sizeof(chaine), " | - remote dns/ip = %s/%s", session->remote_name, session->remote_ip );
       Admin_write( response, chaine );
       g_snprintf( chaine, sizeof(chaine), " | - user_agent    = %s", session->user_agent );
       Admin_write( response, chaine );
       g_snprintf( chaine, sizeof(chaine), " | - is_ssl        = %s", (session->is_ssl ? "yes" : "NO") );
       Admin_write( response, chaine );
       g_snprintf( chaine, sizeof(chaine), " | - last_top      = %.1fs ago", (Partage->top - session->last_top)/10.0 );
       Admin_write( response, chaine );
       liste = liste->next;
     }
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_http_kill: Kill la session en parametre en se basant sur le username ou l'ID de session                              */
/* Entrée: la response et l'id (name ou sid)                                                                                 */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_http_kill ( gchar *response, gchar *id )
  { struct HTTP_SESSION *session;
    gchar chaine[128];
    GSList *liste;

search_again:
    session = NULL;
    pthread_mutex_lock( &Cfg_http.lib->synchro );                                            /* Ajout dans la liste a traiter */
    liste = Cfg_http.Liste_sessions;
    while ( liste )
     { session = (struct HTTP_SESSION *)liste->data;
       if ( ! strncmp ( session->sid, id, strlen(id) ) ) break;
       if ( session->util && ( ! strcmp ( session->util->nom, id ) ) ) break;
       liste = liste->next;
     }
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
    if (liste)
     { g_snprintf( chaine, sizeof(chaine), " | - Session %sl(%s) from %s/%s killed",
                   session->sid, (session->util ? session->util->nom : "nouser"), session->remote_name, session->remote_ip );
       Admin_write( response, chaine );
       Http_Liberer_session ( session );
       goto search_again;
     }
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_command: Gere une commande liée au thread HTTP depuis une response admin                                            */
/* Entrée: le client et la ligne de commande                                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_command ( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "list" ) )
     { response = Admin_http_list ( response ); }
    else if ( ! strcmp ( commande, "status" ) )
     { response = Admin_http_status ( response ); }
    else if ( ! strcmp ( commande, "kill" ) )
     { gchar name[80];
       sscanf ( ligne, "%s %s", commande, name );                                        /* Découpage de la ligne de commande */
       response = Admin_http_kill ( response, name );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'UPS'" );
       response = Admin_write ( response, "  status                                 - Get Status of HTTP Thread");
       response = Admin_write ( response, "  list                                   - Get Sessions list");
       response = Admin_write ( response, "  kill $id                               - Kill session(s) with id $id (name or sid)");
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Unknown command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
