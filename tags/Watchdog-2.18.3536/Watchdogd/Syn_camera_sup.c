/******************************************************************************************************************************/
/* Watchdogd/Synoptiques/camera_sup.c       Ajout/retrait de module camera_sup dans les synoptiques                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           dim 29 jan 2006 15:09:58 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Syn_camera_sup.c
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
/* Retirer_camera_supDB: Elimination d'un camera_sup                                                                          */
/* Entrée: L'ID de la camera a supprimer                                                                                      */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Retirer_camera_supDB ( gint id )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_CAMERASUP, id );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter_modifier_camera_supDB: ajoute ou modifie une des camera de supervision Watchdog                                    */
/* Entrée: La structure referencant la camera                                                                                 */
/* Sortie: -1 si probleme, 0 si modif OK, last_sql_id si ajout                                                                */
/******************************************************************************************************************************/
 static gint Ajouter_Modifier_camera_supDB ( struct CMD_TYPE_CAMERASUP *camera_sup, gint ajout )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;
    gint last_id;

    if (ajout == TRUE)
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "INSERT INTO %s(syn_id, camera_src_id, posx, posy) VALUES "
                   "('%d','%d','%d','%d')", NOM_TABLE_CAMERASUP,
                   camera_sup->syn_id, camera_sup->camera_src_id, camera_sup->posx, camera_sup->posy );
     } else
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE %s SET "             
                   "syn_id=%d,posx='%d',posy='%d' "
                   "WHERE id=%d", NOM_TABLE_CAMERASUP,
                   camera_sup->syn_id, camera_sup->posx, camera_sup->posy,
                   camera_sup->id );
     }

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

    if (ajout==TRUE) last_id = Recuperer_last_ID_SQL ( db );
    Libere_DB_SQL(&db);

    if (ajout==TRUE) return(last_id);
    else return(0);
  }
/******************************************************************************************************************************/
/* Ajouter_camera_supDB: Ajout une camera sur le synoptique en cours                                                          */
/* Entrée: La structure referencant la camera                                                                                 */
/* Sortie: -1 si probleme, 0 si modif OK, last_sql_id si ajout                                                                */
/******************************************************************************************************************************/
 gint Ajouter_camera_supDB ( struct CMD_TYPE_CAMERASUP *camera_sup )
  { return( Ajouter_Modifier_camera_supDB(camera_sup, TRUE) ); }
/******************************************************************************************************************************/
/* Modifier_camera_supDB: Modifie une camera sur le synoptique en cours                                                       */
/* Entrée: La structure referencant la camera                                                                                 */
/* Sortie: -1 si probleme, 0 si modif OK, last_sql_id si ajout                                                                */
/******************************************************************************************************************************/
 gint Modifier_camera_supDB ( struct CMD_TYPE_CAMERASUP *camera_sup )
  { return( Ajouter_Modifier_camera_supDB(camera_sup, FALSE) ); }
/******************************************************************************************************************************/
/* Recuperer_camera_sup_DB: Renvoi la liste des camera de supervision d'un synoptique                                         */
/* Entrée: un pointeur vers la nouvelle connexion base de données                                                             */
/* Sortie: FALSE si erreur                                                                                                    */
/**********************************************************************************************************/
 gboolean Recuperer_camera_supDB ( struct DB **db_retour, gint syn_id )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT camsup.id, camsup.syn_id, cam.libelle, cam.location, camsup.posx, camsup.posy"
                " FROM %s as camsup INNER JOIN %s as cam ON camsup.camera_src_id=cam.id"
                " WHERE syn_id=%d",
                NOM_TABLE_CAMERASUP, NOM_TABLE_CAMERA, syn_id
              );

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
/* Recuperer_camera_supDB_suite: Fonction itérative de récupération des camera                                                */
/* Entrée: un pointeur sur la connexion de baase de données                                                                   */
/* Sortie: une structure nouvellement allouée                                                                                 */
/******************************************************************************************************************************/
 struct CMD_TYPE_CAMERASUP *Recuperer_camera_supDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_CAMERASUP *camera;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    camera = (struct CMD_TYPE_CAMERASUP *)g_try_malloc0( sizeof(struct CMD_TYPE_CAMERASUP) );
    if (!camera) Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else                                                                                /* Recopie dans la nouvelle structure */
     { g_snprintf( camera->libelle,  sizeof(camera->libelle),  "%s", db->row[2] );
       g_snprintf( camera->location, sizeof(camera->location), "%s", db->row[3] );
       camera->id     = atoi(db->row[0]);
       camera->syn_id = atoi(db->row[1]);
       camera->posx   = atoi(db->row[4]);
       camera->posy   = atoi(db->row[5]);
     }
    return(camera);
  }
/******************************************************************************************************************************/
/* Rechercher_camera_supDB: Recupération du camera_sup dont l'id est en parametre                                             */
/* Entrée: un id de camera                                                                                                    */
/* Sortie: une structure referencant la camera                                                                                */
/******************************************************************************************************************************/
 struct CMD_TYPE_CAMERASUP *Rechercher_camera_supDB ( guint id )
  { struct CMD_TYPE_CAMERASUP *camera_sup;
    gchar requete[512];
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT camsup.id, camsup.syn_id, cam.libelle, cam.location, camsup.posx, camsup.posy"
                " FROM %s as camsup INNER JOIN %s as cam ON camsup.camera_src_id=cam.id"
                " WHERE camsup.id=%d",
                NOM_TABLE_CAMERASUP, NOM_TABLE_CAMERA, id );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%sB: DB connexion failed", __func__ );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    camera_sup = Recuperer_camera_supDB_suite( &db );
    if (camera_sup) Libere_DB_SQL ( &db );
    return(camera_sup);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
