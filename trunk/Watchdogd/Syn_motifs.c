/******************************************************************************************************************************/
/* Watchdogd/Synoptiques/motifs.c       Ajout/retrait de motifs dans les motifs                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          mer 05 mai 2004 12:11:21 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * motifs.c
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
/* Synoptique_auto_create_VISUEL: Création automatique d'un visuel depuis la compilation DLS                                  */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Synoptique_auto_create_VISUEL ( struct DLS_PLUGIN *plugin, gchar *acronyme, gchar *libelle_src, gchar *forme_src )
  { gchar *acro, *libelle, *forme;
    gchar requete[1024];
    gboolean retour;
    struct DB *db;

/******************************************** Préparation de la base du mnemo *************************************************/
    acro       = Normaliser_chaine ( acronyme );                                             /* Formatage correct des chaines */
    if ( !acro )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation acro impossible. Mnemo NOT added nor modified.", __func__ );
       return(FALSE);
     }

    libelle    = Normaliser_chaine ( libelle_src );                                          /* Formatage correct des chaines */
    if ( !libelle )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation libelle impossible. Mnemo NOT added nor modified.", __func__ );
       g_free(acro);
       return(FALSE);
     }

    forme      = Normaliser_chaine ( forme_src );                                            /* Formatage correct des chaines */
    if ( !forme )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation forme impossible. Mnemo NOT added nor modified.", __func__ );
       g_free(acro);
       g_free(libelle);
       return(FALSE);
     }

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       g_free(forme);
       g_free(libelle);
       g_free(acro);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT IGNORE INTO %s SET "
                "auto_create=1, syn_id=%d, tech_id='%s', acronyme='%s', forme='%s', icone='-1', libelle='%s', access_level=0, "
                "posx='150', posy='150', larg='-1', haut='-1', angle='0', auto_create=1 "
                "ON DUPLICATE KEY UPDATE forme=VALUES(forme), libelle=VALUES(libelle)",
                NOM_TABLE_MOTIF, plugin->syn_id, plugin->tech_id, acro, forme, libelle );
    Lancer_requete_SQL ( db, requete );                                                        /* Execution de la requete SQL */

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE %s SET forme='%s', libelle='%s' "
                "WHERE tech_id='%s' AND acronyme='%s';",
                NOM_TABLE_MOTIF, forme, libelle, plugin->tech_id, acro );
    Lancer_requete_SQL ( db, requete );                                                        /* Execution de la requete SQL */

    g_free(forme);
    g_free(libelle);
    g_free(acro);

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return (retour);
  }
/******************************************************************************************************************************/
/* Retirer_motifDB: Elimination d'un motif                                                                                    */
/* Entrée: une structure referencant le motif a supprimer                                                                     */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Retirer_motifDB ( struct CMD_TYPE_MOTIF *motif )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MOTIF, motif->id );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter_motifDB: Ajout d'un motif en base de données                                                                       */
/* Entrée: une structure referencant le motif a supprimer                                                                     */
/* Sortie: last_sql_id ou -1 si erreur                                                                                        */
/******************************************************************************************************************************/
 gint Ajouter_motifDB ( struct CMD_TYPE_MOTIF *motif )
  { gchar requete[1024];
    gchar *libelle, *clic_tech_id, *clic_acro;
    gboolean retour;
    struct DB *db;
    gint id;

    libelle = Normaliser_chaine ( motif->libelle );                                          /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       return(-1);
     }

    clic_tech_id = Normaliser_chaine ( motif->clic_tech_id );                                     /* Formatage correct des chaines */
    if (!clic_tech_id)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       g_free(libelle);
       return(-1);
     }

    clic_acro = Normaliser_chaine ( motif->clic_acronyme );                                       /* Formatage correct des chaines */
    if (!clic_acro)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       g_free(libelle);
       g_free(clic_tech_id);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s SET icone='%d',syn_id='%d',libelle='%s',access_level='%d',tech_id='%s',acronyme='%s',"
                "posx='%d',posy='%d',larg='%d',haut='%d',angle='%d',"
                "dialog='%d',gestion='%d',def_color='%s',rafraich='%d',layer='%d',"
                "clic_tech_id='%s',clic_acronyme='%s'",
                NOM_TABLE_MOTIF,
                motif->icone_id, motif->syn_id, libelle, motif->access_level,
                Normaliser_as_ascii ( motif->tech_id ), Normaliser_as_ascii ( motif->acronyme ),
                motif->position_x, motif->position_y, motif->largeur, motif->hauteur, motif->angle,
                motif->type_dialog, motif->type_gestion,
                Normaliser_as_ascii(motif->def_color), motif->rafraich,
                motif->layer, clic_tech_id, clic_acro );
    g_free(libelle);
    g_free(clic_tech_id);
    g_free(clic_acro);

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
/* Recuperer_motifDB: Recupération de la liste des motifs d'un synoptique                                                     */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 gboolean Recuperer_motifDB ( struct DB **db_retour, gint id_syn )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT sm.id,sm.libelle,icone,syn_id,access_level,tech_id,acronyme,posx,posy,larg,haut,angle,"
                "dialog,gestion,def_color,rafraich,layer,"
                "sm.clic_tech_id, sm.clic_acronyme"
                " FROM syns_motifs AS sm"
                " WHERE syn_id='%d' AND auto_create IS NULL ORDER BY layer", id_syn );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_motifDB_suite : Contination de la recupération de la liste des motifs d'un synoptique                            */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 struct CMD_TYPE_MOTIF *Recuperer_motifDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_MOTIF *motif;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    motif = (struct CMD_TYPE_MOTIF *)g_try_malloc0( sizeof(struct CMD_TYPE_MOTIF) );
    if (!motif) Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else
     { motif->id           = atoi(db->row[0]);
       g_snprintf ( motif->libelle, sizeof(motif->libelle), "%s", db->row[1] );                  /* Recopie dans la structure */
       motif->icone_id     = atoi(db->row[2]);                                                  /* Correspond au fichier .gif */
       motif->syn_id       = atoi(db->row[3]);
       motif->access_level = atoi(db->row[4]);                                       /* Nom du groupe d'appartenance du motif */
       g_snprintf ( motif->tech_id, sizeof(motif->tech_id), "%s", db->row[5] );                  /* Recopie dans la structure */
       g_snprintf ( motif->acronyme, sizeof(motif->acronyme), "%s", db->row[6] );                /* Recopie dans la structure */
       motif->position_x   = atoi(db->row[7]);                                                   /* en abscisses et ordonnées */
       motif->position_y   = atoi(db->row[8]);
       motif->largeur      = atoi(db->row[9]);                                         /* Taille de l'image sur le synoptique */
       motif->hauteur      = atoi(db->row[10]);
       motif->angle        = atoi(db->row[11]);
       motif->type_dialog  = atoi(db->row[12]);                      /* Type de la boite de dialogue pour le clic de commande */
       motif->type_gestion = atoi(db->row[13]);
       g_snprintf ( motif->def_color, sizeof(motif->def_color), "%s", db->row[14] );                /* Recopie dans la structure */
       motif->rafraich     = atoi(db->row[15]);
       motif->layer        = atoi(db->row[16]);
       g_snprintf ( motif->clic_tech_id, sizeof(motif->clic_tech_id), "%s", db->row[17] );       /* Recopie dans la structure */
       g_snprintf ( motif->clic_acronyme, sizeof(motif->clic_acronyme), "%s", db->row[18] );     /* Recopie dans la structure */
     }
    return(motif);
  }
