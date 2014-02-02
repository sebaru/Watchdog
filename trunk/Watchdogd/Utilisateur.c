/**********************************************************************************************************/
/* Watchdogd/Utilisateur/Utilisateur.c    Interface DB mots de passe pour watchdog2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 20:35:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Utilisateur.c
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
 #include <unistd.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>

/************************************ Prototypes des fonctions ********************************************/
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Retirer_utilisateur: Elimine un utilisateur dans la base de données                                    */
/* Entrées: un log, une db, un nom                                                                        */
/* Sortie: true si pas de pb, false sinon                                                                 */
/**********************************************************************************************************/
 gboolean Retirer_utilisateurDB( struct CMD_TYPE_UTILISATEUR *util )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    if (util->id < NBR_UTILISATEUR_RESERVE) 
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, 
                "Retirer_utilisateurDB: elimination failed: id (%d) reserve %s", util->id, util->nom );
       return(FALSE);
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_utilisateurDB: DB connexion failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_UTIL, util->id );
    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id_util=%d", NOM_TABLE_GIDS, util->id );
    Lancer_requete_SQL ( db, requete );                                    /* Execution de la requete SQL */

    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Retirer_utilisateur: Elimine un utilisateur dans la base de données                                    */
/* Entrées: un log, une db, un nom                                                                        */
/* Sortie: true si pas de pb, false sinon                                                                 */
/**********************************************************************************************************/
 gboolean Set_enable_utilisateurDB( struct CMD_TYPE_UTILISATEUR *util )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    if (util->id == UID_ROOT || !strcmp(util->nom, "root")) 
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, 
                "Set_enable_utilisateurDB: Root couldn't be disabled" );
       return(FALSE);
     }

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Retirer_utilisateurDB: DB connexion failed" );
       return(FALSE);
     }

    if (util->id != -1)
     { g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                   "UPDATE %s SET enable=%d WHERE id=%d", NOM_TABLE_UTIL, util->enable, util->id );
     }
    else
     { gchar *nom;
       nom        = Normaliser_chaine ( util->nom );                        /* Formatage correct des chaines */
       if (!nom)
        { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                   "Set_enable_utilisateurDB: Normalisation impossible" );
          return(FALSE);
        }
       g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                   "UPDATE %s SET enable=%d WHERE name='%s'", NOM_TABLE_UTIL, util->enable, nom );
       g_free(nom);
     }
    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/**********************************************************************************************************/
/* Modifier_utilisateurDB: Modification d'u nutilisateur Watchdog                                         */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 static gint Ajouter_Modifier_utilisateurDB( gboolean ajout, struct CMD_TYPE_UTILISATEUR *util )
  { gchar requete[512], chaine[512];
    gchar *nom, *comment, *phone, *salt, *hash;
    gboolean retour;
    struct DB *db;
    gint id;

    nom        = Normaliser_chaine ( util->nom );                        /* Formatage correct des chaines */
    if (!nom)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Ajouter_Modifier_utilisateurDB: Normalisation impossible" );
       return(-1);
     }
    comment    = Normaliser_chaine ( util->commentaire );
    if (!comment)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Ajouter_Modifier_utilisateurDB: Normalisation impossible" );
       g_free(nom);
       return(-1);
     }
    salt = Normaliser_chaine ( util->salt );
    if (!salt)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Ajouter_Modifier_utilisateurDB: Normalisation salt impossible" );
       g_free(nom);
       g_free(comment);
       return(-1);
     }
    hash = Normaliser_chaine ( util->hash );
    if (!hash)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Ajouter_Modifier_utilisateurDB: Normalisation hash impossible" );
       g_free(nom);
       g_free(comment);
       g_free(salt);
       return(-1);
     }
    phone = Normaliser_chaine ( util->sms_phone );
    if (!phone)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Ajouter_Modifier_utilisateurDB: Normalisation phone impossible" );
       g_free(nom);
       g_free(comment);
       g_free(salt);
       g_free(hash);
       return(-1);
     }

    if (ajout)
     { g_snprintf( requete, sizeof(requete),                                              /* Requete SQL */
                   "INSERT INTO %s"             
                   "(name,mustchangepwd,cansetpwd,comment,login_failed,enable,"
                   "date_create,enable_expire,date_expire,date_modif,sms_enable,sms_phone,sms_allow_cde)"
                   "VALUES ('%s', 1, 1, '%s', 0, 1, %d, %d, '%d', '%d','%d','%s','%d' );",
                   NOM_TABLE_UTIL, nom,
                   comment, (gint)time(NULL),
                   util->expire, (gint)util->date_expire,
                   (gint)time(NULL), util->sms_enable, phone, util->sms_allow_cde );
     }
    else
     { g_snprintf( requete, sizeof(requete),                                              /* Requete SQL */
                   "UPDATE %s SET "             
                   "mustchangepwd=%d,comment='%s',enable=%d,enable_expire=%d,"
                   "cansetpwd=%d,date_expire='%d',date_modif='%d',"
                   "sms_enable='%d',sms_phone='%s',sms_allow_cde='%d'",
                   NOM_TABLE_UTIL, util->mustchangepwd, comment,
                                   util->enable,
                                   util->expire,
                                   util->cansetpwd,
                                   (gint)util->date_expire,
                                   (gint)time(NULL), util->sms_enable, phone, util->sms_allow_cde );
       if (util->setpwdnow)
        { g_snprintf( chaine, sizeof(chaine), ",salt='%s',hash='%s'", salt, hash );
          g_strlcat ( requete, chaine, sizeof(requete) );
        }
       g_snprintf( chaine, sizeof(chaine), " WHERE id='%d'", util->id ); 
       g_strlcat ( requete, chaine, sizeof(requete) );
     }
    g_free(nom);
    g_free(comment);
    g_free(salt);
    g_free(hash);
    g_free(phone);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_Modifier_utilisateurDB: DB connexion failed" );
       return(-1);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if ( retour == FALSE )
     { Libere_DB_SQL(&db); 
       return(-1);
     }
    if (ajout) { id = Recuperer_last_ID_SQL ( db );
                 util->id = id;
               }
    else id=1;                                          /* Retour = 1 pour une modification d'utilisateur */
    Libere_DB_SQL(&db);
    Groupe_set_groupe_utilDB ( util->id, (guint *)&util->gids );            /* Positionnement des groupes */ 
    return(id);
  }
