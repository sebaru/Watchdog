/**********************************************************************************************************/
/* Watchdogd/Message/Message.c        Déclaration des fonctions pour la gestion des message               */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 30 mai 2010 11:44:56 CEST */
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
 #include "Erreur.h"
 #include "Message_DB.h"

/**********************************************************************************************************/
/* Retirer_msgDB: Elimination d'un message                                                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_messageDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_MESSAGE *msg )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MSG, msg->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_msgDB: Ajout ou edition d'un message                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure msg                           */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_messageDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_MESSAGE *msg )
  { gchar *libelle, *objet, *libelle_audio, *libelle_sms;
    gchar requete[2048];

    libelle = Normaliser_chaine ( log, msg->libelle );                   /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_messageDB: Normalisation libelle impossible" );
       return(-1);
     }
    objet = Normaliser_chaine ( log, msg->objet );                       /* Formatage correct des chaines */
    if (!objet)
     { g_free(libelle);
       Info( log, DEBUG_DB, "Ajouter_messageDB: Normalisation objet impossible" );
       return(-1);
     }
    libelle_audio = Normaliser_chaine ( log, msg->libelle_audio );       /* Formatage correct des chaines */
    if (!libelle_audio)
     { g_free(libelle);
       g_free(objet);
       Info( log, DEBUG_DB, "Ajouter_messageDB: Normalisation libelle_audio impossible" );
       return(-1);
     }
    libelle_sms = Normaliser_chaine ( log, msg->libelle_sms );           /* Formatage correct des chaines */
    if (!libelle_sms)
     { g_free(libelle);
       g_free(objet);
       g_free(libelle_audio);
       Info( log, DEBUG_DB, "Ajouter_messageDB: Normalisation libelle_sms impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "INSERT INTO %s(num,libelle,libelle_audio,libelle_sms,"
                "type,num_syn,bit_voc,enable,objet,sms,type_voc,vitesse_voc,time_repeat) VALUES "
                "(%d,'%s','%s','%s', %d,%d,%d,%s,'%s',%d,%d,%d,%d)", NOM_TABLE_MSG, msg->num,
                libelle, libelle_audio, libelle_sms, msg->type,
                msg->num_syn, msg->bit_voc, (msg->enable ? "true" : "false"), objet,
                msg->sms, msg->type_voc, msg->vitesse_voc, msg->time_repeat
              );
    g_free(libelle);
    g_free(objet);
    g_free(libelle_audio);
    g_free(libelle_sms);

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_messageDB ( struct LOG *log, struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,num,libelle,type,num_syn,bit_voc,enable,objet,sms,libelle_audio,libelle_sms,"
                "type_voc,vitesse_voc,time_repeat"
                " FROM %s ORDER BY objet,num", NOM_TABLE_MSG );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_MESSAGE *Recuperer_messageDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_MESSAGE *msg;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    msg = (struct CMD_TYPE_MESSAGE *)g_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!msg) Info( log, DEBUG_MEM, "Recuperer_messageDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( msg->libelle,       db->row[2],  sizeof(msg->libelle) );      /* Recopie dans la structure */
       memcpy( msg->objet,         db->row[7],  sizeof(msg->objet  ) );
       memcpy( msg->libelle_audio, db->row[9],  sizeof(msg->libelle_audio) );
       memcpy( msg->libelle_sms,   db->row[10], sizeof(msg->libelle_sms) );
       msg->id          = atoi(db->row[0]);
       msg->num         = atoi(db->row[1]);
       msg->type        = atoi(db->row[3]);
       msg->num_syn     = atoi(db->row[4]);
       msg->bit_voc     = atoi(db->row[5]);
       msg->enable      = atoi(db->row[6]);
       msg->sms         = atoi(db->row[8]);
       msg->type_voc    = atoi(db->row[11]);
       msg->vitesse_voc = atoi(db->row[12]);
       msg->time_repeat = atoi(db->row[13]);
     }
    return(msg);
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_MESSAGE *Rechercher_messageDB ( struct LOG *log, struct DB *db, guint num )
  { gchar requete[256];
    struct CMD_TYPE_MESSAGE *msg;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,libelle,type,num_syn,bit_voc,enable,objet,sms,libelle_audio,libelle_sms,"
                "type_voc,vitesse_voc,time_repeat"
                " FROM %s WHERE num=%d",
                NOM_TABLE_MSG, num );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_msgDB: MSG non trouvé dans la BDD", num );
       return(NULL);
     }

    msg = g_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!msg)
     { Info( log, DEBUG_MEM, "Rechercher_msgDB: Mem error" ); }
    else
     { memcpy( msg->libelle,       db->row[1], sizeof(msg->libelle) );       /* Recopie dans la structure */
       memcpy( msg->objet,         db->row[6], sizeof(msg->objet  ) );
       memcpy( msg->libelle_audio, db->row[8], sizeof(msg->libelle_audio) );
       memcpy( msg->libelle_sms,   db->row[9], sizeof(msg->libelle_sms) );
       msg->id          = atoi(db->row[0]);
       msg->num         = num;
       msg->type        = atoi(db->row[2]);
       msg->num_syn     = atoi(db->row[3]);
       msg->bit_voc     = atoi(db->row[4]);
       msg->enable      = atoi(db->row[5]);
       msg->sms         = atoi(db->row[7]);
       msg->type_voc    = atoi(db->row[10]);
       msg->vitesse_voc = atoi(db->row[11]);
       msg->time_repeat = atoi(db->row[12]);
     }
    Liberer_resultat_SQL ( log, db );
    return(msg);
  }
