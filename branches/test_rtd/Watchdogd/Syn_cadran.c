/******************************************************************************************************************************/
/* Watchdogd/Syn_cadran.c       Ajout/retrait de module cadran dans les synoptiques                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           dim 29 jan 2006 15:09:58 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * cadran.c
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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Synoptique_auto_create_CADRAN: Création automatique d'un cadran depuis la compilation DLS                                  */
/* Entrée: le plugin source, la cible, et les parametres du cadran                                                            */
/* Sortie: FALSE si erreu                                                                                                     */
/******************************************************************************************************************************/
 gboolean Synoptique_auto_create_CADRAN ( struct DLS_PLUGIN *plugin, gchar *tech_id, gchar *acronyme, gchar *forme_src,
                                          gdouble min, gdouble max,
                                          gdouble seuil_ntb, gdouble seuil_nb,
                                          gdouble seuil_nh, gdouble seuil_nth,
                                          gint nb_decimal )
  { gchar *acro, *forme;
/******************************************** Préparation de la base du mnemo *************************************************/
    acro       = Normaliser_chaine ( acronyme );                                             /* Formatage correct des chaines */
    if ( !acro )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation acro impossible. Mnemo NOT added nor modified.", __func__ );
       return(FALSE);
     }

    forme      = Normaliser_chaine ( forme_src );                                            /* Formatage correct des chaines */
    if ( !forme )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation forme impossible. Mnemo NOT added nor modified.", __func__ );
       g_free(acro);
       return(FALSE);
     }

    gboolean retour;
    retour = SQL_Write_new
               ("INSERT INTO syns_cadrans SET "
                "dls_id=%d, tech_id='%s', acronyme='%s', forme='%s', minimum='%f', maximum='%f', "
                "seuil_ntb='%f', seuil_nb='%f', seuil_nh='%f', seuil_nth='%f', "
                "nb_decimal='%d', posx='150', posy='150' "
                "ON DUPLICATE KEY UPDATE forme=VALUES(forme), "
                "minimum=VALUES(minimum), maximum=VALUES(maximum), nb_decimal=VALUES(nb_decimal), "
                "seuil_ntb=VALUES(seuil_ntb), seuil_nb=VALUES(seuil_nb), seuil_nh=VALUES(seuil_nh), seuil_nth=VALUES(seuil_nth)",
                plugin->id, tech_id, acro, forme, min, max, seuil_ntb, seuil_nb, seuil_nh, seuil_nth, nb_decimal );

    g_free(forme);
    g_free(acro);
    return (retour);
  }
/******************************************************************************************************************************/
/* Retirer_cadranDB: Elimination d'un cadran                                                                                  */
/* Entrée: un cadran                                                                                                          */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Retirer_cadranDB ( struct CMD_TYPE_CADRAN *cadran )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_cadranDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_CADRAN, cadran->id );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                                               */
/* Entrée: un cadran                                                                                                         */
/* Sortie: -1 si probleme sinon last_id_sql                                                                                   */
/******************************************************************************************************************************/
 gint Ajouter_cadranDB ( struct CMD_TYPE_CADRAN *cadran )
  { gchar requete[512], *tech_id, *acronyme;
    gboolean retour;
    struct DB *db;
    gint id;

    tech_id = Normaliser_chaine ( cadran->tech_id );                                         /* Formatage correct des chaines */
    if (!tech_id)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation tech_id impossible", __func__ );
       return(-1);
     }
    acronyme = Normaliser_chaine ( cadran->acronyme );                                       /* Formatage correct des chaines */
    if (!acronyme)
     { g_free(tech_id);
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation acronyme impossible", __func__ );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s(syn_id,type,posx,posy,angle,tech_id,acronyme,nb_decimal)"
                " VALUES (%d,%d,%d,%d,'%d','%s','%s','%d')", NOM_TABLE_CADRAN,
                cadran->syn_id, cadran->type,
                cadran->position_x, cadran->position_y, cadran->angle, tech_id, acronyme, cadran->nb_decimal );
    g_free(tech_id);
    g_free(acronyme);

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
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                                    */
/* Entrée: un id de synoptique et une database                                                                                */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Recuperer_cadranDB ( struct DB **db_retour, gint id_syn )
  { gchar requete[2048];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT %s.id,%s.syn_id,%s.type,%s.posx,%s.posy,%s.angle,"
                "syns_cadrans.tech_id,syns_cadrans.acronyme, nb_decimal"
                " FROM %s WHERE syn_id=%d",
                NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, NOM_TABLE_CADRAN,
                NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, NOM_TABLE_CADRAN,
                NOM_TABLE_CADRAN,                                                                                    /* From */
                id_syn );
    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                                    */