/**********************************************************************************************************/
/* Ajouter_utilisateurDB : Ajoute un utilisateur en base de données                                       */
/* Entrées: une structure CMD_TYPE_UTILISATEUR complétée                                                  */
/* Sortie: -1 si probleme                                                                                 */
/**********************************************************************************************************/
 gint Ajouter_utilisateurDB( struct CMD_TYPE_UTILISATEUR *util )
  { return( Ajouter_Modifier_utilisateurDB ( TRUE, util ) ); }
/**********************************************************************************************************/
/* Modifier_utilisateurDB: Modification d'u nutilisateur Watchdog                                         */
/* Entrées: une structure CMD_TYPE_UTILISATEUR complétée                                                  */
/* Sortie: FALSE si probleme                                                                              */
/**********************************************************************************************************/
 gint Modifier_utilisateurDB( struct CMD_TYPE_UTILISATEUR *util )
  { return( Ajouter_Modifier_utilisateurDB ( FALSE, util ) ); }
/**********************************************************************************************************/
/* Set_password: Correspond au changement de password de l'utilisateur                                    */
/* Entrées: un log, une db, un id utilisateur, une clef, un password                                      */
/* Sortie: FALSE si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Modifier_utilisateurDB_set_password( struct CMD_TYPE_UTILISATEUR *util )
  { gchar requete[512];
    gchar *salt, *hash;
    gboolean retour;
    struct DB *db;

    salt = Normaliser_chaine ( (gchar *)util->salt );
    if (!salt)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_utilisateurDB: Normalisation salt impossible" );
       return(FALSE);
     }

    hash = Normaliser_chaine ( (gchar *)util->hash );
    if (!hash)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "Modifier_utilisateurDB: Normalisation hash impossible" );
       g_free(salt);
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "salt='%s',hash='%s',date_modif='%d',mustchangepwd=0 WHERE id=%d"
                " AND (mustchangepwd=1 OR id<%d OR (mustchangepwd=0 AND cansetpwd=1))",
                NOM_TABLE_UTIL, salt, hash, (gint)time(NULL), util->id, NBR_UTILISATEUR_RESERVE );
    g_free(salt);
    g_free(hash);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_utilisateurDB_set_password: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    if ( ! retour )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Modifier_utilisateurDB_set_password: update failed %d (%s)", util->id, util->nom );
       return(FALSE);
     }
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                "Modifier_utilisateurDB_set_password: update ok for id=%d (%s)", util->id, util->nom );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Set_password: Correspond au changement de password de l'utilisateur                                    */
/* Entrées: un log, une db, un id utilisateur, une clef, un password                                      */
/* Sortie: FALSE si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Modifier_utilisateurDB_set_cansetpwd( struct CMD_TYPE_UTILISATEUR *util )
  { gchar requete[512];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "cansetpwd=%d,date_modif='%d' WHERE id=%d",
                NOM_TABLE_UTIL, util->cansetpwd, (gint)time(NULL), util->id );
    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Modifier_utilisateurDB_set_cansetpwd: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    if ( ! retour )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Modifier_utilisateurDB_set_cansetpwd: update failed %d (%s)", util->id, util->nom );
       return(FALSE);
     }
    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                "Modifier_utilisateurDB_set_cansetpwd: update ok for id=%d (%s)", util->id, util->nom );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Rechercher_utilsDB: Recuperation de tous les champs des utilisateurs                                   */
