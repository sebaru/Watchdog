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

 static const char *PARAM_POSTFILE[] =
  { "type", "id" };
 enum
  { PARAM_POSTFILE_TYPE,
    PARAM_POSTFILE_ID,
    NBR_PARAM_POSTFILE
  };
/******************************************************************************************************************************/
/* Save_SVG_to_disk: Process le fichier recu et met a jour la base de données                                                 */
/* Entrées: replace!=0 si remplacement, id=numéro de fichier, les XMLData, et XMLLength                                       */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static gint Save_SVG_to_disk ( struct HTTP_SESSION *session, gchar *xmldata, gint xmldata_length )
  { xmlChar *xml_description, *xml_classe;
    xmlNode *root_node, node;
    struct ICONEDBNEW icone;
    gchar filename[80];
    gboolean check;
    xmlDocPtr doc;
    gint fd, id;

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %s) Trying to validate & save new SVG file (length=%d)",
              __func__, Http_get_session_id(session), xmldata_length );
                  
    doc = xmlReadMemory( xmldata, xmldata_length, "newsvg.xml", NULL, 0 );
    if (doc == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %s) Unable to Parse new Doc '%s'", __func__, Http_get_session_id(session), filename );
	      return(HTTP_BAD_REQUEST);
     }
    root_node = xmlDocGetRootElement(doc);                                                        /* Vérification du document */
    if ( strcmp(root_node->name, "svg") )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %s) Root node is not SVG", __func__, Http_get_session_id(session) );
       xmlFreeDoc(doc);                                                                                        /* Parsing NOK */
       return(HTTP_BAD_REQUEST);
     }

    xml_classe = xmlGetProp(root_node, "wtd-classe");
    if (xml_classe == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %s) Properties 'wtd-classe' not found", __func__, Http_get_session_id(session) );
       xmlFreeDoc(doc);                                                                                        /* Parsing NOK */
       return(HTTP_BAD_REQUEST);
     }
    g_snprintf( icone.classe, sizeof(icone.classe), "%s", xml_classe );
    xmlFree(xml_classe);

    xml_description = xmlGetProp(root_node, "wtd-description");
    if (xml_description == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %s) Properties 'wtd-description' not found", __func__, Http_get_session_id(session) );
       xmlFreeDoc(doc);                                                                                        /* Parsing NOK */
       return(HTTP_BAD_REQUEST);
     }
    g_snprintf( icone.description, sizeof(icone.description), "%s", xml_description );
    xmlFree(xml_description);

    xmlFreeDoc(doc);                                /* Parsing OK, on peut libérer le doc et enregistrer le buffer sur disque */

    id = Ajouter_Modifier_iconenewDB ( &icone );
    if (id==-1) return(HTTP_SERVER_ERROR);

    g_snprintf( filename, sizeof(filename), "Svg/%d.svg", id );
    unlink(filename);                                                                      /* Suppression de l'ancien fichier */
    fd = open( filename, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR );                       /* Enregistrement du nouveau document */
    if (fd < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %s) Unable to create new file '%s' (%s)",
                 __func__, Http_get_session_id(session), filename, strerror(errno) );
	      return(HTTP_SERVER_ERROR);
     }

    if (write( fd, xmldata, xmldata_length ) != xmldata_length)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "%s: (sid %s) Writing error for '%s' (%s)",
                 __func__, Http_get_session_id(session), filename, strerror(errno) );
       close(fd);
	      return(HTTP_SERVER_ERROR);
     }
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
             "%s: (sid %s) New SVG file saved: '%s'", __func__, Http_get_session_id(session), filename );
    close(fd);
    return(TRUE);
  }  
/******************************************************************************************************************************/
/* Save_file_to_disk: Process le fichier recu et met a jour la base de données                                                */
/* Entrées: replace!=0 si remplacement, id=numéro de fichier, les XMLData, et XMLLength                                       */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 static gint Save_file_to_disk ( struct HTTP_SESSION *session, const gchar *filename, const gchar *buffer, const gint taille )
  { gint retour, fd;
    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %s) Trying to save new file '%s' (length=%d)",
              __func__, Http_get_session_id(session), filename, taille );

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
             "%s: (sid %s) New file saved: '%s' length=%d", __func__, Http_get_session_id(session), filename, taille );
    return(HTTP_200_OK);
  }