/* Entrée: une database                                                                                                       */
/* Sortie: un cadran                                                                                                         */
/******************************************************************************************************************************/
 struct CMD_TYPE_CADRAN *Recuperer_cadranDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_CADRAN *cadran;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    cadran = (struct CMD_TYPE_CADRAN *)g_try_malloc0( sizeof(struct CMD_TYPE_CADRAN) );
    if (!cadran) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                           "Recuperer_cadranDB_suite: memory error" );
    else
     { cadran->id           = atoi(db->row[0]);
       cadran->syn_id       = atoi(db->row[1]);                                         /* Synoptique ou est placée le cadran */
       cadran->type         = atoi(db->row[2]);
       cadran->position_x   = atoi(db->row[3]);                                                  /* en abscisses et ordonnées */
       cadran->position_y   = atoi(db->row[4]);
       cadran->angle        = atoi(db->row[5]);
       g_snprintf( cadran->tech_id,  sizeof(cadran->tech_id),  "%s" ,db->row[6] );               /* Recopie dans la structure */
       g_snprintf( cadran->acronyme, sizeof(cadran->acronyme), "%s" ,db->row[7] );               /* Recopie dans la structure */
       cadran->nb_decimal   = atoi(db->row[8]);
     }
    return(cadran);
  }
/******************************************************************************************************************************/
/* Rechercher_cadranDB: Recupération du cadran dont l'id est en parametre                                                     */
/* Entrée: un id de cadran                                                                                                    */
/* Sortie: un cadran                                                                                                          */
/******************************************************************************************************************************/
 struct CMD_TYPE_CADRAN *Rechercher_cadranDB ( guint id )
  { struct CMD_TYPE_CADRAN *cadran;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.syn_id,%s.type,%s.posx,%s.posy,%s.angle,"
                "syns_cadrans.tech_id,syns_cadrans.acronyme, nb_decimal"
                " FROM %s WHERE %s.id=%d",
                NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, NOM_TABLE_CADRAN,
                NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, NOM_TABLE_CADRAN,
                NOM_TABLE_CADRAN,                                                                /* From */
                NOM_TABLE_CADRAN, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    cadran = Recuperer_cadranDB_suite( &db );
    if (!cadran) Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: Capteur %03d not found in DB", __func__, id );
    else Libere_DB_SQL( &db );
    return(cadran);
  }
/******************************************************************************************************************************/
/* Modifier_cadranDB: Modification d'un cadran Watchdog                                                                     */
/* Entrées: un cadran                                                                                                        */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Modifier_cadranDB( struct CMD_TYPE_CADRAN *cadran )
  { gchar requete[1024], *tech_id, *acronyme;
    gboolean retour;
    struct DB *db;

    tech_id = Normaliser_chaine ( cadran->tech_id );                                         /* Formatage correct des chaines */
    if (!tech_id)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation tech_id impossible", __func__ );
       return(-1);
     }
    acronyme = Normaliser_chaine ( cadran->acronyme );                                       /* Formatage correct des chaines */
    if (!acronyme)
     { g_free(tech_id);
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation acronyme impossible", __func__ );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE %s SET "
                "type=%d,posx=%d,posy=%d,angle='%d',tech_id='%s',acronyme='%s',nb_decimal='%d'"
                " WHERE id=%d;", NOM_TABLE_CADRAN,
                cadran->type,
                cadran->position_x, cadran->position_y, cadran->angle,
                tech_id, acronyme, cadran->nb_decimal, cadran->id );
    g_free(tech_id);
    g_free(acronyme);

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
