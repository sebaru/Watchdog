/******************************************************************************************************************************/
/* Watchdogd/Dls/Dls_db       Database DLS (gestion des noms de prgs, ...)                                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        dim. 02 janv. 2011 19:06:19 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Dls_db.c
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

 #include <glib.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>

 #include "watchdogd.h"
/******************************************************************************************************************************/
/* Dls_auto_create_plugin: Créé automatiquement le plugin en parametre (tech_id, nom)                                         */
/* Entrées: le tech_id (unique) et le nom associé                                                                             */
/* Sortie: -1 si pb, id sinon                                                                                                 */
/******************************************************************************************************************************/
 gboolean Dls_auto_create_plugin( gchar *tech_id, gchar *nom_src )
  { gchar requete[1024], *nom;
    gboolean retour;
    struct DB *db;

    nom = Normaliser_chaine ( nom_src );                                                     /* Formatage correct des chaines */
    if (!nom)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING, "%s: Normalisation nom impossible", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
               "INSERT INTO dls SET is_thread=1,actif=1,"
               "tech_id=UPPER('%s'),shortname='%s',name='%s',package='custom',"
               "syn_id=1,compil_status=0,sourcecode=' - IO_COMM -> COMM; /* Recopie de la comm IO vers le bit de COMM partagé ! */' "
               "ON DUPLICATE KEY UPDATE tech_id=VALUES(tech_id),shortname=VALUES(shortname),"
               "name=VALUES(name),is_thread=1", tech_id, tech_id, nom );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Creating DLS '%s'", __func__, nom );
    g_free(nom);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Recuperer_plugins_dlsDB: Recuperation de tous les plugins D.L.S                                                            */
/* Entrées: un log, une db                                                                                                    */
/* Sortie: une hquery, null si erreur                                                                                         */
/******************************************************************************************************************************/
 static gboolean Recuperer_plugins_dlsDB_with_conditions( struct DB **db_retour, gchar *conditions )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT dls_id,dls.name,dls.shortname,dls.actif,dls.package,dls.syn_id,parent_syn.page,syn.page,"
                "dls.compil_date,dls.compil_status,dls.nbr_compil,tech_id,nbr_ligne,debug,is_thread"
                " FROM dls INNER JOIN syns as syn ON dls.syn_id = syn.id "
                " INNER JOIN syns AS parent_syn ON parent_syn.id=syn.parent_id"
                " %s "
                " ORDER BY parent_syn.page,syn.page,dls.shortname",
                (conditions ? conditions : " ")
              );
    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_plugins_dlsDB: Recuperation de tous les plugins D.L.S                                                            */
/* Entrées: un log, une db                                                                                                    */
/* Sortie: une hquery, null si erreur                                                                                         */
/******************************************************************************************************************************/
 gboolean Recuperer_plugins_dlsDB( struct DB **db_retour )
  { return( Recuperer_plugins_dlsDB_with_conditions ( db_retour, NULL ) ); }
/******************************************************************************************************************************/
/* Recuperer_plugins_dlsDB: Recuperation de tous les plugins D.L.S                                                            */
/* Entrées: un log, une db                                                                                                    */
/* Sortie: une hquery, null si erreur                                                                                         */
/******************************************************************************************************************************/
 gboolean Recuperer_plugins_dlsDB_by_syn( struct DB **db_retour, gint syn_id )
  { gchar chaine[80];
    g_snprintf( chaine, sizeof(chaine), "WHERE syn.id=%d", syn_id );
    return( Recuperer_plugins_dlsDB_with_conditions ( db_retour, chaine ) );
  }
