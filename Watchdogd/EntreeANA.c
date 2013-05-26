/**********************************************************************************************************/
/* Watchdogd/EntreeANA/EntreeANA.c        Déclaration des fonctions pour la gestion des entreeANA.c       */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 18 avr 2009 13:30:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * EntreeANA.c
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
/* Charger_eana: Chargement des infos sur les Entrees analogiques                                         */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Charger_eana ( void )
  { struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Charger_eana: Connexion DB failed" );
       return;
     }                                                                                  /* Si pas d'accès */

    if (!Recuperer_entreeANADB( Config.log, db ))
     { Libere_DB_SQL( &db );
       return;
     }                                                                         /* Si pas d'enregistrement */

    for( ; ; )
     { struct CMD_TYPE_OPTION_ENTREEANA *entree;
       entree = Recuperer_entreeANADB_suite( Config.log, db );
       if (!entree)
        { Libere_DB_SQL( &db );
          return;
        }

       if (entree->num < NBR_ENTRE_ANA)
        { memcpy( &Partage->ea[entree->num].cmd_type_eana, entree, sizeof(struct CMD_TYPE_OPTION_ENTREEANA) );
          Partage->ea[entree->num].last_arch = 0;    /* Mise à zero du champ de la derniere date d'archivage */
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                   "Charger_eana: entree->num (%d) out of range (max=%d)", entree->num, NBR_ENTRE_ANA ); }
       g_free(entree);
     }
  }
/**********************************************************************************************************/
/* Recuperer_entreeANADB: Recupération de la liste des EANA                                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_entreeANADB ( struct LOG *log, struct DB *db )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id_mnemo,%s.num,%s.min,%s.max,%s.type,%s.unite,%s.libelle,%s.groupe,%s.page,%s.name"
                " FROM %s,%s,%s,%s WHERE %s.id_mnemo=%s.id AND %s.num_syn = %s.id AND %s.num_plugin = %s.id"
                " AND %s.type=%d ORDER BY %s.num",
                NOM_TABLE_MNEMO, NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_MNEMO,
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,/* From */
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* Where */
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_MNEMO, NOM_TABLE_DLS,
                NOM_TABLE_MNEMO, MNEMO_ENTREE_ANA,
                NOM_TABLE_MNEMO /* Order by */
              );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_entreeanaDB: Recupération de la liste des ids des entreeANAs                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_OPTION_ENTREEANA *Recuperer_entreeANADB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_OPTION_ENTREEANA *entreeana;

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       return(NULL);
     }

    entreeana = (struct CMD_TYPE_OPTION_ENTREEANA *)g_try_malloc0( sizeof(struct CMD_TYPE_OPTION_ENTREEANA) );
    if (!entreeana) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                             "Recuperer_entreeANADB_suite: Erreur allocation mémoire" );
    else
     { entreeana->id_mnemo = atoi(db->row[0]);
       entreeana->num      = atoi(db->row[1]);
       entreeana->min      = atof(db->row[2]);
       entreeana->max      = atof(db->row[3]);
       entreeana->type     = atoi(db->row[4]);
       memcpy( &entreeana->unite,      db->row[5], sizeof(entreeana->unite  ) );
       memcpy( &entreeana->libelle,    db->row[6], sizeof(entreeana->libelle) );
       memcpy( &entreeana->groupe,     db->row[7], sizeof(entreeana->groupe ) );
       memcpy( &entreeana->page,       db->row[8], sizeof(entreeana->page   ) );
       memcpy( &entreeana->plugin_dls, db->row[9], sizeof(entreeana->plugin_dls) );
     }
    return(entreeana);
  }
/**********************************************************************************************************/
/* Rechercher_entreeanaDB: Recupération du entreeANA dont l'id est en parametre                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_OPTION_ENTREEANA *Rechercher_entreeANADB ( struct LOG *log, struct DB *db, guint id )
  { struct CMD_TYPE_OPTION_ENTREEANA *entreeana;
    gchar requete[512];
    
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id_mnemo,%s.num,%s.min,%s.max,%s.type,%s.unite,%s.libelle,%s.groupe,%s.page,%s.name"
                " FROM %s,%s,%s,%s WHERE %s.id_mnemo=%s.id AND %s.num_syn = %s.id AND %s.num_plugin = %s.id"
                " AND %s.id_mnemo=%d",
                NOM_TABLE_MNEMO, NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_MNEMO, 
                NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,/* From */
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* Where */
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_MNEMO, NOM_TABLE_DLS,
                NOM_TABLE_ENTREEANA, id
              );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                "Rechercher_entreeanaDB: EntreANA %d not found in DB", id );
       return(NULL);
     }

    entreeana = (struct CMD_TYPE_OPTION_ENTREEANA *)g_try_malloc0( sizeof(struct CMD_TYPE_OPTION_ENTREEANA) );
    if (!entreeana)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_entreeanaDB: Mem error" ); }
    else
     { entreeana->id_mnemo = id;
       entreeana->num      = atoi(db->row[1]);
       entreeana->min      = atof(db->row[2]);
       entreeana->max      = atof(db->row[3]);
       entreeana->type     = atoi(db->row[4]);
       memcpy( &entreeana->unite,      db->row[5], sizeof(entreeana->unite  ) );
       memcpy( &entreeana->libelle,    db->row[6], sizeof(entreeana->libelle) );
       memcpy( &entreeana->groupe,     db->row[7], sizeof(entreeana->groupe ) );
       memcpy( &entreeana->page,       db->row[8], sizeof(entreeana->page   ) );
       memcpy( &entreeana->plugin_dls, db->row[9], sizeof(entreeana->plugin_dls) );
     }
    Liberer_resultat_SQL (db);

    return(entreeana);
  }
/**********************************************************************************************************/
/* Modifier_entreeANADB: Modification d'un entreeANA Watchdog                                             */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_entreeANADB( struct LOG *log, struct DB *db, struct CMD_TYPE_OPTION_ENTREEANA *entreeana )
  { gchar requete[1024];
    gchar *unite;

    unite = Normaliser_chaine ( entreeana->unite );                 /* Formatage correct des chaines */
    if (!unite)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_entreeANADB: Normalisation unite impossible" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "min='%f',max='%f',unite='%s',type=%d WHERE id_mnemo=%d",
                NOM_TABLE_ENTREEANA, entreeana->min, entreeana->max, unite, entreeana->type,
                entreeana->id_mnemo );
    g_free(unite);
    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
