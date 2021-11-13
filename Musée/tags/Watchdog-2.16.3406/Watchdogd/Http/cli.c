/******************************************************************************************************************************/
/* Watchdogd/Http/cli.c       Gestion des request cli pour le thread HTTP de watchdog                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    26.12.2017 15:56:43 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * cli.c
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

 static const char *PARAM_CLI[] =
  { "user", "id", "host", "commande" };
 enum
  { PARAM_CLI_USER,
    PARAM_CLI_ID,
    PARAM_CLI_HOST,
    PARAM_CLI_COMMANDE,
    NBR_PARAM_CLI
  };
/******************************************************************************************************************************/
/* Http_Traiter_request_cli: Traite une requete de cli                                                                        */
/* Entrées: la connexion MHD                                                                                                  */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 gint Http_Traiter_request_body_cli ( struct lws *wsi, void *data, size_t taille )
  { struct HTTP_PER_SESSION_DATA *pss;

    pss = lws_wsi_user ( wsi );
    if (lws_hdr_total_length(wsi, WSI_TOKEN_GET_URI))                                 /* Header de type GET ?? Si oui, erreur */
     { Http_Send_response_code ( wsi, HTTP_BAD_METHOD );
       return(1);
     }

    if (!pss->spa)
     {	pss->spa = lws_spa_create(wsi, PARAM_CLI, NBR_PARAM_CLI, 256, NULL, NULL );
    			if (!pss->spa)	return(1);
     }
    return(lws_spa_process(pss->spa, data, taille));
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_body_completion_cli: le payload est arrivé, il faut traiter le fichier                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 gint Http_Traiter_request_body_completion_cli ( struct lws *wsi )
  { unsigned char header[512], *header_cur, *header_end;
    gchar *user, *commande, *id, *host;
    struct HTTP_PER_SESSION_DATA *pss;
    gchar *buffer;
    gint retour;

    pss = lws_wsi_user ( wsi );
    lws_spa_finalize(pss->spa);

    if (! (lws_spa_get_length(pss->spa, PARAM_CLI_USER)>0 ) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "%s: (sid %s) 'USER' parameter is missing", __func__, Http_get_session_id(pss->session) );
       Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                                      /* Bad Request */
       return(1);
     }
    if (! (lws_spa_get_length(pss->spa, PARAM_CLI_ID)>0 ) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "%s: (sid %s) 'ID' parameter is missing", __func__, Http_get_session_id(pss->session) );
       Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                                      /* Bad Request */
       return(1);
     }
    if (! (lws_spa_get_length(pss->spa, PARAM_CLI_HOST)>0 ) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "%s: (sid %s) 'HOST' parameter is missing", __func__, Http_get_session_id(pss->session) );
       Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                                      /* Bad Request */
       return(1);
     }
    if (! (lws_spa_get_length(pss->spa, PARAM_CLI_COMMANDE)>0) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "%s: (sid %s) 'COMMAND' parameter is missing", __func__, Http_get_session_id(pss->session) );
       Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                                      /* Bad Request */
       return(1);
     }

    user     = lws_spa_get_string ( pss->spa, PARAM_CLI_USER );
    id       = lws_spa_get_string ( pss->spa, PARAM_CLI_ID );
    host     = lws_spa_get_string ( pss->spa, PARAM_CLI_HOST );
    commande = lws_spa_get_string ( pss->spa, PARAM_CLI_COMMANDE );

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: HTTP/CLI request for user '%s' ('%s') from '%s' : '%s'", __func__, user, id, host, commande );
             
    buffer = Processer_commande_admin ( user, host, commande );
    Http_Send_response_code_with_buffer ( wsi, HTTP_200_OK, "text/plain", buffer, strlen(buffer) );
    g_free(buffer);

    lws_spa_destroy ( pss->spa	);
    pss->post_data_length = 0;
    g_free(pss->post_data);
    return(1);                                                                                         /* si erreur, on coupe */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
