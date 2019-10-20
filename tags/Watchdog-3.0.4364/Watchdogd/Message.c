/******************************************************************************************************************************/
/* Watchdogd/Message/Message.c        Déclaration des fonctions pour la gestion des message                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         jeu. 29 déc. 2011 14:55:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Message.c
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

 #define MSGS_SQL_SELECT  "SELECT msg.id,num,msg.libelle,msg.type,syn.libelle,audio,bit_audio,enable,parent_syn.page,syn.page," \
                          "sms,libelle_audio,libelle_sms,time_repeat,dls.id,dls.shortname,syn.id,persist,is_mp3" \
                          " FROM msgs as msg" \
                          " INNER JOIN dls as dls ON msg.dls_id=dls.id" \
                          " INNER JOIN syns as syn ON dls.syn_id=syn.id" \
                          " INNER JOIN syns as parent_syn ON parent_syn.id=syn.parent_id"

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Updater_msg_run: Update la running config du message en parametre                                                          */
/* Entrée: une structure identifiant le message                                                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Updater_msg_run ( struct CMD_TYPE_MESSAGE *msg )
  { if ( msg && msg->num < NBR_MESSAGE_ECRITS )
     { Partage->g[msg->num].persist = msg->persist;
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Charger_messages: Chargement de la configuration des messages depuis la DB vers la running config                          */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_messages ( void )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connexion DB impossible", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT num,persist"
                " FROM %s WHERE num<>'-1'",
                NOM_TABLE_MSG );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while ( Recuperer_ligne_SQL(db) )                                                      /* Chargement d'une ligne resultat */
     { gint num;
       num = atoi( db->row[0] );
       if (num < NBR_MESSAGE_ECRITS)
        { Partage->g[num].persist = atoi( db->row[1] );
          Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                    "%s: Chargement config MSG[%04d]", __func__, num );
        }
       else
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
			       "%s: num (%d) out of range (max=%d)", __func__, num, NBR_MESSAGE_ECRITS ); }
     }
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: DB reloaded", __func__ );
    Libere_DB_SQL (&db);
  }
/******************************************************************************************************************************/
/* Retirer_messageDB: Elimination d'un message                                                                                */
/* Entrée: une structure identifiant le message a retirer                                                                     */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Retirer_messageDB ( struct CMD_TYPE_MESSAGE *msg )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    if (msg->id < 10000) return(FALSE);

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MSG, msg->id );

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Retirer_messageDB: Elimination d'un message                                                                                */
/* Entrée: une structure identifiant le message a retirer                                                                     */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Modifier_messageDB_set_mp3 ( gint id, gboolean valeur )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE %s SET is_mp3='%d' WHERE id='%d'", NOM_TABLE_MSG, valeur, id );

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter_messageDB: Ajout ou edition d'un message                                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gint Ajouter_messageDB ( struct CMD_TYPE_MESSAGE *msg )
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
                "INSERT INTO %s(num,libelle,libelle_audio,libelle_sms,"
                "type,audio,bit_audio,enable,sms,time_repeat,dls_id,persist) VALUES "
                "(%d,'%s','%s','%s',%d,%d,%d,%d,%d,%d,%d,%d)", NOM_TABLE_MSG, msg->num,
                libelle, libelle_audio, libelle_sms, msg->type,
                (msg->audio ? 1 : 0), msg->bit_audio, (msg->enable ? 1 : 0),
                msg->sms, msg->time_repeat, msg->dls_id, (msg->persist ? 1 : 0)
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
    Updater_msg_run ( msg );
    return(id);
  }
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
                "INSERT INTO %s SET num='-1',acronyme='%s',libelle='%s',libelle_audio='%s',libelle_sms='%s',"
                "type='%d',audio='0',bit_audio='0',enable='1',sms='0',time_repeat='0',dls_id='%d',persist='0' "
                " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle), type=VALUES(type)", NOM_TABLE_MSG, msg->acronyme,
                libelle, libelle, libelle, msg->type, msg->dls_id
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
    Updater_msg_run ( msg );
    return(id);
  }
