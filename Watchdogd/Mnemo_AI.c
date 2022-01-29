/******************************************************************************************************************************/
/* Watchdogd/Mnemo_AI.c        Déclaration des fonctions pour la gestion des Analog Input                                     */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 18 avr 2009 13:30:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_AI.c
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
 #include <locale.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Mnemo_auto_create_AI_by_tech_id: Ajoute un mnemonique dans la base via le tech_id                                          */
/* Entrée: le tech_id, l'acronyme, le libelle et l'unite                                                                      */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_AI ( gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src )
  { gchar *acro, *libelle=NULL, *unite=NULL;
    gchar requete[1024];
    gboolean retour;

/******************************************** Préparation de la base du mnemo *************************************************/
    acro       = Normaliser_chaine ( acronyme );                                             /* Formatage correct des chaines */
    if ( !acro )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation acro impossible. Mnemo NOT added nor modified.", __func__ );
       return(FALSE);
     }

    if (libelle_src)
     { libelle    = Normaliser_chaine ( libelle_src );                                        /* Formatage correct des chaines */
       if ( !libelle )
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                   "%s: Normalisation libelle impossible. Mnemo NOT added nor modified.", __func__ );
          g_free(acro);
          return(FALSE);
        }
     }

    if(unite_src)
     { unite = Normaliser_chaine ( unite_src );                                              /* Formatage correct des chaines */
       if ( !unite )
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                   "%s: Normalisation unite impossible. Mnemo NOT added nor modified.", __func__ );
          g_free(acro);
          if (libelle) g_free(libelle);
          return(FALSE);
        }
     } else unite = NULL;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO mnemos_AI SET deletable='%d', tech_id='%s',acronyme='%s' ",
                deletable, tech_id, acro );
    g_free(acro);

    if (libelle)
     { gchar add[128];
       g_snprintf( add, sizeof(add), ",libelle='%s'", libelle );
       g_strlcat ( requete, add, sizeof(requete) );
     }

    if (unite)
     { gchar add[128];
       g_snprintf( add, sizeof(add), ",unite='%s'", unite );
       g_strlcat ( requete, add, sizeof(requete) );
     }

    g_strlcat ( requete, " ON DUPLICATE KEY UPDATE acronyme=VALUES(acronyme) ", sizeof(requete) );

    if (unite)
     { g_strlcat ( requete, ",unite=VALUES(unite)", sizeof(requete) );
       g_free(unite);
     }

    if (libelle)
     { g_strlcat ( requete, ",libelle=VALUES(libelle)", sizeof(requete) );
       g_free(libelle);
     }

    retour = SQL_Write_new ( requete );
    return (retour);
  }
/******************************************************************************************************************************/
/* Mnemo_create_json_AI: Créé un JSON pour une AI                                                                             */
/* Entrée: la structure SUBPROCESS, les parametres de l'AI                                                                    */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 JsonNode *Mnemo_create_thread_AI ( gchar *thread_tech_id, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage )
  { JsonNode *node = Json_node_create();
    if (!node) return(NULL);
    Json_node_add_string ( node, "thread_tech_id", thread_tech_id );
    Json_node_add_string ( node, "thread_acronyme", thread_acronyme );
    Json_node_add_string ( node, "libelle", libelle );
    Json_node_add_string ( node, "unite", unite );
    Json_node_add_int    ( node, "archivage", archivage );
    return(node);
  }
