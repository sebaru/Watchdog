/******************************************************************************************************************************/
/* Watchdogd/Message/Message.c        Déclaration des fonctions pour la gestion des message                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         jeu. 29 déc. 2011 14:55:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Message.c
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

 #define MSGS_SQL_SELECT  "SELECT mnemo.id,mnemo.libelle,"
                          "msg.type,msg.audio,msg.bit_audio,msg.enable," \
                          "msg.sms,msg.libelle_audio,msg.libelle_sms,msg.time_repeat,msg.persist,msg.is_mp3" \
                          " FROM mnemos_Msgs as msg" \
                          " INNER JOIN mnemos as mnemo ON msg.mnemo_id=mnemo.id" \
                          " INNER JOIN dls ON mnemo.dls_id=dls.id"

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Ajouter_messageDB: Ajout ou edition d'un message                                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                                               */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Ajouter_messageDB_for_dls ( gchar *dls_id, gchar *acronyme, gint type_msg, gchar *libelle )
  { gchar *acro, *libelle;
    gchar requete[2048];
    gboolean retour;
    struct DB *db;
    gint id;

/******************************************** Préparation de la base du mnemo *************************************************/
    acro       = Normaliser_chaine ( mnemo->mnemo_base.acronyme );                           /* Formatage correct des chaines */
    if ( !acro )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation impossible. Mnemo NOT added nor modified.", __func__ );
       return(FALSE);
     }

    libelle    = Normaliser_chaine ( mnemo->mnemo_base.libelle );                            /* Formatage correct des chaines */
    if ( !libelle )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation impossible. Mnemo NOT added nor modified.", __func__ );
       g_free(acro);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                "INSERT INTO mnemos SET type='%d',dls_id='%d',acronyme='%s',libelle='%s' "
                " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle)",
                MNEMO_MSG, mnemo->mnemo_base.type, mnemo->mnemo_base.dls_id, acro, libelle, acro_syn );
    g_free(libelle);
    g_free(acro);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

/********************************* Préparation des options et Envoi des requetes **********************************************/
    Lancer_requete_SQL ( db, "START TRANSACTION" );                                            /* Execution de la requete SQL */
    Lancer_requete_SQL ( db, requete );                                                        /* Execution de la requete SQL */
    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
               "INSERT INTO mnemos_Msgs "
               "SET mnemo_id=LAST_INSERT_ID(),type='%d',enable=1"
               " ON DUPLICATE KEY UPDATE type=VALUES(type)",
               type_msg );
    Lancer_requete_SQL ( db, requete );                                                        /* Execution de la requete SQL */
    retour = Lancer_requete_SQL ( db, "COMMIT;" );                                             /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return (retour);
  }
/******************************************************************************************************************************/
/* Recuperer_liste_id_messageDB: Recupération de la liste des ids des messages                                                */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 static struct DB_MESSAGE *Recuperer_messageDB_suite( struct DB **db_orig )
  { 
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    struct DB_MESSAGE *msg = g_try_malloc0( sizeof(struct DB_MESSAGE) );
    if (!msg) Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else
     { g_snprintf( msg->libelle,         sizeof(msg->libelle      ),   "%s", db->row[1]  );      /* Recopie dans la structure */
       g_snprintf( msg->libelle_audio,   sizeof(msg->libelle_audio),   "%s", db->row[7] );
       g_snprintf( msg->libelle_sms,     sizeof(msg->libelle_sms  ),   "%s", db->row[8] );
       msg->mnemo_id    = atoi(db->row[0]);
       msg->type        = atoi(db->row[2]);
       msg->audio       = atoi(db->row[3]);
       msg->bit_audio   = atoi(db->row[4]);
       msg->enable      = atoi(db->row[5]);
       msg->sms         = atoi(db->row[6]);
       msg->time_repeat = atoi(db->row[9]);
       msg->persist     = atoi(db->row[10]);
       msg->is_mp3      = atoi(db->row[11]);
     }
    return(msg);
  }
/******************************************************************************************************************************/
/* Rechercher_messageDB: Recupération du message dont le num est en parametre                                                 */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 struct DB_MESSAGE *Rechercher_messageDB ( gchar *tech_id, gchar *acro )
  { struct DB_MESSAGE *message;
    gchar requete[512];
    struct DB *db;

    g_snprintf( requete, sizeof(requete), MSGS_SQL_SELECT                                                      /* Requete SQL */
                " WHERE tech_id='%s' AND acronyme='%s'", tech_id, acro                                               /* Where */
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
/*----------------------------------------------------------------------------------------------------------------------------*/
