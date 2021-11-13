/******************************************************************************************************************************/
/* Watchdogd/Http/postsvg.c       Gestion des request postSVG pour le thread HTTP de watchdog                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    24.02.2017 19:18:12 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * postsvg.c
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
/* Save_SVG_to_disk: Process le fichier recu et met a jour la base de données                                                 */
/* Entrées: replace!=0 si remplacement, id=numéro de fichier, les XMLData, et XMLLength                                       */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static gboolean Save_SVG_to_disk ( struct HTTP_SESSION *session, gchar *xmldata, gint xmldata_length )
  { xmlChar *xml_description, *xml_classe;
    xmlNode *root_node, node;
    struct ICONEDBNEW icone;
    gchar filename[80];
    gboolean check;
    xmlDocPtr doc;
    gint fd, id;

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %.12s) Trying to validate & save new SVG file (length=%d)",
              __func__, Http_get_session_id(session), xmldata_length );
                  
    doc = xmlReadMemory( xmldata, xmldata_length, "newsvg.xml", NULL, 0 );
    if (doc == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %.12s) Unable to Parse new Doc '%s'", __func__, Http_get_session_id(session), filename );
	      return(FALSE);
     }
    root_node = xmlDocGetRootElement(doc);                                                        /* Vérification du document */
    if ( strcmp(root_node->name, "svg") )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %.12s) Root node is not SVG", __func__, Http_get_session_id(session) );
       xmlFreeDoc(doc);                                                                                        /* Parsing NOK */
       return(FALSE);
     }

    xml_classe = xmlGetProp(root_node, "wtd-classe");
    if (xml_classe == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %.12s) Properties 'wtd-classe' not found", __func__, Http_get_session_id(session) );
       xmlFreeDoc(doc);                                                                                        /* Parsing NOK */
       return(FALSE);
     }
    g_snprintf( icone.classe, sizeof(icone.classe), "%s", xml_classe );
    xmlFree(xml_classe);

    xml_description = xmlGetProp(root_node, "wtd-description");
    if (xml_description == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %.12s) Properties 'wtd-description' not found", __func__, Http_get_session_id(session) );
       xmlFreeDoc(doc);                                                                                        /* Parsing NOK */
       return(FALSE);
     }
    g_snprintf( icone.description, sizeof(icone.description), "%s", xml_description );
    xmlFree(xml_description);

    xmlFreeDoc(doc);                                /* Parsing OK, on peut libérer le doc et enregistrer le buffer sur disque */

    id = Ajouter_Modifier_iconenewDB ( &icone );
    if (id==-1) return(FALSE);
    
    g_snprintf( filename, sizeof(filename), "Svg/%d.svg", id );
    unlink(filename);                                                                      /* Suppression de l'ancien fichier */
    fd = open( filename, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR );                       /* Enregistrement du nouveau document */
    if (fd < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %.12s) Unable to create new file '%s' (%s)",
                 __func__, Http_get_session_id(session), filename, strerror(errno) );
	      return(FALSE);
     }

    if (write( fd, xmldata, xmldata_length ) != xmldata_length)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %.12s) Writing error for '%s' (%s)",
                 __func__, Http_get_session_id(session), filename, strerror(errno) );
	      return(FALSE);
     }
    else
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
                "%s: (sid %.12s) New SVG file saved: '%s'", __func__, Http_get_session_id(session), filename );
     }
    close(fd);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_postsvg: Traite une requete de postsvg                                                                */
/* Entrées: la connexion MHD                                                                                                  */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 gint Http_Traiter_request_postsvg ( struct lws *wsi, struct HTTP_SESSION *session, gchar *remote_name, gchar *remote_ip )
  { struct HTTP_PER_SESSION_DATA *pss;

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
             "%s: (sid %.12s) HTTP request from %s(%s)",
              __func__, Http_get_session_id(session), remote_name, remote_ip );

    pss = lws_wsi_user ( wsi );
    g_snprintf( pss->url, sizeof(pss->url), "/ws/postsvg" );
    return(0);                                                        /* si pas de session, on continue de traiter la request */
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_body_completion_postsvg: le payload est arrivé, il faut traiter le SVG                                */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 gint Http_Traiter_request_body_completion_postsvg ( struct lws *wsi )
  { unsigned char header[512], *header_cur, *header_end;
    struct HTTP_PER_SESSION_DATA *pss;
    gint retour, taille, replace = -1, id = -1;

    pss = lws_wsi_user ( wsi );

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %.12s) HTTP request body completion", __func__, Http_get_session_id(pss->session) );

    header_cur = header;                                                             /* Préparation des headers de la réponse */
    header_end = header + sizeof(header);

    if (Save_SVG_to_disk ( pss->session, pss->post_data, pss->post_data_length ) == FALSE)
     { retour = lws_add_http_header_status( wsi, 500, &header_cur, header_end ); }
    else
     { retour = lws_add_http_header_status( wsi, 200, &header_cur, header_end ); }

    retour = lws_finalize_http_header ( wsi, &header_cur, header_end );
    *header_cur='\0';                                                                            /* Caractere null d'arret */
    lws_write( wsi, header, header_cur - header, LWS_WRITE_HTTP_HEADERS );

    g_free(pss->post_data);
    pss->post_data_length = 0;
    return(lws_http_transaction_completed(wsi));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
