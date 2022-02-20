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
                "UPDATE %s as histo INNER JOIN msgs AS msg ON msg.msg_id = histo.msg_id"
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
/* Ajouter_histo_msgsDB: Ajoute un enregistrement MSGS de la base de données                                                  */
/* Entrée: la structure Json representant l'histo                                                                             */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Ajouter_histo_msgsDB ( JsonNode *histo )
  {
    gchar *libelle = Normaliser_chaine ( Json_get_string ( histo, "libelle" ) );             /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation libelle impossible", __func__ );
       return(FALSE);
     }

    SQL_Write_new ( "INSERT INTO %s(alive,msg_id,date_create,libelle)"
                    " VALUES ('%d','%d','%s','%s') ON DUPLICATE KEY UPDATE date_create=VALUES(`date_create`)",
                    NOM_TABLE_HISTO_MSGS, TRUE, Json_get_int ( histo, "msg_id" ), Json_get_string( histo, "date_create" ), libelle );
    g_free(libelle);
    /* Json_node_add_bool ( histo, "sql_last_id", Recuperer_last_ID_SQL ( db ); */
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Retirer_histo_msgsDB: Retire un enregistrement MSGS de la base de données                                                  */
/* Entrée: la structure Json representant l'histo                                                                             */
/* Sortie: false si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean Retirer_histo_msgsDB ( JsonNode *histo )
  {
    return( SQL_Write_new ( "UPDATE histo_msgs AS histo "
                            "INNER JOIN msgs ON msgs.msg_id = histo.msg_id "
                            "SET histo.alive=NULL,histo.date_fin='%s' "
                            "WHERE histo.alive=1 AND msgs.tech_id='%s' AND msgs.acronyme='%s' ",
                            Json_get_string ( histo, "date_fin" ),
                            Json_get_string ( histo, "tech_id" ), Json_get_string ( histo, "acronyme" ) )
          );
  }
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
                "SELECT histo.id, histo.alive, histo.libelle, msg.typologie, dls.syn_id,"
                "parent_syn.page, syn.page, histo.nom_ack, histo.date_create,"
                "histo.date_fixe, histo.date_fin, dls.shortname, msg.msg_id, msg.tech_id, msg.acronyme"
                " FROM %s as histo"
                " INNER JOIN %s as msg ON msg.msg_id = histo.msg_id"
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
/* Histo_msg_print_to_JSON: Convertir un histo au format JSON                                                                 */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 void Histo_msg_print_to_JSON ( JsonNode *RootNode, JsonNode *histo )
  { Json_node_add_bool   ( RootNode, "alive",           Json_get_bool  ( histo, "alive") ); /* Le message est-il encore d'actualité ? */
    Json_node_add_string ( RootNode, "nom_ack",         Json_get_string( histo, "nom_ack") );
    Json_node_add_string ( RootNode, "date_create",     Json_get_string( histo, "date_create") );
    Json_node_add_string ( RootNode, "date_fixe",       Json_get_string( histo, "date_fixe") );
    Json_node_add_string ( RootNode, "date_fin",        Json_get_string( histo, "date_fin") );
    Json_node_add_string ( RootNode, "tech_id",         Json_get_string( histo, "tech_id") );
    Json_node_add_string ( RootNode, "acronyme",        Json_get_string( histo, "acronyme") );
    Json_node_add_int    ( RootNode, "typologie",       Json_get_int   ( histo, "typologie") );
    Json_node_add_string ( RootNode, "dls_shortname",   Json_get_string( histo, "dls_shortname") );
    Json_node_add_string ( RootNode, "libelle",         Json_get_string( histo, "libelle") );
    Json_node_add_int    ( RootNode, "syn_id",          Json_get_int   ( histo, "syn_id") );
    Json_node_add_string ( RootNode, "syn_parent_page", Json_get_string( histo, "syn_parent_page") );
    Json_node_add_string ( RootNode, "syn_page",        Json_get_string( histo, "syn_page") );
    Json_node_add_string ( RootNode, "syn_libelle",     Json_get_string( histo, "syn_libelle") );
    Json_node_add_int    ( RootNode, "sms_notification",Json_get_int   ( histo, "sms_notiifcation") );
    Json_node_add_string ( RootNode, "audio_libelle",   Json_get_string( histo, "audio_libelle") );
    Json_node_add_string ( RootNode, "audio_profil",    Json_get_string( histo, "audio_profil") );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
