/******************************************************************************************************************************/
/* Watchdogd/Utilisateur/Utilisateur.c    Interface DB mots de passe pour watchdog2.0                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          ven 03 avr 2009 20:35:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Utilisateur.c
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
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>
 #include <crypt.h>

/*********************************************** Prototypes des fonctions *****************************************************/
 #include "watchdogd.h"
 #include "Utilisateur_DB.h"

 extern struct SSRV_CONFIG Cfg_ssrv;
/******************************************************************************************************************************/
/* Tester_level_util: renvoie true si l'utilisateur fait partie du groupe en parametre                                       */
/* Entrées: une structure UTIL et un id de groupe                                                                             */
/* Sortie: false si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Tester_level_util( struct CMD_TYPE_UTILISATEUR *util, guint level )
  { if (!util) return(FALSE);
    if ( util->id == UID_ROOT) return(TRUE);                                             /* Le tech est dans tous les groupes */
    if (!util->enable) return(FALSE);
    return( util->access_level >= level );
  }
/******************************************************************************************************************************/
/* Check_utilisateur_password: Vérifie le mot de passe fourni                                                                 */
/* Entrées: une structure util, un code confidentiel                                                                          */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Check_utilisateur_password( struct CMD_TYPE_UTILISATEUR *util, gchar *pwd )
  { struct crypt_data *Data;
    gboolean retour=FALSE;
    gchar *result;

    Data = (struct crypt_data *)g_try_malloc0(sizeof(struct crypt_data));
    if( !Data)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Memory Error", __func__ );
       return(FALSE);
     }
    result = crypt_r( pwd, util->hash, Data);
    retour = memcmp( result, util->hash, strlen( util->hash ) );                                      /* Comparaison des hash */
    g_free(Data);
    if (retour==0) return(TRUE);
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Rechercher_utilsDB: Recuperation de tous les champs des utilisateurs                                                       */
/* Entrées: un log, une db et un id d'utilisateur                                                                             */
/* Sortie: une structure utilisateur, ou null si erreur                                                                       */
/******************************************************************************************************************************/
 struct CMD_TYPE_UTILISATEUR *Recuperer_utilisateurDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_UTILISATEUR *util;
    struct DB *db;

    db = *db_orig;                                          /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    util = (struct CMD_TYPE_UTILISATEUR *)g_try_malloc0( sizeof(struct CMD_TYPE_UTILISATEUR) );
    if (!util) Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else
     { g_snprintf( util->username,       sizeof(util->username),      "%s", db->row[0] );        /* Recopie dans la structure */
       g_snprintf( util->commentaire,    sizeof(util->commentaire),   "%s", db->row[2] );
       g_snprintf( util->hash,           sizeof(util->hash),          "%s", db->row[7]);
       g_snprintf( util->sms_phone,      sizeof(util->sms_phone),     "%s", db->row[9]);
       g_snprintf( util->imsg_jabberid,  sizeof(util->imsg_jabberid), "%s", db->row[11]);
       g_snprintf( util->date_create,    sizeof(util->date_create),   "%s", db->row[5]);
       g_snprintf( util->date_modif,     sizeof(util->date_modif),    "%s", db->row[6]);
       util->id                = atoi(db->row[1]);
       util->enable            = atoi(db->row[3]);
       util->access_level      = atoi(db->row[4]);
       util->notification      = atoi(db->row[8]);
       util->allow_cde         = atoi(db->row[10]);
       util->imsg_available    = atoi(db->row[12]);
       util->ssrv_bit_presence = atoi(db->row[13]);
     }
    return( util );
  }
/******************************************************************************************************************************/
/* Rechercher_utilDB: Recuperation de tous les champs des utilisateurs                                                        */
/* Entrées: un log, une db et un id d'utilisateur                                                                             */
/* Sortie: une structure utilisateur, ou null si erreur                                                                       */
/******************************************************************************************************************************/
 struct CMD_TYPE_UTILISATEUR *Rechercher_utilisateurDB_by_name( gchar *nom )
  { struct CMD_TYPE_UTILISATEUR *util;
    gchar requete[512], *name;
    struct DB *db;

    name       = Normaliser_chaine ( nom );                                                  /* Formatage correct des chaines */
    if (!name)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: Normalisation impossible", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT username,id,comment,enable,access_level,date_create,"
                "date_modif,hash,sms_enable,sms_phone,sms_allow_cde,"
                "imsg_enable,imsg_jabberid,imsg_available,ssrv_bit_presence "
                "FROM %s WHERE username='%s' LIMIT 1", NOM_TABLE_UTIL, name );
    g_free(name);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed for user '%s'", __func__, nom );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    util = Recuperer_utilisateurDB_suite( &db );
    if (util)
     { Libere_DB_SQL( &db ); }
    return( util );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
