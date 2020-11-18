/******************************************************************************************************************************/
/* Watchdogd/Histo/Histo_msgs.c        Déclaration des fonctions pour la gestion des message                                  */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          ven 15 aoû 2003 13:02:48 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Histo_msgs.c
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
/* Modifier_Ajouter_histo_msgsDB: Ajout ou modifier un enregistrement MSGS de la base de données                              */
/* Entrée: un flag d'ajout et un enregistrement à modifier                                                                    */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Acquitter_histo_msgsDB ( gchar *tech_id, gchar *acronyme, gchar *username, gchar *date_fixe )
  { gchar *nom_ack;
    gchar requete[1024];
    gboolean retour;
    struct DB *db;

    if (!username) username = "unknown";
    nom_ack = Normaliser_chaine ( username );                                                /* Formatage correct des chaines */
    if (!nom_ack)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       return(FALSE);
     }
    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "UPDATE %s as histo INNER JOIN msgs as msg ON msg.id = histo.id_msg"
                " SET nom_ack='%s',date_fixe='%s'"
                " WHERE histo.alive=1 and msg.tech_id='%s' AND msg.acronyme='%s'",
                NOM_TABLE_HISTO_MSGS, nom_ack, date_fixe, tech_id, acronyme );
    g_free(nom_ack);

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
/* Modifier_Ajouter_histo_msgsDB: Ajout ou modifier un enregistrement MSGS de la base de données                              */
/* Entrée: un flag d'ajout et un enregistrement à modifier                                                                    */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Modifier_Ajouter_histo_msgsDB ( gboolean ajout, struct CMD_TYPE_HISTO *histo )
  { gchar *nom_ack;
    gchar requete[1024];
    gboolean retour;
    struct DB *db;

    if (ajout == TRUE)
     { nom_ack = Normaliser_chaine ( histo->nom_ack );                                       /* Formatage correct des chaines */
       if (!nom_ack)
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible", __func__ );
          return(FALSE);
        }

       g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "INSERT INTO %s(alive,id_msg,nom_ack,date_create)"
                   " VALUES ('%d','%d','%s','%s') ON DUPLICATE KEY UPDATE date_create=VALUES(`date_create`)",
                   NOM_TABLE_HISTO_MSGS, TRUE, histo->msg.id, nom_ack, histo->date_create );
     }
    else
     {
       if (histo->alive == TRUE)
        { nom_ack = Normaliser_chaine ( histo->nom_ack );                                    /* Formatage correct des chaines */
          if (!nom_ack)
           { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible", __func__ );
             return(FALSE);
           }
          g_snprintf( requete, sizeof(requete),                                                                /* Requete SQL */
                      "UPDATE %s SET nom_ack='%s',date_fixe='%s'"
                      " WHERE id='%d'", NOM_TABLE_HISTO_MSGS, nom_ack, histo->date_fixe, histo->id );
          g_free(nom_ack);
        }
       else
        { g_snprintf( requete, sizeof(requete),                                                                /* Requete SQL */
                      "UPDATE %s as histo SET histo.alive=NULL,histo.date_fin='%s'"
                      " WHERE histo.alive=1 AND histo.id_msg = '%d' ",
                      NOM_TABLE_HISTO_MSGS, histo->date_fin, histo->msg.id );
        }
     }

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    if (ajout == TRUE)
     { histo->id = Recuperer_last_ID_SQL ( db ); }
    Libere_DB_SQL(&db);
    return(retour);
  }
/******************************************************************************************************************************/
/* Ajouter_histo_msgsDB: Ajoute un enregistrement dans la base de données                                                     */
/* Entrée: L'enregistrement à modifier, en tant que structure complete                                                        */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Ajouter_histo_msgsDB ( struct CMD_TYPE_HISTO *histo )
  { return ( Modifier_Ajouter_histo_msgsDB ( TRUE, histo ) ); }
/******************************************************************************************************************************/
/* Modifier_histo_msgsDB: Modifie l'enregistrement MSGS en parametre                                                          */
/* Entrée: L'enregistrement à modifier, en tant que structure complete                                                        */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Modifier_histo_msgsDB ( struct CMD_TYPE_HISTO *histo )
  { return ( Modifier_Ajouter_histo_msgsDB ( FALSE, histo ) ); }
/******************************************************************************************************************************/
/* Recuperer_histo_msgsDB_alive: Recupération de l'ensemble des messages encore Alive dans le BDD                             */
/* Entrée: La base de données de travail                                                                                      */
/* Sortie: False si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Recuperer_histo_msgsDB_alive ( struct DB **db_retour )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT histo.id, histo.alive, msg.libelle, msg.typologie, dls.syn_id,"
                "parent_syn.page, syn.page, histo.nom_ack, histo.date_create,"
                "histo.date_fixe, histo.date_fin, dls.shortname, msg.id, msg.tech_id, msg.acronyme"
                " FROM %s as histo"
                " INNER JOIN %s as msg ON msg.id = histo.id_msg"
                " INNER JOIN %s as dls ON dls.tech_id = msg.tech_id"
                " INNER JOIN %s as syn ON syn.id = dls.syn_id"
                " INNER JOIN %s as parent_syn ON parent_syn.id = syn.parent_id"
                " WHERE alive = 1 ORDER BY histo.date_create",
                NOM_TABLE_HISTO_MSGS, NOM_TABLE_MSG, NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE, NOM_TABLE_SYNOPTIQUE        /* From */
              );

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
/* Rechercher_histo_msgsDB_by_id: Recupération du l'histo by id dans la BDD                                                   */
/* Entrée: La base de données de travail                                                                                      */
/* Sortie: False si probleme                                                                                                  */
/******************************************************************************************************************************/
 struct CMD_TYPE_HISTO *Rechercher_histo_msgsDB_by_id ( guint id )
  { struct CMD_TYPE_HISTO *histo;
    gchar requete[1024];
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT histo.id, histo.alive, msg.libelle, msg.typologie, dls.syn_id,"
                "syn.groupe, syn.page, histo.nom_ack, histo.date_create,"
                "histo.date_fixe, histo.date_fin, dls.shortname, msg.id, msg.tech_id, msg.acronyme"
                " FROM %s as histo"
                " INNER JOIN %s as msg ON msg.id = histo.id_msg"
                " INNER JOIN %s as dls ON dls.tech_id = msg.tech_id"
                " INNER JOIN %s as syn ON syn.id = dls.syn_id"
                " WHERE histo.id = %d",
                NOM_TABLE_HISTO_MSGS, NOM_TABLE_MSG, NOM_TABLE_DLS, NOM_TABLE_SYNOPTIQUE, /* From */
                id
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

    histo = Recuperer_histo_msgsDB_suite( &db );
    if (histo) Libere_DB_SQL ( &db );
    return(histo);
  }
