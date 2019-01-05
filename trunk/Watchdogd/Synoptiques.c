/******************************************************************************************************************************/
/* Watchdogd/Synoptiques/Synoptiques.c       Déclaration des fonctions pour la gestion des synoptiques                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         jeu. 29 déc. 2011 14:00:49 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Synoptiques.c
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

/******************************************************************************************************************************/
/* Retirer_msgDB: Elimination d'un synoptique                                                                                 */
/* Entrée: un log et une database                                                                                             */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Retirer_synoptiqueDB ( struct CMD_TYPE_SYNOPTIQUE *syn )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    if (syn->id == 1) return(FALSE);                                                    /* Le synoptique 1 est indestructible */

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }
/*************************************************** Retrait de la base SYN ***************************************************/
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_SYNOPTIQUE, syn->id );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if ( ! retour )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: elimination failed %s", __func__, requete ); }
    else { Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: elimination ok", __func__ ); }

    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter_synoptiqueDB: Ajout ou edition d'un synoptique                                                                     */
/* Entrée: la structure synoptique                                                                                            */
/* Sortie: -1 si probleme                                                                                                     */
/******************************************************************************************************************************/
 gint Ajouter_synoptiqueDB ( struct CMD_TYPE_SYNOPTIQUE *syn )
  { gchar requete[512];
    gchar *libelle, *page;
    gboolean retour;
    struct DB *db;
    gint id;

    libelle = Normaliser_chaine ( syn->libelle );                                            /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible libelle", __func__ );
       return(-1);
     }

    page = Normaliser_chaine ( syn->page );                                                  /* Formatage correct des chaines */
    if (!page)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible page", __func__ );
       g_free(libelle);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s SET "
                "parent_id='%d',libelle='%s',page='%s',access_level='%d' ",
                NOM_TABLE_SYNOPTIQUE, syn->parent_id, libelle, page, syn->access_level );
    g_free(libelle);
    g_free(page);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
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
/* Recuperer_synoptiqueDB: Recupération de la liste des synoptiques                                                           */
/* Entrée: une database                                                                                                       */
/* Sortie: False si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Recuperer_synoptiqueDB ( struct DB **db_retour )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT syn.id,syn.libelle,syn.page,syn.access_level,parent.id,parent.page"
                " FROM %s as syn INNER JOIN %s as parent ON syn.parent_id=parent.id "
                "ORDER BY parent.page,syn.page,syn.libelle",
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_synoptiqueDB: Recupération de la liste des synoptiques                                                           */
/* Entrée: une database                                                                                                       */
/* Sortie: False si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Recuperer_synoptiqueDB_enfant ( struct DB **db_retour, gint id_parent )
  { gchar requete[256];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT syn.id,syn.libelle,syn.page,syn.access_level,parent.id,parent.page"
                " FROM syns as syn INNER JOIN syns as parent ON syn.parent_id=parent.id "
                "WHERE syn.parent_id='%d' "
                "ORDER BY parent.page,syn.page,syn.libelle", id_parent );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_synoptiqueDB_suite: Recupération des champs de la liste des synopyiques                                          */
/* Entrée: une database                                                                                                       */
/* Sortie: une structure referencant le synoptique chargé                                                                     */
/******************************************************************************************************************************/
 struct CMD_TYPE_SYNOPTIQUE *Recuperer_synoptiqueDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_SYNOPTIQUE *syn;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    syn = (struct CMD_TYPE_SYNOPTIQUE *)g_try_malloc0( sizeof(struct CMD_TYPE_SYNOPTIQUE) );
    if (!syn) Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else
     { g_snprintf( syn->libelle, sizeof(syn->libelle), "%s", db->row[1] );                       /* Recopie dans la structure */
       g_snprintf( syn->page, sizeof(syn->page), "%s", db->row[2] );                             /* Recopie dans la structure */
       g_snprintf( syn->parent_page, sizeof(syn->parent_page), "%s", db->row[5] );               /* Recopie dans la structure */
       syn->id           = atoi(db->row[0]);
       syn->access_level = atoi(db->row[3]);
       syn->parent_id    = atoi(db->row[4]);
     }
    return(syn);
  }
/******************************************************************************************************************************/
/* Rechercher_synoptiqueDB: Recupération du synoptique dont l'id est en parametre                                             */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 struct CMD_TYPE_SYNOPTIQUE *Rechercher_synoptiqueDB ( guint id )
  { struct CMD_TYPE_SYNOPTIQUE *syn;
    gchar requete[200];
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT syn.id,syn.libelle,syn.page,syn.access_level,parent.id,parent.page"
                " FROM %s as syn INNER JOIN %s as parent ON syn.parent_id=parent.id "
                "WHERE syn.id=%d",
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE, id );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
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
/******************************************************************************************************************************/
/* Modifier_synoptiqueDB: Modification d'un synoptique Watchdog                                                               */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                                                */
/* Sortie: -1 si pb, id sinon                                                                                                 */
/******************************************************************************************************************************/
 gboolean Modifier_synoptiqueDB( struct CMD_TYPE_SYNOPTIQUE *syn )
  { gchar requete[1024];
    gchar *libelle, *page;
    gboolean retour;
    struct DB *db;

    libelle = Normaliser_chaine ( syn->libelle );
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible libelle", __func__ );
       return(FALSE);
     }

    page = Normaliser_chaine ( syn->page );
    if (!page)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible page", __func__ );
       g_free(libelle);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s',page='%s',access_level='%d',parent_id='%d' "
                "WHERE id='%d'",
                NOM_TABLE_SYNOPTIQUE, libelle, page, syn->access_level, syn->parent_id, syn->id );
    g_free(libelle);
    g_free(page);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
