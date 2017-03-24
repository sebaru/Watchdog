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
                " FROM %s",
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
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_messageDB: DB connexion failed" );
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
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_messageDB: Normalisation libelle impossible" );
       return(-1);
     }
    libelle_audio = Normaliser_chaine ( msg->libelle_audio );                                /* Formatage correct des chaines */
    if (!libelle_audio)
     { g_free(libelle);
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_messageDB: Normalisation libelle_audio impossible" );
       return(-1);
     }
    libelle_sms = Normaliser_chaine ( msg->libelle_sms );                                    /* Formatage correct des chaines */
    if (!libelle_sms)
     { g_free(libelle);
       g_free(libelle_audio);
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_messageDB: Normalisation libelle_sms impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO %s(num,libelle,libelle_audio,libelle_sms,"
                "type,audio,bit_audio,enable,sms,time_repeat,dls_id,persist) VALUES "
                "(%d,'%s','%s','%s',%d,%d,%d,%d,%d,%d,%d)", NOM_TABLE_MSG, msg->num,
                libelle, libelle_audio, libelle_sms, msg->type,
                (msg->audio ? 1 : 0), msg->bit_audio, (msg->enable ? 1 : 0),
                msg->sms, msg->time_repeat, msg->dls_id, (msg->persist ? 1 : 0)
              );
    g_free(libelle);
    g_free(libelle_audio);
    g_free(libelle_sms);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_messageDB: DB connexion failed" );
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

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT msg.id,num,msg.libelle,msg.type,syn.libelle,audio,bit_audio,enable,groupe,page,sms,libelle_audio,libelle_sms,"
                "time_repeat,dls.id,dls.shortname,syn.id,persist"
                " FROM %s as msg"
                " INNER JOIN %s as dls ON msg.dls_id=dls.id"
                " INNER JOIN %s as syn ON dls.syn_id=syn.id"
                " WHERE %s"
                " ORDER BY groupe,page,num ",
                NOM_TABLE_MSG, NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE,/* From */
                (conditions ? conditions : "1=1") /* Where */
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
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_messageDB: DB connexion failed" );
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

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    msg = (struct CMD_TYPE_MESSAGE *)g_try_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!msg) Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_messageDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &msg->libelle,       db->row[2],  sizeof(msg->libelle ) );    /* Recopie dans la structure */
       memcpy( &msg->syn_libelle,   db->row[4],  sizeof(msg->syn_libelle) );
       memcpy( &msg->syn_groupe,    db->row[8],  sizeof(msg->syn_groupe  ) );
       memcpy( &msg->syn_page,      db->row[9],  sizeof(msg->syn_page    ) );
       memcpy( &msg->libelle_audio, db->row[11], sizeof(msg->libelle_audio) );
       memcpy( &msg->libelle_sms,   db->row[12], sizeof(msg->libelle_sms  ) );
       memcpy( &msg->dls_shortname, db->row[15], sizeof(msg->dls_shortname) );
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

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT msg.id,num,msg.libelle,msg.type,syn.libelle,audio,bit_audio,enable,groupe,page,sms,libelle_audio,libelle_sms,"
                "time_repeat,dls.id,dls.shortname,syn.id,persist"
                " FROM %s as msg"
                " INNER JOIN %s as dls ON msg.dls_id=dls.id"
                " INNER JOIN %s as syn ON dls.syn_id=syn.id"
                " WHERE num=%d LIMIT 1",
                NOM_TABLE_MSG, NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE,    /* From */
                num /* Where */
              );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_messageDB: DB connexion failed" );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    message = Recuperer_messageDB_suite( &db );
    Libere_DB_SQL ( &db );
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
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_messageDB_par_id: DB connexion failed" );
       return(NULL);
     }
   
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT msg.id,num,msg.libelle,msg.type,syn.libelle,audio,bit_audio,enable,groupe,page,sms,libelle_audio,libelle_sms,"
                "time_repeat,dls.id,dls.shortname,syn.id,persist"
                " FROM %s as msg"
                " INNER JOIN %s as dls ON msg.dls_id=dls.id"
                " INNER JOIN %s as syn ON dls.syn_id=syn.id"
                " WHERE msg.id=%d LIMIT 1",
                NOM_TABLE_MSG, NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE,    /* From */
                id /* Where */
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
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_messageDB: Normalisation libelle impossible" );
       return(-1);
     }
    libelle_audio = Normaliser_chaine ( msg->libelle_audio );       /* Formatage correct des chaines */
    if (!libelle_audio)
     { g_free(libelle);
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_messageDB: Normalisation libelle_audio impossible" );
       return(-1);
     }
    libelle_sms = Normaliser_chaine ( msg->libelle_sms );           /* Formatage correct des chaines */
    if (!libelle_sms)
     { g_free(libelle);
       g_free(libelle_audio);
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_messageDB: Normalisation libelle_sms impossible" );
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
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_messageDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    Updater_msg_run ( msg );
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
