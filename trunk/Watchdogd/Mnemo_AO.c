/******************************************************************************************************************************/
/* Watchdogd/Mnemo_AO.c        Déclaration des fonctions pour la gestion des Entrée TOR                                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    25.03.2019 14:16:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_AO.c
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
/* Ajouter_Modifier_mnemo_baseDB: Ajout ou modifie le mnemo en parametre                                                      */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_AO ( gchar *tech_id, gchar *acronyme, gchar *libelle_src )
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
                "INSERT INTO mnemos_AO SET tech_id='%s',acronyme='%s',libelle='%s' "
                " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle)",
                tech_id, acro, libelle );
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
/* Rechercher_AO: Recupération des champs de base de données pour le AO tech_id:acro en parametre                             */
/* Entrée: le tech_id et l'acronyme a récupérer                                                                               */
/* Sortie: la struct DB                                                                                                       */
/******************************************************************************************************************************/
 struct DB *Rechercher_AO ( gchar *tech_id, gchar *acronyme )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT m.map_host, m.map_thread, m.map_text"
                " FROM mnemos_AO as m"
                " WHERE m.tech_id='%s' AND m.acronyme='%s' LIMIT 1",
                tech_id, acronyme
              );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return(NULL);
     }
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Libere_DB_SQL( &db );
       return(NULL);
     }
    return(db);
  }
/******************************************************************************************************************************/
/* Rechercher_AO_by_text: Recupération des champs de base de données pour le AI par map                                       */
/* Entrée: le tech_id et l'acronyme a récupérer                                                                               */
/* Sortie: la struct DB                                                                                                       */
/******************************************************************************************************************************/
 gboolean Recuperer_mnemos_AO_by_text ( struct DB **db_retour, gchar *thread, gchar *text )
  { gchar requete[1024];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),
               "SELECT m.tech_id, m.acronyme, m.map_text, m.libelle, m.min, m.max, m.type, m.valeur "
               "FROM mnemos_AO as m "
               " WHERE (m.map_host='*' OR m.map_host LIKE '%s') AND (m.map_thread='*' OR m.map_thread LIKE '%s')"
               " AND m.map_text LIKE '%s'", g_get_host_name(), thread, text );

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
 gboolean Recuperer_mnemos_AO_suite( struct DB **db_orig )
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
/* Updater_confDB_AO: Met a jour les valeurs des Sortie analogiques dans la base de données                                   */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Updater_confDB_AO ( void )
  { gchar requete[200];
    struct DB *db;
    GSList *liste;
    gint cpt;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Connexion DB impossible", __func__ );
       return;
     }

    cpt=0;
    liste = Partage->Dls_data_AO;
    while ( liste )
     { struct DLS_AO *ao = (struct DLS_AO *)liste->data;
       g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE mnemos_AO as m SET valeur='%f' "
                   "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                   ao->val_avant_ech, ao->tech_id, ao->acronyme );
       Lancer_requete_SQL ( db, requete );
       liste = g_slist_next(liste);
       cpt++;
     }

    Libere_DB_SQL( &db );
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE, "%s: %d AO updated", __func__, cpt );
  }
/******************************************************************************************************************************/
/* Dls_AO_to_Json: Convertir un AO en JSON                                                                                    */
/* Entrée: l'AO en question et un pointeur recevant la taille du buffer créé                                                  */
/* Sortie: le buffer et sa taille                                                                                             */
/******************************************************************************************************************************/
 gchar *Dls_AO_to_Json ( struct DLS_AO *ao, gsize *taille_buf_p )
  { JsonBuilder *builder;
    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return(NULL);
     }
    json_builder_begin_object (builder);                                                       /* Création du noeud principal */
    Json_add_string ( builder, "tech_id",  ao->tech_id );
    Json_add_string ( builder, "acronyme", ao->acronyme );
    Json_add_double ( builder, "valeur",   ao->val_ech );
    json_builder_end_object (builder);                                                                        /* End Document */
    return(Json_get_buf ( builder, taille_buf_p ));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
