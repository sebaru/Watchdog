/******************************************************************************************************************************/
/* Watchdogd/Imsgs/admin_imsg.c        Gestion des responses Admin IMSG au serveur watchdog                                   */
/* Projet WatchDog version 3.0        Gestion d'habitat                                                   25.02.2018 17:36:21 */
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
 #include "Imsg.h"
 extern struct IMSGS_CONFIG Cfg_imsgs;
/******************************************************************************************************************************/
/* Admin_json_list : fonction appelée pour vérifier la liste des destinataires                                                */
/* Entrée : un JSon Builder                                                                                                   */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_list ( JsonBuilder *builder )
  { struct IMSGSDB *imsgs;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING, "%s: Database Connection Failed", __func__ );
       return;
     }

    if ( ! Recuperer_imsgsDB( db ) )
     { Libere_DB_SQL( &db );
       Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_WARNING, "%s: Recuperer_imsgs Failed", __func__ );
       return;
     }

    while ( (imsgs = Recuperer_imsgsDB_suite( db )) != NULL)
     { json_builder_begin_object (builder);                                                       /* Création du noeud principal */

       Json_add_int    ( builder, "user_id", imsgs->user_id );
       Json_add_string ( builder, "user_name", imsgs->user_name );
       Json_add_bool   ( builder, "user_enable", imsgs->user_enable );
       Json_add_bool   ( builder, "user_imsg_enable", imsgs->user_imsg_enable );
       Json_add_string ( builder, "user_jabber_id", imsgs->user_jabberid );
       Json_add_bool   ( builder, "user_allow_command", imsgs->user_allow_cde );
       Json_add_bool   ( builder, "user_available", imsgs->user_available );
       Json_add_string ( builder, "user_comment", imsgs->user_comment );

       json_builder_end_object (builder);                                                                     /* End Document */
     }

    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelé par le thread http lors d'une requete /run/                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 void Admin_json ( gchar *commande, gchar **buffer_p, gint *taille_p )
  { JsonBuilder *builder;
    gsize taille_buf;

    *buffer_p = NULL;
    *taille_p = 0;

    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_imsgs.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
                                                                      /* Lancement de la requete de recuperation des messages */
    if (!strcmp(commande, "/list")) { Admin_json_list ( builder ); }
    else if ( ! strcmp ( commande, "/add_buddy/" ) )
				 { json_builder_begin_object (builder);                                                    /* Création du noeud principal */
       Json_add_string ( builder, "buddy_name", commande+11 );
       json_builder_end_object (builder);                                                                     /* End Document */
       /*purple_account_add_buddy( Cfg_imsgs.account, purple_buddy_new	( Cfg_imsgs.account, commande + 11, commande + 11 ) );*/
     }
    else if ( ! strcmp ( commande, "/send/" ) )
				 { json_builder_begin_object (builder);                                                    /* Création du noeud principal */
       Json_add_string ( builder, "message_sent", commande+6 );
       json_builder_end_object (builder);                                                                     /* End Document */
       Imsgs_Envoi_message_to_all_available ( commande+6 );
     }

/************************************************ Génération du JSON **********************************************************/
    *buffer_p = Json_get_buf ( builder, &taille_buf );
    *taille_p = taille_buf;
    return;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
