/**********************************************************************************************************/
/* Watchdogd/Include/Utilisateur.h     definition de la structure  MSG et des constantes prg des password */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:50:09 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Utilisateur_DB.h
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
 
#ifndef _UTILISATEUR_H_
 #define _UTILISATEUR_H_

 #include "Reseaux.h"
 #include "Db.h"
 #include "Config.h"
 #include "Cst_utilisateur.h"

 #define NOM_TABLE_UTIL      "users"
 #define NOM_TABLE_GROUPE    "groups"
 #define NOM_TABLE_GIDS      "gids"

 #define NBR_CARAC_CODE_CRYPTE (TAILLE_CRYPTO_KEY + 8)      /* CRYPTO_KEY + 8 de largeur de bloc blowfish */

 struct UTILISATEURDB
  { guint  id;
    gchar  nom[ NBR_CARAC_LOGIN_UTF8+1 ];
    gchar  code[ NBR_CARAC_CODE_CRYPTE ];
    gchar  commentaire[ NBR_CARAC_COMMENTAIRE_UTF8+1 ];
    time_t date_creation;
    time_t date_modif;
    time_t date_expire;
    gboolean actif;
    gboolean expire;
    gboolean changepass;                                                 /* Doit changer son mot de passe */
    gboolean cansetpass;                                                 /* Peut changer son mot de passe */
    guint  login_failed;
    guint  gids[NBR_MAX_GROUPE_PAR_UTIL];                            /* Numéro des groupes d'appartenance */
  };

 enum                                                /* Enumeration des utilisateurs spéciaux de watchdog */
  { UID_ROOT,
    NBR_UTILISATEUR_RESERVE
  };

 enum                                                         /* Enumeration des groupes de base Watchdog */
  { GID_TOUTLEMONDE,
    GID_USERS,
    GID_MESSAGE,
    GID_ICONE,
    GID_SYNOPTIQUE,
    GID_HISTORIQUE,
    GID_DLS,
    GID_HISTO,
    GID_SCENARIO,
    NBR_GROUPE_RESERVE
  };

/************************************** Prototypes de fonctions *******************************************/
 extern gboolean Recuperer_groupesDB( struct LOG *log, struct DB *db );                  /* Dans groupe.c */
 extern struct CMD_TYPE_GROUPE *Recuperer_groupesDB_suite( struct LOG *log, struct DB *db );
 extern gboolean Tester_groupe_util( guint id_util, guint *groupes, guint id_groupe );
 extern gchar *Nom_groupe_reserve( gint id );
 extern gboolean Retirer_groupeDB( struct LOG *log, struct DB *db, struct CMD_TYPE_GROUPE *groupe );
 extern gint Ajouter_groupeDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_GROUPE *groupe );
 extern struct CMD_TYPE_GROUPE *Rechercher_groupeDB( struct LOG *log, struct DB *db, gint id );
 extern gboolean Modifier_groupeDB( struct LOG *log, struct DB *db, struct CMD_TYPE_GROUPE *groupe );
 extern gboolean Groupe_set_groupe_utilDB( struct LOG *log, struct DB *db, guint id_util, guint *gids );
 extern gboolean Groupe_get_groupe_utilDB( struct LOG *log, struct DB *db, guint id, guint *gids );

 extern gboolean Recuperer_utilsDB( struct LOG *log, struct DB *db );
 extern struct UTILISATEURDB *Recuperer_utilsDB_suite( struct LOG *log, struct DB *db );
 extern gboolean Retirer_utilisateurDB( struct LOG *log, struct DB *db,
                                        struct CMD_TYPE_UTILISATEUR *util );
 extern gint Ajouter_utilisateurDB( struct LOG *log, struct DB *db, gchar *clef,
                                    struct CMD_TYPE_UTILISATEUR *util );
 extern struct UTILISATEURDB *Rechercher_utilisateurDB( struct LOG *log, struct DB *db, gint id );
 extern gboolean Modifier_utilisateurDB( struct LOG *log, struct DB *db, gchar *clef,
                                         struct CMD_TYPE_UTILISATEUR *util );
 extern gchar *Nom_utilisateur_reserve( gint id );

 extern gchar *Recuperer_clef ( struct LOG *log, struct DB *db, gchar *nom, gint *id );    /* Dans clef.c */
 extern gchar *Crypter( struct LOG *log, gchar *clef, gchar *pass );
 extern gboolean Set_password( struct LOG *log, struct DB *db,
                               gchar *clef, struct CMD_UTIL_SETPASSWORD *util );

 extern gboolean Raz_login_failed( struct LOG *log, struct DB *db, guint id );     /* Dans login_failed.c */
 extern gboolean Ajouter_one_login_failed( struct LOG *log, struct DB *db,
                                           guint id, gint max_login_failed );

#endif
/*--------------------------------------------------------------------------------------------------------*/
