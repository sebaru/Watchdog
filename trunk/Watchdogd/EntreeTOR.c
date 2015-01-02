/**********************************************************************************************************/
/* Watchdogd/EntreeTOR/EntreeTOR.c        Déclaration des fonctions pour la gestion des digitalInput.c       */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 18 avr 2009 13:30:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * EntreeTOR.c
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
 gboolean Recuperer_digitalInputDB ( struct DB **db_retour )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_digitalInputDB: DB connexion failed" );
       return(FALSE);
     }

#ifdef bouh
                "SELECT id_mnemo,%s.num,%s.libelle,%s.groupe,%s.page,%s.name,"
                "%s.furtif"
                " FROM %s"
                " LEFT JOIN %s on %s.num_plugin = %s.id"
                " LEFT JOIN %s ON %s.num_syn    = %s.id"
                " LEFT JOIN %s ON %s.id_mnemo   = %s.id"
                " WHERE %s.type=%d ORDER BY %s.num",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,
                NOM_TABLE_DIGITAL_INPUT,
                NOM_TABLE_MNEMO,                                                                  /* FROM */
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_MNEMO, NOM_TABLE_DLS,                    /* 1er LEFT JOIN */
                NOM_TABLE_DLS, NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE,                      /* 2nd LEFT JOIN */
                NOM_TABLE_DIGITAL_INPUT, NOM_TABLE_DIGITAL_INPUT, NOM_TABLE_MNEMO,     /* 3ieme LEFT JOIN */
                NOM_TABLE_MNEMO, MNEMO_ENTREE_TOR,                                               /* WHERE */
                NOM_TABLE_MNEMO                                                               /* Order by */
#endif

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.num,"
                "%s.furtif"
                " FROM %s"
                " LEFT JOIN %s ON %s.id_mnemo = %s.id"
                " WHERE %s.type=%d ORDER BY %s.num",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO,
                NOM_TABLE_DIGITAL_INPUT,
                NOM_TABLE_MNEMO,                                                                  /* FROM */
                NOM_TABLE_DIGITAL_INPUT, NOM_TABLE_DIGITAL_INPUT, NOM_TABLE_MNEMO,           /* LEFT JOIN */
                NOM_TABLE_MNEMO, MNEMO_ENTREE,                                                   /* WHERE */
                NOM_TABLE_MNEMO                                                               /* Order by */
              );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_entreetorDB: Recupération de la liste des ids des digitalInputs                     */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_OPTION_DIGITALINPUT *Recuperer_digitalInputDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_OPTION_DIGITALINPUT *entreetor;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    entreetor = (struct CMD_TYPE_OPTION_DIGITALINPUT *)g_try_malloc0( sizeof(struct CMD_TYPE_OPTION_DIGITALINPUT) );
    if (!entreetor) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                             "Recuperer_digitalInputDB_suite: Erreur allocation mémoire" );
    else
     { entreetor->id_mnemo = atoi(db->row[0]);
       entreetor->furtif   = atoi(db->row[2]);
     }
    return(entreetor);
  }
/**********************************************************************************************************/
/* Rechercher_entreetorDB: Recupération du digitalInput dont l'id est en parametre                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_OPTION_DIGITALINPUT *Rechercher_digitalInputDB ( guint id )
  { struct CMD_TYPE_OPTION_DIGITALINPUT *entreetor;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_digitalInputDB: DB connexion failed" );
       return(NULL);
     }
 
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.num,"
                "%s.furtif"
                " FROM %s"
                " LEFT JOIN %s ON %s.id_mnemo = %s.id"
                " WHERE %s.id_mnemo=%d",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO,
                NOM_TABLE_DIGITAL_INPUT,
                NOM_TABLE_MNEMO,                                                                  /* FROM */
                NOM_TABLE_DIGITAL_INPUT, NOM_TABLE_DIGITAL_INPUT, NOM_TABLE_MNEMO,           /* LEFT JOIN */
                NOM_TABLE_DIGITAL_INPUT, id                                                      /* WHERE */
              );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    entreetor = Recuperer_digitalInputDB_suite( &db );
    Liberer_resultat_SQL (db);
    Libere_DB_SQL( &db );
    return(entreetor);
  }
/**********************************************************************************************************/
/* Modifier_digitalInputDB: Modification d'un digitalInput Watchdog                                       */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_digitalInputDB( struct CMD_TYPE_OPTION_DIGITALINPUT *entreetor )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_digitalInputDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "furtif='%d' "
                "WHERE id_mnemo=%d",
                NOM_TABLE_DIGITAL_INPUT, entreetor->furtif,
                entreetor->id_mnemo );

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
     { struct CMD_TYPE_OPTION_DIGITALINPUT *entree;
       entree = Recuperer_digitalInputDB_suite( &db );
       if (!entree) return;

       /*if (entree->num < NBR_ENTRE_TOR)
        { memcpy( &Partage->e[entree->num].confDB, entree,
                  sizeof(struct CMD_TYPE_OPTION_DIGITALINPUT) );
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                   "Charger_digitalInput: entree->num (%d) out of range (max=%d)", entree->num, NBR_ENTRE_TOR );
        }*/
       g_free(entree);
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