/******************************************************************************************************************************/
/* Rechercher_motifDB: Recupération du motif dont l'id est en parametre                                                       */
/* Entrée: un id de motif a rechercher                                                                                        */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 struct CMD_TYPE_MOTIF *Rechercher_motifDB ( guint id )
  { gchar requete[1024];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT sm.id,sm.libelle,icone,syn_id,access_level,tech_id,acronyme,posx,posy,larg,haut,angle,"
                "dialog,gestion,def_color,rafraich,layer,"
                "sm.clic_tech_id, sm.clic_acronyme"
                " FROM syns_motifs AS sm"
                " WHERE sm.id=%d", id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    struct CMD_TYPE_MOTIF *motif = Recuperer_motifDB_suite ( &db );
    Libere_DB_SQL( &db );
    return(motif);
  }
/******************************************************************************************************************************/
/* Modifier_motifDB: Modification d'un motif Watchdog                                                                         */
/* Entrées: une structure motif referancant les modifications a appliquer.                                                    */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Modifier_motifDB( struct CMD_TYPE_MOTIF *motif )
  { gchar requete[1024];
    gchar *libelle, *clic_tech_id, *clic_acro;
    gboolean retour;
    struct DB *db;

    libelle = Normaliser_chaine ( motif->libelle );                                          /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       return(FALSE);
     }

    clic_tech_id = Normaliser_chaine ( motif->clic_tech_id );                                     /* Formatage correct des chaines */
    if (!clic_tech_id)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       g_free(libelle);
       return(FALSE);
     }

    clic_acro = Normaliser_chaine ( motif->clic_acronyme );                                       /* Formatage correct des chaines */
    if (!clic_tech_id)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       g_free(libelle);
       g_free(clic_tech_id);
       return(FALSE);
     }


    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE %s SET "
                "libelle='%s',access_level='%d',tech_id='%s',acronyme='%s',posx='%d',posy='%d',larg='%d',"
                "haut='%d',angle='%d',dialog='%d',gestion='%d',def_color='%s',"
                "rafraich='%d',layer='%d',clic_tech_id='%s',clic_acronyme='%s'"
                " WHERE id=%d;", NOM_TABLE_MOTIF,
                libelle, motif->access_level,
                Normaliser_as_ascii ( motif->tech_id ), Normaliser_as_ascii ( motif->acronyme ),
                motif->position_x, motif->position_y, motif->largeur, motif->hauteur, motif->angle,
                motif->type_dialog, motif->type_gestion,
                motif->def_color, motif->rafraich,
                motif->layer, clic_tech_id, clic_acro,
                motif->id );
    g_free(libelle);
    g_free(clic_tech_id);
    g_free(clic_acro);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }
    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Dls_VISUEL_to_json : Formate un bit au format JSON                                                                         */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_VISUEL_to_json ( JsonBuilder *builder, struct DLS_VISUEL *bit )
  { Json_add_string ( builder, "tech_id",   bit->tech_id );
    Json_add_string ( builder, "acronyme",  bit->acronyme );
    Json_add_int    ( builder, "mode",      bit->mode  );
    Json_add_string ( builder, "color",     bit->color );
    Json_add_bool   ( builder, "cligno",    bit->cligno );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
