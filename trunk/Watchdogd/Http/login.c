/******************************************************************************************************************************/
/* Watchdogd/Http/login.c       Gestion des request login pour le thread HTTP de watchdog                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    05.09.2016 15:01:29 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * login.c
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
 
 #include <openssl/rand.h>
 
/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Http.h"
/******************************************************************************************************************************/
/* Http_Traiter_request_login: Traite une requete de login                                                                    */
/* Entrées: la connexion MHD                                                                                                  */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 gboolean Http_Traiter_request_login ( struct lws *wsi, gchar *remote_name, gchar *remote_ip )
  { unsigned char header[256], *header_cur, *header_end;
   	gboolean retour = FALSE;
	   gchar buffer[4096];

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
             "Http_Traiter_request_login: HTTP request from %s(%s)",
              remote_name, remote_ip );

    buffer[0] = '\0';
    if ( lws_hdr_copy( wsi, buffer, sizeof(buffer), WSI_TOKEN_HTTP_COOKIE ) != -1 )     /* Récupération de la valeur du token */
     { gchar *cookies, *cookie, *savecookies;
       gchar *cookie_name, *cookie_value, *savecookie;
       cookies = buffer;
       while ( (cookie=strtok_r( cookies, ";", &savecookies)) != NULL )
        { cookies=NULL;
          cookie_name=strtok_r( cookie, "=", &savecookie);
          if (cookie_name)
           { cookie_value = strtok_r ( NULL, "=", &savecookie );
             Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "Http_Traiter_request_login: HTTP cookie found for %s(%s): %s=%s",
                 remote_name, remote_ip, cookie_name, cookie_value );
           }
        }       
     }

    header_cur = header;
    header_end = header + sizeof(header);
    
    if (TRUE)
     { gchar cookie_bin[EVP_MAX_MD_SIZE], cookie_hex[2*EVP_MAX_MD_SIZE+1], cookie[256];
       gint cpt;
       RAND_pseudo_bytes( (guchar *)cookie_bin, sizeof(cookie_bin) );                     /* Récupération d'un nouveau COOKIE */
       for (cpt=0; cpt<sizeof(cookie_bin); cpt++)                                              /* Mise en forme au format HEX */
        { g_snprintf( &cookie_hex[2*cpt], 3, "%02X", (guchar)cookie_bin[cpt] ); }
       g_snprintf( cookie, sizeof(cookie), "sessionid=%s; Max-Age=%d; ", cookie_hex, 60*60*12 );
       
       lws_add_http_header_status( wsi, 200, &header_cur, header_end );
       lws_add_http_header_by_token ( wsi, WSI_TOKEN_HTTP_SET_COOKIE, (const unsigned char *)cookie, strlen(cookie),
                                     &header_cur, header_end );
       /*lws_add_http_header_content_length ( wsi, buf->use, &header_cur, header_end );*/
       retour=TRUE;
     }
    else
     { lws_add_http_header_status( wsi, 401, &header_cur, header_end ); }                                     /* Unauthorized */

    lws_finalize_http_header ( wsi, &header_cur, header_end );
    *header_cur='\0';                                                                               /* Caractere null d'arret */
    lws_write( wsi, header, header_cur - header, LWS_WRITE_HTTP_HEADERS );
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
