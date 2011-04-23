/**********************************************************************************************************/
/* Watchdogd/Archive/archiveDB.c       Déclaration des fonctions pour la gestion des valeurs analogiques  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 19 avr 2009 14:42:09 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * archiveDB.c
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

 #include "watchdogd.h"
/**********************************************************************************************************/
/* Ajouter_archDB: Ajout ou edition d'un entreeANA                                                        */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure arch                          */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 void Ajouter_archDB ( struct LOG *log, struct DB *db, struct ARCHDB *arch )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(date_sec,date_usec,type,num,valeur) VALUES "
                "(%d,%d,%d,%d,%d)", NOM_TABLE_ARCH, arch->date_sec, arch->date_usec,
                arch->type, arch->num, arch->valeur );

    Lancer_requete_SQL ( log, db, requete );                               /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_archDB: Recupération de la liste des ids des entreeANAs                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_archDB ( struct LOG *log, struct DB *db, guint type, guint num,
                             time_t date_deb, time_t date_fin )
  { gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT `type`,`num`,`date_sec`,`date_usec`,`valeur`"
                " FROM %s WHERE `type`=%d AND `num`=%d AND `date_sec`>=%d AND `date_sec`<=%d"
                " ORDER BY `date_sec`,`date_usec` ASC",
                NOM_TABLE_ARCH, type, num, (gint)date_deb, (gint)date_fin );

/*    g_snprintf( requete, sizeof(requete),
                "SELECT `type`,`num`,`date_sec`,`date_usec`,`valeur` FROM"
                " (SELECT `type`,`num`,`date_sec`,`date_usec`,`valeur`"
                "  FROM %s WHERE `type`=%d AND `num`=%d ORDER BY `date_sec`,`date_usec` DESC"
                "  LIMIT %d) AS `result`"
                " ORDER BY `date_sec`,`date_usec`",
                NOM_TABLE_ARCH, type, num, TAILLEBUF_HISTO_EANA );
*/
   Info_c( log, DEBUG_ARCHIVE, "Recuperer_archDB: Requete SQL", requete );

   return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_archDB: Recupération de la liste des ids des entreeANAs                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct ARCHDB *Recuperer_archDB_suite( struct LOG *log, struct DB *db )
  { struct ARCHDB *arch;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    arch = (struct ARCHDB *)g_malloc0( sizeof(struct ARCHDB) );
    if (!arch) Info( log, DEBUG_ARCHIVE, "Recuperer_archDB_suite: Erreur allocation mémoire" );
    else
     { arch->date_sec  = atoi(db->row[2]);
       arch->date_usec = atoi(db->row[3]);
       arch->type      = atoi(db->row[0]);
       arch->num       = atoi(db->row[1]);
       arch->valeur    = atoi(db->row[4]);
     }
    return(arch);
  }
/*--------------------------------------------------------------------------------------------------------*/
