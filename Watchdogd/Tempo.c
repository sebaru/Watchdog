/**********************************************************************************************************/
/* Watchdogd/Tempo.c              Déclaration des fonctions pour la gestion des tempo.c                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                     sam. 09 mars 2013 11:47:18 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Tempo.c
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
/* Recuperer_tempoDB: Recupération de la liste des EANA                                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static gboolean Recuperer_tempoDB ( struct LOG *log, struct DB *db )
  { gchar requete[512];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id_mnemo,%s.num,%s.libelle,%s.groupe,%s.page,%s.name,"
                "%s.delai_on,%s.min_on,%s.max_on,%s.delai_off"
                " FROM %s,%s,%s,%s WHERE %s.id_mnemo=%s.id AND %s.num_syn = %s.id AND %s.num_plugin = %s.id"
                " AND %s.type=%d ORDER BY %s.num",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,
                NOM_TABLE_TEMPO, NOM_TABLE_TEMPO, NOM_TABLE_TEMPO, NOM_TABLE_TEMPO,
                NOM_TABLE_TEMPO, NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,/* From */
                NOM_TABLE_TEMPO, NOM_TABLE_MNEMO, /* Where */
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_MNEMO, NOM_TABLE_DLS,
                NOM_TABLE_MNEMO, MNEMO_TEMPO,
                NOM_TABLE_MNEMO /* Order by */
              );

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_tempoDB_suite: Recupération de la liste des informations sur les temposids des               */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct CMD_TYPE_OPTION_TEMPO *Recuperer_tempoDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_OPTION_TEMPO *tempo;

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       return(NULL);
     }

    tempo = (struct CMD_TYPE_OPTION_TEMPO *)g_try_malloc0( sizeof(struct CMD_TYPE_OPTION_TEMPO) );
    if (!tempo) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                             "Recuperer_tempoDB_suite: Erreur allocation mémoire" );
    else
     { tempo->id_mnemo  = atoi(db->row[0]);
       tempo->num       = atoi(db->row[1]);
       memcpy( &tempo->libelle,    db->row[2], sizeof(tempo->libelle) );
       memcpy( &tempo->groupe,     db->row[3], sizeof(tempo->groupe ) );
       memcpy( &tempo->page,       db->row[4], sizeof(tempo->page   ) );
       memcpy( &tempo->plugin_dls, db->row[5], sizeof(tempo->plugin_dls) );
       tempo->delai_on  = atoi(db->row[6]);
       tempo->min_on    = atoi(db->row[7]);
       tempo->max_on    = atoi(db->row[8]);
       tempo->delai_off = atoi(db->row[9]);
     }
    return(tempo);
  }
/**********************************************************************************************************/
/* Charger_tempo: Chargement des infos sur les Temporisations                                             */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Charger_tempo ( void )
  { struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Charger_tempo: Connexion DB failed" );
       return;
     }                                                                                  /* Si pas d'accès */

    if (!Recuperer_tempoDB( Config.log, db ))
     { Libere_DB_SQL( &db );
       return;
     }                                                                         /* Si pas d'enregistrement */

    for( ; ; )
     { struct CMD_TYPE_OPTION_TEMPO *tempo;
       tempo = Recuperer_tempoDB_suite( Config.log, db );
       if (!tempo)
        { Libere_DB_SQL( &db );
          return;
        }

       if (tempo->num < NBR_TEMPO)
        { memcpy( &Partage->Tempo_R[tempo->num].option_tempo, tempo, sizeof(struct CMD_TYPE_OPTION_TEMPO) );
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                   "Charger_tempo: tempo->num (%d) out of range (max=%d)", tempo->num, NBR_TEMPO ); }
       g_free(tempo);
     }
  }
/**********************************************************************************************************/
/* Rechercher_tempoDB: Recupération du tempo dont l'id est en parametre                           */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_OPTION_TEMPO *Rechercher_tempoDB ( struct LOG *log, struct DB *db, guint id )
  { struct CMD_TYPE_OPTION_TEMPO *tempo;
    gchar requete[512];
    
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id_mnemo,%s.num,%s.libelle,%s.groupe,%s.page,%s.name,"
                "%s.delai_on,%s.min_on,%s.max_on,%s.delai_off"
                " FROM %s,%s,%s,%s WHERE %s.id_mnemo=%s.id AND %s.num_syn = %s.id AND %s.num_plugin = %s.id"
                " AND %s.id_mnemo=%d",
                NOM_TABLE_MNEMO, NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,
                NOM_TABLE_TEMPO, NOM_TABLE_TEMPO, NOM_TABLE_TEMPO, NOM_TABLE_TEMPO,
                NOM_TABLE_TEMPO, NOM_TABLE_MNEMO, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_DLS,/* From */
                NOM_TABLE_TEMPO, NOM_TABLE_MNEMO, /* Where */
                NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_MNEMO, NOM_TABLE_DLS,
                NOM_TABLE_TEMPO, id /* And */
              );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Info_new( Config.log, Config.log_msrv, LOG_INFO,
                "Rechercher_tempoDB: Tempo %d (id_mnemo) not found in DB", id );
       return(NULL);
     }

    tempo = (struct CMD_TYPE_OPTION_TEMPO *)g_try_malloc0( sizeof(struct CMD_TYPE_OPTION_TEMPO) );
    if (!tempo)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_tempoDB: Mem error" ); }
    else
     { tempo->id_mnemo  = atoi(db->row[0]);
       tempo->num       = atoi(db->row[1]);
       memcpy( &tempo->libelle,    db->row[2], sizeof(tempo->libelle) );
       memcpy( &tempo->groupe,     db->row[3], sizeof(tempo->groupe ) );
       memcpy( &tempo->page,       db->row[4], sizeof(tempo->page   ) );
       memcpy( &tempo->plugin_dls, db->row[5], sizeof(tempo->plugin_dls) );
       tempo->delai_on  = atoi(db->row[6]);
       tempo->min_on    = atoi(db->row[7]);
       tempo->max_on    = atoi(db->row[8]);
       tempo->delai_off = atoi(db->row[9]);
     }
    Liberer_resultat_SQL (db);

    return(tempo);
  }
/**********************************************************************************************************/
/* Modifier_tempoDB: Modification d'une tempo Watchdog                                                    */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: FALSE si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Modifier_tempoDB( struct LOG *log, struct DB *db, struct CMD_TYPE_OPTION_TEMPO *tempo )
  { gchar requete[1024];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "delai_on='%d', min_on='%d', max_on='%d', delai_off='%d' WHERE id_mnemo=%d",
                NOM_TABLE_TEMPO, tempo->delai_on, tempo->min_on, tempo->max_on, tempo->delai_off,
                tempo->id_mnemo );
    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
