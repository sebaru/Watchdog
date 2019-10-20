/******************************************************************************************************************************/
/* Watchdogd/Mnemo_Registre.c              Déclaration des fonctions pour la gestion des registre.c                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    22.03.2017 10:29:53 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_Registre.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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
/* Rechercher_registreDB: Recupération du registre dont l'id est en parametre                                                 */
/* Entrée: l'id a récupérer                                                                                                   */
/* Sortie: une structure hébergeant le registre                                                                               */
/******************************************************************************************************************************/
 struct CMD_TYPE_MNEMO_REGISTRE *Rechercher_mnemo_registreDB ( guint id )
  { struct CMD_TYPE_MNEMO_REGISTRE *registre;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT r.unite FROM %s as r INNER JOIN %s as m ON r.id_mnemo = m.id"
                " WHERE r.id_mnemo=%d",
                NOM_TABLE_MNEMO_REGISTRE,
                NOM_TABLE_MNEMO,
                id                                                                                                   /* WHERE */
              );

   if (Lancer_requete_SQL ( db, requete ) == FALSE)                                            /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return(NULL);
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    registre = (struct CMD_TYPE_MNEMO_REGISTRE *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_REGISTRE) );
    if (!registre) Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else
     { g_snprintf( registre->unite, sizeof(registre->unite), "%s", db->row[0] );
     }
    Libere_DB_SQL( &db );
    return(registre);
  }
/******************************************************************************************************************************/
/* Modifier_registreDB: Modification d'une registre Watchdog                                                                  */
/* Entrées: une structure hébergeant la registrerisation a modifier                                                           */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Modifier_mnemo_registreDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;
    gchar *unite;

    unite = Normaliser_chaine ( mnemo_full->mnemo_r.unite );                                 /* Formatage correct des chaines */
    if (!unite)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation unite impossible", __func__ );
       return(FALSE);
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s (id_mnemo,unite) VALUES "
                "('%d','%s') "
                "ON DUPLICATE KEY UPDATE "
                "unite=VALUES(unite) ",
                NOM_TABLE_MNEMO_REGISTRE, mnemo_full->mnemo_base.id, unite
              );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Charger_registre: Chargement des infos sur les Registres                                                                   */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_registre ( void )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connexion DB impossible", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT m.num, r.unite FROM %s as r"
                " INNER JOIN %s as m ON r.id_mnemo = m.id ORDER BY num",
                NOM_TABLE_MNEMO_REGISTRE,
                NOM_TABLE_MNEMO                                                                                 /* INNER JOIN */
              );

   if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while ( Recuperer_ligne_SQL(db) )                                                      /* Chargement d'une ligne resultat */
     { gint num;
       num = atoi( db->row[0] );
       if (num < NBR_REGISTRE)
        { g_snprintf( Partage->registre[num].confDB.unite, sizeof(Partage->registre[num].confDB.unite), "%s", db->row[1] );
          Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: Chargement config R[%04d]", __func__, num );
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
			       "%s: num (%d) out of range (max=%d)", __func__, num, NBR_REGISTRE ); }
      }
    Libere_DB_SQL (&db);
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: DB reloaded", __func__ );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
