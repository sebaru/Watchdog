/**********************************************************************************************************/
/* Watchdogd/Synoptiques/capteur.c       Ajout/retrait de module capteur dans les synoptiques             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 29 jan 2006 15:09:58 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * capteur.c
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
/* Retirer_capteurDB: Elimination d'un capteur                                                            */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_capteurDB ( struct CMD_TYPE_CAPTEUR *capteur )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_capteurDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_CAPTEUR, capteur->id );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_capteurDB ( struct CMD_TYPE_CAPTEUR *capteur )
  { gchar requete[512];
    gboolean retour;
    gchar *libelle;
    struct DB *db;
    gint id;

    libelle = Normaliser_chaine ( capteur->libelle );               /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_capteurDB: Normalisation impossible" );
       return(-1);
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_capteurDB: DB connexion failed" );
       g_free(libelle);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(syn_id,type,bitctrl,posx,posy,libelle,angle)"
                " VALUES (%d,%d,%d,%d,%d,'%s','%f')", NOM_TABLE_CAPTEUR,
                capteur->syn_id, capteur->type, capteur->bit_controle,
                capteur->position_x, capteur->position_y, libelle, capteur->angle );
    g_free(libelle);

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if ( retour == FALSE )
     { Libere_DB_SQL(&db); 
       return(-1);
     }
    id = Recuperer_last_ID_SQL ( db );
    Libere_DB_SQL(&db);
    return(id);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_capteurDB ( struct DB **db_retour, gint id_syn )
  { gchar requete[2048];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_plugins_dlsDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,syn_id,type,bitctrl,libelle,posx,posy,angle"
                " FROM %s WHERE syn_id=%d",
                NOM_TABLE_CAPTEUR, id_syn );
    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_CAPTEUR *Recuperer_capteurDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_CAPTEUR *capteur;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    capteur = (struct CMD_TYPE_CAPTEUR *)g_try_malloc0( sizeof(struct CMD_TYPE_CAPTEUR) );
    if (!capteur) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                           "Recuperer_capteurDB_suite: memory error" );
    else
     { capteur->id           = atoi(db->row[0]);
       capteur->syn_id       = atoi(db->row[1]);                   /* Synoptique ou est placée le capteur */
       capteur->type         = atoi(db->row[2]);
       capteur->bit_controle = atoi(db->row[3]);                                            /* Ixxx, Cxxx */
       capteur->position_x   = atoi(db->row[5]);                             /* en abscisses et ordonnées */
       capteur->position_y   = atoi(db->row[6]);
       capteur->angle        = atof(db->row[7]);
       memcpy( &capteur->libelle, db->row[4], sizeof(capteur->libelle) );    /* Recopie dans la structure */
     }
    return(capteur);
  }
/**********************************************************************************************************/
/* Rechercher_capteurDB: Recupération du capteur dont l'id est en parametre                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_CAPTEUR *Rechercher_capteurDB ( guint id )
  { struct CMD_TYPE_CAPTEUR *capteur;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_capteurDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT syn_id,bitctrl,type,libelle,posx,posy,angle "
                "FROM %s WHERE id=%d", 
                NOM_TABLE_CAPTEUR, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Config.log_dls, LOG_INFO, "Rechercher_capteurDB: Capteur %03d not found in DB", id );
       return(NULL);
     }

    capteur = (struct CMD_TYPE_CAPTEUR *)g_try_malloc0( sizeof(struct CMD_TYPE_CAPTEUR) );
    if (!capteur) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_capteurDB: Erreur allocation mémoire" );
    else
     { capteur->id           = id;
       capteur->syn_id       = atoi(db->row[0]);                   /* Synoptique ou est placée le capteur */
       capteur->type         = atoi(db->row[2]);
       capteur->bit_controle = atoi(db->row[1]);                                            /* Ixxx, Cxxx */
       capteur->position_x   = atoi(db->row[4]);                             /* en abscisses et ordonnées */
       capteur->position_y   = atoi(db->row[5]);
       capteur->angle        = atof(db->row[6]);
       memcpy( &capteur->libelle, db->row[3], sizeof(capteur->libelle) );    /* Recopie dans la structure */
     }
    Libere_DB_SQL( &db );
    return(capteur);
  }
/**********************************************************************************************************/
/* Modifier_capteurDB: Modification d'un capteur Watchdog                                                 */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_capteurDB( struct CMD_TYPE_CAPTEUR *capteur )
  { gchar requete[1024];
    gboolean retour;
    gchar *libelle;
    struct DB *db;

    libelle = Normaliser_chaine ( capteur->libelle );               /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_capteurDB: Normalisation impossible" );
       return(-1);
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_capteurDB: DB connexion failed" );
       g_free(libelle);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "type=%d,bitctrl=%d,libelle='%s',posx=%d,posy=%d,angle='%f'"
                " WHERE id=%d;", NOM_TABLE_CAPTEUR,
                capteur->type, capteur->bit_controle, libelle,
                capteur->position_x, capteur->position_y, capteur->angle,
                capteur->id );
    g_free(libelle);

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/*--------------------------------------------------------------------------------------------------------*/
