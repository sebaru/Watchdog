/******************************************************************************************************************************/
/* Watchdogd/Http/getmnemo.c       Gestion des request getmnemo pour le thread HTTP de watchdog                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    22.09.2016 14:18:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getmnemo.c
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
/* Http_Traiter_request_getmnemo: Traite une requete sur l'URI mnemo                                                          */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gboolean Http_Traiter_request_getmnemo ( struct lws *wsi, struct HTTP_SESSION *session, gchar *url )
  { unsigned char header[256], *header_cur, *header_end;
   	const gchar *num_s, *type_s;
    const char *content_type = "application/xml";
    struct CMD_TYPE_MNEMO_BASE *mnemo;
    struct CMD_TYPE_NUM_MNEMONIQUE critere;
    gchar token_num[12], token_type[12];
    gchar requete[256];
    gint type, num;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    gint retour;

    if ( session==NULL || session->util==NULL || Tester_groupe_util( session->util, GID_MNEMO)==FALSE)
     { Http_Send_error_code ( wsi, 401 );
       return(TRUE);
     }

    type_s = lws_get_urlarg_by_name	( wsi, "type=",   token_type,   sizeof(token_type) );
    if (type_s) { critere.type = atoi ( type_s ); } else { critere.type   = -1; }
    num_s  = lws_get_urlarg_by_name	( wsi, "num=",  token_num,  sizeof(token_num) );
    if (num_s)  { critere.num  = atoi ( num_s );  } else { critere.num  = -1; }

/************************************************ Préparation du buffer XML ***************************************************/
    buf = xmlBufferCreate();                                                                        /* Creation du buffer xml */
    if (buf == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getmnemo : XML Buffer creation failed" );
       return(FALSE);
     }

    writer = xmlNewTextWriterMemory(buf, 0);                                                         /* Creation du write XML */
    if (writer == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getmnemo : XML Writer creation failed" );
       xmlBufferFree(buf);
       return(FALSE);
     }

    retour = xmlTextWriterStartDocument(writer, NULL, "UTF-8", "yes" );                               /* Creation du document */
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getmnemo : XML Start document failed" );
       xmlBufferFree(buf);
       return(FALSE);
     }
                                                                        /* Lancement de la requete de recuperation des mnemos */
    mnemo =  Rechercher_mnemo_baseDB_type_num ( &critere );
    if (!mnemo)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Traiter_request_getmnemo : Mnemo %d/%d not found", critere.type, critere.num );
       xmlFreeTextWriter(writer);                                                                 /* Libération du writer XML */
       xmlBufferFree(buf);                                            /* Libération du buffer dont nous n'avons plus besoin ! */
       return(FALSE);
     }
    xmlTextWriterWriteComment(writer, (const unsigned char *)"Start dumping mnemo !!");
    xmlTextWriterStartElement(writer, (const unsigned char *) "mnemo");
/*------------------------------------------------------- Dumping mnemo ----------------------------------------------------*/
    if ( ! strncasecmp ( url, "Light", 5 ) )
     { xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"libelle", "%s", mnemo->libelle );
       if (mnemo->type == MNEMO_ENTREE_ANA)
        {
          xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"unite", "test" );
        }
     }
    g_free(mnemo);
    xmlTextWriterEndElement(writer);                                                                             /* End mnemo */
    xmlTextWriterWriteComment(writer, (const unsigned char *)"Dumping mnemo done !");
    retour = xmlTextWriterEndDocument(writer);                                                                /* End document */
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getmnemo : Failed to end Document" );
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
