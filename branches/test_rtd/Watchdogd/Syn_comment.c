/**********************************************************************************************************/
/* Watchdogd/Synoptiques/comment.c       Ajout/retrait de comment dans les comment                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                      mer 05 mai 2004 12:11:21 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * comment.c
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
 #include "Erreur.h"
 #include "Synoptiques_DB.h"

/**********************************************************************************************************/
/* Retirer_commentDB: Elimination d'un comment                                                            */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_commentDB ( struct CMD_TYPE_COMMENT *comment )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_commentDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_COMMENT, comment->id );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_commentDB: Ajout ou edition d'un message                                                       */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure comment                       */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_commentDB ( struct CMD_TYPE_COMMENT *comment )
  { gchar *libelle, *font;
    gchar requete[512];
    gboolean retour;
    struct DB *db;
    gint id;

    libelle = Normaliser_chaine ( comment->libelle );               /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_commentDB: Normalisation impossible" );
       return(-1);
     }

    font = Normaliser_chaine ( comment->font );                     /* Formatage correct des chaines */
    if (!font)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_commentDB: Normalisation impossible" );
       g_free(libelle);
       return(-1);
     }

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_commentDB: DB connexion failed" );
       g_free(libelle);
       g_free(font);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(syn_id,libelle,font,rouge,vert,bleu,posx,posy,angle) "
                "VALUES (%d,'%s','%s',%d,%d,%d,%d,%d,'%f')", NOM_TABLE_COMMENT,
                comment->syn_id, libelle, font,
                comment->rouge, comment->vert, comment->bleu,
                comment->position_x, comment->position_y, comment->angle );
    g_free(libelle);
    g_free(font);

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
/* Recuperer_liste_id_commentDB: Recupération de la liste des ids des messages                            */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_commentDB ( struct DB **db_retour, gint id_syn )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_commentDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,syn_id,libelle,font,rouge,vert,bleu,posx,posy,angle"
                " FROM %s WHERE syn_id=%d ORDER BY id", NOM_TABLE_COMMENT, id_syn );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_commentDB: Recupération de la liste des ids des messages                            */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_COMMENT *Recuperer_commentDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_COMMENT *comment;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    comment = (struct CMD_TYPE_COMMENT *)g_try_malloc0( sizeof(struct CMD_TYPE_COMMENT) );
    if (!comment) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_commentDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &comment->libelle, db->row[2], sizeof(comment->libelle) );    /* Recopie dans la structure */
       memcpy( &comment->font, db->row[3], sizeof(comment->font) );          /* Recopie dans la structure */
       comment->id         = atoi(db->row[0]);
       comment->syn_id     = atoi(db->row[1]);
       comment->position_x = atoi(db->row[7]);                                     /* en abscisses et ordonnées */
       comment->position_y = atoi(db->row[8]);
       comment->rouge      = atoi(db->row[4]);
       comment->vert       = atoi(db->row[5]);
       comment->bleu       = atoi(db->row[6]);
       comment->angle      = atof(db->row[9]);
     }
    return(comment);
  }
/**********************************************************************************************************/
/* Rechercher_commentDB: Recupération du comment dont l'id est en parametre                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_COMMENT *Rechercher_commentDB ( guint id )
  { struct CMD_TYPE_COMMENT *comment;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_commentDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT syn_id,libelle,font,rouge,vert,bleu,posx,posy,angle "
                "FROM %s WHERE id=%d", NOM_TABLE_COMMENT, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "Rechercher_commentDB: Comment %03d not found in DB", id );
       return(NULL);
     }

    comment = (struct CMD_TYPE_COMMENT *)g_try_malloc0( sizeof(struct CMD_TYPE_COMMENT) );
    if (!comment) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_commentDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &comment->libelle, db->row[1], sizeof(comment->libelle) );    /* Recopie dans la structure */
       memcpy( &comment->font, db->row[2], sizeof(comment->font) );          /* Recopie dans la structure */
       comment->id         = id;
       comment->syn_id     = atoi(db->row[0]);
       comment->position_x = atoi(db->row[6]);                               /* en abscisses et ordonnées */
       comment->position_y = atoi(db->row[7]);
       comment->rouge      = atoi(db->row[3]);
       comment->vert       = atoi(db->row[4]);
       comment->bleu       = atoi(db->row[5]);
       comment->angle      = atof(db->row[8]);
     }
    Libere_DB_SQL( &db );
    return(comment);
  }
/**********************************************************************************************************/
/* Modifier_commentDB: Modification d'un comment Watchdog                                                 */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_commentDB( struct CMD_TYPE_COMMENT *comment )
  { gchar requete[1024];
    gchar *libelle, *font;
    gboolean retour;
    struct DB *db;

    libelle = Normaliser_chaine ( comment->libelle );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_commentDB: Normalisation impossible" );
       return(FALSE);
     }

    font = Normaliser_chaine ( comment->font );                     /* Formatage correct des chaines */
    if (!font)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_commentDB: Normalisation impossible" );
       g_free(libelle);
       return(FALSE);
     }

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_commentDB: DB connexion failed" );
       g_free(libelle);
       g_free(font);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "
                "libelle='%s',font='%s',rouge=%d,vert=%d,bleu=%d,posx=%d,posy=%d,angle='%f' "
                " WHERE id=%d;", NOM_TABLE_COMMENT,
                libelle, font,
                comment->rouge, comment->vert, comment->bleu,
                comment->position_x, comment->position_y, comment->angle, comment->id );
    g_free(libelle);
    g_free(font);

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/*--------------------------------------------------------------------------------------------------------*/
