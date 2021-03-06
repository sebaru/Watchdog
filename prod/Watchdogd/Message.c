/******************************************************************************************************************************/
/* Watchdogd/Message/Message.c        Déclaration des fonctions pour la gestion des message                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         jeu. 29 déc. 2011 14:55:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Message.c
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
/* Ajouter_messageDB: Ajout ou edition d'un message                                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gint Mnemo_auto_create_MSG ( gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gint typologie )
  { gchar *libelle;
    gchar requete[2048];
    gboolean retour;
    struct DB *db;
    gint id;

    libelle = Normaliser_chaine ( libelle_src );                                             /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation libelle impossible", __func__ );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s SET deletable='%d', tech_id='%s',acronyme='%s',libelle='%s',audio_libelle='%s',"
                "typologie='%d',sms_notification='0' "
                " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle), typologie=VALUES(typologie)", NOM_TABLE_MSG,
                deletable, tech_id, acronyme, libelle, libelle, typologie
              );
    g_free(libelle);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(-1);
     }

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
/* Charger_conf_ai: Recupération de la conf de l'entrée analogique en parametre                                               */
/* Entrée: l'id a récupérer                                                                                                   */
/* Sortie: une structure hébergeant l'entrée analogique                                                                       */
/******************************************************************************************************************************/
 void Charger_confDB_MSG ( void )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT m.tech_id, m.acronyme, m.etat FROM msgs as m"
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while (Recuperer_ligne_SQL(db))                                                        /* Chargement d'une ligne resultat */
     { Dls_data_set_MSG_init ( db->row[0], db->row[1], atoi(db->row[2]) );
       Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: MSG '%s:%s'=%d loaded", __func__,
                 db->row[0], db->row[1], atoi(db->row[2]) );
     }
    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Ajouter_cpt_impDB: Ajout ou edition d'un entreeANA                                                                         */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_confDB_MSG ( void )
  { gchar requete[200];
    GSList *liste;
    struct DB *db;
    gint cpt = 0;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connexion DB impossible", __func__ );
       return;
     }

    liste = Partage->Dls_data_MSG;
    while ( liste )
     { struct DLS_MESSAGES *msg = (struct DLS_MESSAGES *)liste->data;
       g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE msgs as m SET m.etat='%d' "
                   "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                   msg->etat, msg->tech_id, msg->acronyme );
       Lancer_requete_SQL ( db, requete );
       liste = g_slist_next(liste);
       cpt++;
     }
    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Dls_MESSAGE_to_json : Formate un bit au format JSON                                                                        */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_MESSAGE_to_json ( JsonBuilder *builder, struct DLS_MESSAGES *bit )
  { Json_add_string ( builder, "tech_id",  bit->tech_id );
    Json_add_string ( builder, "acronyme", bit->acronyme );
    Json_add_bool ( builder, "etat", bit->etat );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
