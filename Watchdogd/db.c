/******************************************************************************************************************************/
/* Watchdog/db.c          Gestion des connexions à la base de données                                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 18 avr 2009 00:44:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * db.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien LEFEVRE
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

 #include <glib.h>
 #include <string.h>
 #include <locale.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>

/**************************************************** Chargement des prototypes ***********************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Normaliser_chaine: Normalise les chaines ( remplace ' par \', " par "" )                                                   */
/* Entrées: un commentaire (gchar *)                                                                                          */
/* Sortie: boolean false si probleme                                                                                          */
/******************************************************************************************************************************/
 gchar *Normaliser_chaine( gchar *pre_comment )
  { gchar *comment, *source, *cible;
    gunichar car;

    g_utf8_validate( pre_comment, -1, NULL );                                                           /* Validate la chaine */
    comment = g_try_malloc0( (2*g_utf8_strlen(pre_comment, -1))*6 + 1 );                  /* Au pire, ts les car sont doublés */
                                                                                                      /* *6 pour gerer l'utf8 */
    if (!comment)
     { Info_new( __func__, Config.log_db, LOG_WARNING, "Normaliser_chaine: memory error %s", pre_comment );
       return(NULL);
     }
    source = pre_comment;
    cible  = comment;

    while( (car = g_utf8_get_char( source )) )
     { if ( car == '\'' )                                                                     /* Dédoublage de la simple cote */
        { g_utf8_strncpy( cible, "\'", 1 ); cible = g_utf8_next_char( cible );
          g_utf8_strncpy( cible, "\'", 1 ); cible = g_utf8_next_char( cible );
        }
       else if (car =='\\')                                                                        /* Dédoublage du backspace */
        { g_utf8_strncpy( cible, "\\", 1 ); cible = g_utf8_next_char( cible );
          g_utf8_strncpy( cible, "\\", 1 ); cible = g_utf8_next_char( cible );
        }
       else
        { g_utf8_strncpy( cible, source, 1 ); cible = g_utf8_next_char( cible );
        }
       source = g_utf8_next_char(source);
     }
    return(comment);
  }
/******************************************************************************************************************************/
/* Recuperer_ligne_SQL: Renvoie les lignes resultat, une par une                                                              */
/* Entrée: la DB                                                                                                              */
/* Sortie: La ligne ou NULL si il n'y en en plus                                                                              */
/******************************************************************************************************************************/
 MYSQL_ROW Recuperer_ligne_SQL ( struct DB *db )
  { if (!db) return(NULL);
    db->row = mysql_fetch_row(db->result);
    return( db->row );
  }
/******************************************************************************************************************************/
/* Liberer_resultat_SQL: Libere la mémoire affectée au resultat SQL                                                           */
/* Entrée: la DB                                                                                                              */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Liberer_resultat_SQL ( struct DB *db )
  { if (!db) return;
encore:
    while( db->row ) Recuperer_ligne_SQL ( db );
    mysql_free_result( db->result );
    if (db->multi_statement)
     { if (mysql_next_result(db->mysql)==0)
        { db->result = mysql_store_result(db->mysql);
          Recuperer_ligne_SQL ( db );
          goto encore;
        }
     }
    db->result = NULL;
    db->free = TRUE;
  }
/******************************************************************************************************************************/
/* Libere_DB_SQL : Se deconnecte d'une base de données en parametre                                                           */
/* Entrée: La DB                                                                                                              */
/******************************************************************************************************************************/
 void Libere_DB_SQL( struct DB **adr_db )
  { /*static struct DLS_AI *ai_nbr_dbrequest = NULL;*/
    struct DB *db, *db_en_cours;
    gboolean found;
    GSList *liste;
    gint taille;
    if ( (!adr_db) || !(*adr_db) ) return;

    db = *adr_db;                                                                       /* Récupération de l'adresse de la db */
    found = FALSE;
    pthread_mutex_lock ( &Partage->com_db.synchro );
    liste = Partage->com_db.Liste;
    while ( liste )
     { db_en_cours = (struct DB *)liste->data;
       if (db_en_cours->id == db->id) found = TRUE;
       liste = g_slist_next( liste );
     }
    pthread_mutex_unlock ( &Partage->com_db.synchro );

    if (!found)
     { Info_new( __func__, Config.log_db, LOG_CRIT,
                "Libere_DB_SQL: DB Free Request not in list ! DB%07d, request=%s",
                 db->id, db->requete );
       return;
     }

    if (db->free==FALSE)
     { Liberer_resultat_SQL ( db ); }
    mysql_close( db->mysql );
    pthread_mutex_lock ( &Partage->com_db.synchro );
    Partage->com_db.Liste = g_slist_remove ( Partage->com_db.Liste, db );
    taille = g_slist_length ( Partage->com_db.Liste );
    pthread_mutex_unlock ( &Partage->com_db.synchro );
    Info_new( __func__, Config.log_db, LOG_DEBUG,
             "Libere_DB_SQL: Deconnexion effective (DB%07d), Nbr_requete_en_cours=%d", db->id, taille );
    g_free( db );
    *adr_db = NULL;
/*    Dls_data_set_AI ( "SYS", "DB_NBR_REQUEST", (gpointer)&ai_nbr_dbrequest, (gdouble)taille, TRUE );*/
  }
