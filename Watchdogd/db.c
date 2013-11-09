/**********************************************************************************************************/
/* Watchdog/db.c          Gestion des connexions à la base de données                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 18 avr 2009 00:44:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * db.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
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

/******************************************** Chargement des prototypes ***********************************/
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Normaliser_chaine: Normalise les chaines ( remplace ' par \', " par "" )                               */
/* Entrées: un commentaire (gchar *)                                                                      */
/* Sortie: boolean false si probleme                                                                      */
/**********************************************************************************************************/
 gchar *Normaliser_chaine( gchar *pre_comment )
  { gchar *comment, *source, *cible;
    gunichar car;

    g_utf8_validate( pre_comment, -1, NULL );                                       /* Validate la chaine */
    comment = g_try_malloc0( (2*g_utf8_strlen(pre_comment, -1))*6 + 1 );  /* Au pire, ts les car sont doublés */
                                                                                  /* *6 pour gerer l'utf8 */
    if (!comment)
     { Info_new( Config.log, Config.log_db, LOG_WARNING, "Normaliser_chaine: memory error %s", pre_comment );
       return(NULL);
     }
    source = pre_comment;
    cible  = comment;
    
    while( (car = g_utf8_get_char( source )) )
     { if ( car == '\'' )                                                 /* Dédoublage de la simple cote */
        { g_utf8_strncpy( cible, "\'", 1 ); cible = g_utf8_next_char( cible );
          g_utf8_strncpy( cible, "\'", 1 ); cible = g_utf8_next_char( cible );
        }
       else if (car =='\\')                                                    /* Dédoublage du backspace */
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
/**********************************************************************************************************/
/* Init_DB_SQL: essai de connexion à la DataBase db                                                       */
/* Entrée: toutes les infos necessaires a la connexion                                                    */
/* Sortie: une structure DB de référence                                                                  */
/**********************************************************************************************************/
 struct DB *Init_DB_SQL ( void )
  { static gint id = 1, taille;
    struct DB *db;
    my_bool reconnect;

    db = (struct DB *)g_try_malloc0( sizeof(struct DB) );
    if (!db)                                                          /* Si probleme d'allocation mémoire */
     { Info_new( Config.log, Config.log_db, LOG_ERR, "Init_DB_SQL: Memory error" );
       return(NULL);
     }

    db->mysql = mysql_init(NULL);
    if (!db->mysql)
     { Info_new( Config.log, Config.log_db, LOG_ERR, "Init_DB_SQL: Mysql_init failed (%s)",
                              (char *) mysql_error(db->mysql)  );
       g_free(db);
       return (NULL);
     }

    reconnect = 1;
    mysql_options( db->mysql, MYSQL_OPT_RECONNECT, &reconnect );
    if ( ! mysql_real_connect( db->mysql, Config.db_host, Config.db_username,
                               Config.db_password, Config.db_database, Config.db_port, NULL, 0 ) )
     { Info_new( Config.log, Config.log_db, LOG_ERR,
                 "Init_DB_SQL: mysql_real_connect failed (%s)",
                 (char *) mysql_error(db->mysql)  );
       mysql_close( db->mysql );
       g_free(db);
       return (NULL);
     }
    db->free = TRUE;
    pthread_mutex_lock ( &Partage->com_db.synchro );
    db->id = id++;
    Partage->com_db.Liste = g_slist_prepend ( Partage->com_db.Liste, db );
    taille = g_slist_length ( Partage->com_db.Liste );
    pthread_mutex_unlock ( &Partage->com_db.synchro );
    Info_new( Config.log, Config.log_db, LOG_DEBUG,
              "Init_DB_SQL: Database Connection OK with %s@%s (id=%05d). Nbr_requete_en_cours=%d",
               Config.db_username, Config.db_database, db->id, taille );
    return(db);
  }
/**********************************************************************************************************/
/* Libere_DB_SQL : Se deconnecte d'une base de données en parametre                                       */
/* Entrée: La DB                                                                                          */
/**********************************************************************************************************/
 void Libere_DB_SQL( struct DB **adr_db )
  { struct DB *db, *db_en_cours;
    gboolean found;
    GSList *liste;
    gint taille;
    if ( (!adr_db) || !(*adr_db) ) return;

    db = *adr_db;                                                   /* Récupération de l'adresse de la db */
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
     { Info_new( Config.log, Config.log_db, LOG_CRIT,
                "Libere_DB_SQL: DB Free Request not in list ! ID=%05d, request=%s",
                 db->id, db->requete );
       return;
     }

    if (db->free==FALSE)
     { Info_new( Config.log, Config.log_db, LOG_WARNING,
                "Libere_DB_SQL: Reste un result a FREEer (id=%05d)!", db->id );
       Liberer_resultat_SQL ( db );
     }
    mysql_close( db->mysql );
    pthread_mutex_lock ( &Partage->com_db.synchro );
    Partage->com_db.Liste = g_slist_remove ( Partage->com_db.Liste, db );
    taille = g_slist_length ( Partage->com_db.Liste );
    pthread_mutex_unlock ( &Partage->com_db.synchro );
    Info_new( Config.log, Config.log_db, LOG_DEBUG,
             "Libere_DB_SQL: Deconnexion effective (id=%05d), Nbr_requete_en_cours=%d", db->id, taille );
    g_free( db );
    *adr_db = NULL;
  }
/**********************************************************************************************************/
/* Lancer_requete_SQL : lance une requete en parametre, sur la structure de reférence                     */
/* Entrée: La DB, la requete                                                                              */
/* Sortie: TRUE si pas de souci                                                                           */
/**********************************************************************************************************/
 gboolean Lancer_requete_SQL ( struct DB *db, gchar *requete )
  { if (!db) return(FALSE);

    if (db->free==FALSE)
     { Info_new( Config.log, Config.log_db, LOG_WARNING,
                "Lancer_requete_SQL (id=%05d): Reste un result a FREEer!", db->id );
     }

    g_snprintf( db->requete, sizeof(db->requete), "%s", requete );                  /* Save for later use */
    Info_new( Config.log, Config.log_db, LOG_DEBUG,
             "Lancer_requete_SQL (id=%05d): requete NEW    (%s)", db->id, requete );
    if ( mysql_query ( db->mysql, requete ) )
     { Info_new( Config.log, Config.log_db, LOG_WARNING,
                "Lancer_requete_SQL (id=%05d): requete FAILED (%s)",
                 db->id, (char *)mysql_error(db->mysql) );
       return(FALSE);
     }

    if ( ! strncmp ( requete, "SELECT", 6 ) )
     { db->result = mysql_store_result ( db->mysql );
       db->free = FALSE;
       if ( ! db->result )
        { Info_new( Config.log, Config.log_db, LOG_WARNING,
                   "Lancer_requete_SQL (id=%05d): store_result failed (%s)",
                    db->id, (char *) mysql_error(db->mysql) );
          db->nbr_result = 0;
        }
       else 
        { /*Info( Config.log, DEBUG_DB, "Lancer_requete_SQL: store_result OK" );*/
          db->nbr_result = mysql_num_rows ( db->result );
        }
     }
    Info_new( Config.log, Config.log_db, LOG_DEBUG,
             "Lancer_requete_SQL (id=%05d): requete OK     (%s)", db->id, requete );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Liberer_resultat_SQL: Libere la mémoire affectée au resultat SQL                                       */
/* Entrée: la DB                                                                                          */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Liberer_resultat_SQL ( struct DB *db )
  { if (db)
     { while( db->row ) Recuperer_ligne_SQL ( db );
       mysql_free_result( db->result );
       db->result = NULL;
       db->free = TRUE;
      /*Info( Config.log, DEBUG_DB, "Liberer_resultat_SQL: free OK" );*/
     }
  }
/**********************************************************************************************************/
/* Recuperer_ligne_SQL: Renvoie les lignes resultat, une par une                                          */
/* Entrée: la DB                                                                                          */
/* Sortie: La ligne ou NULL si il n'y en en plus                                                          */
/**********************************************************************************************************/
 MYSQL_ROW Recuperer_ligne_SQL ( struct DB *db )
  { if (!db) return(NULL);
    db->row = mysql_fetch_row(db->result);
    return( db->row );
  }
/**********************************************************************************************************/
/* Recuperer_last_ID_SQL: Renvoie le dernier ID inséré                                                    */
/* Entrée: la DB                                                                                          */
/* Sortie: Le dernier ID                                                                                  */
/**********************************************************************************************************/
 guint Recuperer_last_ID_SQL ( struct DB *db )
  { if (!db) return(0);
    return ( mysql_insert_id(db->mysql) );
  }
/**********************************************************************************************************/
/* Print_SQL_status : permet de logguer le statut SQL                                                     */
/* Entrée: néant                                                                                          */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Print_SQL_status ( void )
  { GSList *liste;
    pthread_mutex_lock ( &Partage->com_db.synchro );
    Info_new( Config.log, Config.log_db, LOG_DEBUG,
             "Print_SQL_status: %03d running connexions",
              g_slist_length(Partage->com_db.Liste) );

    liste = Partage->com_db.Liste;
    while ( liste )
     { struct DB *db;
       db = (struct DB *)liste->data;
       Info_new( Config.log, Config.log_db, LOG_DEBUG,
              "Print_SQL_status: Connexion %03d requete %s",
               db->id, db->requete );
       liste = g_slist_next( liste );
     }
    pthread_mutex_unlock ( &Partage->com_db.synchro );
  }
/*--------------------------------------------------------------------------------------------------------*/
