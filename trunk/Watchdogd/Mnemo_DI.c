/**********************************************************************************************************/
/* Watchdogd/Mnemo_DI.c        Déclaration des fonctions pour la gestion des digitalInput.c               */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 04 janv. 2015 00:01:55 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Mnemo_DI.c
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
 
 #include <glib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "watchdogd.h"

/**********************************************************************************************************/
/* Recuperer_digitalInputDB: Recupération de la liste des ETOR                                            */
/* Entrée: un pointeur vers une nouvelle DB                                                               */
/* Sortie: TRUE si OK                                                                                     */
/**********************************************************************************************************/
 static gboolean Recuperer_digitalInputDB ( struct DB **db_retour )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_digitalInputDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.num"
                " FROM %s"
                " INNER JOIN %s ON %s.id_mnemo = %s.id"
                " WHERE %s.type=%d ORDER BY %s.num",
                NOM_TABLE_MNEMO,
                NOM_TABLE_MNEMO,                                                                  /* FROM */
                NOM_TABLE_MNEMO_DI, NOM_TABLE_MNEMO_DI, NOM_TABLE_MNEMO,                    /* INNER JOIN */
                NOM_TABLE_MNEMO, MNEMO_ENTREE,                                                   /* WHERE */
                NOM_TABLE_MNEMO                                                               /* Order by */
              );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_mnemo_diDB: Recupération de la liste des ids des digitalInputs                     */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct CMD_TYPE_MNEMO_DI *Recuperer_digitalInputDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_MNEMO_DI *mnemo_di;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    mnemo_di = (struct CMD_TYPE_MNEMO_DI *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_DI) );
    if (!mnemo_di) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                             "Recuperer_digitalInputDB_suite: Erreur allocation mémoire" );
    else
     { mnemo_di->num    = atoi(db->row[0]);
     }
    return(mnemo_di);
  }
/**********************************************************************************************************/
/* Rechercher_mnemo_diDB: Recupération du digitalInput dont l'id est en parametre                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_MNEMO_DI *Rechercher_mnemo_diDB ( guint id )
  { struct CMD_TYPE_MNEMO_DI *mnemo_di;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_mnemo_diDB: DB connexion failed" );
       return(NULL);
     }
 
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.num"
                " FROM %s"
                " INNER JOIN %s ON %s.id_mnemo = %s.id"
                " WHERE %s.id_mnemo=%d",
                NOM_TABLE_MNEMO,
                NOM_TABLE_MNEMO,                                                                  /* FROM */
                NOM_TABLE_MNEMO_DI, NOM_TABLE_MNEMO_DI, NOM_TABLE_MNEMO,                     /* LEFT JOIN */
                NOM_TABLE_MNEMO_DI, id                                                           /* WHERE */
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                       /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return(NULL);
     }

    mnemo_di = Recuperer_digitalInputDB_suite( &db );
    if (mnemo_di) Libere_DB_SQL( &db );
    return(mnemo_di);
  }
/**********************************************************************************************************/
/* Modifier_digitalInputDB: Modification d'un digitalInput Watchdog                                       */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_mnemo_diDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_mnemo_diDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s (id_mnemo) VALUES "
                "('%d) "
                /*"ON DUPLICATE KEY UPDATE "
                "furtif=VALUES(furtif) "*/,
                NOM_TABLE_MNEMO_DI, mnemo_full->mnemo_base.id
              );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Charger_digitalInput: Chargement des infos sur les Entrees TOR                                         */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Charger_digitalInput ( void )
  { struct DB *db;

    if (!Recuperer_digitalInputDB( &db ))
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Charger_digitalInput: DB Connexion Failed" );
       return;
     }                                                                         /* Si pas d'enregistrement */

    for( ; ; )
     { struct CMD_TYPE_MNEMO_DI *entree;
       entree = Recuperer_digitalInputDB_suite( &db );
       if (!entree) break;

       if (entree->num < NBR_ENTRE_TOR)
        { memcpy( &Partage->e[entree->num].confDB, entree, sizeof(struct CMD_TYPE_MNEMO_DI) ); }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                   "Charger_digitalInput: entree->num (%d) out of range (max=%d)", entree->num, NBR_ENTRE_TOR );
        }
       g_free(entree);
     }
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_digitalInput: DB reloaded" );
  }
/*--------------------------------------------------------------------------------------------------------*/
