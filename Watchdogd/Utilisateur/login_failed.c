/**********************************************************************************************************/
/* Commun/Utilisateur/login_failed.c    Interface DB mots de passe pour watchdog2.0                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 20:35:05 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * login_failed.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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
 #include <stdlib.h>

 #include <sql.h>                                             /* Entetes de gestion de la base de données */
 #include <sqlext.h> 
 #include <sqltypes.h>

 #include "watchdogd.h"
 #include "Erreur.h"
 #include "Utilisateur_DB.h"

/************************************ Prototypes des fonctions ********************************************/
 #include "proto_util.h"

/**********************************************************************************************************/
/* Get_login_failed: Recupere la valeur du login failed                                             */
/* Entrées: un log, une db et nom d'utilisateur                                                           */
/* Sortie: le nombre de login failed, -1 si erreur                                                        */
/**********************************************************************************************************/
 gint Get_login_failed( struct LOG *log, struct DB *db, guint id )
  { gchar requete[200];
    gint nbr_login;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT login_failed FROM %s WHERE id=%d",
                NOM_TABLE_UTIL, id );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(0); }

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Get_login_failed: Login_failed non trouvé dans la BDD", id );
       return(0);
     }

    nbr_login = atoi(db->row[0]);
    Liberer_resultat_SQL ( log, db );
    return( nbr_login );
  }
/**********************************************************************************************************/
/* Ajouter_one_login_failed: Ajoute 1 au login failed de l'utilisateur                                    */
/* Entrées: un log, une db et nom d'utilisateur, un max_login_failed                                      */
/* Sortie: le nombre de login failed                                                                      */
/**********************************************************************************************************/
 gboolean Ajouter_one_login_failed( struct LOG *log, struct DB *db, guint id, gint max_login_failed )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET login_failed = login_failed+1 WHERE id=%d",
                NOM_TABLE_UTIL, id );

    Lancer_requete_SQL ( log, db, requete );                               /* Execution de la requete SQL */
    if (Get_login_failed( log, db, id )>=max_login_failed)                     /* Desactivation du compte */
     { Info_n( log, DEBUG_DB, "Desactivation compte sur trop login failed", id );
       Set_compte_actif( log, db, id, FALSE );
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Raz_login_failed: Reset le login failed d'un utilisateur                                               */
/* Entrées: un log, une db et nom d'utilisateur                                                           */
/* Sortie: le nombre de login failed                                                                      */
/**********************************************************************************************************/
 gboolean Raz_login_failed( struct LOG *log, struct DB *db, guint id )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET login_failed = 0 WHERE id=%d",
                NOM_TABLE_UTIL, id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/*--------------------------------------------------------------------------------------------------------*/
