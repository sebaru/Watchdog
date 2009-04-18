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

 #include "Erreur.h"
 #include "EntreeANA_DB.h"
 #include "Mnemonique_DB.h"

/**********************************************************************************************************/
/* Retirer_entreeanaDB: Elimination d'un entreeANA                                                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_entreeANADB ( struct LOG *log, struct DB *db, struct CMD_ID_ENTREEANA *entreeana )
  { gchar requete[200];
    
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_ENTREEANA, entreeana->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }

/**********************************************************************************************************/
/* Ajouter_entreeanaDB: Ajout ou edition d'un entreeANA                                                   */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure entreeana                     */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_entreeANADB ( struct LOG *log, struct DB *db, struct CMD_ADD_ENTREEANA *entreeana )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(num,min,max,unite) VALUES "
                "(%d,%f,%f,%d)", NOM_TABLE_ENTREEANA, entreeana->num,
                entreeana->min, entreeana->max, entreeana->unite );
    
    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_entreeanaDB: Recupération de la liste des ids des entreeANAs                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_entreeANADB ( struct LOG *log, struct DB *db )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.num,%s.min,%s.max,%s.unite,%s.libelle"
                " FROM %s,%s WHERE %s.num=%s.num AND %s.type=%d ORDER BY %s.num",
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_MNEMO,
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* Where */
                NOM_TABLE_MNEMO, MNEMO_ENTREE_ANA, /* And */
                NOM_TABLE_ENTREEANA /* Order by */
              );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_entreeanaDB: Recupération de la liste des ids des entreeANAs                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ENTREEANA_DB *Recuperer_entreeANADB_suite( struct LOG *log, struct DB *db )
  { struct ENTREEANA_DB *entreeana;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    entreeana = (struct ENTREEANA_DB *)g_malloc0( sizeof(struct ENTREEANA_DB) );
    if (!entreeana) Info( log, DEBUG_MEM, "Recuperer_entreeANADB_suite: Erreur allocation mémoire" );
    else
     { entreeana->id    = atoi(db->row[0]);
       entreeana->num   = atoi(db->row[1]);
       entreeana->min   = atof(db->row[2]);
       entreeana->max   = atof(db->row[3]);
       entreeana->unite = atoi(db->row[4]);
       memcpy( entreeana->libelle, db->row[5], sizeof(entreeana->libelle) ); /* Recopie dans la structure */
     }
    return(entreeana);
  }
/**********************************************************************************************************/
/* Rechercher_entreeanaDB: Recupération du entreeANA dont l'id est en parametre                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ENTREEANA_DB *Rechercher_entreeANADB ( struct LOG *log, struct DB *db, guint id )
  { struct ENTREEANA_DB *entreeana;
    gchar requete[512];
    
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.num,%s.min,%s.max,%s.unite,%s.libelle"
                " FROM %s,%s WHERE %s.num=%s.num AND %s.type=%d AND %s.id=%d",
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_MNEMO,
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* Where */
                NOM_TABLE_MNEMO, MNEMO_ENTREE_ANA, /* And */
                NOM_TABLE_ENTREEANA, id /* AND */
              );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_entreeanaDB: EntreANA non trouvé dans la BDD", id );
       return(NULL);
     }

    entreeana = (struct ENTREEANA_DB *)g_malloc0( sizeof(struct ENTREEANA_DB) );
    if (!entreeana)
     { Info( log, DEBUG_MEM, "Rechercher_entreeanaDB: Mem error" ); }
    else
     { entreeana->id    = id;
       entreeana->num   = atoi(db->row[0]);
       entreeana->min   = atof(db->row[1]);
       entreeana->max   = atof(db->row[2]);
       entreeana->unite = atoi(db->row[3]);
       memcpy( entreeana->libelle, db->row[4], sizeof(entreeana->libelle) ); /* Recopie dans la structure */
     }
    Liberer_resultat_SQL ( log, db );

    return(entreeana);
  }
/**********************************************************************************************************/
/* Rechercher_entreeanaDB: Recupération du entreeANA dont l'id est en parametre                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ENTREEANA_DB *Rechercher_entreeANADB_par_num ( struct LOG *log, struct DB *db, guint num )
  { struct ENTREEANA_DB *entreeana;
    gchar requete[512];
    
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,%s.min,%s.max,%s.unite,%s.libelle"
                " FROM %s,%s WHERE %s.num=%s.num AND %s.type=%d AND %s.num=%d",
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_ENTREEANA, NOM_TABLE_ENTREEANA,
                NOM_TABLE_MNEMO,
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* From */
                NOM_TABLE_ENTREEANA, NOM_TABLE_MNEMO, /* Where */
                NOM_TABLE_MNEMO, MNEMO_ENTREE_ANA, /* And */
                NOM_TABLE_ENTREEANA, num /* AND */
              );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_entreeanaDB_par_num: EntreANA non trouvé dans la BDD", num );
       return(NULL);
     }

    entreeana = (struct ENTREEANA_DB *)g_malloc0( sizeof(struct ENTREEANA_DB) );
    if (!entreeana)
     { Info( log, DEBUG_MEM, "Rechercher_entreeanaDB_par_num: Mem error" ); }
    else
     { entreeana->id    = atoi(db->row[0]);
       entreeana->num   = num;
       entreeana->min   = atof(db->row[1]);
       entreeana->max   = atof(db->row[2]);
       entreeana->unite = atoi(db->row[3]);
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
 gboolean Modifier_entreeANADB( struct LOG *log, struct DB *db, struct CMD_EDIT_ENTREEANA *entreeana )
  { gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "num=%d,min=%f,max=%f,unite=%d WHERE id=%d",
                NOM_TABLE_ENTREEANA, entreeana->num,entreeana->min, entreeana->max,
                entreeana->unite, entreeana->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
