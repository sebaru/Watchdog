/******************************************************************************************************************************/
/* Watchdogd/Syn_cadran.c       Ajout/retrait de module cadran dans les synoptiques                                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           dim 29 jan 2006 15:09:58 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * cadran.c
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
/* Retirer_cadranDB: Elimination d'un cadran                                                                                */
/* Entrée: un cadran                                                                                                         */
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
  { gchar requete[512];
    gboolean retour;
    struct DB *db;
    gint id;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_cadranDB: DB connexion failed" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s(syn_id,type,bitctrl,posx,posy,angle)"
                " VALUES (%d,%d,%d,%d,%d,'%f')", NOM_TABLE_CADRAN,
                cadran->syn_id, cadran->type, cadran->bit_controle,
                cadran->position_x, cadran->position_y, cadran->angle );

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
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_plugins_dlsDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT %s.id,%s.syn_id,%s.type,%s.bitctrl,%s.libelle,%s.posx,%s.posy,%s.angle,acro_syn"
                " FROM %s,%s WHERE %s.type=%s.type AND %s.bitctrl=%s.num AND syn_id=%d",
                NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, 
                NOM_TABLE_MNEMO, NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, 
                NOM_TABLE_CADRAN, NOM_TABLE_MNEMO,                                                                   /* From */
                NOM_TABLE_CADRAN, NOM_TABLE_MNEMO, NOM_TABLE_CADRAN, NOM_TABLE_MNEMO,
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
       cadran->syn_id       = atoi(db->row[1]);                                       /* Synoptique ou est placée le cadran */
       cadran->type         = atoi(db->row[2]);
       cadran->bit_controle = atoi(db->row[3]);                                                                /* Ixxx, Cxxx */
       cadran->position_x   = atoi(db->row[5]);                                                 /* en abscisses et ordonnées */
       cadran->position_y   = atoi(db->row[6]);
       cadran->angle        = atof(db->row[7]);
       g_snprintf( cadran->libelle, sizeof(cadran->libelle), "%s" ,db->row[4] );               /* Recopie dans la structure */
       g_snprintf( cadran->acro_syn, sizeof(cadran->acro_syn), "%s" ,db->row[8] );             /* Recopie dans la structure */
     }
    return(cadran);
  }
/******************************************************************************************************************************/
/* Rechercher_cadranDB: Recupération du cadran dont l'id est en parametre                                                   */
/* Entrée: un id de cadran                                                                                                   */
/* Sortie: un cadran                                                                                                         */
/******************************************************************************************************************************/
 struct CMD_TYPE_CADRAN *Rechercher_cadranDB ( guint id )
  { struct CMD_TYPE_CADRAN *cadran;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_cadranDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.syn_id,%s.type,%s.bitctrl,%s.libelle,%s.posx,%s.posy,%s.angle,acro_syn"
                " FROM %s,%s WHERE %s.type=%s.type AND %s.bitctrl=%s.num AND %s.id=%d",
                NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, 
                NOM_TABLE_MNEMO, NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, NOM_TABLE_CADRAN, 
                NOM_TABLE_CADRAN, NOM_TABLE_MNEMO,                                               /* From */
                NOM_TABLE_CADRAN, NOM_TABLE_MNEMO, NOM_TABLE_CADRAN, NOM_TABLE_MNEMO,
                NOM_TABLE_CADRAN, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    cadran = Recuperer_cadranDB_suite( &db );
    Libere_DB_SQL( &db );
    if (!cadran) Info_new( Config.log, Config.log_dls, LOG_INFO, "Rechercher_cadranDB: Capteur %03d not found in DB", id );
    return(cadran);
  }
/******************************************************************************************************************************/
/* Modifier_cadranDB: Modification d'un cadran Watchdog                                                                     */
/* Entrées: un cadran                                                                                                        */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Modifier_cadranDB( struct CMD_TYPE_CADRAN *cadran )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_cadranDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE %s SET "             
                "type=%d,bitctrl=%d,posx=%d,posy=%d,angle='%f'"
                " WHERE id=%d;", NOM_TABLE_CADRAN,
                cadran->type, cadran->bit_controle,
                cadran->position_x, cadran->position_y, cadran->angle,
                cadran->id );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
