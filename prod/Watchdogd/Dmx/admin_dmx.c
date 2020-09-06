/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_dmx.c        Gestion des responses Admin MODBUS au serveur watchdog                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       dim. 05 sept. 2010 12:01:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_dmx.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

 #include "watchdogd.h"
 #include "Dmx.h"
 extern struct DMX_CONFIG Cfg_dmx;
/******************************************************************************************************************************/
/* Admin_json_list : fonction appelée pour lister les modules dmx                                                             */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_status ( JsonBuilder *builder )
  { gint cpt;
    Json_add_string ( builder, "tech_id", Cfg_dmx.tech_id );
    Json_add_string ( builder, "device", Cfg_dmx.device );
    Json_add_int    ( builder, "nbr_request", Cfg_dmx.nbr_request );
    Json_add_int    ( builder, "taille_trame_dmx", Cfg_dmx.taille_trame_dmx );
    Json_add_bool   ( builder, "comm", Cfg_dmx.comm_status );

    if (Cfg_dmx.Canal)
     { for (cpt=0; cpt<64; cpt++)
        { gchar canal[12];
          g_snprintf( canal, sizeof(canal), "canal_%d", cpt+1 );
          Json_add_int ( builder, canal, Cfg_dmx.Trame_dmx.channel[cpt] ); /*Canal[cpt].val_avant_ech );*/
        }
     }
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelé par le thread http lors d'une requete /run/                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 void Admin_json ( gchar *commande, gchar **buffer_p, gsize *taille_p )
  { JsonBuilder *builder;

    *buffer_p = NULL;
    *taille_p = 0;

    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
                                                                      /* Lancement de la requete de recuperation des messages */
    if (!strcmp(commande, "/status")) { Admin_json_status ( builder ); }

/************************************************ Génération du JSON **********************************************************/
    *buffer_p = Json_get_buf ( builder, taille_p );
    return;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
