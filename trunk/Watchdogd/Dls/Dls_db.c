/******************************************************************************************************************************/
/* Watchdogd/Dls/Dls_db       Database DLS (gestion des noms de prgs, ...)                                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        dim. 02 janv. 2011 19:06:19 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Dls_db.c
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
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>

 #include "watchdogd.h"
/******************************************************************************************************************************/
/* Retirer_dlsDB: Elimine un prg DLS dans la base de données                                                                  */
/* Entrées: iune structure identifiant le plugin                                                                              */
/* Sortie: true si pas de pb, false sinon                                                                                     */
/******************************************************************************************************************************/
 gboolean Retirer_plugin_dlsDB( struct CMD_TYPE_PLUGIN_DLS *dls )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    if (dls->id == 1) return(FALSE);                                                /* On ne peut pas effacer le plugin n°1 ! */

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( (gchar *)requete, sizeof(requete),                                                             /* Requete SQL */
                "DELETE FROM %s WHERE id=%d",
                NOM_TABLE_DLS, dls->id );
    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */

    Libere_DB_SQL(&db);

    g_snprintf( (gchar *)requete, sizeof(requete), "Dls/%06d.dls", dls->id );
    unlink( (gchar *)requete );
    g_snprintf( (gchar *)requete, sizeof(requete), "Dls/libdls%06d.so", dls->id );
    unlink( (gchar *)requete );
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter_dlsDB: Ajout d'un programme DLS dans la base de données                                                            */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                                                */
/* Sortie: -1 si pb, id sinon                                                                                                 */
/******************************************************************************************************************************/
 static gint Ajouter_Modifier_plugin_dlsDB( struct CMD_TYPE_PLUGIN_DLS *dls, gint ajout )
  { gchar *nom, *shortname;
    gchar requete[1024];
    gboolean retour;
    struct DB *db;
    gint id;

    nom = Normaliser_chaine ( dls->nom );                                                    /* Formatage correct des chaines */
    if (!nom)
     { Info_new( Config.log, Config.log_dls, LOG_WARNING, "%s: Normalisation nom impossible", __func__ );
       return(-1);
     }

    shortname = Normaliser_chaine ( dls->shortname );                                        /* Formatage correct des chaines */
    if (!shortname)
     { g_free(nom);
       Info_new( Config.log, Config.log_dls, LOG_WARNING, "%s: Normalisation shortname impossible", __func__ );
       return(-1);
     }

    if (ajout)
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "INSERT INTO %s"             
                   "(name,shortname,actif,type,syn_id,compil_date,compil_status,nbr_compil,sourcecode) "
                   "VALUES ('%s','%s','%d','%d',%d,0,0,0,'/* Source Code */');",
                   NOM_TABLE_DLS, nom, shortname, dls->on, dls->type, dls->syn_id );
     }
    else
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                  "UPDATE %s SET "             
                  "name='%s',shortname='%s',actif='%d',type='%d',syn_id=%d WHERE id=%d",
                   NOM_TABLE_DLS, nom, shortname, dls->on, dls->type, dls->syn_id, dls->id );
     }

    g_free(nom);
    g_free(shortname);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(-1);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (ajout)
     { if ( retour == FALSE )
            { id=-1; }
       else { id = Recuperer_last_ID_SQL ( db ); }
       Libere_DB_SQL(&db);
       return(id);
     }
    Libere_DB_SQL(&db);
    if (retour==FALSE) return(-1);
    return(0);
  }
/******************************************************************************************************************************/
/* Ajouter_dlsDB: Ajout d'un programme DLS dans la base de données                                                            */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                                                */
/* Sortie: -1 si pb, id sinon                                                                                                 */
/******************************************************************************************************************************/
 gint Ajouter_plugin_dlsDB( struct CMD_TYPE_PLUGIN_DLS *dls )
  { return( Ajouter_Modifier_plugin_dlsDB ( dls, TRUE ) ); }
/******************************************************************************************************************************/
/* Modifier_plugin_dlsDB: Modification d'un plugin Watchdog                                                                   */
/* Entrées: une structure decrivant le plugin a modifier                                                                      */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Modifier_plugin_dlsDB( struct CMD_TYPE_PLUGIN_DLS *dls )
  { gint retour;
    retour = Ajouter_Modifier_plugin_dlsDB ( dls, FALSE );
    if (retour == -1) return(FALSE);
    return(TRUE);
  }  
/******************************************************************************************************************************/
/* Recuperer_plugins_dlsDB: Recuperation de tous les plugins D.L.S                                                            */
/* Entrées: un log, une db                                                                                                    */
/* Sortie: une hquery, null si erreur                                                                                         */
/******************************************************************************************************************************/
 gboolean Recuperer_plugins_dlsDB( struct DB **db_retour )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT dls.id,dls.name,dls.shortname,dls.actif,dls.type,dls.syn_id,parent_syn.page,syn.page,"
                "dls.compil_date,dls.compil_status,dls.nbr_compil"
                " FROM %s as dls INNER JOIN %s as syn ON dls.syn_id = syn.id "
                " INNER JOIN %s AS parent_syn ON parent_syn.id=syn.parent_id"
                " ORDER BY parent_syn.page,syn.page,shortname",
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE
              );
    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_plugins_dlsDB_suite: poursuite de la recherche des plugins DLS                                                   */
