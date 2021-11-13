/**********************************************************************************************************/
/* Watchdogd/Synoptiques/palette.c       Ajout/retrait de palette dans les synoptiques                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                       jeu 03 fév 2005 14:23:39 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * palette.c
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

/**********************************************************************************************************/
/* Retirer_paletteDB: Elimination d'une palette                                                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_paletteDB ( struct CMD_TYPE_PALETTE *palette )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_paletteDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_PALETTE, palette->id );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_paletteDB ( struct CMD_TYPE_PALETTE *palette )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;
    gint id;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_paletteDB: DB connexion failed" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(syn_id,syn_cible_id,pos)"
                " VALUES (%d,%d,(SELECT MAX(pos)+1 FROM %s WHERE syn_id = %d))", NOM_TABLE_PALETTE,
                palette->syn_id, palette->syn_cible_id, NOM_TABLE_PALETTE, palette->syn_id );

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
 gboolean Recuperer_paletteDB ( struct DB **db_retour, gint id_syn )
  { gchar requete[2048];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_plugins_dlsDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.syn_id,%s.syn_cible_id,%s.libelle,%s.pos"
                " FROM %s,%s WHERE %s.syn_id=%d AND %s.id=%s.syn_cible_id ORDER BY %s.pos",
                NOM_TABLE_PALETTE, NOM_TABLE_PALETTE, NOM_TABLE_PALETTE,
                NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_PALETTE,
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PALETTE,                                          /* From */
                NOM_TABLE_PALETTE, id_syn, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PALETTE,           /* Jointure */
                NOM_TABLE_PALETTE );

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
 struct CMD_TYPE_PALETTE *Recuperer_paletteDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_PALETTE *palette;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    palette = (struct CMD_TYPE_PALETTE *)g_try_malloc0( sizeof(struct CMD_TYPE_PALETTE) );
    if (!palette) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_paletteDB_suite: Erreur allocation mémoire" );
    else
     { palette->id           = atoi(db->row[0]);
       palette->syn_id       = atoi(db->row[1]);                   /* Synoptique ou est placée la palette */
       palette->syn_cible_id = atoi(db->row[2]);                        /* Synoptique cible de la palette */
       palette->position     = atoi(db->row[4]);                             /* en abscisses et ordonnées */
       memcpy ( &palette->libelle, db->row[3], sizeof(palette->libelle) );
     }
    return(palette);
  }
/**********************************************************************************************************/
/* Rechercher_paletteDB: Recupération du palette dont l'id est en parametre                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_PALETTE *Rechercher_paletteDB ( guint id )
  { struct CMD_TYPE_PALETTE *palette;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_paletteDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.syn_id,%s.syn_cible_id,%s.titre,%s.pos "
                "FROM %s,%s WHERE %s.id=%d AND %s.id=%s.syn_cible_id",
                NOM_TABLE_PALETTE, NOM_TABLE_PALETTE, NOM_TABLE_SYNOPTIQUE,
                NOM_TABLE_PALETTE,
                NOM_TABLE_PALETTE, NOM_TABLE_SYNOPTIQUE,                                          /* From */
                NOM_TABLE_PALETTE, id, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PALETTE );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "Rechercher_paletteDB: DLS %03d not found in DB", id );
       return(NULL);
     }

    palette = (struct CMD_TYPE_PALETTE *)g_try_malloc0( sizeof(struct CMD_TYPE_PALETTE) );
    if (!palette) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_paletteDB: Erreur allocation mémoire" );
    else
     { palette->id           = id;
       palette->syn_id       = atoi(db->row[0]);                   /* Synoptique ou est placée la palette */
       palette->syn_cible_id = atoi(db->row[1]);                        /* Synoptique cible de la palette */
       palette->position     = atoi(db->row[3]);                             /* en abscisses et ordonnées */
       memcpy ( &palette->libelle, db->row[2], sizeof(palette->libelle) );
     }
    Libere_DB_SQL( &db );
    return(palette);
  }
/**********************************************************************************************************/
/* Modifier_paletteDB: Modification d'un palette Watchdog                                                 */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_paletteDB( struct CMD_TYPE_PALETTE *palette )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_paletteDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "
                "pos=(SELECT pos FROM %s WHERE id=%d)"
                " WHERE syn_id=%d AND pos=%d;"
                "UPDATE %s SET "
                "pos=%d"
                " WHERE id=%d;",
                NOM_TABLE_PALETTE, NOM_TABLE_PALETTE, palette->id,
                palette->syn_id,
                palette->position,
                NOM_TABLE_PALETTE,
                palette->position,
                palette->id
              );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/*--------------------------------------------------------------------------------------------------------*/
