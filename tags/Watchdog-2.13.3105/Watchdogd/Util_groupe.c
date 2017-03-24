/******************************************************************************************************************************/
/* Watchdogd/Utilisateur/groupe.c    Gestion de l'interface SQL pour les groupes                                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          ven 03 avr 2009 20:32:27 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * groupe.c
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
 
 #include <unistd.h>
 #include <stdio.h>
 #include <string.h>
 #include <time.h>
 #include <stdlib.h>

/******************************************** Prototypes des fonctions ********************************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Groupe_set_groupe_utilDB: Positionne les groupes associes a un utilisateur                                                 */
/* Entrée: l'id utilisateur et son tableau des groups ip                                                                      */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Groupe_set_groupe_utilDB( guint id_util, guint *gids )
  { gchar requete[1024];
    gint cpt;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_groupeDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
               "DELETE FROM %s WHERE id_util=%d;",
		              NOM_TABLE_GIDS, id_util );

    if ( ! Lancer_requete_SQL ( db, requete ))
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Groupe_set_groupe_utilDB: Delete failed id=%d", id_util );
       Libere_DB_SQL(&db);
       return(FALSE);
     }
				   
    for ( cpt=0; cpt<NBR_MAX_GROUPE_PAR_UTIL; cpt++ )
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                  "INSERT INTO %s"
                  "(id_util,gids)"
                  "VALUES (%d, %d);",
                   NOM_TABLE_GIDS, id_util, *(gids + cpt) );

       if ( ! Lancer_requete_SQL ( db, requete ))
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                   "Groupe_set_groupe_utilDB: set gids failed for id=%d", id_util );
          Libere_DB_SQL(&db);
          return(FALSE);
        }
       if ( *(gids + cpt) == 0 ) break;                                                 /* Le groupe "0" est le groupe de fin */
     }
    Libere_DB_SQL(&db);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Groupe_get_groupe_utilDB: Recuperation des groupes d'un utilisateur                                                        */
/* Entrées: un id utilisateur et un tableau des groupes                                                                       */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Groupe_get_groupe_utilDB( guint id, guint *gids )
  { gchar requete[256];
    guint cpt;
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete), "SELECT DISTINCT gids FROM %s WHERE id_util=%d OR %d=%d ORDER BY gids DESC",
                NOM_TABLE_GIDS, id, id, UID_ROOT );                               /* Root est dans tous les groupes existants */

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_groupeDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour==FALSE) 
     { Libere_DB_SQL(&db);
       return(FALSE);
     }

    memset ( gids, 0, sizeof(guint)*NBR_MAX_GROUPE_PAR_UTIL );
    cpt=0;

    while ( Recuperer_ligne_SQL(db) != NULL && cpt<NBR_MAX_GROUPE_PAR_UTIL)
     { gint gid;
       gid = atoi(db->row[0]);
       *(gids + cpt) = gid;
       cpt++;
     }
    *(gids + cpt) = 0;                                                             /* Fin de tableau = groupe "tout le monde" */
    Libere_DB_SQL(&db);
    return(TRUE);
  }	    
/******************************************************************************************************************************/
/* Rechercher_groupeDB: Recupere un groupe par son id                                                                         */
/* Entrées: un id de groupe                                                                                                   */
/* Sortie: une structure GROUPE                                                                                               */
/******************************************************************************************************************************/
 struct CMD_TYPE_GROUPE *Rechercher_groupeDB( gint id )
  { struct CMD_TYPE_GROUPE *groupe;
    gchar requete[200];
    struct DB *db;

    g_snprintf( requete, sizeof(requete), "SELECT id, name, comment FROM %s WHERE id=%d", NOM_TABLE_GROUPE, id );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_groupeDB: DB connexion failed" );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    groupe = Recuperer_groupesDB_suite( &db );
    Libere_DB_SQL ( &db );
    return(groupe);
  }