/* Entrées: une base de données                                                                                               */
/* Sortie: une structure plugindls, ou null si erreur ou fin de requete                                                       */
/******************************************************************************************************************************/
 struct CMD_TYPE_PLUGIN_DLS *Recuperer_plugins_dlsDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_PLUGIN_DLS *dls;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    dls = (struct CMD_TYPE_PLUGIN_DLS *)g_try_malloc0( sizeof(struct CMD_TYPE_PLUGIN_DLS) );
    if (!dls) Info_new( Config.log, Config.log_dls, LOG_ERR,
                       "%s: Erreur allocation mémoire", __func__ );
    else
     { memcpy( &dls->nom,      db->row[1], sizeof(dls->nom       ) );                            /* Recopie dans la structure */
       memcpy( &dls->shortname,db->row[2], sizeof(dls->shortname ) );                            /* Recopie dans la structure */
       memcpy( &dls->syn_parent_page,db->row[6], sizeof(dls->syn_parent_page) );
       memcpy( &dls->syn_page,  db->row[7], sizeof(dls->syn_page   ) );
       dls->id            = atoi(db->row[0]);
       dls->on            = atoi(db->row[3]);
       dls->type          = atoi(db->row[4]);
       dls->syn_id        = atoi(db->row[5]);
       dls->compil_date   = atoi(db->row[8]);
       dls->compil_status = atoi(db->row[9]);
       dls->nbr_compil    = atoi(db->row[10]);
     }
    return( dls );
  }
/******************************************************************************************************************************/
/* Rechercher_plugin_dlsDB: Recuperation du plugin en parametre                                                               */
/* Entrées: un id plugin DLS                                                                                                  */
/* Sortie: une structure PLUGIN_DLS, ou null si erreur                                                                        */
/******************************************************************************************************************************/
 struct CMD_TYPE_PLUGIN_DLS *Rechercher_plugin_dlsDB( gint id )
  { struct CMD_TYPE_PLUGIN_DLS *dls;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_plugin_dlsDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT dls.id,dls.name,dls.shortname,dls.actif,dls.type,dls.syn_id,parent_syn.page,syn.page,"
                "dls.compil_date,dls.compil_status,dls.nbr_compil"
                " FROM %s as dls INNER JOIN %s as syn ON dls.syn_id = syn.id "
                " INNER JOIN %s AS parent_syn ON parent_syn.id=syn.parent_id"
                " WHERE dls.id = %d",
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE,
                id                                                                                                   /* Where */
              );

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
 gboolean Set_compil_status_plugin_dlsDB( gint id, gint status )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE %s SET "
                "compil_date='%d', compil_status='%d', nbr_compil=nbr_compil+1 WHERE id=%d",
                NOM_TABLE_DLS, (gint)time(NULL), status, id );

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
/* Get_source_dls_from_DB: Recupere le source code DLS dont l'id est en parametre                                             */
/* Entrée: l'id associé et deux variables de retour: le buffer et sa taille                                                   */
/* Sortie: FALSE si PB                                                                                                        */
/******************************************************************************************************************************/
 gboolean Get_source_dls_from_DB ( gint id, gchar **result_buffer, gint *result_taille )
  { gchar requete[200];
    gchar *buffer;
    gint taille;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT sourcecode,LENGTH(sourcecode) FROM %s WHERE id='%d'",
                NOM_TABLE_DLS, id
              );

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
     { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: Memory Error", __func__ ); }
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
 gboolean Save_source_dls_to_DB( gint id, gchar *buffer_raw, gint taille )
  { gchar *source, *requete, *buffer;
    gint taille_requete;
    gboolean retour;
    struct DB *db;

    buffer = (gchar *)g_try_malloc0( taille+1 );
    if (!buffer)
     { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: Memory Error", __func__ );
       return(FALSE);
     }
    memcpy ( buffer, buffer_raw, taille );                                            /* On s'assure du caractere nul d'arret */

    source = Normaliser_chaine ( buffer );                                                   /* Formatage correct des chaines */
    g_free(buffer);
    if (!source)
     { Info_new( Config.log, Config.log_dls, LOG_WARNING, "%s: Normalisation source impossible", __func__ );
       return(FALSE);
     }

    taille_requete = taille+256;
    requete = (gchar *)g_try_malloc( taille_requete );
    if (!requete)
     { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: Memory Error", __func__ );
       g_free(source);
       return(FALSE);
     }

    g_snprintf( requete, taille_requete,                                                                       /* Requete SQL */
               "UPDATE %s SET sourcecode='%s' WHERE id='%d'",
                NOM_TABLE_DLS, source, id );

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
