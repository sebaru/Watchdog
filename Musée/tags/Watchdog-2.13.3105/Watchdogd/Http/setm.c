/**********************************************************************************************************/
/* Watchdogd/Http/setm.c       Gestion des request setm pour le thread HTTP de watchdog                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 20 nov. 2013 18:18:49 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * setm.c
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
/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Http.h"
/**********************************************************************************************************/
/* Traiter_dynxml: Traite une requete sur l'URI dynxml                                                    */
/* Entrées: la connexion MHD                                                                              */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 gboolean Http_Traiter_request_setm ( struct HTTP_SESSION *session, struct MHD_Connection *connection )
  {
#ifdef bouh
	   const char *Setm_response = "<html><body>OK</body></html>";
    struct MHD_Response *response;
    const gchar *m_num_char;
    gint m_num;

    m_num_char = MHD_lookup_connection_value ( connection, MHD_GET_ARGUMENT_KIND, "num" );
    if (!m_num_char) { return(FALSE); }
                else { m_num = atoi(m_num_char); }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_DEBUG,
             "Http_Traiter_request_setm: Setting M%04d = 1 for User %s, SID %s",
              m_num, session->util->nom, session->sid );
    Envoyer_commande_dls ( m_num );

    response = MHD_create_response_from_buffer ( strlen (Setm_response)+1,
                                                (void*) Setm_response, MHD_RESPMEM_PERSISTENT);
    if (response == NULL) return(FALSE);
    Http_Add_response_header ( response );
    MHD_queue_response (connection, MHD_HTTP_OK, response);
    MHD_destroy_response (response);

    return(TRUE);
#endif
  }
/*--------------------------------------------------------------------------------------------------------*/