/******************************************************************************************************************************/
/* Recuperer_plugins_dlsDB_suite: poursuite de la recherche des plugins DLS                                                   */
/* Entrées: une base de données                                                                                               */
/* Sortie: une structure plugindls, ou null si erreur ou fin de requete                                                       */
/******************************************************************************************************************************/
 struct DLS_PLUGIN *Recuperer_plugins_dlsDB_suite( struct DB **db_orig )
  { struct DLS_PLUGIN *dls;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    dls = (struct DLS_PLUGIN *)g_try_malloc0( sizeof(struct DLS_PLUGIN) );
    if (!dls) Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
                       "%s: Erreur allocation mémoire", __func__ );
    else
     { g_snprintf( dls->tech_id, sizeof(dls->tech_id), "%s", db->row[11] );
       g_snprintf( dls->package, sizeof(dls->package), "%s", db->row[4] );
       g_snprintf( dls->nom, sizeof(dls->nom), "%s", db->row[1] );
       g_snprintf( dls->shortname, sizeof(dls->shortname), "%s", db->row[2] );
       g_snprintf( dls->syn_parent_page, sizeof(dls->syn_parent_page), "%s", db->row[6] );
       g_snprintf( dls->syn_page, sizeof(dls->syn_page), "%s", db->row[7] );
       g_snprintf( dls->compil_date, sizeof(dls->compil_date), "%s", db->row[8] );
       dls->id            = atoi(db->row[0]);
       dls->on            = atoi(db->row[3]);
       dls->syn_id        = atoi(db->row[5]);
       dls->compil_status = atoi(db->row[9]);
       dls->nbr_compil    = atoi(db->row[10]);
       dls->nbr_ligne     = atoi(db->row[12]);
       dls->debug         = atoi(db->row[13]);
       dls->is_thread     = atoi(db->row[14]);
     }
    return( dls );
  }
/******************************************************************************************************************************/
/* Rechercher_plugin_dlsDB: Recuperation du plugin en parametre                                                               */
/* Entrées: un id plugin DLS                                                                                                  */
/* Sortie: une structure DLS_PLUGIN, ou null si erreur                                                                        */
/******************************************************************************************************************************/
 struct DLS_PLUGIN *Rechercher_plugin_dlsDB( gchar *tech_id_src )
  { struct DLS_PLUGIN *dls;
    gchar requete[512], *tech_id;
    struct DB *db;

    tech_id = Normaliser_chaine ( tech_id_src );                                             /* Formatage correct des chaines */
    if (!tech_id)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING, "%s: Normalisation tech_id impossible", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT dls_id,dls.name,dls.shortname,dls.actif,dls.package,dls.syn_id,parent_syn.page,syn.page,"
                "dls.compil_date,dls.compil_status,dls.nbr_compil,tech_id,nbr_ligne,is_thread"
                " FROM %s as dls INNER JOIN %s as syn ON dls.syn_id = syn.id "
                " INNER JOIN %s AS parent_syn ON parent_syn.id=syn.parent_id"
                " WHERE dls.tech_id = '%s'",
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE,
                tech_id                                                                                              /* Where */
              );
    g_free(tech_id);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    dls = Recuperer_plugins_dlsDB_suite( &db );
    Libere_DB_SQL( &db );
    return( dls );
  }
