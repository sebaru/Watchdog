/******************************************************************************************************************************/
/* Watchdogd/Synoptiques/passerelle.c       Ajout/retrait de passerelle dans les synoptiques                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          dim 13 mai 2007 13:41:33 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

/******************************************************************************************************************************/
/* Retirer_passerelleDB: Elimination d'une passerelle                                                                         */
/* Entrée: une structure PASSRELLE a supprimer                                                                                */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Retirer_passerelleDB ( struct CMD_TYPE_PASSERELLE *passerelle )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_passerelleDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_PASSERELLE, passerelle->id );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter_passerelleDB: Ajout d'une passrelle en parametre                                                                   */
/* Entrée: la passerelle                                                                                                      */
/* Sortie: l'id SQL ajouté                                                                                                    */
/******************************************************************************************************************************/
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

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s(syn_id,syn_cible_id,posx,posy,angle)"
                " VALUES (%d,%d,%d,%d,'%f')", NOM_TABLE_PASSERELLE,
                passerelle->syn_id, passerelle->syn_cible_id,
                passerelle->position_x, passerelle->position_y, passerelle->angle );

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
/* Recuperer_liste_id_msgDB: Recupération de la liste des passerelles dans un synoptique en parametre                         */
/* Entrée: une base de données et l'id du synoptique pour lequel fournir les passerelles                                      */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Recuperer_passerelleDB ( struct DB **db_retour, gint id_syn )
  { gchar requete[2048];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_plugins_dlsDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT sp.id,sp.syn_id,sp.syn_cible_id,s.page,"
                "sp.posx,sp.posy,sp.angle,s.vignette_activite,s.vignette_secu_bien,s.vignette_secu_personne"
                " FROM %s as s INNER JOIN %s as sp ON s.id=sp.syn_cible_id WHERE sp.syn_id=%d",
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PASSERELLE, id_syn );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_passerelleDB_suite: Recupération des passerelles et insertion en structure                                       */
/* Entrée: La base de données ouverte                                                                                         */
/* Sortie: une structure PASSERELLE                                                                                           */
/******************************************************************************************************************************/
 struct CMD_TYPE_PASSERELLE *Recuperer_passerelleDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_PASSERELLE *passerelle;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    passerelle = (struct CMD_TYPE_PASSERELLE *)g_try_malloc0( sizeof(struct CMD_TYPE_PASSERELLE) );
    if (!passerelle) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_passerelleDB_suite: Erreur allocation mémoire" );
    else
     { passerelle->id             = atoi(db->row[0]);
       passerelle->syn_id         = atoi(db->row[1]);                               /* Synoptique ou est placée la passerelle */
       passerelle->syn_cible_id   = atoi(db->row[2]);                                    /* Synoptique cible de la passerelle */
       passerelle->position_x     = atoi(db->row[4]);                                            /* en abscisses et ordonnées */
       passerelle->position_y     = atoi(db->row[5]);
       passerelle->angle          = atof(db->row[6]);
       passerelle->vignette_activite      = atoi(db->row[7]);                                                   /* Ixxx, Cxxx */
       passerelle->vignette_secu_bien     = atoi(db->row[8]);                                                   /* Ixxx, Cxxx */
       passerelle->vignette_secu_personne = atoi(db->row[9]);                                                  /* Ixxx, Cxxx */
       memcpy ( &passerelle->libelle, db->row[3], sizeof(passerelle->libelle) );
     }
    return(passerelle);
  }
/******************************************************************************************************************************/
/* Rechercher_passerelleDB: Recupération du passerelle dont l'id est en parametre                                             */
/* Entrée: un id de passerelle                                                                                                */
/* Sortie: la passerelle, ou NULL si pb                                                                                       */
/******************************************************************************************************************************/
 struct CMD_TYPE_PASSERELLE *Rechercher_passerelleDB ( guint id )
  { struct CMD_TYPE_PASSERELLE *pass;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_passerelleDB: DB connexion failed" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT sp.id,sp.syn_id,sp.syn_cible_id,s.page,"
                "sp.posx,sp.posy,sp.angle,s.vignette_activite,s.vignette_secu_bien,s.vignette_secu_personne"
                " FROM %s as s INNER JOIN %s as sp ON s.id=sp.syn_cible_id WHERE sp.id=%d",
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PASSERELLE, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    pass = Recuperer_passerelleDB_suite( &db );
    if (pass) Libere_DB_SQL ( &db );
    return(pass);
  }
/******************************************************************************************************************************/
/* Modifier_passerelleDB: Modification d'un passerelle Watchdog                                                               */
/* Entrées: une structure PASSERELLE a integrer en base                                                                       */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Modifier_passerelleDB( struct CMD_TYPE_PASSERELLE *passerelle )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_passerelleDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE %s as s INNER JOIN %s as sp ON s.id=sp.syn_cible_id SET "             
                "vignette_activite=%d, vignette_secu_bien=%d, vignette_secu_personne=%d,"
                "posx=%d,posy=%d,angle='%f'"
                " WHERE sp.id=%d;", NOM_TABLE_SYNOPTIQUE, NOM_TABLE_PASSERELLE,
                passerelle->vignette_activite, passerelle->vignette_secu_bien, passerelle->vignette_secu_personne,
                passerelle->position_x, passerelle->position_y, passerelle->angle,
                passerelle->id );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
