/******************************************************************************************************************************/
/* Watchdogd/Imsg/admin_imsg.c        Gestion des connexions Admin IMSG au serveur watchdog                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                      sam. 28 juil. 2012 16:35:09 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_imsg.c
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
 #include "Radio.h"
 extern struct RADIO_CONFIG Cfg_radio;
/******************************************************************************************************************************/
/* Admin_json_list : fonction appelée pour lister les modules modbus                                                          */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_status ( JsonBuilder *builder )
  {
    json_builder_begin_object (builder);                                                       /* Création du noeud principal */
    Json_add_int ( builder, "nbr_diffusion_radio", Cfg_radio.nbr_diffusion );
    Json_add_int ( builder, "radio_en_cours", Cfg_radio.radio_en_cours );
    json_builder_end_object (builder);                                                                        /* End Document */
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
     { Info_new( Config.log, Cfg_radio.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
                                                                      /* Lancement de la requete de recuperation des messages */
    if (!strcmp(commande, "/status")) { Admin_json_status ( builder ); }

    *buffer_p = Json_get_buf ( builder, taille_p );
    return;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