/******************************************************************************************************************************/
/* Set_compil_status_plugin_dlsDB: Met a jour la date et statut de compilation                                                */
/* Entrées: l'id du plugin DLS                                                                                                */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Set_compil_status_plugin_dlsDB( gchar *tech_id_src, gint status, gchar *log_buffer )
  { gchar *log, *tech_id;
    gboolean retour;

    tech_id = Normaliser_chaine ( tech_id_src );                                             /* Formatage correct des chaines */
    if (!tech_id)
     { Info_new( Config.log, Config.log_trad, LOG_WARNING, "%s: Normalisation tech_id impossible", __func__ );
       return(FALSE);
     }

    log = Normaliser_chaine ( log_buffer );                                                  /* Formatage correct des chaines */
    if (!log)
     { Info_new( Config.log, Config.log_trad, LOG_WARNING, "%s: Normalisation buffer impossible", __func__ );
       g_free(tech_id);
       return(FALSE);
     }

    if (status >= DLS_COMPIL_OK)
     { retour = SQL_Write_new ( "UPDATE %s SET "
                                "compil_date=NOW(), compil_status='%d', nbr_compil=nbr_compil+1, "
                                "nbr_ligne = LENGTH(`sourcecode`)-LENGTH(REPLACE(`sourcecode`,'\n',''))+1, "
                                "errorlog='%s' "
                                "WHERE tech_id='%s'",
                                NOM_TABLE_DLS, status, log, tech_id );
     }
    else
     { retour = SQL_Write_new ( "UPDATE %s SET "
                                "compil_status='%d', nbr_compil=nbr_compil+1, "
                                "nbr_ligne = LENGTH(`sourcecode`)-LENGTH(REPLACE(`sourcecode`,'\n',''))+1, "
                                "errorlog='%s' "
                                "WHERE tech_id='%s'",
                                NOM_TABLE_DLS, status, log, tech_id );
     }
    g_free(log);
    g_free(tech_id);
    return(retour);
  }
/******************************************************************************************************************************/
/* Get_source_dls_from_DB: Recupere le source code DLS dont l'id est en parametre                                             */
/* Entrée: l'id associé et deux variables de retour: le buffer et sa taille                                                   */
/* Sortie: FALSE si PB                                                                                                        */
/******************************************************************************************************************************/
 gboolean Get_source_dls_from_DB ( gchar *tech_id_src, gchar **result_buffer, gint *result_taille )
  { gchar requete[200];
    gchar *buffer, *tech_id;
    gint taille;
    struct DB *db;

    tech_id = Normaliser_chaine ( tech_id_src );                                             /* Formatage correct des chaines */
    if (!tech_id)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING, "%s: Normalisation tech_id impossible", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT sourcecode,LENGTH(sourcecode) FROM %s WHERE tech_id='%s'",
                NOM_TABLE_DLS, tech_id
              );
    g_free(tech_id);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed" );
       return(FALSE);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(FALSE);
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(FALSE);
     }
    taille = atoi(db->row[1]);
    buffer = (gchar *)g_try_malloc0( taille + 1 );
    if (!buffer)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory Error", __func__ ); }
    else
     { memcpy( buffer, db->row[0], taille ); }                                                   /* Recopie dans la structure */
    *result_buffer = buffer;
    *result_taille = taille;
    Libere_DB_SQL( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Save_source_dls_to_DB: Sauvegarde en base le source dls en parametre                                                       */
/* Entrées: l'id du plugin associé, le sourcecode et sa taille                                                                */
/* Sortie: FALSE si PB                                                                                                        */
/******************************************************************************************************************************/
 gboolean Save_source_dls_to_DB( gchar *tech_id, gchar *buffer_raw, gint taille )
  { gchar *source, *requete, *buffer;
    gint taille_requete;
    gboolean retour;
    struct DB *db;

    buffer = (gchar *)g_try_malloc0( taille+1 );
    if (!buffer)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory Error", __func__ );
       return(FALSE);
     }
    memcpy ( buffer, buffer_raw, taille );                                            /* On s'assure du caractere nul d'arret */

    source = Normaliser_chaine ( buffer );                                                   /* Formatage correct des chaines */
    g_free(buffer);
    if (!source)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Normalisation source impossible", __func__ );
       return(FALSE);
     }

    taille_requete = taille+256;
    requete = (gchar *)g_try_malloc( taille_requete );
    if (!requete)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory Error", __func__ );
       g_free(source);
       return(FALSE);
     }

    g_snprintf( requete, taille_requete,                                                                       /* Requete SQL */
               "UPDATE %s SET sourcecode='%s' WHERE tech_id='%s'",
                NOM_TABLE_DLS, source, tech_id );

    g_free(source);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       g_free(requete);
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    g_free(requete);
    Libere_DB_SQL( &db );
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
