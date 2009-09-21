/**********************************************************************************************************/
/* Watchdogd/Camera/Camera.c        Déclaration des fonctions pour la gestion des camera                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                  dim. 13 sept. 2009 10:57:58 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Camera.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - 
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
/* Retirer_cameraDB: Elimination d'une camera                                                             */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_cameraDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *camera )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_CAMERA, camera->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_cameraDB: Ajout ou edition d'un camera                                                         */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure camera                        */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_cameraDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA *camera )
  { gchar requete[200];
    gchar *location;

    location = Normaliser_chaine ( log, camera->location );              /* Formatage correct des chaines */
    if (!location)
     { Info( log, DEBUG_DB, "Ajouter_cameraDB: Normalisation impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(location,type,num) VALUES "
                "('%s',%d,%d)", NOM_TABLE_CAMERA, location, camera->type, camera->num );
    g_free(location);

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_cameraDB: Recupération de la liste des ids des cameras                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_cameraDB ( struct LOG *log, struct DB *db )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,location,%s.type,%s.num,libelle"
                " FROM %s,%s WHERE %s.num=%s.num AND %s.type=%d ORDER BY libelle",
                NOM_TABLE_CAMERA, NOM_TABLE_CAMERA,
                NOM_TABLE_CAMERA, NOM_TABLE_MNEMO,                                                /* FROM */
                NOM_TABLE_CAMERA, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, MNEMO_CAMERA                 /* Where */
              );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_cameraDB: Recupération de la liste des ids des cameras                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CAMERADB *Recuperer_cameraDB_suite( struct LOG *log, struct DB *db )
  { struct CAMERADB *camera;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    camera = (struct CAMERADB *)g_malloc0( sizeof(struct CAMERADB) );
    if (!camera) Info( log, DEBUG_MEM, "Recuperer_cameraDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( camera->location, db->row[1], sizeof(camera->location  ) );
       memcpy( camera->libelle, db->row[4], sizeof(camera->libelle ) );
       camera->id          = atoi(db->row[0]);
       camera->type        = atoi(db->row[2]);
       camera->num         = atoi(db->row[3]);
     }
    return(camera);
  }
/**********************************************************************************************************/
/* Rechercher_cameraDB: Recupération du camera dont le num est en parametre                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CAMERADB *Rechercher_cameraDB ( struct LOG *log, struct DB *db, guint id )
  { gchar requete[200];
    struct CAMERADB *camera;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT location,%s.type,%s.num,libelle"
                " FROM %s,%s WHERE %s.num=%s.num AND %s.type=%d AND %s.id=%d",
                NOM_TABLE_CAMERA, NOM_TABLE_CAMERA,
                NOM_TABLE_CAMERA, NOM_TABLE_MNEMO,                                                /* FROM */
                NOM_TABLE_CAMERA, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, MNEMO_CAMERA,                /* Where */
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

    camera = g_malloc0( sizeof(struct CAMERADB) );
    if (!camera)
     { Info( log, DEBUG_MEM, "Rechercher_cameraDB: Mem error" ); }
    else
     { memcpy( camera->libelle,  db->row[3], sizeof(camera->libelle) );       /* Recopie dans la structure */
       memcpy( camera->location, db->row[0], sizeof(camera->location  ) );
       camera->type = atoi(db->row[1]);
       camera->num  = atoi(db->row[2]);
       camera->id   = id;
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
                "location='%s',type=%d,num=%d "
                "WHERE id=%d",
                NOM_TABLE_CAMERA, location, camera->type, camera->num, camera->id );
    g_free(location);

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
