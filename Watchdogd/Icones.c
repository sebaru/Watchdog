/**********************************************************************************************************/
/* Watchdogd/Icones/Icones.c        Déclaration des fonctions pour la gestion des icones                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 30 sep 2003 10:38:04 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Icones.c
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
/* Retirer_msgDB: Elimination d'un message                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_iconeDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_ICONE *icone )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_ICONE, icone->id );

    Lancer_requete_SQL ( log, db, requete );                               /* Execution de la requete SQL */

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE icone=%d", NOM_TABLE_MOTIF, icone->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_iconeDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_ICONE *icone )
  { gchar requete[200];
    gchar *libelle;

    libelle = Normaliser_chaine ( icone->libelle );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_iconeDB: Normalisation impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(libelle,id_classe) VALUES "
                "('%s',%d)", NOM_TABLE_ICONE, libelle, icone->id_classe );
    g_free(libelle);

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_iconeDB ( struct LOG *log, struct DB *db, guint classe )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,id_classe"
                " FROM %s WHERE id_classe=%d ORDER BY libelle", NOM_TABLE_ICONE, classe );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ICONEDB *Recuperer_iconeDB_suite( struct LOG *log, struct DB *db )
  { struct ICONEDB *icone;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    icone = (struct ICONEDB *)g_try_malloc0( sizeof(struct ICONEDB) );
    if (!icone) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_iconeDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &icone->libelle, db->row[1], sizeof(icone->libelle) );        /* Recopie dans la structure */
       icone->id          = atoi(db->row[0]);
       icone->id_classe   = atoi(db->row[2]);
     }
    return(icone);
  }
/**********************************************************************************************************/
/* Rechercher_iconeDB: Recupération du icone dont l'id est en parametre                                   */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ICONEDB *Rechercher_iconeDB ( struct LOG *log, struct DB *db, guint id )
  { struct ICONEDB *icone;
    gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT libelle,id_classe FROM %s WHERE id=%d", NOM_TABLE_ICONE, id );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "Rechercher_iconeDB: Icone %d not found in DB", id );
       return(NULL);
     }

    icone = (struct ICONEDB *)g_try_malloc0( sizeof(struct ICONEDB) );
    if (!icone) { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_iconeDB: Mem error" ); }
    else
     { memcpy( &icone->libelle, db->row[0], sizeof(icone->libelle) );        /* Recopie dans la structure */
       icone->id          = id;
       icone->id_classe   = atoi(db->row[1]);
     }
    return(icone);
  }
/**********************************************************************************************************/
/* Modifier_iconeDB: Modification d'un icone Watchdog                                                     */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_iconeDB( struct LOG *log, struct DB *db, struct CMD_TYPE_ICONE *icone )
  { gchar requete[1024];
    gchar *libelle;

    libelle = Normaliser_chaine ( icone->libelle );
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_iconeDB: Normalisation impossible" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "libelle='%s',id_classe=%d WHERE id=%d",
                NOM_TABLE_ICONE, libelle, icone->id_classe, icone->id );
    g_free(libelle);

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
