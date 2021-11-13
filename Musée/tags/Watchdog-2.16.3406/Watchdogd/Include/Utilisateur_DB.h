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

 #define NOM_TABLE_UTIL      "users"
 #define NOM_TABLE_GROUPE    "groups"
 #define NOM_TABLE_GIDS      "gids"

 enum                                                /* Enumeration des utilisateurs spéciaux de watchdog */
  { UID_ROOT,
    UID_GUEST,
    NBR_UTILISATEUR_RESERVE
  };

                                                                                  /* Enumeration des groupes de base Watchdog */

 #define ACCESS_LEVEL_DLS       6
 #define ACCESS_LEVEL_CLI       6
 #define ACCESS_LEVEL_MSG       6
 #define ACCESS_LEVEL_USER      6
 #define ACCESS_LEVEL_SATELLITE 6
 #define ACCESS_LEVEL_ATELIER   6
 #define ACCESS_LEVEL_HISTO     6
 #define ACCESS_LEVEL_ICONE     6
 #define ACCESS_LEVEL_ALL       0

/************************************************ Prototypes de fonctions *****************************************************/
 extern gboolean Recuperer_groupesDB( struct DB **db );                                  /* Dans groupe.c */
 extern struct CMD_TYPE_GROUPE *Recuperer_groupesDB_suite( struct DB **db );
 extern gboolean Tester_level_util( struct CMD_TYPE_UTILISATEUR *util, guint id_groupe );
 extern gboolean Retirer_groupeDB( struct CMD_TYPE_GROUPE *groupe );
 extern gint Ajouter_groupeDB ( struct CMD_TYPE_GROUPE *groupe );
 extern struct CMD_TYPE_GROUPE *Rechercher_groupeDB( gint id );
 extern gboolean Modifier_groupeDB( struct CMD_TYPE_GROUPE *groupe );
 extern gboolean Groupe_set_groupe_utilDB( guint id_util, guint *gids );
 extern gboolean Groupe_get_groupe_utilDB( guint id, guint *gids );

 extern gboolean Recuperer_utilisateurDB( struct DB **db );                         /* Dans Utilisateur.c */
 extern struct CMD_TYPE_UTILISATEUR *Recuperer_utilisateurDB_suite( struct DB **db );
 extern gboolean Retirer_utilisateurDB( struct CMD_TYPE_UTILISATEUR *util );
 extern gboolean Set_enable_utilisateurDB( struct CMD_TYPE_UTILISATEUR *util );
 extern gint Ajouter_utilisateurDB( struct CMD_TYPE_UTILISATEUR *util );
 extern struct CMD_TYPE_UTILISATEUR *Rechercher_utilisateurDB_by_id( gint id );
 extern struct CMD_TYPE_UTILISATEUR *Rechercher_utilisateurDB_by_name( gchar *nom );
 extern gboolean Modifier_utilisateurDB( struct CMD_TYPE_UTILISATEUR *util );
 extern gchar *Nom_utilisateur_reserve( gint id );
 extern gboolean Modifier_utilisateurDB_set_password( struct CMD_TYPE_UTILISATEUR *util, gchar *password );
 extern gboolean Modifier_utilisateurDB_set_cansetpwd( struct CMD_TYPE_UTILISATEUR *util );
 extern gboolean Modifier_utilisateurDB_set_mustchangepwd( struct CMD_TYPE_UTILISATEUR *util );
 extern void Utilisateur_set_new_salt ( struct CMD_TYPE_UTILISATEUR *util );
 extern gchar *Utilisateur_hash_password ( struct CMD_TYPE_UTILISATEUR *util, gchar *pwd );
 extern gboolean Check_utilisateur_password( struct CMD_TYPE_UTILISATEUR *util, gchar *pwd );

 extern gboolean Raz_login_failed( guint id );                                     /* Dans login_failed.c */
 extern gboolean Ajouter_one_login_failed( guint id, gint max_login_failed );


 extern gchar *Groupes_vers_string ( guint *source );
 extern gint Get_login_failed( guint id );
 extern gboolean Ajouter_one_login_failed( guint id, gint max_login_failed );
 extern gboolean Set_compte_actif( guint id, gboolean enable );
 extern guint *String_vers_groupes ( gchar *source );
 extern gboolean Set_compte_actif( guint id, gboolean enable );

#endif
/*--------------------------------------------------------------------------------------------------------*/