/******************************************************************************************************************************/
/* lws_fileupload_cb: Récupère petit a petit le fichier en cours d'upload                                                     */
/* Entrées: la connexion WSI, le buffer, la taille, le statut                                                                 */
/* Sortie : 1 si erreur, 0 si poursuite                                                                                       */
/******************************************************************************************************************************/
 static int lws_fileupload_cb (void *data, const char *name, const char *filename, char *buf, int len, enum lws_spa_fileupload_states state)
  { struct lws *wsi;
    wsi = (struct lws *)data;
    return(Http_CB_file_upload( wsi, buf, len ));
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_postfile: Traite une requete de postfile                                                              */
/* Entrées: la connexion MHD                                                                                                  */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 gint Http_Traiter_request_body_postfile ( struct lws *wsi, void *data, size_t taille )
  { struct HTTP_PER_SESSION_DATA *pss;

    pss = lws_wsi_user ( wsi );
    if (lws_hdr_total_length(wsi, WSI_TOKEN_GET_URI))                                 /* Header de type GET ?? Si oui, erreur */
     { Http_Send_response_code ( wsi, HTTP_BAD_METHOD );
       return(1);
     }

    if (!pss->spa)
     {	pss->spa = lws_spa_create(wsi, PARAM_POSTFILE, NBR_PARAM_POSTFILE, 256, lws_fileupload_cb, wsi );
    			if (!pss->spa)	return(1);
     }
    return(lws_spa_process(pss->spa, data, taille));
  }
/******************************************************************************************************************************/
/* Http_Traiter_request_body_completion_postfile: le payload est arrivé, il faut traiter le fichier                           */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : 0 ou 1 selon si la transaction est completed                                                                      */
/******************************************************************************************************************************/
 gint Http_Traiter_request_body_completion_postfile ( struct lws *wsi )
  { unsigned char header[512], *header_cur, *header_end;
    struct HTTP_PER_SESSION_DATA *pss;
    gint retour, code, id;
    const gchar *type;

    pss = lws_wsi_user ( wsi );
    lws_spa_finalize(pss->spa);

    if (! (lws_spa_get_length(pss->spa, PARAM_POSTFILE_ID)>0 ) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "%s: (sid %s) 'ID' parameter is missing", __func__, Http_get_session_id(pss->session) );
       Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                                      /* Bad Request */
       return(1);
     }
    if (! (lws_spa_get_length(pss->spa, PARAM_POSTFILE_TYPE)>0) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                "%s: (sid %s) 'TYPE' parameter is missing", __func__, Http_get_session_id(pss->session) );
       Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                                      /* Bad Request */
       return(1);
     }

    type = lws_spa_get_string ( pss->spa, PARAM_POSTFILE_TYPE );
    if (sscanf ( lws_spa_get_string ( pss->spa, PARAM_POSTFILE_ID ), "%d", &id ) != 1) id=-1;

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "%s: (sid %s) HTTP request for type='%s', id='%d'", __func__, Http_get_session_id(pss->session),
              type, id );

    code = HTTP_BAD_REQUEST;
    if (!strcasecmp(type,"dls"))
     { /* code = Save_dls_to_disk == FALSE); */
     }
    else if( !strcasecmp(type,"mp3"))
     { if (id==-1)
        { code = HTTP_BAD_REQUEST; }
       else
        { gchar filename[80];
          g_snprintf( filename, sizeof(filename), "Son/%d.mp3", id );
          code = Save_file_to_disk( pss->session, filename, pss->post_data, pss->post_data_length);
        }
     }
    else if( !strcasecmp(type,"svg"))
     { code = Save_file_to_disk( pss->session, "unused", pss->post_data, pss->post_data_length);
     }
    else
     { Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );                                                      /* Bad Request */
       return(1);
     }
         
    lws_spa_destroy ( pss->spa	);
    Http_Send_response_code ( wsi, code );
    pss->post_data_length = 0;
    g_free(pss->post_data);
    if (code==HTTP_200_OK) return( lws_http_transaction_completed(wsi) );
    return(1);                                                                                         /* si erreur, on coupe */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
