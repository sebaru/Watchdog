/**********************************************************************************************************/
/* Watchdogd/Icones/classe.c        Déclaration des fonctions pour la gestion des classes-classes          */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 30 sep 2003 10:38:04 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * classe.c
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
/* Retirer_msgDB: Elimination d'une classe                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_classeDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CLASSE *classe )
  { gchar requete[200];
    
    if (classe->id == 0) return(TRUE);                              /* La classe 0 n'est pas destructible */

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET id_classe=0 WHERE id_classe=%d",
                NOM_TABLE_ICONE, classe->id );

    Lancer_requete_SQL ( db, requete );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_classeDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CLASSE *classe )
  { gchar requete[200];
    gchar *libelle;

    libelle = Normaliser_chaine ( classe->libelle );                   /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Ajouter_classeDB: Normalisation impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(libelle) VALUES "
                "('%s')", NOM_TABLE_CLASSE, libelle );
    g_free(libelle);

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_classeDB ( struct LOG *log, struct DB *db )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle"
                " FROM %s ORDER BY libelle", NOM_TABLE_CLASSE );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CLASSEDB *Recuperer_classeDB_suite( struct LOG *log, struct DB *db )
  { struct CLASSEDB *classe;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    classe = (struct CLASSEDB *)g_try_malloc0( sizeof(struct CLASSEDB) );
    if (!classe) Info_new( Config.log, Config.log_all, LOG_ERR, "Recuperer_classeDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &classe->libelle, db->row[1], sizeof(classe->libelle) );      /* Recopie dans la structure */
       classe->id          = atoi(db->row[0]);
     }
    return(classe);
  }
/**********************************************************************************************************/
/* Rechercher_classeDB: Recupération du classe dont l'id est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CLASSEDB *Rechercher_classeDB ( struct LOG *log, struct DB *db, guint id )
  { struct CLASSEDB *classe;
    gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle FROM %s WHERE id=%d", NOM_TABLE_CLASSE, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_new( Config.log, Config.log_all, LOG_INFO, "Rechercher_classeDB: Classe %d not found in DB", id );
       return(NULL);
     }

    classe = (struct CLASSEDB *)g_try_malloc0( sizeof(struct CLASSEDB) );
    if (!classe) Info_new( Config.log, Config.log_all, LOG_ERR, "Rechercher_classeDB: Mem error" );
    else
     { memcpy( &classe->libelle, db->row[0], sizeof(classe->libelle) );      /* Recopie dans la structure */
       classe->id          = id;
     }
    return(classe);
  }
/**********************************************************************************************************/
/* Modifier_classeDB: Modification d'un classe Watchdog                                                   */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_classeDB( struct LOG *log, struct DB *db, struct CMD_TYPE_CLASSE *classe )
  { gchar requete[1024];
    gchar *libelle;

    libelle = Normaliser_chaine ( classe->libelle );
    if (!libelle)
     { Info_new( Config.log, Config.log_all, LOG_WARNING, "Modifier_classeDB: Normalisation impossible" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s' WHERE id=%d",
                NOM_TABLE_CLASSE, libelle, classe->id );
    g_free(libelle);

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