/******************************************************************************************************************************/
/* Init_DB_SQL: essai de connexion à la DataBase db                                                                           */
/* Entrée: toutes les infos necessaires a la connexion                                                                        */
/* Sortie: une structure DB de référence                                                                                      */
/******************************************************************************************************************************/
 struct DB *Init_DB_SQL_with ( gchar *host, gchar *username, gchar *password, gchar *database, guint port, gboolean multi_statements )
  { static gint id = 1, taille;
    struct DB *db;
    my_bool reconnect;

    db = (struct DB *)g_try_malloc0( sizeof(struct DB) );
    if (!db)                                                                              /* Si probleme d'allocation mémoire */
     { Info_new( __func__, Config.log_db, LOG_ERR, "Memory error" );
       return(NULL);
     }

    db->mysql = mysql_init(NULL);
    if (!db->mysql)
     { Info_new( __func__, Config.log_db, LOG_ERR, "Mysql_init failed (%s)",
                              (char *) mysql_error(db->mysql)  );
       g_free(db);
       return (NULL);
     }

    reconnect = 1;
    mysql_options( db->mysql, MYSQL_OPT_RECONNECT, &reconnect );
    gint timeout = 10;
    mysql_options( db->mysql, MYSQL_OPT_CONNECT_TIMEOUT, &timeout );                         /* Timeout en cas de non reponse */
    mysql_options( db->mysql, MYSQL_SET_CHARSET_NAME, (void *)"utf8" );
    if ( ! mysql_real_connect( db->mysql, host, username, password, database, port, NULL, (multi_statements ? CLIENT_MULTI_STATEMENTS : 0) ) )
     { Info_new( __func__, Config.log_db, LOG_ERR,
                 "mysql_real_connect failed (%s)",
                 (char *) mysql_error(db->mysql)  );
       mysql_close( db->mysql );
       g_free(db);
       return (NULL);
     }
    db->free = TRUE;
    db->multi_statement = multi_statements;
    pthread_mutex_lock ( &Partage->com_db.synchro );
    db->id = id++;
    Partage->com_db.Liste = g_slist_prepend ( Partage->com_db.Liste, db );
    taille = g_slist_length ( Partage->com_db.Liste );
    pthread_mutex_unlock ( &Partage->com_db.synchro );
    Info_new( __func__, Config.log_db, LOG_DEBUG,
              "Database Connection OK with %s@%s:%d on %s (DB%07d). Nbr_requete_en_cours=%d",
               username, host, port, database, db->id, taille );
    return(db);
  }
/******************************************************************************************************************************/
/* Init_DB_SQL: essai de connexion à la DataBase db                                                                           */
/* Entrée: toutes les infos necessaires a la connexion                                                                        */
/* Sortie: une structure DB de référence                                                                                      */
/******************************************************************************************************************************/
 struct DB *Init_DB_SQL ( void )
  { return( Init_DB_SQL_with ( Config.db_hostname, Config.db_username,
                               Config.db_password, Config.db_database, Config.db_port, FALSE ) );
  }
/******************************************************************************************************************************/
/* SQL_Field_to_Json : Intéègre un MYSQL_FIELD dans une structure JSON                                                        */
/* Entrée: Le JsonNode et le MYSQL_FIELD                                                                                      */
/* Sortie: le jsonnode est mis a jour                                                                                         */
/******************************************************************************************************************************/
 static void SQL_Field_to_Json ( JsonNode *node, MYSQL_FIELD *field, gchar *chaine )
  { if ( field->type == MYSQL_TYPE_FLOAT || field->type==MYSQL_TYPE_DOUBLE )
     { if (chaine) Json_node_add_double( node, field->name, atof(chaine) );
              else Json_node_add_null  ( node, field->name );
     }
    else if ( field->type == MYSQL_TYPE_TINY )
     { if (chaine) Json_node_add_bool ( node, field->name, atoi(chaine) );
              else Json_node_add_null ( node, field->name );
     }
    else if ( IS_NUM(field->type) )
     { if (chaine) Json_node_add_int  ( node, field->name, atoi(chaine) );
              else Json_node_add_null ( node, field->name );
     }
    else
     { Json_node_add_string( node, field->name, chaine ); }
  }
