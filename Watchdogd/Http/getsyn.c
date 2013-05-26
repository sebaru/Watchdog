/**********************************************************************************************************/
/* Watchdogd/Http/getsyn.c       Gestion des request getsyn pour le thread HTTP de watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 05 mai 2013 16:33:43 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * getsyn.c
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
 #include <microhttpd.h>
 #include <libxml/xmlwriter.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Http.h"
/**********************************************************************************************************/
/* Traiter_dynxml: Traite une requete sur l'URI dynxml                                                    */
/* Entrées: la connexion MHD                                                                              */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 gboolean Http_Traiter_request_getsyn ( struct MHD_Connection *connection )
  { struct CMD_TYPE_SYNOPTIQUE *syndb;
    struct MHD_Response *response;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    gchar *syn_id_char;
    struct DB *db;
    gint retour, syn_id;

    syn_id_char = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, "syn_id" );
    if (!syn_id_char) { syn_id = 1; }
                 else { syn_id = atoi(syn_id_char); }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getsyn : DB Connexion failed" );
       return(FALSE);
     }

    buf = xmlBufferCreate();                                                    /* Creation du buffer xml */
    if (buf == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getsyn : XML Buffer creation failed" );
       Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    writer = xmlNewTextWriterMemory(buf, 0);                                     /* Creation du write XML */
    if (writer == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getsyn : XML Writer creation failed" );
       xmlBufferFree(buf);
       Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    retour = xmlTextWriterStartDocument(writer, NULL, "UTF-8", "yes" );           /* Creation du document */
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getsyn : XML Start document failed" );
       xmlBufferFree(buf);
       Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    syndb = Rechercher_synoptiqueDB ( Config.log, db, syn_id );
    if ( ! syndb )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                 "Http_Traiter_request_getsyn : Synoptique %d not found in DB", syn_id );
       xmlBufferFree(buf);
       Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    retour = xmlTextWriterStartElement(writer, (const unsigned char *) "synoptique");
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getsyn : XML Failed to Start element synoptique" );
       xmlBufferFree(buf);
       Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"id",      "%d", syndb->id );
    xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"groupe",  "%s", syndb->groupe );
    xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"page",    "%s", syndb->page );
    xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"libelle", "%s", syndb->libelle );
    g_free(syndb);                                           /* On a terminé avec la structure synoptique */

/*------------------------------------------- Dumping Passerelle -----------------------------------------*/
    xmlTextWriterWriteComment(writer, (const unsigned char *)"Start dumping passerelles !!");
    if ( Recuperer_passerelleDB( Config.log, db, syn_id ) )
     { for ( ; ; )
        { struct CMD_TYPE_PASSERELLE *pass;
          pass = Recuperer_passerelleDB_suite( Config.log, db );
          if (!pass) break;                                                                 /* Terminé ?? */

          xmlTextWriterStartElement(writer, (const unsigned char *)"passerelle");     /* Start Passerelle */
          xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"id",           "%d", pass->id );
          xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"syn_cible_id", "%d", pass->syn_cible_id );
          xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"libelle",      "%s", pass->libelle );
          xmlTextWriterEndElement(writer);                                              /* End passerelle */
          g_free(pass);
        }
     }
    xmlTextWriterWriteComment(writer, (const unsigned char *)"End dumping passerelles !!");

/*------------------------------------------- Dumping capteur --------------------------------------------*/
    xmlTextWriterWriteComment(writer, (const unsigned char *)"Start dumping capteurs !!");
    if ( Recuperer_capteurDB( Config.log, db, syn_id ) )
     { for ( ; ; )
        { struct CMD_TYPE_CAPTEUR *capteur;
          capteur = Recuperer_capteurDB_suite( Config.log, db );
          if (!capteur) break;                                                              /* Terminé ?? */

          xmlTextWriterStartElement(writer, (const unsigned char *)"capteur");           /* Start Capteur */
          xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"id",      "%d", capteur->id );
          xmlTextWriterWriteFormatAttribute( writer, (const unsigned char *)"libelle", "%s", capteur->libelle );
          xmlTextWriterEndElement(writer);                                              /* End passerelle */
          g_free(capteur);
        }
     }
    xmlTextWriterWriteComment(writer, (const unsigned char *)"End dumping capteurs !!");

    Libere_DB_SQL( Config.log, &db );                           /* On a plus besoin de la base de données */

    retour = xmlTextWriterEndElement(writer);                                           /* End synoptique */
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getsyn : Failed to end element Synoptique" );
       xmlBufferFree(buf);
       return(FALSE);
     }

    retour = xmlTextWriterEndDocument(writer);
    if (retour < 0)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                 "Http_Traiter_request_getsyn : Failed to end Document" );
       xmlBufferFree(buf);
       return(FALSE);
     }

    xmlFreeTextWriter(writer);                                                /* Libération du writer XML */
    response = MHD_create_response_from_buffer (buf->use, buf->content, MHD_RESPMEM_MUST_COPY); /* Response */
    xmlBufferFree(buf);                           /* Libération du buffer dont nous n'avons plus besoin ! */
    if (response == NULL) return(FALSE);       /* Si erreur de creation de la reponse, on sort une erreur */
    MHD_add_response_header (response, "Content-Type", "application/xml");
    MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
