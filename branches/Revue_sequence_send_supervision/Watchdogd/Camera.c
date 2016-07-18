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

/**********************************************************************************************************/
/* Retirer_cameraDB: Elimination d'un camera                                                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_cameraDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *camera )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_CAMERA, camera->id );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_cameraDB: Ajout ou edition d'un camera                                                         */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure camera                        */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_cameraDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *camera )
  { gchar *location, *objet, *libelle;
    gchar requete[2048];
    
    location = Normaliser_chaine ( camera->location );              /* Formatage correct des chaines */
    if (!location)
     { Info_new( Config.log, FALSE, LOG_WARNING, "Ajouter_cameraDB: Normalisation location impossible" );
       return(-1);
     }
    objet = Normaliser_chaine ( camera->objet );                    /* Formatage correct des chaines */
    if (!objet)
     { g_free(location);
       Info_new( Config.log, FALSE, LOG_WARNING, "Ajouter_cameraDB: Normalisation objet impossible" );
       return(-1);
     }
    libelle = Normaliser_chaine ( camera->libelle );              /* Formatage correct des chaines */
    if (!libelle)
     { g_free(location);
       g_free(objet);
       Info_new( Config.log, FALSE, LOG_WARNING, "Ajouter_cameraDB: Normalisation libelle impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s"
                "(num,type,bit,objet,location,libelle)"
                "VALUES (%d,%d,%d,'%s','%s','%s')",
                NOM_TABLE_CAMERA, camera->num, camera->type, camera->bit, objet, location, libelle
              );
    g_free(location);
    g_free(objet);
    g_free(libelle);

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL ( db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_cameraDB: Recupération de la liste des ids des cameras                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_cameraDB ( struct LOG *log, struct DB *db )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,num,type,bit,objet,location,libelle"
                " FROM %s ORDER BY objet, libelle",
                NOM_TABLE_CAMERA                                                                  /* FROM */
              );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_cameraDB: Recupération de la liste des ids des cameras                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_CAMERA *Recuperer_cameraDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_CAMERA *camera;

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       return(NULL);
     }

    camera = (struct CMD_TYPE_CAMERA *)g_try_malloc0( sizeof(struct CMD_TYPE_CAMERA) );
    if (!camera) Info_new( Config.log, FALSE, LOG_ERR, "Recuperer_cameraDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &camera->objet,    db->row[4], sizeof(camera->objet    ) );
       memcpy( &camera->location, db->row[5], sizeof(camera->location  ) );
       memcpy( &camera->libelle,  db->row[6], sizeof(camera->libelle ) );
       camera->id         = atoi(db->row[0]);
       camera->num        = atoi(db->row[1]);
       camera->type       = atoi(db->row[2]);
       camera->bit        = atoi(db->row[3]);
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
                "SELECT num,type,bit,objet,location,libelle"
                " FROM %s WHERE id=%d",
                NOM_TABLE_CAMERA,                                                                 /* FROM */
                id                                                                               /* Where */
              );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Info_new( Config.log, FALSE, LOG_NOTICE, "Rechercher_cameraDB: CAMERA %d not found", id );
       return(NULL);
     }

    camera = g_try_malloc0( sizeof(struct CMD_TYPE_CAMERA) );
    if (!camera)
     { Info_new( Config.log, FALSE, LOG_WARNING, "Rechercher_cameraDB: Mem error" ); }
    else
     { memcpy( &camera->objet,    db->row[3], sizeof(camera->objet    ) );
       memcpy( &camera->location, db->row[4], sizeof(camera->location  ) );
       memcpy( &camera->libelle,  db->row[5], sizeof(camera->libelle ) );
       camera->num        = atoi(db->row[0]);
       camera->type       = atoi(db->row[1]);
       camera->bit        = atoi(db->row[2]);
       camera->id         = id;
     }
    Liberer_resultat_SQL (db);
    return(camera);
  }
/**********************************************************************************************************/
/* Modifier_cameraDB: Modification d'un camera Watchdog                                                 */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_cameraDB( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *camera )
  { gchar *location, *libelle, *objet;
    gchar requete[1024];

    location = Normaliser_chaine ( camera->location );              /* Formatage correct des chaines */
    if (!location)
     { Info_new( Config.log, FALSE, LOG_WARNING, "Ajouter_cameraDB: Normalisation location impossible" );
       return(-1);
     }
    objet = Normaliser_chaine ( camera->objet );                    /* Formatage correct des chaines */
    if (!objet)
     { g_free(location);
       Info_new( Config.log, FALSE, LOG_WARNING, "Ajouter_cameraDB: Normalisation objet impossible" );
       return(-1);
     }
    libelle = Normaliser_chaine ( camera->libelle );              /* Formatage correct des chaines */
    if (!libelle)
     { g_free(location);
       g_free(objet);
       Info_new( Config.log, FALSE, LOG_WARNING, "Ajouter_cameraDB: Normalisation libelle impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "location='%s',objet='%s',libelle='%s',type=%d,bit=%d,num=%d "
                "WHERE id=%d",
                NOM_TABLE_CAMERA, location, objet, libelle, camera->type,
                camera->bit, camera->num,
                camera->id
              );
    g_free(location);
    g_free(objet);
    g_free(libelle);
    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
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
                "SELECT id FROM %s",
                NOM_TABLE_CAMERA_MOTION                                                                  /* FROM */
              );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       return(NULL);
     }

    camera = Rechercher_cameraDB( log, db, atoi(db->row[0]) );
    if (!camera)
     { Info_new( Config.log, FALSE, LOG_WARNING, "Rechercher_cameraDB_motion: Mem error" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id = %d",
                NOM_TABLE_CAMERA_MOTION, camera->id );
    Lancer_requete_SQL ( db, requete );

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
          { Info_new( Config.log, FALSE, LOG_INFO, "Camera_check_motion: Mise a un du bit M%03d", camera->bit );
            Envoyer_commande_dls ( camera->bit ); 
            g_free(camera);
          }
       }
    while (camera);
  }
/*--------------------------------------------------------------------------------------------------------*/