/******************************************************************************************************************************/
/* Rechercher_AI_by_text: Recupération des champs de base de données pour le AI par map                                       */
/* Entrée: le tech_id et l'acronyme a récupérer                                                                               */
/* Sortie: la struct DB                                                                                                       */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemos_AI_by_tag ( struct DB **db_retour, gchar *tech_id, gchar *tag )
  { gchar requete[1024];
    gchar *commande;
    gboolean retour;
    struct DB *db;

    commande = Normaliser_chaine ( tag );
    if (!commande)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible tag", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),
               "SELECT m.tech_id, m.acronyme, m.map_tag, m.libelle "
               "FROM mnemos_AI as m "
               " WHERE m.map_tech_id='%s' AND m.map_tag LIKE '%s'", tech_id, commande );
    g_free(commande);

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
/* Rechercher_AI_by_map_snips: Recupere l'AI lié au parametre snips                                                           */
/* Entrée: le map_snips a rechercher                                                                                          */
/* Sortie: la struct DB                                                                                                       */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemos_AI_by_map_question_vocale ( struct DB **db_retour, gchar *map_snips )
  { gchar requete[1024];
    gchar *commande;
    gboolean retour;
    struct DB *db;

    commande = Normaliser_chaine ( map_snips );
    if (!commande)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible commande", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),
               "SELECT m.tech_id, m.acronyme, m.libelle, m.map_question_vocale, m.map_reponse_vocale "
               "FROM mnemos_AI as m"
               " WHERE m.map_question_vocale LIKE '%s'", commande );
    g_free(commande);

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
/* Recuperer_mnemo_base_DB_suite: Fonction itérative de récupération des mnémoniques de base                                  */
/* Entrée: un pointeur sur la connexion de baase de données                                                                   */
/* Sortie: une structure nouvellement allouée                                                                                 */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemos_AI_suite( struct DB **db_orig )
  { struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(FALSE);
     }

    return(TRUE);                                                                                    /* Résultat dans db->row */
  }
/******************************************************************************************************************************/
/* Charger_conf_ai: Recupération de la conf de l'entrée analogique en parametre                                               */
/* Entrée: l'id a récupérer                                                                                                   */
/* Sortie: une structure hébergeant l'entrée analogique                                                                       */
/******************************************************************************************************************************/
 void Charger_confDB_AI ( gchar *tech_id, gchar *acronyme )
  { gchar requete[512];
    struct DLS_AI *ai;
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT a.tech_id, a.acronyme, a.valeur, a.min, a.max, a.type, a.unite, a.archivage"
                " FROM mnemos_AI as a"
              );
    if (tech_id && acronyme)
     { gchar critere[128];
       g_snprintf( critere, sizeof(critere), " WHERE tech_id='%s' AND acronyme='%s'", tech_id, acronyme );
       g_strlcat ( requete, critere, sizeof(requete) );
     }

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    while (Recuperer_ligne_SQL(db))                                                        /* Chargement d'une ligne resultat */
     { ai = NULL;
       Dls_data_set_AI ( db->row[0], db->row[1], (void *)&ai, atof(db->row[2]), FALSE );
       ai->min       = atof(db->row[3]);
       ai->max       = atof(db->row[4]);
       ai->type      = atoi(db->row[5]);
       g_snprintf( ai->unite, sizeof(ai->unite), "%s", db->row[6] );
       ai->archivage = atoi(db->row[7]);
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "%s: AI '%s:%s'=%f %s loaded", __func__,
                 ai->tech_id, ai->acronyme, ai->valeur, ai->unite );
     }
    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Updater_confDB_R: Mise a jour des valeurs de R en base                                                                   */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_confDB_AI( void )
  { GSList *liste;
    gint cpt;

    cpt = 0;
    liste = Partage->Dls_data_AI;
    while ( liste )
     { struct DLS_AI *ai = (struct DLS_AI *)liste->data;
       SQL_Write_new( "UPDATE mnemos_AI as m SET valeur='%f' "
                      "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                      ai->valeur, ai->tech_id, ai->acronyme );
       liste = g_slist_next(liste);
       cpt++;
     }

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: %d AI updated", __func__, cpt );
  }
/******************************************************************************************************************************/
/* Dls_AI_to_json : Formate un bit au format JSON                                                                             */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_AI_to_json ( JsonNode *element, struct DLS_AI *bit )
  { Json_node_add_string ( element, "tech_id",      bit->tech_id );
    Json_node_add_string ( element, "acronyme",     bit->acronyme );
    Json_node_add_double ( element, "valeur_min",   bit->min );
    Json_node_add_double ( element, "valeur_max",   bit->max );
    Json_node_add_double ( element, "valeur",       bit->valeur );
    Json_node_add_string ( element, "unite",        bit->unite );
    Json_node_add_int    ( element, "type",         bit->type );
    Json_node_add_int    ( element, "in_range",     bit->inrange );
    Json_node_add_int    ( element, "last_arch",    bit->last_arch );
    Json_node_add_int    ( element, "archivage",    bit->archivage );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
