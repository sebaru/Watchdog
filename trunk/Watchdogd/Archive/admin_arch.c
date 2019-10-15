/******************************************************************************************************************************/
/* Watchdogd/Archive/admin_arch.c  Gestion des responses Admin du thread "Archive" de watchdog                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    17.03.2017 08:37:09 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_arch.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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

 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Admin_arch_testdb: Test la response vers le serveur de base de donn�es                                                    */
/* Entr�e: la response pour sortiee client et la ligne de commande                                                           */
/* Sortie: N�ant                                                                                                              */
/******************************************************************************************************************************/
 static void Admin_arch_json_testdb ( JsonBuilder *builder )
  { gchar chaine[256];
    struct DB*db;

    db = Init_ArchDB_SQL();
    g_snprintf( chaine, sizeof(chaine), " Response to DB %s (Host='%s':%d, User='%s' DB='%s')", (db ? "OK" : "Failed"),
                Partage->com_arch.archdb_host, Partage->com_arch.archdb_port, Partage->com_arch.archdb_username, Partage->com_arch.archdb_database );
    json_builder_begin_object (builder);                                                       /* Cr�ation du noeud principal */
    Json_add_string ( builder, "result", chaine );
    json_builder_end_object (builder);                                                         /* Cr�ation du noeud principal */
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appel� par le thread http lors d'une requete /run/                                                   */
/* Entr�e : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entr�e sont mis � jour                                                                           */
/******************************************************************************************************************************/
 void Admin_arch_json ( gchar *commande, gchar **buffer_p, gint *taille_p )
  { JsonBuilder *builder;
    gsize taille_buf;

    *buffer_p = NULL;
    *taille_p = 0;

    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return;
     }
/************************************************ Pr�paration du buffer JSON **************************************************/
                                                                      /* Lancement de la requete de recuperation des messages */
    /*if (!strcmp(commande, "/clear")) { Admin_arch_json_clear ( builder ); }
    else if (!strcmp(commande, "/purge")) { Admin_arch_json_purge ( builder ); }
    else*/ if (!strcmp(commande, "/testdb")) { Admin_arch_json_testdb ( builder ); }

/************************************************ G�n�ration du JSON **********************************************************/
    *buffer_p = Json_get_buf ( builder, &taille_buf );
    *taille_p = taille_buf;

  }
/*----------------------------------------------------------------------------------------------------------------------------*/
