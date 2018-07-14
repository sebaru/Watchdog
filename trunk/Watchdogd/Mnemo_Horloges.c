/******************************************************************************************************************************/
/* Watchdogd/Mnemo_Horloges.c        D�claration des fonctions pour la gestion des Horloges                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    03.07.2018 21:25:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_Horloges.c
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
/* Activer_holorgeDB: Recherche toutes les actives � date et les positionne dans la m�moire partag�e                          */
/* Entr�e: rien                                                                                                               */
/* Sortie: Les horloges sont directement pilot�e dans la structure DLS_DATA                                                   */
/******************************************************************************************************************************/
 void Activer_horlogeDB ( void )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return;
     }
 
    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT m.acronyme, d.tech_id"
                " FROM %s as m INNER JOIN %s as d ON m.dls_id = d.id"
                " INNER JOIN %s as h ON h.id_mnemo = m.id"
                " WHERE CURTIME() LIKE CONCAT(LPAD(h.heure,2,'0'),':',LPAD(h.minute,2,'0'),':%')",
                NOM_TABLE_MNEMO, NOM_TABLE_DLS, NOM_TABLE_MNEMO_HORLOGE
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while (Recuperer_ligne_SQL(db))                                                        /* Chargement d'une ligne resultat */
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Mise � un de l'horloge %s_%s", __func__, db->row[0], db->row[1] );
       Envoyer_commande_dls_data ( db->row[0], db->row[1] );
     }
    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Modifier_analogInputDB: Modification d'un entreeANA Watchdog                                                               */
/* Entr�es: une structure h�bergeant l'entr�e analogique a modifier                                                           */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Modifier_mnemo_add_horlogeDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s (id_mnemo,heure,minute) VALUES "
                "('%d','%d','%d') ",
                NOM_TABLE_MNEMO_HORLOGE, mnemo_full->mnemo_base.id, 
                mnemo_full->mnemo_horloge.heure, mnemo_full->mnemo_horloge.minute
              );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Modifier_analogInputDB: Modification d'un entreeANA Watchdog                                                               */
/* Entr�es: une structure h�bergeant l'entr�e analogique a modifier                                                           */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Modifier_mnemo_del_all_horlogeDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "DELETE FROM %s WHERE id_mnemo=%d ",
                NOM_TABLE_MNEMO_HORLOGE, mnemo_full->mnemo_base.id
              );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
  #ifdef bouh
/******************************************************************************************************************************/
/* Charger_analogInput: Chargement des infos sur les Entrees ANA                                                              */
/* Entr�e: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_analogInput ( void )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Charger_analogInput: Connexion DB impossible" );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT num,min,max,%s.type,%s.unite"
                " FROM %s"
                " INNER JOIN %s ON id_mnemo = id ORDER BY num",
                NOM_TABLE_MNEMO_AI, NOM_TABLE_MNEMO_AI,
                NOM_TABLE_MNEMO,                                                                                      /* FROM */
                NOM_TABLE_MNEMO_AI                                                                              /* INNER JOIN */
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while ( Recuperer_ligne_SQL(db) )                                                      /* Chargement d'une ligne resultat */
     { gint num;
       num = atoi( db->row[0] );
       if (num < NBR_ENTRE_ANA)
        { Partage->ea[num].confDB.min  = atof(db->row[1]);
          Partage->ea[num].confDB.max      = atof(db->row[2]);
          Partage->ea[num].confDB.type     = atoi(db->row[3]);
          g_snprintf( Partage->ea[num].confDB.unite, sizeof(Partage->ea[num].confDB.unite), "%s", db->row[4] );
          Partage->ea[num].last_arch = 0;                             /* Mise � zero du champ de la derniere date d'archivage */
          Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                   "Charger_analogInput: Chargement config EA[%04d]=%d", num );
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
			       "Charger_analogInput: num (%d) out of range (max=%d)", num, NBR_ENTRE_ANA ); }
     }
    Libere_DB_SQL (&db);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "Charger_analogInput: DB reloaded" );
  }
  #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