/******************************************************************************************************************************/
/* Recuperer_groupeDB: Recupere les groupes                                                                                   */
/* Entrées: un pointeur sur un *DB                                                                                            */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Recuperer_groupesDB( struct DB **db_retour )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete), "SELECT id, name, comment FROM %s ORDER BY name", NOM_TABLE_GROUPE );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_groupeDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_groupesDB_suite: Recupération de l'enregistrement suivant                                                        */
/* Entrée: un pointeur vers la connexion DB en cours                                                                          */
/* Sortie: une structure GROUPE                                                                                               */
/******************************************************************************************************************************/
 struct CMD_TYPE_GROUPE *Recuperer_groupesDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_GROUPE *groupe;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    groupe = (struct CMD_TYPE_GROUPE *)g_try_malloc0( sizeof(struct CMD_TYPE_GROUPE) );
    if (!groupe) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                          "Recuperer_groupeDB_suite: memory error" );
    else
     { groupe->id = atoi(db->row[0]);
       memcpy( &groupe->nom, db->row[1], sizeof(groupe->nom) );                                  /* Recopie dans la structure */
       memcpy( &groupe->commentaire, db->row[2], sizeof(groupe->commentaire) );
     }
    return(groupe);
  } 
/******************************************************************************************************************************/
/* Tester_groupe_util: renvoie true si l'utilisateur fait partie du groupe en parametre                                       */
/* Entrées: une structure UTIL et un id de groupe                                                                             */
/* Sortie: false si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Tester_groupe_util( struct CMD_TYPE_UTILISATEUR *util, guint id_groupe )
  { gint cpt;
    if (!util) return(FALSE);
    if ( util->id == UID_ROOT) return(TRUE);                                             /* Le tech est dans tous les groupes */
    if (!util->enable) return(FALSE);
    if (id_groupe==GID_TOUTLEMONDE) return(TRUE);
    cpt=0;
    while( util->gids[cpt] )
     { if( util->gids[cpt] == id_groupe ) return(TRUE);
       cpt++;
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Retirer_groupe: Elimine un groupe dans la base de données                                                                  */
/* Entrées: une structure de groupe                                                                                           */
/* Sortie: true si pas de pb, false sinon                                                                                     */
/******************************************************************************************************************************/
 gboolean Retirer_groupeDB( struct CMD_TYPE_GROUPE *groupe )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    if (groupe->id < NBR_GROUPE_RESERVE)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Retirer_groupe: elimination failed: id reserve %s", groupe->nom );
       return(FALSE);
     }
       
    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "DELETE FROM %s WHERE id=%d",
                NOM_TABLE_GROUPE, groupe->id );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_groupeDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter_groupeDB: ajoute un groupe à la database                                                                           */
/* Entrée: une structure groupe a ajouter                                                                                     */
/* Sortie: -1 si probleme, id sinon                                                                                           */
/******************************************************************************************************************************/
 gint Ajouter_groupeDB ( struct CMD_TYPE_GROUPE *groupe )
  { gchar *nom, *comment;
    gchar requete[4096];
    gboolean retour;
    struct DB *db;
    gint id;

    nom     = Normaliser_chaine ( groupe->nom );
    if (!nom)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_groupeDB: Normalisation impossible" );
       return(-1);
     }
    comment = Normaliser_chaine ( groupe->commentaire );
    if (!comment)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_groupeDB: Normalisation impossible" );
       g_free(nom);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s(name,comment) VALUES ('%s', '%s');",
                 NOM_TABLE_GROUPE, nom, comment );
    g_free(nom); g_free(comment);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_groupeDB: DB connexion failed" );
       return(-1);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if ( retour == FALSE )
     { Libere_DB_SQL(&db); 
       return(-1);
     }
    id = Recuperer_last_ID_SQL ( db );
    Libere_DB_SQL(&db);
    return(id);
  }
/******************************************************************************************************************************/
/* Modifier_groupe: positionne le commentaire du groupe en parametre                                                          */
/* Entrées: une structure groupe identifiant le groupe a modifier                                                             */
/* Sortie: boolean false si probleme                                                                                          */
/******************************************************************************************************************************/
 gboolean Modifier_groupeDB( struct CMD_TYPE_GROUPE *groupe )
  { gchar requete[200];
    gchar *comment;
    gboolean retour;
    struct DB *db;

    comment = Normaliser_chaine( groupe->commentaire );
    if (!comment) return(FALSE);

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE %s SET comment = '%s' WHERE id=%d",
                NOM_TABLE_GROUPE, comment, groupe->id );
    g_free(comment);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_groupeDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
