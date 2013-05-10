/**********************************************************************************************************/
/* Watchdogd/Http/set_internal.c       Gestion des request set_internal pour le thread HTTP de watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 05 mai 2013 16:33:43 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * set_internal.c
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
 gboolean Http_Traiter_request_set_internal ( struct MHD_Connection *connection )
  { const char *Handled_OK = "<html><body>OK</body></html>";
    gint retour, type_int, value_int;
    struct MHD_Response *response;
    xmlTextWriterPtr writer;
    xmlBufferPtr buf;
    gchar *type, *value;
    struct DB *db;

#ifdef bouh
    type = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, (const char *)"type" );
    if (!type)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                "Http_Traiter_request_set_internal: type not found in URL argument." );
       return(FALSE);
     }

    value = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, (const char *)"value" );
    if (!value)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                "Http_Traiter_request_set_internal: value not found in URL argument." );
       return(FALSE);
     }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_INFO,
             "Http_Traiter_request_set_internal: Setting Internal bit %s to %s", type, value );
#endif

    response = MHD_create_response_from_buffer ( strlen (Handled_OK),
                                                (void*)Handled_OK, MHD_RESPMEM_PERSISTENT);
    if (response == NULL)                      /* Si erreur de creation de la reponse, on sort une erreur */
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_WARNING,
                "Http_Traiter_request_set_internal: Response Creation Error." );
       return(FALSE);
     }
    MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
