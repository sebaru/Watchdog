/******************************************************************************************************************************/
/* Watchdogd/Http/postfile.c       Gestion des request postfile pour le thread HTTP de watchdog                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    25.03.2017 18:27:20 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * postfile.c
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
 
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Http.h"

/******************************************************************************************************************************/
/* Save_file_to_disk: Process le fichier recu et met a jour la base de données                                                 */
/* Entrées: replace!=0 si remplacement, id=numéro de fichier, les XMLData, et XMLLength                                       */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static gint Save_mp3_to_disk ( struct HTTP_SESSION *session, const gchar *name, gchar *buffer, gint taille )
  { gchar filename[80];
    gint id, retour, fd;
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %s) Trying to validate & save new MP3 file '%s' (length=%d)",
              __func__, Http_get_session_id(session), name, taille );

    if ( session==NULL || session->util==NULL || Tester_groupe_util( session->util, GID_MESSAGE )==FALSE )
     { return(HTTP_UNAUTHORIZED); }

    if (sscanf ( name, "%d", &id ) != 1) return(HTTP_BAD_REQUEST);

    g_snprintf( filename, sizeof(filename), "Son/%d.mp3", id );

    unlink(filename);                                                                      /* Suppression de l'ancien fichier */
    fd = open( filename, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR );                       /* Enregistrement du nouveau document */
    if (fd < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %s) Unable to create new file '%s' (%s)",
                 __func__, Http_get_session_id(session), filename, strerror(errno) );
	      return(HTTP_SERVER_ERROR);
     }

    retour = write( fd, buffer, taille );                                         /* Enregistrement du buffer dans le fichier */
    close(fd);
    if (retour != taille)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %s) Writing error for '%s' (%s)",
                 __func__, Http_get_session_id(session), filename, strerror(errno) );
       return(HTTP_SERVER_ERROR);
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
             "%s: (sid %s) New file saved: '%s'", __func__, Http_get_session_id(session), filename );
    return(HTTP_200_OK);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_postfile: Traite une requete de postfile                                                              */
/* Entrées: la connexion MHD                                                                                                  */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 gint Http_Traiter_request_postfile ( struct lws *wsi, struct HTTP_SESSION *session )
  { struct HTTP_PER_SESSION_DATA *pss;

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
             "%s: (sid %s) HTTP request", __func__, Http_get_session_id(session) );

    if (lws_hdr_total_length(wsi, WSI_TOKEN_GET_URI))                                 /* Header de type GET ?? Si oui, erreur */
     { Http_Send_response_code ( wsi, HTTP_BAD_METHOD );
       return(1);
     }

    pss = lws_wsi_user ( wsi );
    g_snprintf( pss->url, sizeof(pss->url), "/ws/postfile" );
    return(0);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_body_completion_postfile: le payload est arrivé, il faut traiter le fichier                           */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 gint Http_Traiter_request_body_completion_postfile ( struct lws *wsi )
  { unsigned char header[512], *header_cur, *header_end;
    struct HTTP_PER_SESSION_DATA *pss;
   	gchar token_type[12], token_name[20];
    const gchar *type, *name;
    gint retour, code;

    pss = lws_wsi_user ( wsi );

    type = lws_get_urlarg_by_name	( wsi, "type=", token_type, sizeof(token_type) );
    name = lws_get_urlarg_by_name	( wsi, "name=", token_name, sizeof(token_name) );

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %s) HTTP request for type='%s', name='%s'", __func__, Http_get_session_id(pss->session),
             (type ? type : "none"), (name ? name : "none") );

    if ( ! (type && name) )
     { Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                             /* Bad Request */
       g_free(pss->post_data);
       pss->post_data_length = 0;
       return(1);
     }

    code = HTTP_BAD_REQUEST;
    if (!strcasecmp(type,"dls"))
     { /* code = Save_dls_to_disk == FALSE); */
     }
    else if( !strcasecmp(type,"mp3"))
     { code = Save_mp3_to_disk( pss->session, name, pss->post_data, pss->post_data_length);
     }
    else if( !strcasecmp(type,"svg"))
      { /* Save_dls_to_disk */
     }
    else
     { Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                             /* Bad Request */
       g_free(pss->post_data);
       pss->post_data_length = 0;
       return(1);
     }
         
    Http_Send_response_code ( wsi, code );
    g_free(pss->post_data);
    pss->post_data_length = 0;
    if (code==HTTP_200_OK) return( lws_http_transaction_completed(wsi) );
    return(1);                                                                                         /* si erreur, on coupe */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