/* Entrées: un log, une db et un id d'utilisateur                                                         */
/* Sortie: une structure utilisateur, ou null si erreur                                                   */
/**********************************************************************************************************/
 gboolean Recuperer_utilisateurDB( struct DB **db_retour )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT name,id,mustchangepwd,comment,enable,date_create,"
                "enable_expire,date_expire,cansetpwd,date_modif,salt,hash,sms_enable,sms_phone,sms_allow_cde "
                "FROM %s", NOM_TABLE_UTIL );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Recuperer_utilisateurDB: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    if (retour == FALSE) Libere_DB_SQL (&db);
    *db_retour = db;
    return ( retour );
  }
/**********************************************************************************************************/
/* Rechercher_utilsDB: Recuperation de tous les champs des utilisateurs                                   */
/* Entrées: un log, une db et un id d'utilisateur                                                         */
/* Sortie: une structure utilisateur, ou null si erreur                                                   */
/**********************************************************************************************************/
 struct CMD_TYPE_UTILISATEUR *Recuperer_utilisateurDB_suite( struct DB **db_orig )
  { struct CMD_TYPE_UTILISATEUR *util;
    struct DB *db;

    db = *db_orig;                      /* Récupération du pointeur initialisé par la fonction précédente */
    Recuperer_ligne_SQL(db);                                           /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       Libere_DB_SQL( &db );
       return(NULL);
     }

    util = (struct CMD_TYPE_UTILISATEUR *)g_try_malloc0( sizeof(struct CMD_TYPE_UTILISATEUR) );
    if (!util) Info_new( Config.log, Config.log_msrv, LOG_ERR,
                        "Recuperer_utilisateurDB_suite: Erreur allocation mémoire" );
    else
     { g_snprintf( util->nom,         sizeof(util->nom),         "%s", db->row[0] );/* Recopie dans la structure */
       g_snprintf( util->commentaire, sizeof(util->commentaire), "%s", db->row[3] );
       g_snprintf( util->sms_phone,   sizeof(util->sms_phone),   "%s", db->row[13]);
       memcpy( &util->salt, db->row[10], sizeof(util->salt)-1 );
       memcpy( &util->hash, db->row[11], sizeof(util->hash)-1 );
       util->id            = atoi(db->row[1]);
       util->mustchangepwd = atoi(db->row[2]);
       util->enable        = atoi(db->row[4]);
       util->date_creation = atoi(db->row[5]);
       util->expire        = atoi(db->row[6]);
       util->date_expire   = atoi(db->row[7]);
       util->cansetpwd     = atoi(db->row[8]);
       util->date_modif    = atoi(db->row[9]);
       util->sms_enable    = atoi(db->row[12]);
       util->sms_allow_cde = atoi(db->row[14]);
     }
    return( util );
  }
/**********************************************************************************************************/
/* Rechercher_utilDB: Recuperation de tous les champs des utilisateurs                                    */
/* Entrées: un log, une db et un id d'utilisateur                                                         */
/* Sortie: une structure utilisateur, ou null si erreur                                                   */
/**********************************************************************************************************/
 struct CMD_TYPE_UTILISATEUR *Rechercher_utilisateurDB_by_id( gint id )
  { struct CMD_TYPE_UTILISATEUR *util;
    gchar requete[512];
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT name,id,mustchangepwd,comment,enable,date_create,"
                "enable_expire,date_expire,cansetpwd,date_modif,salt,hash,sms_enable,sms_phone,sms_allow_cde "
                "FROM %s WHERE id=%d LIMIT 1", NOM_TABLE_UTIL, id );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_utilisateurDB: DB connexion failed" );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    util = Recuperer_utilisateurDB_suite( &db );
    Libere_DB_SQL( &db );
    if (util) Groupe_get_groupe_utilDB ( util->id, (guint *)&util->gids );
    return( util );
  }
/**********************************************************************************************************/
/* Rechercher_utilDB: Recuperation de tous les champs des utilisateurs                                    */
/* Entrées: un log, une db et un id d'utilisateur                                                         */
/* Sortie: une structure utilisateur, ou null si erreur                                                   */
/**********************************************************************************************************/
 struct CMD_TYPE_UTILISATEUR *Rechercher_utilisateurDB_by_name( gchar *nom )
  { struct CMD_TYPE_UTILISATEUR *util;
    gchar requete[200], *name;
    struct DB *db;

    name       = Normaliser_chaine ( nom );                              /* Formatage correct des chaines */
    if (!name)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Recherche_utilisateurDB_by_name: Normalisation impossible" );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT name,id,mustchangepwd,comment,enable,date_create,"
                "enable_expire,date_expire,cansetpwd,date_modif,salt,hash,sms_enable,sms_phone,sms_allow_cde "
                "FROM %s WHERE name='%s' LIMIT 1", NOM_TABLE_UTIL, nom );
    g_free(name);

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Rechercher_utilisateurDB_by_name: DB connexion failed" );
       return(NULL);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(NULL);
     }

    util = Recuperer_utilisateurDB_suite( &db );
    Libere_DB_SQL( &db );
    if (util) Groupe_get_groupe_utilDB ( util->id, (guint *)&util->gids );
    return( util );
  }
/*--------------------------------------------------------------------------------------------------------*/