/**********************************************************************************************************/
/* Rechercher_messageDB_par_id: Recupération du message dont l'id est en parametre                        */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_MESSAGE *Rechercher_messageDB_par_id ( struct LOG *log, struct DB *db, guint id )
  { gchar requete[200];
    struct CMD_TYPE_MESSAGE *msg;
    
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT num,libelle,type,num_syn,bit_voc,enable,objet,sms,libelle_audio,libelle_sms,"
                "type_voc,vitesse_voc,time_repeat"
                " FROM %s WHERE id=%d",
                NOM_TABLE_MSG, id );
    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_msgDB: MSG non trouvé dans la BDD", id );
       return(NULL);
     }

    msg = g_malloc0( sizeof(struct CMD_TYPE_MESSAGE) );
    if (!msg)
     { Info( log, DEBUG_MEM, "Rechercher_msgDB_par_id: Mem error" ); }
    else
     { memcpy( msg->libelle,       db->row[1], sizeof(msg->libelle) );       /* Recopie dans la structure */
       memcpy( msg->objet,         db->row[6], sizeof(msg->objet  ) );
       memcpy( msg->libelle_audio, db->row[8], sizeof(msg->libelle_audio) );
       memcpy( msg->libelle_sms,   db->row[9], sizeof(msg->libelle_sms) );
       msg->id          = id;
       msg->num         = atoi(db->row[0]);
       msg->type        = atoi(db->row[2]);
       msg->num_syn     = atoi(db->row[3]);
       msg->bit_voc     = atoi(db->row[4]);
       msg->enable      = atoi(db->row[5]);
       msg->sms         = atoi(db->row[7]);
       msg->type_voc    = atoi(db->row[10]);
       msg->vitesse_voc = atoi(db->row[11]);
       msg->time_repeat = atoi(db->row[12]);
     }
    Liberer_resultat_SQL ( log, db );
    return(msg);
  }
/**********************************************************************************************************/
/* Modifier_messageDB: Modification d'un message Watchdog                                                 */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_messageDB( struct LOG *log, struct DB *db, struct CMD_TYPE_MESSAGE *msg )
  { gchar requete[2048];
    gchar *libelle, *objet, *libelle_audio, *libelle_sms;

    libelle = Normaliser_chaine ( log, msg->libelle );                   /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Modifier_messageDB: Normalisation libelle impossible" );
       return(-1);
     }
    objet = Normaliser_chaine ( log, msg->objet );                       /* Formatage correct des chaines */
    if (!objet)
     { g_free(libelle);
       Info( log, DEBUG_DB, "Modifier_messageDB: Normalisation objet impossible" );
       return(-1);
     }
    libelle_audio = Normaliser_chaine ( log, msg->libelle_audio );       /* Formatage correct des chaines */
    if (!libelle_audio)
     { g_free(libelle);
       g_free(objet);
       Info( log, DEBUG_DB, "Modifier_messageDB: Normalisation libelle_audio impossible" );
       return(-1);
     }
    libelle_sms = Normaliser_chaine ( log, msg->libelle_sms );           /* Formatage correct des chaines */
    if (!libelle_sms)
     { g_free(libelle);
       g_free(objet);
       g_free(libelle_audio);
       Info( log, DEBUG_DB, "Modifier_messageDB: Normalisation libelle_sms impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "num=%d,libelle='%s',type=%d,num_syn=%d,bit_voc=%d,enable=%s,objet='%s',sms=%d,"
                "libelle_audio='%s',libelle_sms='%s',type_voc=%d,vitesse_voc=%d,time_repeat=%d "
                "WHERE id=%d",
                NOM_TABLE_MSG, msg->num, libelle, msg->type, msg->num_syn, msg->bit_voc,
                               (msg->enable ? "true" : "false"), objet, msg->sms,
                               libelle_audio, libelle_sms, msg->type_voc, msg->vitesse_voc,
                               msg->time_repeat,
                msg->id );
    g_free(libelle);
    g_free(objet);
    g_free(libelle_audio);
    g_free(libelle_sms);

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
