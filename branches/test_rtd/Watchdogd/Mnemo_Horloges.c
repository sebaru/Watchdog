/******************************************************************************************************************************/
/* Watchdogd/Mnemo_Horloges.c        Déclaration des fonctions pour la gestion des Horloges                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    03.07.2018 21:25:00 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_Horloges.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
/* Ajouter_Modifier_mnemo_baseDB: Ajout ou modifie le mnemo en parametre                                                      */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_HORLOGE ( gint deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
  { gchar *acro, *libelle;
    gchar requete[1024];
    gboolean retour;
    struct DB *db;

/******************************************** Préparation de la base du mnemo *************************************************/
    acro       = Normaliser_chaine ( acronyme );                                             /* Formatage correct des chaines */
    if ( !acro )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation acro impossible. Mnemo NOT added nor modified.", __func__ );
       return(FALSE);
     }

    libelle    = Normaliser_chaine ( libelle_src );                                          /* Formatage correct des chaines */
    if ( !libelle )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation libelle impossible. Mnemo NOT added nor modified.", __func__ );
       g_free(acro);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO mnemos_HORLOGE SET deletable=%d, tech_id='%s',acronyme='%s',libelle='%s' "
                " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle)",
                deletable, tech_id, acro, libelle );
    g_free(libelle);
    g_free(acro);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }
    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return (retour);
  }
/******************************************************************************************************************************/
/* Horloge_del_all_ticks: Retire tous les ticks d'une horloge                                                                 */
/* Entrée: le tech_id/acronyme de l'horloge                                                                                   */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Horloge_del_all_ticks ( gchar *tech_id, gchar *acronyme )
  { return( SQL_Write_new ( "DELETE mnemos_HORLOGE_ticks FROM mnemos_HORLOGE_ticks "
                            "INNER JOIN mnemos_HORLOGE ON mnemos_HORLOGE.id = mnemos_HORLOGE_ticks.horloge_id "
                            "WHERE tech_id='%s' AND acronyme='%s'", tech_id, acronyme
                          ) );
  }
/******************************************************************************************************************************/
/* Horloge_add_tick: Ajout un tick a heure/minute en parametre pour l'horloge tech_id:acronyme                                */
/* Entrée: le tech_id/acronyme de l'horloge, l'heure et la minute                                                             */
/* Sortie: FALSE si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Horloge_add_tick ( gchar *tech_id, gchar *acronyme, gint heure, gint minute )
  { return( SQL_Write_new ( "INSERT INTO mnemos_HORLOGE_ticks SET "
                            "horloge_id = (SELECT id FROM mnemos_HORLOGE WHERE tech_id='%s' AND acronyme='%s'), "
                            "lundi=1, mardi=1, mercredi=1, jeudi=1, vendredi=1, samedi=1, dimanche=1, "
                            "heure = %d, minute = %d", tech_id, acronyme, heure, minute ) );
  }
/******************************************************************************************************************************/
/* Activer_holorgeDB: Recherche toutes les actives à date et les positionne dans la mémoire partagée                          */
/* Entrée: rien                                                                                                               */
/* Sortie: Les horloges sont directement pilotée dans la structure DLS_DATA                                                   */
/******************************************************************************************************************************/
 void Activer_horlogeDB ( void )
  { gchar requete[1024];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT m.tech_id, m.acronyme"
                " FROM mnemos_HORLOGE as m INNER JOIN mnemos_HORLOGE_ticks as t ON m.id = t.horloge_id"
                " WHERE CURTIME() LIKE CONCAT(LPAD(t.heure,2,'0'),':',LPAD(t.minute,2,'0'),':%%')"
                " AND ("
                 "(DAYNAME(CURRENT_DATE()) = 'Monday' AND t.lundi=1) OR "
                 "(DAYNAME(CURRENT_DATE()) = 'Tuesday' AND t.mardi=1) OR "
                 "(DAYNAME(CURRENT_DATE()) = 'Wednesday' AND t.mercredi=1) OR "
                 "(DAYNAME(CURRENT_DATE()) = 'Thursday' AND t.jeudi=1) OR "
                 "(DAYNAME(CURRENT_DATE()) = 'Friday' AND t.vendredi=1) OR "
                 "(DAYNAME(CURRENT_DATE()) = 'Saturday' AND t.samedi=1) OR "
                 "(DAYNAME(CURRENT_DATE()) = 'Sunday' AND t.dimanche=1) "
                "     )"
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while (Recuperer_ligne_SQL(db))                                                        /* Chargement d'une ligne resultat */
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: Mise à un de l'horloge %s:%s 1/2",
                 __func__, db->row[0], db->row[1] );
       Envoyer_commande_dls_data ( db->row[0], db->row[1] );
     }
    Libere_DB_SQL( &db );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
