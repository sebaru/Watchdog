/**********************************************************************************************************/
/* Include/Db.h           Gestion des bases de données Watchdog via ODBC                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mer 22 avr 2009 23:18:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Db.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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

 #ifndef _DB_H_
  #define _DB_H_
 #include <mysql.h>
 #include "Erreur.h"

 #define TAILLE_DB_HOST           32
 #define TAILLE_DB_USERNAME       64
 #define TAILLE_DB_PASSWORD       64
 #define TAILLE_DB_DATABASE       64

 struct DB
  { MYSQL *mysql;
    MYSQL_RES *result;
    gint nbr_result;
    gboolean free;                                                                        /* Le resultat est-il free ou non ? */
    gboolean multi_statement;                                                             /* Le resultat est-il free ou non ? */
    MYSQL_ROW row;
    gint id;
    gchar requete[256];
  };
/************************************* Prototypes de fonctions ********************************************/
 extern gchar *Normaliser_chaine( gchar *pre_comment );
 extern struct DB *Init_DB_SQL ( void );
 extern struct DB *Init_DB_SQL_with ( gchar *host, gchar *username, gchar *password, gchar *database, guint port, gboolean multi_statements );
 extern void Print_SQL_status ( void );
 extern gboolean SQL_Select_to_json_node ( JsonNode *RootNode, gchar *array_name, gchar *format, ... );
 #endif
/*--------------------------------------------------------------------------------------------------------*/