/******************************************************************************************************************************/
/* SQL_Select_to_JSON : lance une requete en parametre, sur la structure de reférence                                         */
/* Entrée: La DB, la requete                                                                                                  */
/* Sortie: TRUE si pas de souci                                                                                               */
/******************************************************************************************************************************/
 static gboolean SQL_Select_to_json_node_reel ( gboolean db_arch, JsonNode *RootNode, gchar *array_name, gchar *requete )
  { struct DB *db;
    db = Init_DB_SQL ();
    if (!db)
     { Info_new( __func__, Config.log_db, LOG_WARNING, "Init DB FAILED for '%s'", requete );
       return(FALSE);
     }

    if ( mysql_query ( db->mysql, requete ) )
     { Info_new( __func__, Config.log_db, LOG_ERR, "FAILED (%s) for '%s'", (char *)mysql_error(db->mysql), requete );
       Libere_DB_SQL ( &db );
       if (array_name)
        { gchar chaine[80];
          g_snprintf(chaine, sizeof(chaine), "nbr_%s", array_name );
          Json_node_add_int ( RootNode, chaine, 0 );
          Json_node_add_array( RootNode, array_name );                            /* Ajoute un array vide en cas d'erreur SQL */
        }
       return(FALSE);
     }
    else Info_new( __func__, Config.log_db, LOG_DEBUG, "DB OK for '%s'", requete );

    db->result = mysql_store_result ( db->mysql );
    if ( ! db->result )
     { Info_new( __func__, Config.log_db, LOG_WARNING, "store_result failed (%s)", (char *) mysql_error(db->mysql) );
       db->nbr_result = 0;
     }
    if (array_name)
     { gchar chaine[80];
       g_snprintf(chaine, sizeof(chaine), "nbr_%s", array_name );
       Json_node_add_int ( RootNode, chaine, mysql_num_rows ( db->result ));
       JsonArray *array = Json_node_add_array( RootNode, array_name );
       while ( (db->row = mysql_fetch_row(db->result)) != NULL )
        { JsonNode *element = Json_node_create();
          for (gint cpt=0; cpt<mysql_num_fields(db->result); cpt++)
           { MYSQL_FIELD *field = mysql_fetch_field_direct(db->result, cpt);
             SQL_Field_to_Json ( element, field, db->row[cpt] );
           }
          Json_array_add_element ( array, element );
        }
     }
    else
     { while ( (db->row = mysql_fetch_row(db->result)) != NULL )
        { for (gint cpt=0; cpt<mysql_num_fields(db->result); cpt++)
           { MYSQL_FIELD *field = mysql_fetch_field_direct(db->result, cpt);
             SQL_Field_to_Json ( RootNode, field, db->row[cpt] );
           }
        }
     }
    mysql_free_result( db->result );
    Libere_DB_SQL( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* SQL_Write_new: Envoie une requete en parametre au serveur de base de données                                               */
/* Entrée: le format de la requete, ainsi que tous les parametres associés                                                    */
/******************************************************************************************************************************/
 gboolean SQL_Select_to_json_node ( JsonNode *RootNode, gchar *array_name, gchar *format, ... )
  { va_list ap;

    va_start( ap, format );
    gsize taille = g_printf_string_upper_bound (format, ap);
    va_end ( ap );
    gchar *chaine = g_try_malloc(taille+1);
    if (chaine)
     { va_start( ap, format );
       g_vsnprintf ( chaine, taille, format, ap );
       va_end ( ap );

       gboolean retour = SQL_Select_to_json_node_reel ( FALSE, RootNode, array_name, chaine );
       g_free(chaine);
       return(retour);
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Print_SQL_status : permet de logguer le statut SQL                                                                         */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Print_SQL_status ( void )
  { GSList *liste;
    pthread_mutex_lock ( &Partage->com_db.synchro );
    Info_new( __func__, Config.log_db, LOG_DEBUG,
             "Print_SQL_status: %03d running connexions",
              g_slist_length(Partage->com_db.Liste) );

    liste = Partage->com_db.Liste;
    while ( liste )
     { struct DB *db;
       db = (struct DB *)liste->data;
       Info_new( __func__, Config.log_db, LOG_DEBUG,
                "Print_SQL_status: Connexion DB%07d (db->free=%d) requete %s",
                 db->id, db->free, db->requete );
       liste = g_slist_next( liste );
     }
    pthread_mutex_unlock ( &Partage->com_db.synchro );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
