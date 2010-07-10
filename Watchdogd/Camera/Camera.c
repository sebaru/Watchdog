/**********************************************************************************************************/
/* Watchdogd/Camera/Camera.c        Déclaration des fonctions pour la gestion des camera                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                  dim. 13 sept. 2009 10:57:58 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Camera.c
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
 #include "Camera_DB.h"

/**********************************************************************************************************/
/* Recuperer_liste_id_cameraDB: Recupération de la liste des ids des cameras                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_cameraDB ( struct LOG *log, struct DB *db )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT location,%s.type,%s.num,libelle,objet,id_mnemo,bit"
                " FROM %s,%s WHERE id_mnemo=%s.id ORDER BY libelle",
                NOM_TABLE_CAMERA, NOM_TABLE_MNEMO,
                NOM_TABLE_CAMERA, NOM_TABLE_MNEMO,                                                /* FROM */
                NOM_TABLE_MNEMO                                                                  /* Where */
              );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_cameraDB: Recupération de la liste des ids des cameras                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_CAMERA *Recuperer_cameraDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_CAMERA *camera;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    camera = (struct CMD_TYPE_CAMERA *)g_malloc0( sizeof(struct CMD_TYPE_CAMERA) );
    if (!camera) Info( log, DEBUG_MEM, "Recuperer_cameraDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( camera->location, db->row[0], sizeof(camera->location  ) );
       memcpy( camera->libelle,  db->row[3], sizeof(camera->libelle ) );
       memcpy( camera->objet,    db->row[4], sizeof(camera->objet    ) );
       camera->id_mnemo   = atoi(db->row[5]);
       camera->num        = atoi(db->row[2]);
       camera->type       = atoi(db->row[1]);
       camera->bit        = atoi(db->row[6]);
     }
    return(camera);
  }
/**********************************************************************************************************/
/* Rechercher_cameraDB: Recupération du camera dont le num est en parametre                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_CAMERA *Rechercher_cameraDB ( struct LOG *log, struct DB *db, guint id )
  { gchar requete[200];
    struct CMD_TYPE_CAMERA *camera;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT location,%s.type,libelle,objet,num,bit"
                " FROM %s,%s WHERE %s.id=%s.id_mnemo AND %s.id_mnemo=%d",
                NOM_TABLE_CAMERA,
                NOM_TABLE_CAMERA, NOM_TABLE_MNEMO,                                                /* FROM */
                NOM_TABLE_MNEMO, NOM_TABLE_CAMERA,                                               /* Where */
                NOM_TABLE_CAMERA, id
              );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_cameraDB: CAMERA non trouvé dans la BDD", id );
       return(NULL);
     }

    camera = g_malloc0( sizeof(struct CMD_TYPE_CAMERA) );
    if (!camera)
     { Info( log, DEBUG_MEM, "Rechercher_cameraDB: Mem error" ); }
    else
     { memcpy( camera->libelle,  db->row[2], sizeof(camera->libelle  ) );    /* Recopie dans la structure */
       memcpy( camera->location, db->row[0], sizeof(camera->location ) );
       memcpy( camera->objet,    db->row[3], sizeof(camera->objet    ) );
       camera->type     = atoi(db->row[1]);
       camera->num      = atoi(db->row[4]);
       camera->bit      = atoi(db->row[5]);
       camera->id_mnemo = id;
     }
    Liberer_resultat_SQL ( log, db );
    return(camera);
  }
/**********************************************************************************************************/
/* Modifier_cameraDB: Modification d'un camera Watchdog                                                 */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_cameraDB( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *camera )
  { gchar requete[1024];
    gchar *location;

    location = Normaliser_chaine ( log, camera->location );              /* Formatage correct des chaines */
    if (!location)
     { Info( log, DEBUG_DB, "Modifier_cameraDB: Normalisation impossible" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "location='%s',type=%d,bit=%d "
                "WHERE id_mnemo=%d",
                NOM_TABLE_CAMERA, location, camera->type, camera->bit, camera->id_mnemo );
    g_free(location);

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Rechercher_cameraDB: Recupération du camera dont le num est en parametre                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct CMD_TYPE_CAMERA *Rechercher_cameraDB_motion ( struct LOG *log, struct DB *db )
  { gchar requete[256];
    struct CMD_TYPE_CAMERA *camera;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT location,%s.type,libelle,objet,num,bit,%s.id"
                " FROM %s,%s,%s WHERE %s.id=%s.id_mnemo AND %s.id_mnemo=%s.id_mnemo",
                NOM_TABLE_CAMERA, NOM_TABLE_MNEMO,
                NOM_TABLE_CAMERA, NOM_TABLE_MNEMO, NOM_TABLE_CAMERA_MOTION,                       /* FROM */
                NOM_TABLE_MNEMO, NOM_TABLE_CAMERA,                                               /* Where */
                NOM_TABLE_CAMERA, NOM_TABLE_CAMERA_MOTION
              );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    camera = g_malloc0( sizeof(struct CMD_TYPE_CAMERA) );
    if (!camera)
     { Info( log, DEBUG_MEM, "Rechercher_cameraDB: Mem error" );
       return(NULL);
     }
    else
     { memcpy( camera->libelle,  db->row[2], sizeof(camera->libelle  ) );    /* Recopie dans la structure */
       memcpy( camera->location, db->row[0], sizeof(camera->location ) );
       memcpy( camera->objet,    db->row[3], sizeof(camera->objet    ) );
       camera->type     = atoi(db->row[1]);
       camera->num      = atoi(db->row[4]);
       camera->bit      = atoi(db->row[5]);
       camera->id_mnemo = atoi(db->row[6]);
       Liberer_resultat_SQL ( log, db );
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id_mnemo = %d",
                NOM_TABLE_CAMERA_MOTION, camera->id_mnemo );

    Lancer_requete_SQL ( log, db, requete );
    return(camera);
  }
/**********************************************************************************************************/
/* Camera_check_motion : Vérifie si l'outil motion a donner un bit a activer                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: néant. Les bits DLS sont positionnés                                                           */
/**********************************************************************************************************/
 void Camera_check_motion ( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_CAMERA *camera;

    do { camera = Rechercher_cameraDB_motion ( log, db );
         if (camera)
          { Info_n( log, DEBUG_INFO, "Camera_check_motion: Mise a un du bit M", camera->bit );
            g_free(camera);
          }
       }
    while (camera);
  }
/*--------------------------------------------------------------------------------------------------------*/
