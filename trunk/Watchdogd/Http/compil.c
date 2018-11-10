/******************************************************************************************************************************/
/* Watchdogd/Http/compil.c       Gestion des request Compil pour le thread HTTP de watchdog                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    31.10.2018 18:30:09 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * compil.c
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
/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Http.h"
/******************************************************************************************************************************/
/* Http_Traiter_request_compil: Traite une requete COMPIL                                                                     */
/* Entrées: la connexion WSI                                                                                                  */
/* Sortie : code de retour libwebsocket                                                                                       */
/******************************************************************************************************************************/
 gint Http_Traiter_request_compil ( struct lws *wsi )
  { gchar token_id[12];
    const gchar *id_s;
    gint retour, num, id;
    
    id_s    = lws_get_urlarg_by_name	( wsi, "id=", token_id, sizeof(token_id) );
    if (id_s) { id = atoi (id_s); }
    else { Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );
           return(1);
         }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE, "%s: Compiling DLS %d", __func__, id );
    Compiler_source_dls( TRUE, id, NULL, 0 );

    Http_Send_response_code ( wsi, HTTP_200_OK );
    return(1);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
