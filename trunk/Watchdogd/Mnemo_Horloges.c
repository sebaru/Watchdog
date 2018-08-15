/******************************************************************************************************************************/
/* Watchdogd/Mnemo_Horloges.c        Déclaration des fonctions pour la gestion des Horloges                                   */
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
/* Activer_holorgeDB: Recherche toutes les actives à date et les positionne dans la mémoire partagée                          */
/* Entrée: rien                                                                                                               */
/* Sortie: Les horloges sont directement pilotée dans la structure DLS_DATA                                                   */
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
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Mise à un de l'horloge %s_%s", __func__, db->row[0], db->row[1] );
       Envoyer_commande_dls_data ( db->row[0], db->row[1] );
     }
    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Modifier_analogInputDB: Modification d'un entreeANA Watchdog                                                               */
/* Entrées: une structure hébergeant l'entrée analogique a modifier                                                           */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gint Ajouter_mnemo_horlogeDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[256];
    gboolean retour;
    struct DB *db;
    gint id;

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
    if ( retour == FALSE )
     { Libere_DB_SQL(&db); 
       return(-1);
     }
    id = Recuperer_last_ID_SQL ( db );
    Libere_DB_SQL(&db);
    return(id);
  }
/******************************************************************************************************************************/
/* Modifier_analogInputDB: Modification d'un entreeANA Watchdog                                                               */
/* Entrées: une structure hébergeant l'entrée analogique a modifier                                                           */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Modifier_mnemo_horlogeDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[256];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE %s SET heure='%d',minute='%d' WHERE id='%d' ",
                NOM_TABLE_MNEMO_HORLOGE, mnemo_full->mnemo_horloge.heure, mnemo_full->mnemo_horloge.minute,
                mnemo_full->mnemo_horloge.id
              );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Modifier_analogInputDB: Modification d'un entreeANA Watchdog                                                               */
/* Entrées: une structure hébergeant l'entrée analogique a modifier                                                           */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Retirer_horlogeDB( struct CMD_TYPE_MNEMO_FULL *mnemo_full )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "DELETE FROM %s WHERE id=%d ",
                NOM_TABLE_MNEMO_HORLOGE, mnemo_full->mnemo_horloge.id
              );

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Recuperer_mnemo_base_db: Récupération de la liste des mnemos de base                                                       */
/* Entrée: un pointeur vers la nouvelle connexion base de données                                                             */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Recuperer_horloge_by_id_mnemo ( struct DB **db_retour, gint id_mnemo )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT h.id, m.id, m.libelle, s.id" 
                " FROM mnemos as m" 
                " INNER JOIN %s as h ON h.id_mnemo = m.id" 
                " INNER JOIN dls as d ON m.dls_id = d.id" 
                " INNER JOIN syns as s ON d.syn_id = s.id" 
                " WHERE m.id='%d'", NOM_TABLE_MNEMO_HORLOGE, id_mnemo
              );                                                                                    /* order by test 25/01/06 */


    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/******************************************************************************************************************************/
/* Recuperer_mnemo_base_db: Récupération de la liste des mnemos de base                                                       */
/* Entrée: un pointeur vers la nouvelle connexion base de données                                                             */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 struct CMD_TYPE_MNEMO_FULL *Rechercher_horloge_by_id ( gint id )
  { struct CMD_TYPE_MNEMO_FULL *mnemo;
    gchar requete[1024];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT h.id, h.heure, h.minute" 
                " FROM mnemos as m" 
                " INNER JOIN %s as h ON h.id_mnemo = m.id" 
                " WHERE h.id='%d'", NOM_TABLE_MNEMO_HORLOGE, id
              );                                                                                    /* order by test 25/01/06 */


    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (retour == FALSE)
     { Libere_DB_SQL (&db);
       return(NULL);
     }

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    mnemo = (struct CMD_TYPE_MNEMO_FULL *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_FULL) );
    if (!mnemo) Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else                                                                                /* Recopie dans la nouvelle structure */
     { mnemo->mnemo_horloge.id     = atoi(db->row[0]);
       mnemo->mnemo_horloge.heure  = atoi(db->row[1]);
       mnemo->mnemo_horloge.minute = atoi(db->row[2]);
     }
    Libere_DB_SQL(&db);
    return ( mnemo );
  }
/******************************************************************************************************************************/
/* Recuperer_mnemo_base_DB_suite: Fonction itérative de récupération des mnémoniques de base                                  */
/* Entrée: un pointeur sur la connexion de baase de données                                                                   */
/* Sortie: une structure nouvellement allouée                                                                                 */
/******************************************************************************************************************************/
 struct CMD_TYPE_MNEMO_FULL *Recuperer_horlogeDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_MNEMO_FULL *mnemo;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    mnemo = (struct CMD_TYPE_MNEMO_FULL *)g_try_malloc0( sizeof(struct CMD_TYPE_MNEMO_FULL) );
    if (!mnemo) Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else                                                                                /* Recopie dans la nouvelle structure */
     { g_snprintf( mnemo->mnemo_base.libelle, sizeof(mnemo->mnemo_base.libelle), "%s", db->row[2] );
       mnemo->mnemo_horloge.id  = atoi(db->row[0]);
       mnemo->mnemo_base.id     = atoi(db->row[1]);
       mnemo->mnemo_base.syn_id = atoi(db->row[3]);
     }
    return(mnemo);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
