/******************************************************************************************************************************/
/* Watchdogd/Camera.c        Déclaration des fonctions pour la gestion des cameras                                            */
/* Projet WatchDog version 3.0       Gestion d'habitat                                      dim. 13 sept. 2009 10:57:58 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Camera.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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
/* Retirer_cameraDB: Elimination d'un camera                                                                                  */
/* Entrée: un log et une database                                                                                             */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Retirer_cameraDB ( gint id )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_CAMERA, id );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter_Modifier_cameraDB: Ajout ou edition d'une camera                                                                   */
/* Entrée: La structure referencant la camera                                                                                 */
/* Sortie: -1 si probleme, 0 si modif OK, last_sql_id si ajout                                                                */
/******************************************************************************************************************************/
 static gint Ajouter_Modifier_cameraDB ( struct CMD_TYPE_CAMERA *camera, gint ajout )
  { gchar *location, *libelle;
    gchar requete[512];
    gboolean retour;
    struct DB *db;
    gint last_id;

    location = Normaliser_chaine ( camera->location );              /* Formatage correct des chaines */
    if (!location)
     { Info_new( Config.log, FALSE, LOG_WARNING, "%s: Normalisation location impossible", __func__ );
       return(-1);
     }

    libelle = Normaliser_chaine ( camera->libelle );              /* Formatage correct des chaines */
    if (!libelle)
     { g_free(location);
       Info_new( Config.log, FALSE, LOG_WARNING, "%s: Normalisation libelle impossible", __func__ );
       return(-1);
     }

    if (ajout == TRUE)
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "INSERT INTO %s (num,location,libelle) VALUES ('%d','%s','%s')",
                   NOM_TABLE_CAMERA, camera->num, location, libelle );
     } else
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE %s SET num='%d', location='%s',libelle='%s' "
                   "WHERE id=%d", NOM_TABLE_CAMERA, camera->num, location, libelle, camera->id );
     }
    g_free(libelle);
    g_free(location);
    

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
/* Ajouter_cameraDB: Ajout une camera dans la base de données                                                                 */
/* Entrée: La structure referencant la camera                                                                                 */
/* Sortie: -1 si probleme, 0 si modif OK, last_sql_id si ajout                                                                */
/******************************************************************************************************************************/
 gint Ajouter_cameraDB ( struct CMD_TYPE_CAMERA *camera )
  { return( Ajouter_Modifier_cameraDB(camera, TRUE) ); }
/******************************************************************************************************************************/
/* Modifier_cameraDB: Modifie une camera dans la base de données                                                              */
/* Entrée: La structure referencant la camera                                                                                 */
/* Sortie: -1 si probleme, 0 si modif OK, last_sql_id si ajout                                                                */
/******************************************************************************************************************************/
 gint Modifier_cameraDB ( struct CMD_TYPE_CAMERA *camera )
  { return( Ajouter_Modifier_cameraDB(camera, FALSE) ); }
/******************************************************************************************************************************/
/* Recuperer_cameraDB: Renvoi la liste des camera en base de données                                                          */
/* Entrée: un pointeur vers la nouvelle connexion base de données                                                             */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Recuperer_cameraDB ( struct DB **db_retour )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT id,num,location,libelle"
                " FROM %s ORDER BY libelle",
                NOM_TABLE_CAMERA                                                                  /* FROM */
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
/* Recuperer_cameraDB_suite: Fonction itérative de récupération des camera                                                    */
/* Entrée: un pointeur sur la connexion de base de données                                                                    */
/* Sortie: une structure nouvellement allouée                                                                                 */
/******************************************************************************************************************************/
 struct CMD_TYPE_CAMERA *Recuperer_cameraDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_CAMERA *camera;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    camera = (struct CMD_TYPE_CAMERA *)g_try_malloc0( sizeof(struct CMD_TYPE_CAMERA) );
    if (!camera) Info_new( Config.log, FALSE, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else                                                                                /* Recopie dans la nouvelle structure */
     { g_snprintf( camera->location, sizeof(camera->location), "%s", db->row[2] );
       g_snprintf( camera->libelle,  sizeof(camera->libelle),  "%s", db->row[3] );
       camera->id  = atoi(db->row[0]);
       camera->num = atoi(db->row[1]);
     }
    return(camera);
  }
/******************************************************************************************************************************/
/* Rechercher_cameraDB: Recupération de la camera dont l'id est en parametre                                                  */
/* Entrée: un id de camera                                                                                                    */
/* Sortie: une structure referencant la camera                                                                                */
/******************************************************************************************************************************/
 struct CMD_TYPE_CAMERA *Rechercher_cameraDB ( guint id )
  { struct CMD_TYPE_CAMERA *camera;
    gchar requete[200];
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT id,num,location,libelle"
                " FROM %s WHERE id=%d",
                NOM_TABLE_CAMERA,                                                                                     /* FROM */
                id                                                                                                   /* Where */
              );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    camera = Recuperer_cameraDB_suite( &db );
    if (camera) Libere_DB_SQL ( &db );
    return(camera);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
