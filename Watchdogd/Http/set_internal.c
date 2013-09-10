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
 #include <libxml/xmlreader.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Http.h"
/**********************************************************************************************************/
/* Http_Traiter_XML_set_internal: Traite le document XML recu de la requete MHD                           */
/* Entrées: la structure de connexion info                                                                */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 void Http_Traiter_XML_set_internal ( struct HTTP_CONNEXION_INFO *infos )
  { const xmlChar *name, *value;
    xmlTextReaderPtr reader;
    reader =  xmlReaderForMemory( infos->buffer, infos->buffer_size, "set_internal.xml", NULL, XML_PARSE_NONET );
    if (reader == NULL)
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Traiter_XML_set_internal: Error Reading XML from buffer (size=%d, buffer=%s)",
                 infos->buffer_size, infos->buffer );
       return;
     }

    if (xmlTextReaderRead(reader) != 1 ) goto end;             /* On se positionne sur le premier element */

    name = xmlTextReaderConstName(reader);
    if ( name == NULL || (!xmlStrEqual ( name, (xmlChar*) "SatelliteInfos" )) )
     { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                "Http_Traiter_XML_set_internal: Error First Element <> SatelliteInfos (name=%s)",
                 (name ? name : (xmlChar *)"none") );
       goto end;
     }

    while ( xmlTextReaderRead(reader) == 1 )
     { name = xmlTextReaderConstName(reader);
       if (name != NULL)
	{ value = xmlTextReaderConstValue(reader);
          Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                   "Http_Traiter_XML_set_internal: depth %d type %d name %s is_empty %d has value %d has attr %d value %s", 
                    xmlTextReaderDepth(reader),
                    xmlTextReaderNodeType(reader),
                    name,
                    xmlTextReaderIsEmptyElement(reader),
                    xmlTextReaderHasValue(reader),
                    xmlTextReaderHasAttributes (reader),
                    (value ? value : (xmlChar *)"None")
                  );
          if ( xmlStrEqual ( name, (xmlChar *)"EntreeANA" ) )    /* Avons-nous une entrée ana à traiter ? */
           { xmlChar *num, *val_avant_ech, *in_range;
             num           = xmlTextReaderGetAttribute (reader, (xmlChar *)"num" );
             in_range      = xmlTextReaderGetAttribute (reader, (xmlChar *)"in_range" );
             val_avant_ech = xmlTextReaderGetAttribute (reader, (xmlChar *)"val_avant_ech" );
             if (num && val_avant_ech && in_range)
              { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                         "Http_Traiter_XML_set_internal: setting EA%03d = %08.2f",
                          atoi( (char *)num ), atof( (char *)val_avant_ech )
                        );
                SEA(atoi( (char *)num ), atof( (char *)val_avant_ech ));
                SEA_range( atoi( (char *)num ), atoi( (char *)in_range ) );
              }
             if(num)           free(num);
             if(in_range)      free(in_range);
             if(val_avant_ech) free(val_avant_ech);
           }
          else if ( xmlStrEqual ( name, (xmlChar *)"Ident" ) )             /* Identification du satellite */
           { xmlChar *instance_id, *bit_state;
             instance_id   = xmlTextReaderGetAttribute (reader, (xmlChar *)"instance_id" );
             bit_state     = xmlTextReaderGetAttribute (reader, (xmlChar *)"bit_state" );
             if (instance_id && bit_state)
              { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
                         "Http_Traiter_XML_set_internal: Get infos from %s (bit_state=%s)",
                          instance_id, atoi( (char *)bit_state )
                        );
              }
             if(instance_id)   free(instance_id);
             if(bit_state)     free(bit_state);
           }
        }
     }
end:
    xmlFreeTextReader(reader);
                                                                       /* infos est libéré par l'appelant */
  }
/**********************************************************************************************************/
/* Http_Traiter_request_set_internal: Traite la requete XML en cours de recuperation par MHD              */
/* Entrées: la connexion MHD                                                                              */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 gboolean Http_Traiter_request_set_internal ( struct MHD_Connection *connection, const char *upload_data, 
                                              size_t *upload_data_size, void **con_cls )
  { const char *Handled_OK = "<html><body>OK</body></html>";
    struct HTTP_CONNEXION_INFO *infos;
    struct MHD_Response *response;

    if (!*con_cls)
     { infos = (struct HTTP_CONNEXION_INFO *) g_try_malloc0 ( sizeof( struct HTTP_CONNEXION_INFO ) );
       if (!infos)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "Http_Traiter_request_set_internal: Memory Alloc ERROR infos" );
          return(FALSE);
        }

       *con_cls = (void *) infos;
       return(TRUE);
     } else infos = (struct HTTP_CONNEXION_INFO *)*con_cls;     /* Récupération de la structure de suivie */

    if (*upload_data_size != 0)                                                   /* Transfert en cours ? */
     { gchar *new_buffer;
       new_buffer = g_try_realloc( infos->buffer, infos->buffer_size + *upload_data_size );
       if (!new_buffer)
        { Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_ERR,
                   "Http_Traiter_request_set_internal: Memory Alloc ERROR realloc buffer" );
          g_free(infos->buffer);
          infos->buffer = NULL;
          return(FALSE);
        } else infos->buffer = new_buffer;
       memcpy ( infos->buffer + infos->buffer_size, upload_data, *upload_data_size );          /* Recopie */
       infos->buffer_size += *upload_data_size;
       *upload_data_size = 0;              /* Indique à MHD que l'on a traité l'ensemble des octets recus */
       return(TRUE);
     }
/*-------------------------------- Fin de transfert. On envoie une reponse OK ----------------------------*/
    pthread_mutex_lock( &Cfg_http.lib->synchro );             /* On envoie au thread HTTP pour traitement */
    Cfg_http.Liste_XML_docs = g_slist_prepend ( Cfg_http.Liste_XML_docs, infos );
    infos->type = HTTP_CONNEXION_SET_INTERNAL;
    infos->request_processed = TRUE;           /* Indique que la requete est traitée, pas de free mémoire */
    pthread_mutex_unlock( &Cfg_http.lib->synchro );

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
