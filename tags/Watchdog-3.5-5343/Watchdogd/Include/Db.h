/**********************************************************************************************************/
/* Include/Db.h           Gestion des bases de données Watchdog via ODBC                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mer 22 avr 2009 23:18:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Db.h
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

 #ifndef _DB_H_
  #define _DB_H_
 #include <mysql.h>
 #include "Erreur.h"

 #define TAILLE_DB_HOST           30
 #define TAILLE_DB_USERNAME       30
 #define TAILLE_DB_PASSWORD       48
 #define TAILLE_DB_DATABASE       20

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
 extern gchar *Normaliser_as_ascii( gchar *chaine );
 extern struct DB *Init_DB_SQL ( void );
 extern struct DB *Init_ArchDB_SQL ( void );
 extern struct DB *Init_DB_SQL_with ( gchar *host, gchar *username, gchar *password, gchar *database, guint port, gboolean multi_statements );
 extern void Libere_DB_SQL( struct DB **adr_db );
 extern gboolean Lancer_requete_SQL ( struct DB *db, gchar *requete );
 extern MYSQL_ROW Recuperer_ligne_SQL ( struct DB *db );
 extern void Liberer_resultat_SQL ( struct DB *db );
 extern guint Recuperer_last_ID_SQL ( struct DB *db );
 extern void Print_SQL_status ( void );
 extern void Update_database_schema ( void );
 extern gboolean SQL_Select_to_JSON ( JsonBuilder *builder, gchar *array_name, gchar *requete );
 extern gboolean SQL_Select_to_JSON_new ( JsonBuilder *builder, gchar *array_name, gchar *format, ... );
 extern gboolean SQL_Write ( gchar *requete );
 extern gboolean SQL_Writes ( gchar *requete );
 extern gboolean SQL_Write_new( gchar *format, ... );
 extern gboolean SQL_Arch_to_JSON ( JsonBuilder *builder, gchar *array_name, gchar *requete );
 extern gboolean SQL_Arch_Write ( gchar *requete );
 #endif
/*--------------------------------------------------------------------------------------------------------*/
