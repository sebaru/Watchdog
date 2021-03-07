/******************************************************************************************************************************/
/* Watchdogd/Mnemo_Registre.c              Déclaration des fonctions pour la gestion des registre.c                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    22.03.2017 10:29:53 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_Registre.c
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
/* Ajouter_Modifier_mnemo_baseDB: Ajout ou modifie le mnemo en parametre                                                      */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_REGISTRE ( gchar *tech_id, gchar *acronyme, gchar *libelle_src, gchar *unite_src )
  { gchar *acro, *libelle, *unite;
    gchar requete[1024];
    gboolean retour;
    struct DB *db;

/******************************************** Préparation de la base du mnemo *************************************************/
    acro       = Normaliser_chaine ( acronyme );                                             /* Formatage correct des chaines */
    if ( !acro )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation acro impossible. Mnemo NOT added nor modified.", __func__ );
       return(FALSE);
     }

    libelle    = Normaliser_chaine ( libelle_src );                                          /* Formatage correct des chaines */
    if ( !libelle )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation libelle impossible. Mnemo NOT added nor modified.", __func__ );
       g_free(acro);
       return(FALSE);
     }

    if (unite_src)
     { unite = Normaliser_chaine ( unite_src );                                               /* Formatage correct des chaines */
       if ( !unite )
         { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                    "%s: Normalisation unite impossible. Mnemo NOT added nor modified.", __func__ );
           g_free(libelle);
           g_free(acro);
           return(FALSE);
         }
     } else unite = NULL;

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO mnemos_R SET tech_id='%s',acronyme='%s',libelle='%s'",
                tech_id, acro, libelle );
    g_free(libelle);
    g_free(acro);

    if (unite)
     { gchar add[128];
       g_snprintf( add, sizeof(add), ",unite='%s'", unite );
       g_strlcat ( requete, add, sizeof(requete) );
     }
    g_strlcat ( requete, " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle) ", sizeof(requete) );
    if (unite)
     { g_strlcat ( requete, ",unite=VALUES(unite)", sizeof(requete) );
       g_free(unite);
     }

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }
    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return (retour);
  }
/******************************************************************************************************************************/
/* Rechercher_AI_by_map_snips: Recupere l'AI lié au parametre snips                                                           */
/* Entrée: le map_snips a rechercher                                                                                          */
/* Sortie: la struct DB                                                                                                       */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemos_R_by_map_question_vocale ( struct DB **db_retour, gchar *map_snips )
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
               "SELECT tech_id, acronyme, libelle, map_question_vocale, map_reponse_vocale "
               "FROM mnemos_R"
               " WHERE map_question_vocale LIKE '%s' ", commande );
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
 gboolean Recuperer_mnemos_R_suite( struct DB **db_orig )
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
/* Charger_conf_R: Recupération de la conf de l'entrée analogique en parametre                                               */
/* Entrée: l'id a récupérer                                                                                                   */
/* Sortie: une structure hébergeant l'entrée analogique                                                                       */
/******************************************************************************************************************************/
 void Charger_confDB_Registre ( gchar *tech_id )
  { gchar requete[512], critere[80];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return;
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT m.tech_id, m.acronyme, m.valeur, m.unite, m.archivage FROM mnemos_R as m"
              );
    if (tech_id)
     { g_snprintf( critere, sizeof(critere), " WHERE tech_id='%s'", tech_id );
       g_strlcat ( requete, critere, sizeof(requete) );
     }

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return;
     }

    struct DLS_REGISTRE *reg;
    while (Recuperer_ligne_SQL(db))                                                        /* Chargement d'une ligne resultat */
     { reg = NULL;
       Dls_data_set_R ( NULL, db->row[0], db->row[1], (gpointer)&reg, atof(db->row[2]) );
       g_snprintf( reg->unite, sizeof(reg->unite), "%s", db->row[3] );
       reg->archivage = atoi(db->row[4]);
       Info_new( Config.log, Config.log_msrv, LOG_DEBUG, "%s: REGISTRE '%s:%s'=%f %s loaded (archivage=%d)", __func__,
                 db->row[0], db->row[1], atof(db->row[2]), reg->unite, reg->archivage );
     }
    Libere_DB_SQL( &db );
  }
/******************************************************************************************************************************/
/* Updater_confDB_R: Mise a jour des valeurs de R en base                                                                   */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_confDB_Registre( void )
  { gchar requete[200];
    GSList *liste;
    struct DB *db;
    gint cpt;

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connexion DB impossible", __func__ );
       return;
     }

    cpt = 0;
    liste = Partage->Dls_data_REGISTRE;
    while ( liste )
     { struct DLS_REGISTRE *r = (struct DLS_REGISTRE *)liste->data;
       g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE mnemos_R as m SET valeur='%f' "
                   "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                   r->valeur, r->tech_id, r->acronyme );
       Lancer_requete_SQL ( db, requete );
       liste = g_slist_next(liste);
       cpt++;
     }

    Libere_DB_SQL( &db );
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: %d REGISTRE updated", __func__, cpt );
  }
/******************************************************************************************************************************/
/* Dls_REGISTRE_to_json : Formate un bit au format JSON                                                                       */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_REGISTRE_to_json ( JsonNode *element, struct DLS_REGISTRE *bit )
  { Json_node_add_string ( element, "tech_id",  bit->tech_id );
    Json_node_add_string ( element, "acronyme", bit->acronyme );
    Json_node_add_double ( element, "valeur", bit->valeur );
    Json_node_add_string ( element, "unite", bit->unite );
    Json_node_add_int    ( element, "archivage", bit->archivage );
    Json_node_add_int    ( element, "last_arch", bit->last_arch );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
