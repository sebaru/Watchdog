/******************************************************************************************************************************/
/* Watchdogd/Http/getpluginsDLS.c       Gestion des request Plugins DLS pour le thread HTTP de watchdog                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    02.03.2017 14:26:57 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getpluginsDLS.c
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
 
 #include <string.h>
 #include <unistd.h>
 #include <libxml/xmlwriter.h>

/******************************************************* Prototypes de fonctions **********************************************/
 #include "watchdogd.h"
 #include "Http.h"
/******************************************************************************************************************************/
/* Http_Traiter_request_getpluginsDLS: Traite une requete sur l'URI pluginDLS                                                 */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gboolean Http_Traiter_request_getpluginsDLS ( struct lws *wsi, struct HTTP_SESSION *session )
  { unsigned char header[256], *header_cur, *header_end;
   	const char *content_type = "application/xml";
    gchar requete[1024], critere[512];
    struct CMD_TYPE_PLUGIN_DLS *dls;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    struct DB *db;
    gint retour;

/*

gchar token_length[12];
    const gchar *length_s;
    gint length;

        type_s   = lws_get_urlarg_by_name	( wsi, "shornttype=",    token_type,    sizeof(token_type) );

    g_snprintf( requete, sizeof(requete), "1=1" );
    if (type_s)
     { g_snprintf( critere, sizeof(critere), " AND msg.type=%d", atoi(type_s) );
       g_strlcat( requete, critere, sizeof(requete) );
     }

    if (num_s)
     { g_snprintf( critere, sizeof(critere), " AND msg.num=%d", atoi(num_s) );
       g_strlcat( requete, critere, sizeof(requete) );
     }

*/

/************************************************ Préparation du buffer XML ***************************************************/
    buf = xmlBufferCreate();                                                                        /* Creation du buffer xml */
    if (buf == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "%s : XML Buffer creation failed", __func__ );
       return(FALSE);
     }

    writer = xmlNewTextWriterMemory(buf, 0);                                                         /* Creation du write XML */
    if (writer == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "%s : XML Writer creation failed", __func__ );
       xmlBufferFree(buf);
       return(FALSE);
     }

    retour = xmlTextWriterStartDocument(writer, NULL, "UTF-8", "yes" );                               /* Creation du document */
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "%s : XML Start document failed", __func__ );
       xmlBufferFree(buf);
       return(FALSE);
     }
                                                                      /* Lancement de la requete de recuperation des messages */
    if ( ! Recuperer_plugins_dlsDB(&db) )
     { xmlFreeTextWriter(writer);                                                                 /* Libération du writer XML */
       xmlBufferFree(buf);                                            /* Libération du buffer dont nous n'avons plus besoin ! */
       return(FALSE);
     }
    xmlTextWriterWriteComment(writer, (const unsigned char *)"Start dumping messages !!");
    xmlTextWriterStartElement(writer, (const unsigned char *) "PluginsDLS");
/*------------------------------------------------------- Dumping message ----------------------------------------------------*/
    while ( (dls=Recuperer_plugins_dlsDB_suite( &db )) != NULL )                 /* Mise en forme avant envoi au client léger */
     { xmlTextWriterStartElement(writer, (const unsigned char *) "PluginDLS");
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"id", "%d", dls->id );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"nom", "%s", dls->nom );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"shortname", "%s", dls->shortname );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"enable", "%d", dls->on );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"type", "%d", dls->type );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"compil_date", "%d", dls->compil_date );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"compil_status", "%d", dls->compil_status );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"syn_groupe", "%s", dls->syn_groupe );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"syn_page", "%s", dls->syn_page );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"syn_id", "%d", dls->syn_id );
       xmlTextWriterEndElement(writer);                                                                        /* End Element */
       g_free(dls);
     }
    xmlTextWriterEndElement(writer);                                                                          /* End messages */
    xmlTextWriterWriteComment(writer, (const unsigned char *)"Dumping Plugins done !");
    retour = xmlTextWriterEndDocument(writer);                                                                /* End document */
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "%s : Failed to end Document", __func__ );
       xmlFreeTextWriter(writer);                                                                 /* Libération du writer XML */
       xmlBufferFree(buf);
       return(FALSE);
     }
    xmlFreeTextWriter(writer);                                                                    /* Libération du writer XML */

/*************************************************** Envoi au client **********************************************************/
    header_cur = header;
    header_end = header + sizeof(header);
    
    retour = lws_add_http_header_status( wsi, 200, &header_cur, header_end );
    retour = lws_add_http_header_by_token ( wsi, WSI_TOKEN_HTTP_CONTENT_TYPE, (const unsigned char *)content_type, strlen(content_type),
                                           &header_cur, header_end );
    retour = lws_add_http_header_content_length ( wsi, buf->use, &header_cur, header_end );
    retour = lws_finalize_http_header ( wsi, &header_cur, header_end );
    *header_cur='\0';                                                                               /* Caractere null d'arret */
    lws_write( wsi, header, header_cur - header, LWS_WRITE_HTTP_HEADERS );
    lws_write ( wsi, buf->content, buf->use, LWS_WRITE_HTTP);                                               /* Send to client */
    xmlBufferFree(buf);                                               /* Libération du buffer dont nous n'avons plus besoin ! */
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
