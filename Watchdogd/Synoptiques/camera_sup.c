/**********************************************************************************************************/
/* Watchdogd/Db/Synoptiques/camera_sup.c       Ajout/retrait de module camera_sup dans les synoptiques    */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 29 jan 2006 15:09:58 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * camera_sup.c
 * This file is part of <program name>
 *
 * Copyright (C) 2009 - sebastien
 *
 * <program name> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <program name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <program name>; if not, write to the Free Software
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

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_camera_supDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA_SUP *camera_sup )
  { gchar requete[512];
    gchar *libelle;

    libelle = Normaliser_chaine ( log, camera_sup->libelle );               /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_camera_supDB: Normalisation impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(syn_id,libelle,camera_src_id,posx,posy,largeur,hauteur,angle)"
                " VALUES (%d,'%s',%d,%d,%d,%d,%d,%f)", NOM_TABLE_CAMERASUP,
                camera_sup->syn_id, libelle, camera_sup->camera_src_id,
                camera_sup->position_x, camera_sup->position_y,
                camera_sup->largeur, camera_sup->hauteur,
                camera_sup->angle );
    g_free(libelle);

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_camera_supDB ( struct LOG *log, struct DB *db, gint id_syn )
  { gchar requete[2048];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,syn_id,libelle,camera_src_id,location,posx,posy,largeur,hauteur,angle"
                " FROM %s,%s WHERE syn_id=%d AND camera_src_id=%s.id",
                NOM_TABLE_CAMERASUP, NOM_TABLE_CAMERASUP, NOM_TABLE_CAMERA, id_syn, NOM_TABLE_CAMERA );
    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CAMERASUPDB *Recuperer_camera_supDB_suite( struct LOG *log, struct DB *db )
  { struct CAMERASUPDB *camera_sup;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    camera_sup = (struct CAMERASUPDB *)g_malloc0( sizeof(struct CAMERASUPDB) );
    if (!camera_sup) Info( log, DEBUG_MEM, "Recuperer_camera_supDB_suite: Erreur allocation mémoire" );
    else
     { camera_sup->id           = atoi(db->row[0]);
       camera_sup->syn_id       = atoi(db->row[1]);                   /* Synoptique ou est placée le camera_sup */
       camera_sup->camera_src_id= atoi(db->row[3]);
       camera_sup->position_x   = atoi(db->row[5]);                             /* en abscisses et ordonnées */
       camera_sup->position_y   = atoi(db->row[6]);
       camera_sup->largeur      = atof(db->row[7]);
       camera_sup->hauteur      = atof(db->row[8]);
       camera_sup->angle        = atof(db->row[9]);
       memcpy( camera_sup->libelle, db->row[2], sizeof(camera_sup->libelle) );  /* Recopie dans la structure */
       memcpy( camera_sup->location, db->row[4], sizeof(camera_sup->location) );/* Recopie dans la structure */
     }
    return(camera_sup);
  }
/**********************************************************************************************************/
/* Rechercher_camera_supDB: Recupération du camera_sup dont l'id est en parametre                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CAMERASUPDB *Rechercher_camera_supDB ( struct LOG *log, struct DB *db, guint id )
  { struct CAMERASUPDB *camera_sup;
    gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,syn_id,libelle,camera_src_id,location,posx,posy,largeur,hauteur,angle"
                " FROM %s,%s WHERE id=%d AND camera_src_id=%s.num",
                NOM_TABLE_CAMERASUP, NOM_TABLE_CAMERASUP, NOM_TABLE_CAMERA, id, NOM_TABLE_CAMERA );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_camera_supDB: Capteur non trouvé dans la BDD", id );
       return(NULL);
     }

    camera_sup = (struct CAMERASUPDB *)g_malloc0( sizeof(struct CAMERASUPDB) );
    if (!camera_sup) Info( log, DEBUG_MEM, "Recuperer_camera_supDB: Erreur allocation mémoire" );
    else
     { camera_sup->id           = id;
       camera_sup->syn_id       = atoi(db->row[0]);                   /* Synoptique ou est placée le camera_sup */
       camera_sup->camera_src_id= atoi(db->row[2]);
       camera_sup->position_x   = atoi(db->row[4]);                             /* en abscisses et ordonnées */
       camera_sup->position_y   = atoi(db->row[5]);
       camera_sup->largeur      = atof(db->row[6]);
       camera_sup->hauteur      = atof(db->row[7]);
       camera_sup->angle        = atof(db->row[8]);
       memcpy( camera_sup->libelle, db->row[1], sizeof(camera_sup->libelle) );  /* Recopie dans la structure */
       memcpy( camera_sup->location, db->row[3], sizeof(camera_sup->location) );/* Recopie dans la structure */
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
    gchar *libelle;

    libelle = Normaliser_chaine ( log, camera_sup->libelle );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Modifier_camera_supDB: Normalisation impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s',posx=%d,posy=%d,largeur=%d,hauteur=%d,angle=%f"
                " WHERE id=%d;", NOM_TABLE_CAMERASUP,
                libelle, camera_sup->position_x, camera_sup->position_y,
                camera_sup->largeur, camera_sup->hauteur, camera_sup->angle,
                camera_sup->id );
    g_free(libelle);

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
