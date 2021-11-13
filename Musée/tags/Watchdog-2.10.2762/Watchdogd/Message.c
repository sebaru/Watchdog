/**********************************************************************************************************/
/* Watchdogd/Message/Message.c        Déclaration des fonctions pour la gestion des message               */
/* Projet WatchDog version 2.0       Gestion d'habitat                     jeu. 29 déc. 2011 14:55:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

/**********************************************************************************************************/
/* Retirer_messageDB: Elimination d'un message                                                            */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_messageDB ( struct CMD_TYPE_MESSAGE *msg )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    if (msg->id < 10000) return(FALSE);

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MSG, msg->id );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_messageDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_messageDB: Ajout ou edition d'un message                                                       */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_messageDB ( struct CMD_TYPE_MESSAGE *msg )
  { gchar *libelle, *libelle_audio, *libelle_sms;
    gchar requete[2048];
    gboolean retour;
    struct DB *db;
    gint id;

    libelle = Normaliser_chaine ( msg->libelle );                   /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_messageDB: Normalisation libelle impossible" );
       return(-1);
     }
    libelle_audio = Normaliser_chaine ( msg->libelle_audio );       /* Formatage correct des chaines */
    if (!libelle_audio)
     { g_free(libelle);
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_messageDB: Normalisation libelle_audio impossible" );
       return(-1);
     }
    libelle_sms = Normaliser_chaine ( msg->libelle_sms );           /* Formatage correct des chaines */
    if (!libelle_sms)
     { g_free(libelle);
       g_free(libelle_audio);
       Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Ajouter_messageDB: Normalisation libelle_sms impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(num,libelle,libelle_audio,libelle_sms,"
                "type,id_syn,bit_voc,enable,sms,type_voc,vitesse_voc,time_repeat) VALUES "
                "(%d,'%s','%s','%s',%d,%d,%d,%s,%d,%d,%d,%d)", NOM_TABLE_MSG, msg->num,
                libelle, libelle_audio, libelle_sms, msg->type,
                msg->id_syn, msg->bit_voc, (msg->enable ? "true" : "false"),
                msg->sms, msg->type_voc, msg->vitesse_voc, msg->time_repeat
              );
    g_free(libelle);
    g_free(libelle_audio);
    g_free(libelle_sms);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_messageDB: DB connexion failed" );
       return(-1);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if ( retour == FALSE )
     { Libere_DB_SQL(&db); 
       return(-1);
     }
    id = Recuperer_last_ID_SQL ( db );
    Libere_DB_SQL(&db);
    return(id);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_messageDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_messageDB ( struct DB **db_retour )
  { gchar requete[256];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,num,%s.libelle,type,id_syn,bit_voc,enable,groupe,page,sms,libelle_audio,libelle_sms,"
                "type_voc,vitesse_voc,time_repeat"
                " FROM %s,%s"
                " WHERE %s.id_syn = %s.id"
                " ORDER BY groupe,page,num",
                NOM_TABLE_MSG, NOM_TABLE_MSG,
                NOM_TABLE_MSG, NOM_TABLE_SYNOPTIQUE, /* From */
                NOM_TABLE_MSG, NOM_TABLE_SYNOPTIQUE /* Where */
              );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_messageDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_messageDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
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
       memcpy( &msg->groupe,        db->row[7],  sizeof(msg->groupe  ) );
       memcpy( &msg->page,          db->row[8],  sizeof(msg->page    ) );
       memcpy( &msg->libelle_audio, db->row[10], sizeof(msg->libelle_audio) );
       memcpy( &msg->libelle_sms,   db->row[11], sizeof(msg->libelle_sms) );
       msg->id          = atoi(db->row[0]);
       msg->num         = atoi(db->row[1]);
       msg->type        = atoi(db->row[3]);
       msg->id_syn     = atoi(db->row[4]);
       msg->bit_voc     = atoi(db->row[5]);
       msg->enable      = atoi(db->row[6]);
       msg->sms         = atoi(db->row[9]);
       msg->type_voc    = atoi(db->row[12]);
       msg->vitesse_voc = atoi(db->row[13]);
       msg->time_repeat = atoi(db->row[14]);
     }
    return(msg);
  }
/**********************************************************************************************************/
/* Rechercher_messageDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_MESSAGE *Rechercher_messageDB ( guint num )
  { struct CMD_TYPE_MESSAGE *message;
    gchar requete[512];
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT %s.id,num,%s.libelle,type,id_syn,bit_voc,enable,groupe,page,sms,libelle_audio,libelle_sms,"
                "type_voc,vitesse_voc,time_repeat"
                " FROM %s,%s"
                " WHERE %s.id_syn = %s.id AND num=%d LIMIT 1",
                NOM_TABLE_MSG, NOM_TABLE_MSG,
                NOM_TABLE_MSG, NOM_TABLE_SYNOPTIQUE,     /* From */
                NOM_TABLE_MSG, NOM_TABLE_SYNOPTIQUE, num /* Where */
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
/**********************************************************************************************************/
/* Rechercher_messageDB_par_id: Recupération du message dont l'id est en parametre                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
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
                "SELECT %s.id,num,%s.libelle,type,id_syn,bit_voc,enable,groupe,page,sms,libelle_audio,"
                "libelle_sms,type_voc,vitesse_voc,time_repeat"
                " FROM %s,%s"
                " WHERE %s.id_syn = %s.id AND %s.id=%d LIMIT 1",
                NOM_TABLE_MSG, NOM_TABLE_MSG,
                NOM_TABLE_MSG, NOM_TABLE_SYNOPTIQUE,     /* From */
                NOM_TABLE_MSG, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_MSG, id /* Where */
              );
    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    message = Recuperer_messageDB_suite( &db );
    if (message) Libere_DB_SQL ( &db );
    return(message);
  }
/**********************************************************************************************************/
/* Modifier_messageDB: Modification d'un message Watchdog                                                 */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
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
                "num=%d,libelle='%s',type=%d,id_syn=%d,bit_voc=%d,enable=%s,sms=%d,"
                "libelle_audio='%s',libelle_sms='%s',type_voc=%d,vitesse_voc=%d,time_repeat=%d "
                "WHERE id=%d",
                NOM_TABLE_MSG, msg->num, libelle, msg->type, msg->id_syn, msg->bit_voc,
                               (msg->enable ? "true" : "false"), msg->sms,
                               libelle_audio, libelle_sms, msg->type_voc, msg->vitesse_voc,
                               msg->time_repeat,
                msg->id );
    g_free(libelle);
    g_free(libelle_audio);
    g_free(libelle_sms);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_messageDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/*--------------------------------------------------------------------------------------------------------*/
