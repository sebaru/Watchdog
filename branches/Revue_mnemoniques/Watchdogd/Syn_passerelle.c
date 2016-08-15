/**********************************************************************************************************/
/* Watchdogd/Synoptiques/passerelle.c       Ajout/retrait de passerelle dans les synoptiques              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 13 mai 2007 13:41:33 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * passerelle.c
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
/* Retirer_passerelleDB: Elimination d'une passerelle                                                     */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_passerelleDB ( struct CMD_TYPE_PASSERELLE *passerelle )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_passerelleDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_PASSERELLE, passerelle->id );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_passerelleDB ( struct CMD_TYPE_PASSERELLE *passerelle )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;
    gint id;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_passerelleDB: DB connexion failed" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(syn_id,syn_cible_id,bitctrl,bitctrl1,bitctrl2,bitctrl3,posx,posy,angle)"
                " VALUES (%d,%d,%d,%d,%d,%d,%d,%d,'%f')", NOM_TABLE_PASSERELLE,
                passerelle->syn_id, passerelle->syn_cible_id, passerelle->bit_controle,
                passerelle->bit_controle_1, passerelle->bit_controle_2, passerelle->bit_controle_3,
                passerelle->position_x, passerelle->position_y, passerelle->angle );

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
 gboolean Recuperer_passerelleDB ( struct DB **db_retour, gint id_syn )
  { gchar requete[2048];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_plugins_dlsDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.syn_id,%s.syn_cible_id,%s.page,%s.bitctrl,%s.bitctrl1,%s.bitctrl2,"
                "%s.bitctrl3,%s.posx,%s.posy,%s.angle"
                " FROM %s,%s WHERE %s.syn_id=%d AND %s.id=%s.syn_cible_id",
                NOM_TABLE_PASSERELLE, NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PASSERELLE,
                NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,
                NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PASSERELLE,                                       /* From */
                NOM_TABLE_PASSERELLE, id_syn, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PASSERELLE );   /* Jointure */

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
 struct CMD_TYPE_PASSERELLE *Recuperer_passerelleDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_PASSERELLE *passerelle;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    passerelle = (struct CMD_TYPE_PASSERELLE *)g_try_malloc0( sizeof(struct CMD_TYPE_PASSERELLE) );
    if (!passerelle) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_passerelleDB_suite: Erreur allocation mémoire" );
    else
     { passerelle->id             = atoi(db->row[0]);
       passerelle->syn_id         = atoi(db->row[1]);           /* Synoptique ou est placée la passerelle */
       passerelle->syn_cible_id   = atoi(db->row[2]);                /* Synoptique cible de la passerelle */
       passerelle->bit_controle   = atoi(db->row[4]);                                       /* Ixxx, Cxxx */
       passerelle->bit_controle_1 = atoi(db->row[5]);                                       /* Ixxx, Cxxx */
       passerelle->bit_controle_2 = atoi(db->row[6]);                                       /* Ixxx, Cxxx */
       passerelle->bit_controle_3 = atoi(db->row[7]);                                       /* Ixxx, Cxxx */
       passerelle->position_x     = atoi(db->row[8]);                        /* en abscisses et ordonnées */
       passerelle->position_y     = atoi(db->row[9]);
       passerelle->angle          = atof(db->row[10]);
       memcpy ( &passerelle->libelle, db->row[3], sizeof(passerelle->libelle) );
     }
    return(passerelle);
  }
/**********************************************************************************************************/
/* Rechercher_passerelleDB: Recupération du passerelle dont l'id est en parametre                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_PASSERELLE *Rechercher_passerelleDB ( guint id )
  { struct CMD_TYPE_PASSERELLE *pass;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_passerelleDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.syn_id,%s.syn_cible_id,%s.page,%s.bitctrl,%s.bitctrl1,%s.bitctrl2,"
                "%s.bitctrl3,%s.posx,%s.posy,%s.angle"
                " FROM %s,%s WHERE %s.id=%d AND %s.id=%s.syn_cible_id", 
                NOM_TABLE_PASSERELLE, NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PASSERELLE,
                NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,
                NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,NOM_TABLE_PASSERELLE,
                NOM_TABLE_PASSERELLE, NOM_TABLE_SYNOPTIQUE,                                       /* From */
                NOM_TABLE_PASSERELLE, id, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PASSERELLE );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    pass = Recuperer_passerelleDB_suite( &db );
    if (pass) Libere_DB_SQL ( &db );
    return(pass);
  }
/**********************************************************************************************************/
/* Modifier_passerelleDB: Modification d'un passerelle Watchdog                                           */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_passerelleDB( struct CMD_TYPE_PASSERELLE *passerelle )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_passerelleDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "bitctrl=%d,bitctrl1=%d,bitctrl2=%d,bitctrl3=%d,posx=%d,posy=%d,angle='%f'"
                " WHERE id=%d;", NOM_TABLE_PASSERELLE,
                passerelle->bit_controle, passerelle->bit_controle_1, passerelle->bit_controle_2,
                passerelle->bit_controle_3,
                passerelle->position_x, passerelle->position_y, passerelle->angle,
                passerelle->id );

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/*--------------------------------------------------------------------------------------------------------*/
