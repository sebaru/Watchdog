/**********************************************************************************************************/
/* Watchdogd/Synoptiques/camera_sup.c       Ajout/retrait de module camera_sup dans les synoptiques       */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 29 jan 2006 15:09:58 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * camera_sup.c
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
 #include "Erreur.h"
 #include "Synoptiques_DB.h"

/**********************************************************************************************************/
/* Retirer_camera_supDB: Elimination d'un camera_sup                                                      */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_camera_supDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA_SUP *camera_sup )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_CAMERASUP, camera_sup->id );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_camera_supDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA_SUP *camera_sup )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(syn_id,camera_src_id,posx,posy)"
                " VALUES (%d,%d,%d,%d)", NOM_TABLE_CAMERASUP,
                camera_sup->syn_id, camera_sup->camera_src_id,
                camera_sup->position_x, camera_sup->position_y );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL ( db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_camera_supDB ( struct LOG *log, struct DB *db, gint id_syn )
  { gchar requete[2048];
    
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,camera_src_id,syn_id,posx,posy,type,num,bit,objet,location,libelle"
                " FROM %s,%s WHERE syn_id=%d AND camera_src_id=%s.id",
                NOM_TABLE_CAMERASUP,
                NOM_TABLE_CAMERASUP, NOM_TABLE_CAMERA,                                            /* From */
                id_syn, NOM_TABLE_CAMERA                                                         /* Where */
              );
    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_CAMERA_SUP *Recuperer_camera_supDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_CAMERA_SUP *camera_sup;

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       return(NULL);
     }

    camera_sup = (struct CMD_TYPE_CAMERA_SUP *)g_try_malloc0( sizeof(struct CMD_TYPE_CAMERA_SUP) );
    if (!camera_sup) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_camera_supDB_suite: Erreur allocation mémoire" );
    else
     { camera_sup->id           = atoi(db->row[0]);
       camera_sup->camera_src_id= atoi(db->row[1]);
       camera_sup->syn_id       = atoi(db->row[2]);                   /* Synoptique ou est placée le camera_sup */
       camera_sup->position_x   = atoi(db->row[3]);                             /* en abscisses et ordonnées */
       camera_sup->position_y   = atoi(db->row[4]);
       camera_sup->type         = atoi(db->row[5]);
       camera_sup->num          = atoi(db->row[6]);
       camera_sup->bit          = atoi(db->row[7]);
       memcpy( &camera_sup->objet,    db->row[8],  sizeof(camera_sup->objet) );
       memcpy( &camera_sup->location, db->row[9],  sizeof(camera_sup->location) );
       memcpy( &camera_sup->libelle,  db->row[10], sizeof(camera_sup->libelle) );
     }
    return(camera_sup);
  }
/**********************************************************************************************************/
/* Rechercher_camera_supDB: Recupération du camera_sup dont l'id est en parametre                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_CAMERA_SUP *Rechercher_camera_supDB ( struct LOG *log, struct DB *db, guint id )
  { struct CMD_TYPE_CAMERA_SUP *camera_sup;
    gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT camera_src_id,syn_id,posx,posy,type,num,bit,objet,location,libelle"
                " FROM %s,%s WHERE %s.id=%d AND camera_src_id=%s.id",
                NOM_TABLE_CAMERASUP, NOM_TABLE_CAMERA,                                            /* FROM */
                NOM_TABLE_CAMERASUP, id, NOM_TABLE_CAMERA );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(NULL);
     }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "Rechercher_camera_supDB: Camera %d not found in DB", id );
       return(NULL);
     }

    camera_sup = (struct CMD_TYPE_CAMERA_SUP *)g_try_malloc0( sizeof(struct CMD_TYPE_CAMERA_SUP) );
    if (!camera_sup) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_camera_supDB: Erreur allocation mémoire" );
    else
     { camera_sup->id           = id;
       camera_sup->camera_src_id= atoi(db->row[0]);
       camera_sup->syn_id       = atoi(db->row[1]);             /* Synoptique ou est placée le camera_sup */
       camera_sup->position_x   = atoi(db->row[2]);                          /* en abscisses et ordonnées */
       camera_sup->position_y   = atoi(db->row[3]);
       camera_sup->type         = atoi(db->row[4]);
       camera_sup->num          = atoi(db->row[5]);
       camera_sup->bit          = atoi(db->row[6]);
       memcpy( &camera_sup->objet,    db->row[7], sizeof(camera_sup->objet) );
       memcpy( &camera_sup->location, db->row[8], sizeof(camera_sup->location) );
       memcpy( &camera_sup->libelle,  db->row[9], sizeof(camera_sup->libelle) );
     }
    return(camera_sup);
  }
/**********************************************************************************************************/
/* Modifier_camera_supDB: Modification d'un camera_sup Watchdog                                           */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_camera_supDB( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA_SUP *camera_sup )
  { gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "posx=%d,posy=%d"
                " WHERE id=%d;", NOM_TABLE_CAMERASUP,
                camera_sup->position_x, camera_sup->position_y,
                camera_sup->id );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
