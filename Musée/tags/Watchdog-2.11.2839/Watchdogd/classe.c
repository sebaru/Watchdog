/**********************************************************************************************************/
/* Watchdogd/Icones/classe.c        Déclaration des fonctions pour la gestion des classes-classes          */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 30 sep 2003 10:38:04 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * classe.c
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
/* Retirer_msgDB: Elimination d'une classe                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_classeDB ( struct CMD_TYPE_CLASSE *classe )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    if (classe->id == 0) return(TRUE);                              /* La classe 0 n'est pas destructible */

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_classeDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET id_classe=0 WHERE id_classe=%d",
                NOM_TABLE_ICONE, classe->id );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL( &db );
    return ( retour );
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_classeDB ( struct CMD_TYPE_CLASSE *classe )
  { gchar requete[200];
    gboolean retour;
    gchar *libelle;
    struct DB *db;
    gint id;

    libelle = Normaliser_chaine ( classe->libelle );                   /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_classeDB: Normalisation impossible" );
       return(-1);
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_classeDB: DB connexion failed" );
       g_free(libelle);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(libelle) VALUES "
                "('%s')", NOM_TABLE_CLASSE, libelle );
    g_free(libelle);

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */

    if ( retour == FALSE )
     { Libere_DB_SQL( &db );
       return(-1);
     }
    id = Recuperer_last_ID_SQL ( db );
    Libere_DB_SQL( &db );
    return(id);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_classeDB ( struct DB **db_retour )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_classeDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle"
                " FROM %s ORDER BY libelle", NOM_TABLE_CLASSE );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CLASSEDB *Recuperer_classeDB_suite( struct DB **db_orig )
  { struct CLASSEDB *classe;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    classe = (struct CLASSEDB *)g_try_malloc0( sizeof(struct CLASSEDB) );
    if (!classe) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_classeDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &classe->libelle, db->row[1], sizeof(classe->libelle) );      /* Recopie dans la structure */
       classe->id          = atoi(db->row[0]);
     }
    return(classe);
  }
/**********************************************************************************************************/
/* Rechercher_classeDB: Recupération du classe dont l'id est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CLASSEDB *Rechercher_classeDB ( guint id )
  { struct CLASSEDB *classe;
    gchar requete[200];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_classeDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle FROM %s WHERE id=%d", NOM_TABLE_CLASSE, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "Rechercher_classeDB: Classe %d not found in DB", id );
       return(NULL);
     }

    classe = (struct CLASSEDB *)g_try_malloc0( sizeof(struct CLASSEDB) );
    if (!classe) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_classeDB: Mem error" );
    else
     { memcpy( &classe->libelle, db->row[0], sizeof(classe->libelle) );      /* Recopie dans la structure */
       classe->id = id;
     }
    Libere_DB_SQL( &db );
    return(classe);
  }
/**********************************************************************************************************/
/* Modifier_classeDB: Modification d'un classe Watchdog                                                   */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_classeDB( struct CMD_TYPE_CLASSE *classe )
  { gchar requete[1024];
    gboolean retour;
    gchar *libelle;
    struct DB *db;

    libelle = Normaliser_chaine ( classe->libelle );
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_classeDB: Normalisation impossible" );
       return(FALSE);
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_classeDB: DB connexion failed" );
       g_free(libelle);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s' WHERE id=%d",
                NOM_TABLE_CLASSE, libelle, classe->id );
    g_free(libelle);

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL( &db );
    return( retour );
  }
/*--------------------------------------------------------------------------------------------------------*/