/******************************************************************************************************************************/
/* Recuperer_liste_id_msgDB: Recupération de la liste des ids des messages                                                    */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 struct CMD_TYPE_HISTO *Recuperer_histo_msgsDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_HISTO *histo_msgs;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    histo_msgs = (struct CMD_TYPE_HISTO *)g_try_malloc0( sizeof(struct CMD_TYPE_HISTO) );
    if (!histo_msgs) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                              "%s: Erreur allocation mémoire", __func__ );
    else                                                                                         /* Recopie dans la structure */
     { g_snprintf( histo_msgs->msg.libelle,         sizeof(histo_msgs->msg.libelle),         "%s", db->row[2]  );
       g_snprintf( histo_msgs->msg.syn_parent_page, sizeof(histo_msgs->msg.syn_parent_page), "%s", db->row[5]  );
       g_snprintf( histo_msgs->msg.syn_page,        sizeof(histo_msgs->msg.syn_page),        "%s", db->row[6]  );
       g_snprintf( histo_msgs->nom_ack,             sizeof(histo_msgs->nom_ack),             "%s", db->row[7]  );
       g_snprintf( histo_msgs->msg.dls_shortname,   sizeof(histo_msgs->msg.dls_shortname),   "%s", db->row[11] );
       g_snprintf( histo_msgs->msg.tech_id,         sizeof(histo_msgs->msg.tech_id),         "%s", db->row[13] );
       g_snprintf( histo_msgs->msg.acronyme,        sizeof(histo_msgs->msg.acronyme),        "%s", db->row[14] );
       histo_msgs->id               = atoi(db->row[0]);
       histo_msgs->alive            = atoi(db->row[1]);
       histo_msgs->msg.typologie    = atoi(db->row[3]);
       histo_msgs->msg.syn_id       = atoi(db->row[4]);
       g_snprintf ( histo_msgs->date_create, sizeof(histo_msgs->date_create), "%s", db->row[8] );
       if (db->row[9])  g_snprintf ( histo_msgs->date_fixe,   sizeof(histo_msgs->date_fixe), "%s", db->row[9] );
       if (db->row[10]) g_snprintf ( histo_msgs->date_fin,    sizeof(histo_msgs->date_fin),  "%s", db->row[10] );
       histo_msgs->msg.id           = atoi(db->row[12]);
     }
    return(histo_msgs);
  }
/******************************************************************************************************************************/
/* Histo_msg_print_to_JSON: Convertir un histo au format JSON                                                                 */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 void Histo_msg_print_to_JSON ( JsonBuilder *builder, JsonNode *histo )
  { Json_add_bool   ( builder, "alive",           Json_get_bool( histo, "alive") ); /* Le message est-il encore d'actualité ? */
    Json_add_string ( builder, "nom_ack",         Json_get_string( histo, "nom_ack") );
    Json_add_string ( builder, "date_create",     Json_get_string( histo, "date_create") );
    Json_add_string ( builder, "date_fixe",       Json_get_string( histo, "date_fixe") );
    Json_add_string ( builder, "date_fin",        Json_get_string( histo, "date_fin") );
    Json_add_string ( builder, "tech_id",         Json_get_string( histo, "tech_id") );
    Json_add_string ( builder, "acronyme",        Json_get_string( histo, "acronyme") );
    Json_add_int    ( builder, "typologie",       Json_get_int( histo, "typologie") );
    Json_add_string ( builder, "dls_shortname",   Json_get_string( histo, "dls_shortname") );
    Json_add_string ( builder, "libelle",         Json_get_string( histo, "libelle") );
    Json_add_string ( builder, "libelle_sms",     Json_get_string( histo, "libelle_sms") );
    Json_add_int    ( builder, "syn_id",          Json_get_int( histo, "syn_id") );
    Json_add_string ( builder, "syn_parent_page", Json_get_string( histo, "syn_parent_page") );
    Json_add_string ( builder, "syn_page",        Json_get_string( histo, "syn_page") );
    Json_add_string ( builder, "syn_libelle",     Json_get_string( histo, "syn_libelle") );
    Json_add_int    ( builder, "sms",             Json_get_int( histo, "sms") );
    Json_add_string ( builder, "libelle_audio",   Json_get_string( histo, "libelle_audio") );
    Json_add_string ( builder, "profil_audio",    Json_get_string( histo, "profil_audio") );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
