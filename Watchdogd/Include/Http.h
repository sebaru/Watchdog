/******************************************************************************************************************************/
/* Watchdogd/Include/Http.h        Déclaration structure internes des WebServices                                             */
/* Projet Abls-Habitat version 4.3       Gestion d'habitat                                   mer. 24 avril 2013 18:48:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * http.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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

#ifndef _HTTP_H_
 #define _HTTP_H_

 #define HTTP_DEFAUT_FILE_CERT         "https_bus_cert.pem"
 #define HTTP_DEFAUT_FILE_KEY          "https_bus_key.pem"
 #define HTTP_DEFAUT_TCP_PORT          5559

 struct COM_HTTP                                                                     /* Communication entre le serveur et DLS */
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_debug;                                                             /* TRUE si le thread doit tout logguer */
    SoupServer *local_socket;
    GMainLoop *loop;
    GMainContext *loop_context;
    GSList *liste_http_clients;
    gint num_session;
  };

/*************************************************** Définitions des prototypes ***********************************************/

 extern void Run_HTTP ( void );

 extern void Http_traiter_status  ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query, gpointer user_data );
 extern void Http_traiter_get_io ( SoupServer *server, SoupServerMessage *msg, const char *path, GHashTable *query );

 extern JsonNode *Http_Msg_to_Json ( SoupServerMessage *msg );
 extern JsonNode *Http_Get_from_local_BUS ( struct THREAD *module, gchar *uri );
 extern void Http_Add_Thread_signature ( struct THREAD *module, SoupMessage *msg, gchar *buf, gint buf_size );
 extern gboolean Http_Check_Thread_signature ( gchar *path, SoupServerMessage *msg, gchar **thread_tech_id_p );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
