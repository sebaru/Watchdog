/**********************************************************************************************************/
/* Watchdogd/Db/EntreeANA/EntreeANA.c        Déclaration des fonctions pour la gestion des entreeANA.c    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 18 avr 2009 13:30:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * EntreeANA.c
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
 #include "EntreeANA_DB.h"
 #include "Mnemonique_DB.h"

/**********************************************************************************************************/
/* Charger_eana: Chargement des infos sur les Entrees analogiques                                         */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Charger_eana ( void )
  { struct DB *db;
    gint i;

    for (i = 0; i<NBR_ENTRE_ANA; i++)                                                   /* RAZ du tableau */
     { Partage->ea[i].min = 0.0;
       Partage->ea[i].max = 100.0;
       Partage->ea[i].unite = 0;
     }

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info( Config.log, DEBUG_INFO, "Charger_eana: Connexion DB failed" );
       return;
     }                                                                                  /* Si pas d'accès */

    if (!Recuperer_entreeANADB( Config.log, db ))
     { Libere_DB_SQL( Config.log, &db );
       return;
     }                                                                         /* Si pas d'enregistrement */

    for( ; ; )
     { struct CMD_TYPE_ENTREEANA *entree;
       entree = Recuperer_entreeANADB_suite( Config.log, db );
       if (!entree)
        { Libere_DB_SQL( Config.log, &db );
          return;
        }

       Partage->ea[entree->num].min   = entree->min;                            /* Mise a jour du tableau */
       Partage->ea[entree->num].max   = entree->max;
       Partage->ea[entree->num].unite = entree->unite;
       g_free(entree);
     }
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_entreeanaDB: Recupération de la liste des ids des entreeANAs                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_entreeANADB ( struct LOG *log, struct DB *db )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.min,%s.max,%s.unite,%s.libelle,id_mnemo,%s.num"
                " FROM %s,%s WHERE %s.id_mnemo=%s.id ORDER BY %s.num",
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, NOM_TABLE_MNEMO,
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* Where */
                NOM_TABLE_MNEMO /* Order by */
              );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_entreeanaDB: Recupération de la liste des ids des entreeANAs                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_ENTREEANA *Recuperer_entreeANADB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_ENTREEANA *entreeana;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    entreeana = (struct CMD_TYPE_ENTREEANA *)g_malloc0( sizeof(struct CMD_TYPE_ENTREEANA) );
    if (!entreeana) Info( log, DEBUG_MEM, "Recuperer_entreeANADB_suite: Erreur allocation mémoire" );
    else
     { entreeana->id_mnemo = atoi(db->row[4]);
       entreeana->num      = atoi(db->row[5]);
       entreeana->min      = atof(db->row[0]);
       entreeana->max      = atof(db->row[1]);
       entreeana->unite    = atoi(db->row[2]);
       memcpy( entreeana->libelle, db->row[3], sizeof(entreeana->libelle) ); /* Recopie dans la structure */
     }
    return(entreeana);
  }
/**********************************************************************************************************/
/* Rechercher_entreeanaDB: Recupération du entreeANA dont l'id est en parametre                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_ENTREEANA *Rechercher_entreeANADB ( struct LOG *log, struct DB *db, guint id )
  { struct CMD_TYPE_ENTREEANA *entreeana;
    gchar requete[512];
    
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.num,%s.min,%s.max,%s.unite,%s.libelle"
                " FROM %s,%s WHERE %s.id=%s.id_mnemo AND %s.id_mnemo=%d",
                NOM_TABLE_MNEMO, NOM_TABLE_ENTREEANA,
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_MNEMO,
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_MNEMO, NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA, id /* WHERE */
              );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_entreeanaDB: EntreANA non trouvé dans la BDD", id );
       return(NULL);
     }

    entreeana = (struct CMD_TYPE_ENTREEANA *)g_malloc0( sizeof(struct CMD_TYPE_ENTREEANA) );
    if (!entreeana)
     { Info( log, DEBUG_MEM, "Rechercher_entreeanaDB: Mem error" ); }
    else
     { entreeana->id_mnemo = id;;
       entreeana->num      = atoi(db->row[0]);
       entreeana->min      = atof(db->row[1]);
       entreeana->max      = atof(db->row[2]);
       entreeana->unite    = atoi(db->row[3]);
       memcpy( entreeana->libelle, db->row[4], sizeof(entreeana->libelle) ); /* Recopie dans la structure */
     }
    Liberer_resultat_SQL ( log, db );

    return(entreeana);
  }
/**********************************************************************************************************/
/* Modifier_entreeANADB: Modification d'un entreeANA Watchdog                                             */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_entreeANADB( struct LOG *log, struct DB *db, struct CMD_TYPE_ENTREEANA *entreeana )
  { gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "min=%f,max=%f,unite=%d WHERE id_mnemo=%d",
                NOM_TABLE_ENTREEANA, entreeana->min, entreeana->max,
                entreeana->unite, entreeana->id_mnemo );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
