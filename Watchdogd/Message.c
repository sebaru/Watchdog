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
 gint Mnemo_auto_create_MSG ( gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gint typologie, gint groupe )
  { gchar *libelle;
    gboolean retour;

    libelle = Normaliser_chaine ( libelle_src );                                             /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation libelle impossible", __func__ );
       return(-1);
     }

    retour = SQL_Write_new ( "INSERT INTO msgs SET deletable='%d', tech_id='%s',acronyme='%s',libelle='%s',audio_libelle='%s',"
                             "typologie='%d',sms_notification='0', groupe='%d' "
                             " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle), typologie=VALUES(typologie), groupe=VALUES(groupe)",
                             deletable, tech_id, acronyme, libelle, libelle, typologie, groupe
                           );
    g_free(libelle);

    struct DLS_MESSAGES *msg = Dls_data_MSG_lookup ( tech_id, acronyme );          /* Recherche ou Création du message en RAM */
    if (msg) { msg->groupe = groupe; }                           /* Pas de modification de l'etat, on vient de la compilation */

    return(retour);
  }
/******************************************************************************************************************************/
/* Charger_confDB_MSG: Recupération de la conf des messages                                                                   */
/* Entrée: néant                                                                                                              */
/* Sortie: le message est chargé en mémoire                                                                                   */
/******************************************************************************************************************************/
 static void Charger_confDB_un_MSG (JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { gint  *cpt_p    = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    gint   groupe   = Json_get_int    ( element, "groupe" );
    gboolean etat   = Json_get_int    ( element, "etat" );
    (*cpt_p)++;
    struct DLS_MESSAGES *msg = Dls_data_MSG_lookup ( tech_id, acronyme );          /* Recherche ou Création du message en RAM */
    if (msg) /* A l'init, on recopie tous les champs */
     { msg->groupe = groupe;
       msg->etat   = etat;
     }
    Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: MSG '%s:%s'=%d loaded", __func__, tech_id, acronyme, etat );
  }
/******************************************************************************************************************************/
/* Charger_confDB_MSG: Recupération de la conf des messages                                                                   */
/* Entrée: néant                                                                                                              */
/* Sortie: le message est chargé en mémoire                                                                                   */
/******************************************************************************************************************************/
 void Charger_confDB_MSG ( void )
  { gint cpt = 0;

    JsonNode *RootNode = Json_node_create ();
    if (RootNode)
     { SQL_Select_to_json_node ( RootNode, "msgs", "SELECT m.tech_id, m.acronyme, m.etat, m.groupe FROM msgs as m" );
       Json_node_foreach_array_element ( RootNode, "msgs", Charger_confDB_un_MSG, &cpt );
       json_node_unref ( RootNode );
     } else Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Memory Error", __func__ );

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: %d MSG loaded", __func__, cpt );
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
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_MESSAGE_to_json ( JsonNode *element, struct DLS_MESSAGES *bit )
  { Json_node_add_string ( element, "tech_id",  bit->tech_id );
    Json_node_add_string ( element, "acronyme", bit->acronyme );
    Json_node_add_bool   ( element, "etat",     bit->etat );
    Json_node_add_int    ( element, "groupe",   bit->groupe );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
