/**********************************************************************************************************/
/* Watchdogd/Dls/Dls_db       Database DLS (gestion des noms de prgs, ...)                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 02 janv. 2011 19:06:19 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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
/**********************************************************************************************************/
/* Retirer_dlsDB: Elimine un prg DLS dans la base de données                                              */
/* Entrées: un log, une db, un nom                                                                        */
/* Sortie: true si pas de pb, false sinon                                                                 */
/**********************************************************************************************************/
 gboolean Retirer_plugin_dlsDB( struct LOG *log, struct DB *db, struct CMD_TYPE_PLUGIN_DLS *dls )
  { gchar requete[200];

    g_snprintf( (gchar *)requete, sizeof(requete),                                           /* Requete SQL */
                "DELETE FROM %s WHERE id=%d",
                NOM_TABLE_DLS, dls->id );
    Lancer_requete_SQL ( log, db, requete );                               /* Execution de la requete SQL */

    g_snprintf( (gchar *)requete, sizeof(requete), "%d.dls", dls->id );
    unlink( (gchar *)requete );
    g_snprintf( (gchar *)requete, sizeof(requete), "lib%d.dls.so", dls->id );
    unlink( (gchar *)requete );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_dlsDB: Ajout d'un programme DLS dans la base de données                                        */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gint Ajouter_plugin_dlsDB( struct LOG *log, struct DB *db, struct CMD_TYPE_PLUGIN_DLS *dls )
  { gchar requete[200];
    gchar *nom;

    nom = Normaliser_chaine ( log, dls->nom );                           /* Formatage correct des chaines */
    if (!nom)
     { Info( log, DEBUG_DB, "Ajouter_dlsDB: Normalisation impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                   "INSERT INTO %s"             
                   "(name,actif,type)"
                   "VALUES ('%s','%s','%d');",
                   NOM_TABLE_DLS, nom, (dls->on ? "true" : "false"), dls->type );
    g_free(nom);

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }

/**********************************************************************************************************/
/* Recuperer_plugins_dlsDB: Recuperation de tous les plugins D.L.S                                        */
/* Entrées: un log, une db                                                                                */
/* Sortie: une hquery, null si erreur                                                                     */
/**********************************************************************************************************/
 gboolean Recuperer_plugins_dlsDB( struct LOG *log, struct DB *db )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                         /* Requete SQL */
                "SELECT name,id,actif "
                "FROM %s ORDER BY name", NOM_TABLE_DLS );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_plugins_dlsDB_suite: poursuite de la recherche des plugins DLS                               */
/* Entrées: une hquery                                                                                    */
/* Sortie: une structure plugindls, ou null si erreur ou fin de requete                                   */
/**********************************************************************************************************/
 struct PLUGIN_DLS *Recuperer_plugins_dlsDB_suite( struct LOG *log, struct DB *db )
  { struct PLUGIN_DLS *dls;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    dls = (struct PLUGIN_DLS *)g_malloc0( sizeof(struct PLUGIN_DLS) );
    if (!dls) Info( log, DEBUG_DLS, "Recuperer_plugins_dlsDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( dls->nom, db->row[0], sizeof(dls->nom) );                            /* Recopie dans la structure */
       dls->id   = atoi(db->row[1]);
       dls->on   = atoi(db->row[2]);
       dls->type = atoi(db->row[3]);
     }
    return( dls );
  }
/**********************************************************************************************************/
/* Rechercher_utilDB: Recuperation de tous les champs des utilisateurs                                    */
/* Entrées: un log, une db et un id d'utilisateur                                                         */
/* Sortie: une structure utilisateur, ou null si erreur                                                   */
/**********************************************************************************************************/
 struct PLUGIN_DLS *Rechercher_plugin_dlsDB( struct LOG *log, struct DB *db, gint id )
  { gchar requete[200];
    struct PLUGIN_DLS *dls;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT name,id,actif,type "
                "FROM %s WHERE id=%d", NOM_TABLE_DLS, id );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_dlsDB: DLS non trouvé dans la BDD", id );
       return(NULL);
     }

    dls = (struct PLUGIN_DLS *)g_malloc0( sizeof(struct PLUGIN_DLS) );
    if (!dls) Info( log, DEBUG_DLS, "Rechercher_dlsDB: Erreur allocation mémoire" );
    else
     { memcpy( dls->nom, db->row[0], sizeof(dls->nom) );                            /* Recopie dans la structure */
       dls->id   = atoi(db->row[1]);
       dls->on   = atoi(db->row[2]);
       dls->type = atoi(db->row[3]);
     }
    return( dls );
  }
/**********************************************************************************************************/
/* Modifier_plugin_dlsDB: Modification d'un plugin Watchdog                                               */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_plugin_dlsDB( struct LOG *log, struct DB *db, struct CMD_TYPE_PLUGIN_DLS *dls )
  { gchar requete[1024];
    gchar *nom;

    nom = Normaliser_chaine ( log, dls->nom );                           /* Formatage correct des chaines */
    if (!nom)
     { Info( log, DEBUG_DB, "Modifier_plugin_dlsDB: Normalisation impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "name='%s',actif='%d',type='%d' WHERE id=%d",
                NOM_TABLE_DLS, nom, dls->on, dls->type, dls->id );
    g_free(nom);

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
