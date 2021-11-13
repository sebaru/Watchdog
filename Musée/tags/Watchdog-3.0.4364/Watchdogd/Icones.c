/**********************************************************************************************************/
/* Watchdogd/Icones/Icones.c        Déclaration des fonctions pour la gestion des icones                  */
/* Projet WatchDog version 3.0       Gestion d'habitat                      mar 30 sep 2003 10:38:04 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Icones.c
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
 
 #include <glib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "watchdogd.h"

/**********************************************************************************************************/
/* Retirer_msgDB: Elimination d'un message                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_iconeDB ( struct CMD_TYPE_ICONE *icone )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    if (icone->id < 10000) return(FALSE);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_iconeDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_ICONE, icone->id );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */

/*************************************** Re-affectation des mnémoniques ***********************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE icone=%d", NOM_TABLE_MOTIF, icone->id );

    Lancer_requete_SQL ( db, requete );
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_iconeDB ( struct CMD_TYPE_ICONE *icone )
  { gchar requete[200];
    gboolean retour;
    gchar *libelle;
    struct DB *db;
    gint id;

    libelle = Normaliser_chaine ( icone->libelle );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_iconeDB: Normalisation impossible" );
       return(-1);
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_iconeDB: DB connexion failed" );
       g_free(libelle);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(libelle,id_classe) VALUES "
                "('%s',%d)", NOM_TABLE_ICONE, libelle, icone->id_classe );
    g_free(libelle);

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if ( retour == FALSE )
     { Libere_DB_SQL(&db); 
       return(-1);
     }
    id = Recuperer_last_ID_SQL ( db );
    Libere_DB_SQL(&db);
    return(id);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_iconeDB ( struct DB **db_retour, guint classe )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_iconeDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,id_classe"
                " FROM %s WHERE id_classe=%d ORDER BY libelle", NOM_TABLE_ICONE, classe );

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
 struct ICONEDB *Recuperer_iconeDB_suite( struct DB **db_orig )
  { struct ICONEDB *icone;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    icone = (struct ICONEDB *)g_try_malloc0( sizeof(struct ICONEDB) );
    if (!icone) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_iconeDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &icone->libelle, db->row[1], sizeof(icone->libelle) );        /* Recopie dans la structure */
       icone->id          = atoi(db->row[0]);
       icone->id_classe   = atoi(db->row[2]);
     }
    return(icone);
  }
/**********************************************************************************************************/
/* Rechercher_iconeDB: Recupération du icone dont l'id est en parametre                                   */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ICONEDB *Rechercher_iconeDB ( guint id )
  { struct ICONEDB *icone;
    gchar requete[200];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_iconeDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle,id_classe FROM %s WHERE id=%d", NOM_TABLE_ICONE, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Config.log_dls, LOG_INFO, "Rechercher_iconeDB: Icone %04d not found in DB", id );
       return(NULL);
     }

    icone = (struct ICONEDB *)g_try_malloc0( sizeof(struct ICONEDB) );
    if (!icone) { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_iconeDB: Mem error" ); }
    else
     { memcpy( &icone->libelle, db->row[0], sizeof(icone->libelle) );        /* Recopie dans la structure */
       icone->id          = id;
       icone->id_classe   = atoi(db->row[1]);
     }
    Libere_DB_SQL( &db );
    return(icone);
  }
/**********************************************************************************************************/
/* Modifier_iconeDB: Modification d'un icone Watchdog                                                     */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_iconeDB( struct CMD_TYPE_ICONE *icone )
  { gchar requete[1024];
    gboolean retour;
    gchar *libelle;
    struct DB *db;

    libelle = Normaliser_chaine ( icone->libelle );
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_iconeDB: Normalisation impossible" );
       return(FALSE);
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_icone_dlsDB: DB connexion failed" );
       g_free(libelle);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s',id_classe=%d WHERE id=%d",
                NOM_TABLE_ICONE, libelle, icone->id_classe, icone->id );
    g_free(libelle);

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Icone_get_data_version: Récupère le numéro de version des données Icones en bases de données                               */
/* Entrée: Néant                                                                                                              */
/* Sortie: int (time_t en fait)                                                                                               */
/******************************************************************************************************************************/
 gint Icone_get_data_version ( void )
  { gint icone_version;
    gchar *nom, *valeur;
    struct DB *db;

    if (Config.instance_is_master != TRUE)                                                  /* Do not update DB if not master */
     { Info_new( Config.log, Config.log_db, LOG_WARNING,
                "Icone_get_data_version: Instance is not master. Quitting." );
       return(-1);
     }

    icone_version = -1;                                                                                  /* valeur par défaut */
    if ( ! Recuperer_configDB( &db, "global" ) )                                            /* Connexion a la base de données */
     { Info_new( Config.log, Config.log_db, LOG_WARNING,
                "Icone_get_data_version: Database connexion failed" );
       return(-1);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { if ( ! g_ascii_strcasecmp ( nom, "icone_version" ) )
        { icone_version = atoi( valeur ); }
     }

    if (icone_version == -1)                                                      /* si pas trouvé ... On met la date du jour */
     { Info_new( Config.log, Config.log_db, LOG_WARNING,                                                      /* Print Config */
             "Icone_get_data_version: not found in Database. Using 'NOW'" );
       Icone_set_data_version();
       icone_version = (gint)time(NULL);
     }
    Info_new( Config.log, Config.log_db, LOG_INFO,                                                            /* Print Config */
             "Icone_get_data_version: found icone_version = %d", icone_version );
    return(icone_version);
  }
/******************************************************************************************************************************/
/* Ajouter_Modifier_iconeDB: Ajoute ou modifie un icone Watchdog                                                              */
/* Entrées: une structure referencant l'icone a ajouter ou modifier                                                           */
/* Sortie: -1 si pb, nouvel id sinon                                                                                          */
/******************************************************************************************************************************/
 gint Ajouter_Modifier_iconenewDB( struct ICONEDBNEW *icone )
  { gchar *description, *classe;
    gchar requete[1024];
    gboolean retour;
    struct DB *db;
    gint id;

    description = Normaliser_chaine ( icone->description );
    if (!description)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible %s", __func__, icone->description );
       return(-1);
     }

    classe = Normaliser_chaine ( icone->classe );
    if (!classe)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible %s", __func__, icone->classe );
       g_free(description);
       return(-1);
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       g_free(description);
       g_free(classe);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s (description,classe) VALUES ('%s','%s') "
                "ON DUPLICATE KEY UPDATE id=LAST_INSERT_ID(id), classe=VALUES(classe)",
                NOM_TABLE_ICONE_NEW, description, icone->classe );
    g_free(description);
    g_free(classe);

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if ( retour == FALSE )
     { Libere_DB_SQL(&db); 
       return(-1);
     }
    id = Recuperer_last_ID_SQL ( db );
    Libere_DB_SQL(&db);
    return(id);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
