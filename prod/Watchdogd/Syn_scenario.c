/******************************************************************************************************************************/
/* Watchdogd/Syn_scenario.c       Ajout/retrait de scenario dans les synoptiques                                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    25.06.2017 16:49:14 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Syn_scenario.c
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

/******************************************************************************************************************************/
/* Retirer_scenarioDB: Elimination d'un scenario en base de données                                                           */
/* Entrée: l'id du scenario a virer                                                                                           */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Retirer_scenarioDB ( guint id )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed. Unable to delete id '%d'", __func__, id );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_SCENARIO, id );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter__modifier_scenarioDB: Ajout ou edition d'un scenario                                                               */
/* Entrée: une structure represantant le scenario et un flag d'edition                                                        */
/* Sortie: -1 si erreur sinon, last_sql_id                                                                                    */
/******************************************************************************************************************************/
 static gint Ajouter_modifier_scenarioDB ( struct CMD_TYPE_SCENARIO *scenario, gboolean edition )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;
    gint id;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(-1);
     }

    if (edition==FALSE)
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                  "INSERT INTO %s(syn_id, num, posx, posy) VALUES "
                  "('%d','%d','%d','%d')",
                   NOM_TABLE_SCENARIO, scenario->syn_id, scenario->num, scenario->posx, scenario->posy );
     }
    else
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                  "UPDATE %s SET "             
                  "num='%d',posx='%d',posy='%d'"
                  " WHERE id=%d;", NOM_TABLE_SCENARIO, scenario->num, scenario->posx, scenario->posy,
                  scenario->id );
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if ( retour == FALSE )
     { Libere_DB_SQL(&db); 
       return(-1);
     }
    if (edition==FALSE)
     { id = Recuperer_last_ID_SQL ( db ); }
    else { id=0; }
    Libere_DB_SQL(&db);
    return(id);
  }
/******************************************************************************************************************************/
/* Ajouter_scenarioDB: Ajout ou edition d'un scenario                                                                         */
/* Entrée: une structure represantant le scenario                                                                             */
/* Sortie: -1 si erreur sinon, last_sql_id                                                                                    */
/******************************************************************************************************************************/
 gint Ajouter_scenarioDB ( struct CMD_TYPE_SCENARIO *scenario )
  { return ( Ajouter_modifier_scenarioDB ( scenario, FALSE ) ); }
/******************************************************************************************************************************/
/* Modifier_scenarioDB: Ajout ou edition d'un scenario                                                                        */
/* Entrée: une structure represantant le scenario                                                                             */
/* Sortie: -1 si erreur sinon 0                                                                                               */
/******************************************************************************************************************************/
 gint Modifier_scenarioDB ( struct CMD_TYPE_SCENARIO *scenario )
  { return ( Ajouter_modifier_scenarioDB ( scenario, TRUE ) ); }
 /*****************************************************************************************************************************/
/* Recuperer_scenarioDB: Recupération de la liste des scenario d'un synoptique                                                */
/* Entrée: une database (retour) et un id synoptique                                                                          */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Recuperer_scenarioDB ( struct DB **db_retour, gint syn_id )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed. Unable to load id '%d'", __func__, syn_id );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT id,syn_id,num,posx,posy"
                " FROM %s WHERE syn_id='%d'", NOM_TABLE_SCENARIO, syn_id );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_scenarioDB_suite : Contination de la recupération de la liste des scenario d'un synoptique                       */
/* Entrée: une database                                                                                                       */
/* Sortie: une structure representant le scenario                                                                             */
/******************************************************************************************************************************/
 struct CMD_TYPE_SCENARIO *Recuperer_scenarioDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_SCENARIO *scenario;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    scenario = (struct CMD_TYPE_SCENARIO *)g_try_malloc0( sizeof(struct CMD_TYPE_SCENARIO) );
    if (!scenario) Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else
     { scenario->id     = atoi(db->row[0]);
       scenario->syn_id = atoi(db->row[1]);
       scenario->num    = atoi(db->row[2]);
       scenario->posx   = atoi(db->row[3]);                                                      /* en abscisses et ordonnées */
       scenario->posy   = atoi(db->row[4]);
     }
    return(scenario);
  }
/******************************************************************************************************************************/
/* Rechercher_scenarioDB: Recupération du scenario dont l'id est en parametre                                                 */
/* Entrée: un id de scenario                                                                                                  */
/* Sortie: une structure representant le scenario                                                                             */
/******************************************************************************************************************************/
 struct CMD_TYPE_SCENARIO *Rechercher_scenarioDB ( guint id )
  { struct CMD_TYPE_SCENARIO *scenario;
    gchar requete[512];
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT id,syn_id,num,posx,posy"
                " FROM %s WHERE id='%d'", NOM_TABLE_SCENARIO, id );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed. Unable to edit id '%d'", __func__, id );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    scenario = Recuperer_scenarioDB_suite( &db );
    Libere_DB_SQL ( &db );
    return(scenario);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
