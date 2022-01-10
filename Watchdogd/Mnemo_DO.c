/******************************************************************************************************************************/
/* Watchdogd/Mnemo_DO.c        Déclaration des fonctions pour la gestion des Entrée TOR                                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    25.03.2019 14:16:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_DO.c
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
/* Mnemo_auto_create_DO: Ajoute un mnemo DO en base                                                                           */
/* Entrée: les parametres du mnemo                                                                                            */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_DO ( gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
  { gchar *acro, *libelle;
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

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "INSERT INTO mnemos_DO SET deletable='%d', tech_id='%s',acronyme='%s',libelle='%s' "
                " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle)",
                deletable, tech_id, acro, libelle );
    g_free(libelle);
    g_free(acro);

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
/* Recuperer_mnemo_baseDB_by_command_text: Recupération de la liste des mnemo par command_text                                */
/* Entrée: un pointeur vers une nouvelle connexion de base de données, le critere de recherche                                */
/* Sortie: FALSE si erreur                                                      ********************                          */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemos_DO_by_tag ( struct DB **db_retour, gchar *tech_id, gchar *tag )
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
               "FROM mnemos_DO as m"
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
/* Recuperer_mnemo_base_DB_suite: Fonction itérative de récupération des mnémoniques de base                                  */
/* Entrée: un pointeur sur la connexion de baase de données                                                                   */
/* Sortie: une structure nouvellement allouée                                                                                 */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemos_DO_suite( struct DB **db_orig )
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
/* Dls_DO_to_json : Formate un bit au format JSON                                                                             */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_DO_to_json ( JsonNode *element, struct DLS_DO *bit )
  { Json_node_add_string ( element, "tech_id",  bit->tech_id );
    Json_node_add_string ( element, "acronyme", bit->acronyme );
    Json_node_add_bool   ( element, "etat",     bit->etat );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
