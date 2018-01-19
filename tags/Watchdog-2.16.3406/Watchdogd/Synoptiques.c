/**********************************************************************************************************/
/* Watchdogd/Synoptiques/Synoptiques.c       Déclaration des fonctions pour la gestion des synoptiques    */
/* Projet WatchDog version 2.0       Gestion d'habitat                     jeu. 29 déc. 2011 14:00:49 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Synoptiques.c
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
/* Retirer_msgDB: Elimination d'un synoptique                                                             */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_synoptiqueDB ( struct CMD_TYPE_SYNOPTIQUE *syn )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    if (syn->id == 1) return(FALSE);                                /* Le synoptique 1 est indestructible */

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_synoptiqueDB: DB connexion failed" );
       return(FALSE);
     }
/****************************************** Retrait de la base SYN ****************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_SYNOPTIQUE, syn->id );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if ( ! retour )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: elimination failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: elimination ok" ); }

/****************************************** Retrait des cadrans ******************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn_id=%d", NOM_TABLE_CADRAN, syn->id );

    if ( ! Lancer_requete_SQL ( db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: elimination cadran failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: elimination cadran ok" ); }

/****************************************** Retrait des comment *******************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn_id=%d", NOM_TABLE_COMMENT, syn->id );

    if ( ! Lancer_requete_SQL ( db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: elimination comment failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: elimination comment ok" ); }

/****************************************** Retrait des motif *********************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn=%d", NOM_TABLE_MOTIF, syn->id );

    if ( ! Lancer_requete_SQL ( db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: elimination syn failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: elimination syn ok" ); }

/****************************************** Retrait des palette *******************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn_cible_id=%d", NOM_TABLE_PALETTE, syn->id );

    if ( ! Lancer_requete_SQL ( db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: elimination palette failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: elimination palette ok" ); }

/****************************************** Retrait des passerelle ****************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE syn_cible_id=%d", NOM_TABLE_PASSERELLE, syn->id );

    if ( ! Lancer_requete_SQL ( db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: elimination passerelle failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: elimination passerelle ok" ); }

/******************************************** Re-affectation des modules D.L.S ************************************************/
    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE %s SET syn_id=1 WHERE syn_id=%d", NOM_TABLE_DLS, syn->id );

    if ( ! Lancer_requete_SQL ( db, requete ) )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Retirer_synoptiqueDB: re-affectation plugin D.L.S failed %s", requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "Retirer_synoptiqueDB: re-affectation plugin D.L.S passerelle ok" ); }

    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_synoptiqueDB ( struct CMD_TYPE_SYNOPTIQUE *syn )
  { gchar requete[512];
    gchar *libelle, *page, *groupe;
    gboolean retour;
    struct DB *db;
    gint id;

    libelle = Normaliser_chaine ( syn->libelle );                   /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_synoptiqueDB: Normalisation impossible libelle" );
       return(-1);
     }

    page = Normaliser_chaine ( syn->page );                       /* Formatage correct des chaines */
    if (!page)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_synoptiqueDB: Normalisation impossible page" );
       g_free(libelle);
       return(-1);
     }

    groupe = Normaliser_chaine ( syn->groupe );                     /* Formatage correct des chaines */
    if (!groupe)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_synoptiqueDB: Normalisation impossible groupe" );
       g_free(libelle);
       g_free(page);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                               /* Requete SQL */
                "INSERT INTO %s(libelle,groupe,page,access_level) VALUES "
                "('%s','%s','%d','%s')", NOM_TABLE_SYNOPTIQUE, libelle, groupe, page, syn->access_level );
    g_free(libelle);
    g_free(page);
    g_free(groupe);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_synoptiqueDB: DB connexion failed" );
       return(-1);
     }

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
 gboolean Recuperer_synoptiqueDB ( struct DB **db_retour )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,page,access_level,groupe"
                " FROM %s ORDER BY groupe,page,libelle", NOM_TABLE_SYNOPTIQUE );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_synoptiqueDB: DB connexion failed" );
       return(FALSE);
     }

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
 struct CMD_TYPE_SYNOPTIQUE *Recuperer_synoptiqueDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_SYNOPTIQUE *syn;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    syn = (struct CMD_TYPE_SYNOPTIQUE *)g_try_malloc0( sizeof(struct CMD_TYPE_SYNOPTIQUE) );
    if (!syn) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_synoptiqueDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &syn->libelle, db->row[1], sizeof(syn->libelle) );            /* Recopie dans la structure */
       memcpy( &syn->page,    db->row[2], sizeof(syn->page   ) );            /* Recopie dans la structure */
       memcpy( &syn->groupe,  db->row[4], sizeof(syn->groupe ) );            /* Recopie dans la structure */
       syn->id           = atoi(db->row[0]);
       syn->access_level = atoi(db->row[3]);
     }
    return(syn);
  }
/**********************************************************************************************************/
/* Rechercher_synoptiqueDB: Recupération du synoptique dont l'id est en parametre                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_SYNOPTIQUE *Rechercher_synoptiqueDB ( guint id )
  { struct CMD_TYPE_SYNOPTIQUE *syn;
    gchar requete[200];
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,page,access_level,groupe"
                " FROM %s WHERE id=%d", NOM_TABLE_SYNOPTIQUE, id );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_synoptiqueDB: DB connexion failed" );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    syn = Recuperer_synoptiqueDB_suite( &db );
    Libere_DB_SQL ( &db );
    return(syn);
  }
/**********************************************************************************************************/
/* Modifier_synoptiqueDB: Modification d'un synoptique Watchdog                                           */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_synoptiqueDB( struct CMD_TYPE_SYNOPTIQUE *syn )
  { gchar requete[1024];
    gchar *libelle, *page, *groupe;
    gboolean retour;
    struct DB *db;

    libelle = Normaliser_chaine ( syn->libelle );
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_synoptiqueDB: Normalisation impossible libelle" );
       return(FALSE);
     }

    page = Normaliser_chaine ( syn->page );
    if (!page)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_synoptiqueDB: Normalisation impossible page" );
       g_free(libelle);
       return(FALSE);
     }

    groupe = Normaliser_chaine ( syn->groupe );
    if (!groupe)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_synoptiqueDB: Normalisation impossible groupe" );
       g_free(libelle);
       g_free(page);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                              /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s',page='%s',access_level='%d',groupe='%s' "
                "WHERE id='%d'",
                NOM_TABLE_SYNOPTIQUE, libelle, page, syn->access_level, groupe, syn->id );
    g_free(libelle);
    g_free(page);
    g_free(groupe);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_synoptiqueDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/*--------------------------------------------------------------------------------------------------------*/
