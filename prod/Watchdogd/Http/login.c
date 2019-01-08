/******************************************************************************************************************************/
/* Watchdogd/Http/login.c       Gestion des request login pour le thread HTTP de watchdog                                     */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    05.09.2016 15:01:29 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * login.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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

 static const char *PARAM_LOGIN[] =
  { "username", "password" };
 enum
  { PARAM_LOGIN_USERNAME,
    PARAM_LOGIN_PASSWORD,
    NBR_PARAM_LOGIN
  };

/******************************************************************************************************************************/
/* Http_get_session : Vérifie le numéro de session en parametre                                                               */
/* Entrées: le SID a tester                                                                                                   */
/* Sortie : la session, ou NULL si non trouvée                                                                                */
/******************************************************************************************************************************/
 gboolean Get_phpsessionid_cookie ( struct lws *wsi )
  { struct WS_PER_SESSION_DATA *pss;
    gchar buffer[4096];

    pss = lws_wsi_user ( wsi );
    g_snprintf( pss->sid, sizeof(pss->sid), "none" );
    if ( lws_hdr_total_length( wsi, WSI_TOKEN_HTTP_COOKIE ) <= 0) return(FALSE);
    if ( lws_hdr_copy( wsi, buffer, sizeof(buffer), WSI_TOKEN_HTTP_COOKIE ) != -1 )     /* Récupération de la valeur du token */
     { gchar *cookies, *cookie, *savecookies;
       gchar *cookie_name, *cookie_value, *savecookie;
       cookies = buffer;
       while ( (cookie=strtok_r( cookies, ";", &savecookies)) != NULL )                                  /* Découpage par ';' */
        { cookies=NULL;
          cookie_name=strtok_r( cookie, "=", &savecookie);                                               /* Découpage par "=" */
          if (cookie_name)
           { cookie_value = strtok_r ( NULL, "=", &savecookie );
             if (!strcmp(cookie_name, "PHPSESSID"))
              { g_snprintf( pss->sid, sizeof(pss->sid), "%s", cookie_value );
                return(TRUE);
              }
             Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                      "%s: Cookie found for: %s=%s", __func__,
                      (cookie_name ? cookie_name : "none"), (cookie_value ? cookie_value : "none") );
           }
        }       
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Http_get_session_id : Isole le SID d'une session d'une WSI                                                                 */
/* Entrées: le WSI en question                                                                                                */
/* Sortie : sid ou "---" si erreur                                                                                            */
/******************************************************************************************************************************/
 gchar *Http_get_session_id ( struct HTTP_SESSION *session )
  { if (session) return(session->sid_string);
    return("--- none ---");
  }
/******************************************************************************************************************************/
/* Http Liberer_session : Libere la mémoire réservée par la structure session                                                 */
/* Entrées : la session à libérer                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_Liberer_session ( struct HTTP_SESSION *session )
  { pthread_mutex_lock( &Cfg_http.lib->synchro );                                     /* Recherche dans la liste des sessions */
    Cfg_http.Liste_sessions = g_slist_remove ( Cfg_http.Liste_sessions, session );
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
             "%s: (sid %s) session closed", __func__, Http_get_session_id (session) );
    if (session->util) g_free(session->util);
    g_free(session);                                                                     /* Libération mémoire le cas échéant */
  }
/******************************************************************************************************************************/
/* Http Close_session : Termine une session sur demande terminal final                                                        */
/* Entrées : la session à libérer                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_Close_session ( struct lws *wsi, struct HTTP_SESSION *session )
  { unsigned char header[256], *header_cur, *header_end;
    gint retour;
    if (session)
     { header_cur = header;
       header_end = header + sizeof(header);
       retour = lws_add_http_header_status( wsi, 200, &header_cur, header_end );
       retour = lws_finalize_http_header ( wsi, &header_cur, header_end );
       *header_cur='\0';                                                                            /* Caractere null d'arret */
       lws_write( wsi, header, header_cur - header, LWS_WRITE_HTTP_HEADERS );                      /* Send to client */
       Http_Liberer_session ( session );
     }
  }
/******************************************************************************************************************************/
/* Http_new_session : Création d'une nouvelle session pour la requete reçue                                                   */
/* Entrées : la session WSI                                                                                                   */
/* Sortie  : NULL si erreur                                                                                                   */
/******************************************************************************************************************************/
 static struct HTTP_SESSION *Http_new_session( struct lws *wsi, gchar *remote_name, gchar *remote_ip )
  { gchar cookie_bin[EVP_MAX_MD_SIZE], cookie_hex[2*EVP_MAX_MD_SIZE+1], cookie[256];
    struct HTTP_SESSION *session;
    gchar buffer[4096];
    gint cpt;

    session = (struct HTTP_SESSION *) g_try_malloc0 ( sizeof( struct HTTP_SESSION ) );
    if (!session)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: Memory Alloc ERROR session", __func__ );
       return(NULL);
     }
    session->last_top = Partage->top;
    session->is_ssl = lws_is_ssl ( wsi );

    if ( lws_hdr_copy( wsi, buffer, sizeof(buffer), WSI_TOKEN_HTTP_USER_AGENT ) != -1 ) /* Récupération de la valeur du token */
     { g_snprintf( session->user_agent, sizeof(session->user_agent), "%s", buffer ); }

    g_snprintf( session->remote_name, sizeof(session->remote_name), "%s", remote_name );
    g_snprintf( session->remote_ip,   sizeof(session->remote_ip),   "%s", remote_ip   );

    RAND_bytes( (guchar *)cookie_bin, sizeof(cookie_bin) );                               /* Récupération d'un nouveau COOKIE */
    for (cpt=0; cpt<sizeof(cookie_bin); cpt++)                                                 /* Mise en forme au format HEX */
     { g_snprintf( &session->sid[2*cpt], 3, "%02X", (guchar)cookie_bin[cpt] ); }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: Creation session '%s' for %s/%s", __func__, session->sid, session->remote_name, session->remote_ip );

    pthread_mutex_lock( &Cfg_http.lib->synchro );                                         /* Ajout dans la liste des sessions */
    Cfg_http.Liste_sessions = g_slist_prepend( Cfg_http.Liste_sessions, session );
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
    return(session);
  }
/******************************************************************************************************************************/
/* Http_Check_sessions : Fait tomber les sessions sur timeout                                                                 */
/* Entrées: néant                                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Http_Check_sessions ( void )
  { struct HTTP_SESSION *session = NULL;
    GSList *liste;

search_again:
    pthread_mutex_lock( &Cfg_http.lib->synchro );                                            /* Ajout dans la liste a traiter */
    liste = Cfg_http.Liste_sessions;
    while ( liste )
     { session = (struct HTTP_SESSION *)liste->data;
       if (session->last_top && Partage->top - session->last_top >= 864000 )
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                   "%s: closing timeout for SID %.12s", __func__, session->sid );
          break;
        }
       liste = liste->next;
     }
    pthread_mutex_unlock( &Cfg_http.lib->synchro );
    if (liste)
     { Http_Liberer_session(session);
       goto search_again;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
