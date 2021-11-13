/******************************************************************************************************************************/
/* Watchdogd/Archive/admin_arch.c  Gestion des responses Admin du thread "Archive" de watchdog                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    17.03.2017 08:37:09 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_arch.c
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

 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Admin_arch_testdb: Test la response vers le serveur de base de données                                                    */
/* Entrée: la response pour sortiee client et la ligne de commande                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Admin_arch_json_testdb ( JsonBuilder *builder )
  { gchar chaine[256];
    struct DB *db;

    db = Init_ArchDB_SQL();
    g_snprintf( chaine, sizeof(chaine), " Response to DB %s (Host='%s':%d, User='%s' DB='%s')", (db ? "OK" : "Failed"),
                Partage->com_arch.archdb_host, Partage->com_arch.archdb_port, Partage->com_arch.archdb_username, Partage->com_arch.archdb_database );
    Json_add_string ( builder, "result", chaine );
    Libere_DB_SQL ( &db );
  }
/******************************************************************************************************************************/
/* Admin_arch_json_clear: Supprime tous les enregistrements dans le tampon d'attente                                          */
/* Entrée: le JSON builder pour préparer la réponse au client                                                                 */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Admin_arch_json_clear ( JsonBuilder *builder )
  { gint nbr;

    nbr = Arch_Clear_list();
    Json_add_int ( builder, "nbr_archive_deleted", nbr );
  }
/******************************************************************************************************************************/
/* Admin_arch_json_purge: Lance le thread de purge des archives                                                               */
/* Entrée: le JSON builder pour préparer la réponse au client                                                                 */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Admin_arch_json_purge ( JsonBuilder *builder )
  { pthread_t tid;
    if (pthread_create( &tid, NULL, (void *)Arch_Update_SQL_Partitions_thread, NULL ))
     { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s: pthread_create failed for Update SQL Partitions", __func__ );
       Json_add_string ( builder, "exec_purge_thread", "success" );
     }
     else
     { pthread_detach( tid );                                        /* On le detache pour qu'il puisse se terminer tout seul */
       Json_add_string ( builder, "exec_purge_thread", "failed" );
     }
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelé par le thread http lors d'une requete /run/                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
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
/************************************************ Préparation du buffer JSON **************************************************/
                                                                      /* Lancement de la requete de recuperation des messages */
    if (!strcmp(commande, "/clear")) { Admin_arch_json_clear ( builder ); }
    else if (!strcmp(commande, "/purge")) { Admin_arch_json_purge ( builder ); }
    else if (!strcmp(commande, "/testdb")) { Admin_arch_json_testdb ( builder ); }

/************************************************ Génération du JSON **********************************************************/
    *buffer_p = Json_get_buf ( builder, &taille_buf );
    *taille_p = taille_buf;

  }
/*----------------------------------------------------------------------------------------------------------------------------*/
