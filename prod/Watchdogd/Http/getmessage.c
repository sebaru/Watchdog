/******************************************************************************************************************************/
/* Watchdogd/Http/getmessage.c       Gestion des request getmessage pour le thread HTTP de watchdog                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    22.09.2016 14:18:41 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getmessage.c
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
/* Http_Traiter_request_getmessage: Traite une requete sur l'URI message                                                      */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gboolean Http_Traiter_request_getmessage ( struct lws *wsi, struct HTTP_SESSION *session )
  { unsigned char header[256], *header_cur, *header_end;
   	const char *content_type = "application/xml";
    gchar requete[256], critere[128];
    gchar token_length[12], *length_s;
    gchar token_start[12], *start_s;
    gchar token_type[12], *type_s;
    struct CMD_TYPE_MESSAGE *msg;
    gint type, start, length;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    struct DB *db;
    gint retour;

    type_s   = "0"; //lws_get_urlarg_by_name	( wsi, "type",   token_type,   sizeof(token_type) );
    if (type_s)   { type   = atoi ( type_s );   } else { type   = -1; }
    start_s  = "0"; //lws_get_urlarg_by_name	( wsi, "start",  token_start,  sizeof(token_start) );
    if (start_s)  { start  = atoi ( start_s );  } else { start  = -1; }
    length_s = "100";//lws_get_urlarg_by_name	( wsi, "length", token_length, sizeof(token_length) );
    if (length_s) { length = atoi ( length_s ); } else { length = -1; }

    memset( requete, 0, sizeof(requete) );                                                   /* Critere de choix des messages */
    if (type != -1)
     { g_snprintf( critere, sizeof(critere), " %s.type=%d", NOM_TABLE_MSG, type );
       g_strlcat( requete, critere, sizeof(requete) );
     }

/************************************************ Préparation du buffer XML ***************************************************/
    buf = xmlBufferCreate();                                                                        /* Creation du buffer xml */
    if (buf == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getmessage : XML Buffer creation failed" );
       return(FALSE);
     }

    writer = xmlNewTextWriterMemory(buf, 0);                                                         /* Creation du write XML */
    if (writer == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getmessage : XML Writer creation failed" );
       xmlBufferFree(buf);
       return(FALSE);
     }

    retour = xmlTextWriterStartDocument(writer, NULL, "UTF-8", "yes" );                               /* Creation du document */
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getmessage : XML Start document failed" );
       xmlBufferFree(buf);
       return(FALSE);
     }
                                                                      /* Lancement de la requete de recuperation des messages */
    if ( ! Recuperer_messageDB_with_conditions( &db, requete, start, length ) )
     { xmlFreeTextWriter(writer);                                                                 /* Libération du writer XML */
       xmlBufferFree(buf);                                            /* Libération du buffer dont nous n'avons plus besoin ! */
       return(FALSE);
     }
    xmlTextWriterWriteComment(writer, (const unsigned char *)"Start dumping messages !!");
    xmlTextWriterStartElement(writer, (const unsigned char *) "Messages");
/*------------------------------------------------------- Dumping message ----------------------------------------------------*/
    while ( (msg=Recuperer_messageDB_suite( &db )) != NULL )                     /* Mise en forme avant envoi au client léger */
     { xmlTextWriterStartElement(writer, (const unsigned char *) "Message");
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"ID", "%d", msg->id );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"num", "%d", msg->num );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"type", "%d", msg->type );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"enable", "%d", msg->enable );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"type", "%d", msg->type );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"sms", "%d", msg->sms );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"bit_voc", "%d", msg->bit_voc );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"vitesse_voc", "%d", msg->vitesse_voc );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"type_voc", "%d", msg->type_voc );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"time_repeat", "%d", msg->time_repeat );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"libelle", "%s", msg->libelle );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"libelle_audio", "%s", msg->libelle_audio );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"libelle_sms", "%s", msg->libelle_sms );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"syn_groupe", "%s", msg->syn_groupe );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"syn_page", "%s", msg->syn_page );
       xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"syn_libelle", "%s", msg->syn_libelle );
       xmlTextWriterEndElement(writer);                                                                        /* End message */
       g_free(msg);
     }
    xmlTextWriterEndElement(writer);                                                                          /* End messages */
    xmlTextWriterWriteComment(writer, (const unsigned char *)"Dumping messages done !");
    retour = xmlTextWriterEndDocument(writer);                                                                /* End document */
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getmessage : Failed to end Document" );
       xmlFreeTextWriter(writer);                                                                 /* Libération du writer XML */
       xmlBufferFree(buf);
       return(FALSE);
     }
    xmlFreeTextWriter(writer);                                                                    /* Libération du writer XML */

/*************************************************** Envoi au client **********************************************************/
    header_cur = header;
    header_end = header + sizeof(header);
    
    lws_add_http_header_status( wsi, 200, &header_cur, header_end );
    lws_add_http_header_by_token ( wsi, WSI_TOKEN_HTTP_CONTENT_TYPE, (const unsigned char *)content_type, strlen(content_type),
                                  &header_cur, header_end );
    lws_add_http_header_content_length ( wsi, buf->use, &header_cur, header_end );
    lws_finalize_http_header ( wsi, &header_cur, header_end );
    *header_cur='\0';                                                                               /* Caractere null d'arret */
    lws_write( wsi, header, header_cur - header, LWS_WRITE_HTTP_HEADERS );
    lws_write ( wsi, buf->content, buf->use, LWS_WRITE_HTTP);                                               /* Send to client */
    xmlBufferFree(buf);                                               /* Libération du buffer dont nous n'avons plus besoin ! */
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
