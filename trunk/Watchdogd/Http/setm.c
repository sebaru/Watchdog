/******************************************************************************************************************************/
/* Watchdogd/Http/setm.c       Gestion des request setm pour le thread HTTP de watchdog                                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         mer. 20 nov. 2013 18:18:49 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Http.h"
/******************************************************************************************************************************/
/* Http_Traiter_request_setm: Traite une requete SETM                                                                         */
/* Entrées: la connexion WSI                                                                                                  */
/* Sortie : code de retour libwebsocket                                                                                       */
/******************************************************************************************************************************/
 gint Http_Traiter_request_setm ( struct lws *wsi )
  { gchar token_num[12];
    const gchar *num_s;
    gint retour, num;
    
    num_s    = lws_get_urlarg_by_name	( wsi, "num=", token_num, sizeof(token_num) );
    if (num_s) { num = atoi (num_s); }
          else { Http_Send_response_code ( wsi, HTTP_BAD_REQUEST );
                 return(1);
               }

    Info_new( Config.log, Cfg_http.lib->Thread_debug, LOG_NOTICE,
             "%s: Setting M%04d = 1", __func__, num );
    Envoyer_commande_dls ( num );

    Http_Send_response_code ( wsi, HTTP_200_OK );
    return(1);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
