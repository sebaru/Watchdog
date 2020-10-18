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

 #define MSGS_SQL_SELECT  "SELECT msg.id,msg.libelle,msg.type,syn.libelle,enable,parent_syn.page,syn.page," \
                          "sms,libelle_audio,libelle_sms,dls.shortname,syn.id,profil_audio,msg.tech_id,msg.acronyme" \
                          " FROM msgs as msg" \
                          " INNER JOIN dls as dls ON msg.tech_id=dls.tech_id" \
                          " INNER JOIN syns as syn ON dls.syn_id=syn.id" \
                          " INNER JOIN syns as parent_syn ON parent_syn.id=syn.parent_id"

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Ajouter_messageDB: Ajout ou edition d'un message                                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gint Mnemo_auto_create_MSG ( struct CMD_TYPE_MESSAGE *msg )
  { gchar *libelle, *libelle_audio, *libelle_sms;
    gchar requete[2048];
    gboolean retour;
    struct DB *db;
    gint id;

    libelle = Normaliser_chaine ( msg->libelle );                                            /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation libelle impossible", __func__ );
       return(-1);
     }
    libelle_audio = Normaliser_chaine ( msg->libelle_audio );                                /* Formatage correct des chaines */
    if (!libelle_audio)
     { g_free(libelle);
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation libelle_audio impossible", __func__ );
       return(-1);
     }
    libelle_sms = Normaliser_chaine ( msg->libelle_sms );                                    /* Formatage correct des chaines */
    if (!libelle_sms)
     { g_free(libelle);
       g_free(libelle_audio);
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation libelle_sms impossible", __func__ );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s SET tech_id='%s',acronyme='%s',libelle='%s',libelle_audio='%s',libelle_sms='%s',"
                "type='%d',enable='1',sms='0' "
                " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle), type=VALUES(type)", NOM_TABLE_MSG, msg->tech_id, msg->acronyme,
                libelle, libelle, libelle, msg->type
              );
    g_free(libelle);
    g_free(libelle_audio);
    g_free(libelle_sms);

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
/* Recuperer_liste_id_messageDB: Recupération de la liste des ids des messages                                                */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 struct CMD_TYPE_MESSAGE *Recuperer_messageDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_MESSAGE *msg;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    msg = (struct CMD_TYPE_MESSAGE *)g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!msg) Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else
     { g_snprintf( msg->libelle,         sizeof(msg->libelle      ),   "%s", db->row[1]  );      /* Recopie dans la structure */
       g_snprintf( msg->syn_libelle,     sizeof(msg->syn_libelle  ),   "%s", db->row[3]  );
       g_snprintf( msg->syn_parent_page, sizeof(msg->syn_parent_page), "%s", db->row[5]  );
       g_snprintf( msg->syn_page,        sizeof(msg->syn_page     ),   "%s", db->row[6]  );
       g_snprintf( msg->libelle_audio,   sizeof(msg->libelle_audio),   "%s", db->row[8] );
       g_snprintf( msg->libelle_sms,     sizeof(msg->libelle_sms  ),   "%s", db->row[9] );
       g_snprintf( msg->dls_shortname,   sizeof(msg->dls_shortname),   "%s", db->row[10] );
       g_snprintf( msg->profil_audio,    sizeof(msg->profil_audio ),   "%s", db->row[12] );
       g_snprintf( msg->tech_id,         sizeof(msg->tech_id      ),   "%s", db->row[13] );
       g_snprintf( msg->acronyme,        sizeof(msg->acronyme     ),   "%s", db->row[14] );
       msg->id          = atoi(db->row[0]);
       msg->type        = atoi(db->row[2]);
       msg->enable      = atoi(db->row[4]);
       msg->sms         = atoi(db->row[7]);
       msg->syn_id      = atoi(db->row[11]);
     }
    return(msg);
  }
/******************************************************************************************************************************/
/* Rechercher_messageDB_par_id: Recupération du message dont l'id est en parametre                                            */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 struct CMD_TYPE_MESSAGE *Rechercher_messageDB_par_acronyme ( gchar *tech_id, gchar *acronyme )
  { struct CMD_TYPE_MESSAGE *message;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete), MSGS_SQL_SELECT                                                      /* Requete SQL */
                " WHERE dls.tech_id='%s' AND msg.acronyme='%s' LIMIT 1", tech_id, acronyme                           /* Where */
              );
    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    message = Recuperer_messageDB_suite( &db );
    if (message) Libere_DB_SQL ( &db );
    return(message);
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
     { Dls_data_set_MSG ( NULL, db->row[0], db->row[1], NULL, FALSE, atoi(db->row[2]) );
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