/******************************************************************************************************************************/
/* Recuperer_messageDB_with_conditions: Recupération de la liste des ids des messages avec conditions en paramètre            */
/* Entrée: une database et des conditions                                                                                     */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Recuperer_messageDB_with_conditions ( struct DB **db_retour, gchar *conditions, gint start, gint length )
  { gchar requete[512], critere[80];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete), MSGS_SQL_SELECT                                                      /* Requete SQL */
                " WHERE %s AND num<>'-1' ORDER BY parent_syn.page,syn.page,num ", (conditions ? conditions : "1=1")  /* Where */
              );

    if (start != -1 && length != -1)                                                 /* Critere d'affichage (offset et count) */
     { g_snprintf( critere, sizeof(critere), " LIMIT %d,%d", start, length );
       g_strlcat( requete, critere, sizeof(requete) );
     }
    else if (length!=-1)
     { g_snprintf( critere, sizeof(critere), " LIMIT %d", length );
       g_strlcat( requete, critere, sizeof(requete) );
     }

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
/* Recuperer_liste_id_messageDB: Recupération de la liste des ids des messages                                                */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 gboolean Recuperer_messageDB ( struct DB **db_retour )
  { return( Recuperer_messageDB_with_conditions ( db_retour, NULL, -1, -1 ) ); }
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
     { g_snprintf( msg->libelle,         sizeof(msg->libelle      ),   "%s", db->row[2]  );      /* Recopie dans la structure */
       g_snprintf( msg->syn_libelle,     sizeof(msg->syn_libelle  ),   "%s", db->row[4]  );
       g_snprintf( msg->syn_parent_page, sizeof(msg->syn_parent_page), "%s", db->row[8]  );
       g_snprintf( msg->syn_page,        sizeof(msg->syn_page     ),   "%s", db->row[9]  );
       g_snprintf( msg->libelle_audio,   sizeof(msg->libelle_audio),   "%s", db->row[11] );
       g_snprintf( msg->libelle_sms,     sizeof(msg->libelle_sms  ),   "%s", db->row[12] );
       g_snprintf( msg->dls_shortname,   sizeof(msg->dls_shortname),   "%s", db->row[15] );
       msg->id          = atoi(db->row[0]);
       msg->num         = atoi(db->row[1]);
       msg->type        = atoi(db->row[3]);
       msg->audio       = atoi(db->row[5]);
       msg->bit_audio   = atoi(db->row[6]);
       msg->enable      = atoi(db->row[7]);
       msg->sms         = atoi(db->row[10]);
       msg->time_repeat = atoi(db->row[13]);
       msg->dls_id      = atoi(db->row[14]);
       msg->syn_id      = atoi(db->row[16]);
       msg->persist     = atoi(db->row[17]);
       msg->is_mp3      = atoi(db->row[18]);
     }
    return(msg);
  }
/******************************************************************************************************************************/
/* Rechercher_messageDB: Recupération du message dont le num est en parametre                                                 */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 struct CMD_TYPE_MESSAGE *Rechercher_messageDB ( guint num )
  { struct CMD_TYPE_MESSAGE *message;
    gchar requete[512];
    struct DB *db;

    g_snprintf( requete, sizeof(requete), MSGS_SQL_SELECT                                                      /* Requete SQL */
                " WHERE num=%d LIMIT 1", num                                                                         /* Where */
              );

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    message = Recuperer_messageDB_suite( &db );
    if (message) Libere_DB_SQL ( &db );
    return(message);
  }
/******************************************************************************************************************************/
/* Rechercher_messageDB_par_id: Recupération du message dont l'id est en parametre                                            */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 struct CMD_TYPE_MESSAGE *Rechercher_messageDB_par_id ( guint id )
  { struct CMD_TYPE_MESSAGE *message;
    gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete), MSGS_SQL_SELECT                                                      /* Requete SQL */
                " WHERE msg.id=%d LIMIT 1", id                                                                       /* Where */
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
/* Modifier_messageDB: Modification d'un message Watchdog                                                                     */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                                                */
/* Sortie: -1 si pb, id sinon                                                                                                 */
/******************************************************************************************************************************/
 gboolean Modifier_messageDB( struct CMD_TYPE_MESSAGE *msg )
  { gchar *libelle, *libelle_audio, *libelle_sms;
    gchar requete[2048];
    gboolean retour;
    struct DB *db;

    libelle = Normaliser_chaine ( msg->libelle );                   /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation libelle impossible", __func__ );
       return(-1);
     }
    libelle_audio = Normaliser_chaine ( msg->libelle_audio );       /* Formatage correct des chaines */
    if (!libelle_audio)
     { g_free(libelle);
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation libelle_audio impossible", __func__ );
       return(-1);
     }
    libelle_sms = Normaliser_chaine ( msg->libelle_sms );           /* Formatage correct des chaines */
    if (!libelle_sms)
     { g_free(libelle);
       g_free(libelle_audio);
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation libelle_sms impossible", __func__ );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "
                "num=%d,libelle='%s',type=%d,audio=%d,bit_audio=%d,enable=%d,sms=%d,"
                "libelle_audio='%s',libelle_sms='%s',time_repeat=%d,dls_id=%d,persist=%d "
                "WHERE id=%d",
                NOM_TABLE_MSG, msg->num, libelle, msg->type, (msg->audio ? 1 : 0), msg->bit_audio,
                               (msg->enable ? 1 : 0), msg->sms,
                               libelle_audio, libelle_sms, msg->time_repeat, msg->dls_id, (msg->persist ? 1 : 0),
                msg->id );
    g_free(libelle);
    g_free(libelle_audio);
    g_free(libelle_sms);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    Updater_msg_run ( msg );
    return(retour);
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
     { Dls_data_set_MSG ( db->row[0], db->row[1], NULL, atoi(db->row[2]) );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: MSG '%s:%s'=%d loaded", __func__,
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
                   "UPDATE msgs as m INNER JOIN dls as d ON m.dls_id = d.id SET m.etat='%d' "
                   "WHERE d.tech_id='%s' AND m.acronyme='%s';",
                   msg->etat, msg->tech_id, msg->acronyme );
       Lancer_requete_SQL ( db, requete );
       liste = g_slist_next(liste);
       cpt++;
     }

    Libere_DB_SQL( &db );
    Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: %d MSG updated", __func__, cpt );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
