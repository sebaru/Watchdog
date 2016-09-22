/******************************************************************************************************************************/
/* Watchdogd/Http/getstatus.c       Gestion des request getstatus pour le thread HTTP de watchdog                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        sam. 15 juin 2013 11:44:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * getstatus.c
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
/* Http_Traiter_request_getstatus: Traite une requete sur l'URI status                                                        */
/* Entrées: la connexion Websocket                                                                                            */
/* Sortie : FALSE si pb                                                                                                       */
/******************************************************************************************************************************/
 gboolean Http_Traiter_request_getstatus ( struct lws *wsi )
  { unsigned char header[256], *header_cur, *header_end;
   	const char *content_type = "application/xml";
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    gint retour, num;
	   gchar host[128];

    buf = xmlBufferCreate();                                                                        /* Creation du buffer xml */
    if (buf == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getstatus : XML Buffer creation failed" );
       return(FALSE);
     }

    writer = xmlNewTextWriterMemory(buf, 0);                                                         /* Creation du write XML */
    if (writer == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getstatus : XML Writer creation failed" );
       xmlBufferFree(buf);
       return(FALSE);
     }

    retour = xmlTextWriterStartDocument(writer, NULL, "UTF-8", "yes" );                               /* Creation du document */
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getstatus : XML Start document failed" );
       xmlBufferFree(buf);
       return(FALSE);
     }

/*------------------------------------------------------- Dumping status -----------------------------------------------------*/
    retour = xmlTextWriterStartElement(writer, (const unsigned char *) "Status");
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getstatus : XML Failed to Start element status" );
       xmlBufferFree(buf);
       return(FALSE);
     }

    xmlTextWriterWriteComment(writer, (const unsigned char *)"Start dumping Status !!");

/*---------------------------------------------------- Dumping Identification ------------------------------------------------*/
    gethostname( host, sizeof(host) );
    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"Host",     "%s", host);
    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"Version",  "%s", VERSION);
    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"Instance", "%s", Config.instance_id);
    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"Start_time","%d", (int)Partage->start_time);
    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"Licence",  "GPLv2 or newer");
    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"Author_Name", "%s",
                                    "Sébastien LEFEVRE");
    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"Author_Email", "%s",
                                    "sebastien.lefevre@abls-habitat.fr");
/*------------------------------------------------------- Dumping Running config ---------------------------------------------*/
    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"Top",
                                     "%d", Partage->top);
    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"Bit_par_sec",
                                     "%d", Partage->audit_bit_interne_per_sec_hold);
    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"Tour_par_sec",
                                     "%d", Partage->audit_tour_dls_per_sec_hold );
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                                              /* Synchro */
    num = g_slist_length( Partage->com_msrv.liste_i );                                            /* Recuperation du nbr de i */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"I_a_traiter", "%d", num );
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                                              /* Synchro */
    num = g_slist_length( Partage->com_msrv.liste_msg );                                       /* Recuperation du numero de i */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"MSG_a_traiter", "%d", num );
    pthread_mutex_lock( &Partage->com_msrv.synchro );                                                              /* Synchro */
    num = g_slist_length( Partage->com_msrv.liste_msg_repeat );                                           /* liste des repeat */
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
    xmlTextWriterWriteFormatElement( writer, (const unsigned char *)"MSG_en_repeat", "%d", num );
/*------------------------------------------------------- End dumping Status -------------------------------------------------*/
    xmlTextWriterWriteComment(writer, (const unsigned char *)"End dumping Status !!");
    retour = xmlTextWriterEndElement(writer);                                                                   /* End Status */
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getstatus : Failed to end element Status" );
       xmlBufferFree(buf);
       return(FALSE);
     }

    retour = xmlTextWriterEndDocument(writer);                                                                /* End document */
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getstatus : Failed to end Document" );
       xmlFreeTextWriter(writer);                                                                 /* Libération du writer XML */
       xmlBufferFree(buf);
       return(FALSE);
     }

    xmlFreeTextWriter(writer);                                                                    /* Libération du writer XML */

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
