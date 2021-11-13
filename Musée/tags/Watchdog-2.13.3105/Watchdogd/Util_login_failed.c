/**********************************************************************************************************/
/* Watchdogd/Utilisateur/login_failed.c    Interface DB mots de passe pour watchdog2.0                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 20:35:05 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * login_failed.c
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
 #include <stdlib.h>

/************************************ Prototypes des fonctions ********************************************/
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Get_login_failed: Recupere la valeur du login failed                                                   */
/* Entrées: un log, une db et nom d'utilisateur                                                           */
/* Sortie: le nombre de login failed, -1 si erreur                                                        */
/**********************************************************************************************************/
 gint Get_login_failed( guint id )
  { gchar requete[200];
    gint nbr_login;
    struct DB *db;

    if (id < NBR_UTILISATEUR_RESERVE) 
     { return(0); }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT login_failed FROM %s WHERE id=%d",
                NOM_TABLE_UTIL, id );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Get_login_failed: DB connexion failed" );
       return(-1);
     }

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(-1);
     }

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Libere_DB_SQL( &db );
       Info_new( Config.log, Config.log_msrv, LOG_INFO, "Get_login_failed: USER %03d not found in DB", id );
       return(-1);
     }

    nbr_login = atoi(db->row[0]);
    Libere_DB_SQL(&db);
    return( nbr_login );
  }
/**********************************************************************************************************/
/* Ajouter_one_login_failed: Ajoute 1 au login failed de l'utilisateur                                    */
/* Entrées: un log, une db et nom d'utilisateur, un max_login_failed                                      */
/* Sortie: le nombre de login failed                                                                      */
/**********************************************************************************************************/
 gboolean Ajouter_one_login_failed( guint id, gint max_login_failed )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    if (id < NBR_UTILISATEUR_RESERVE) 
     { return(FALSE); }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET login_failed = login_failed+1 WHERE id=%d",
                NOM_TABLE_UTIL, id );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Ajouter_one_login_failed: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);

    if (Get_login_failed( id )>=max_login_failed)                              /* Desactivation du compte */
     { Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                "Ajouter_one_login_failed: Desactivation compte sur trop login failed for id=%d", id );
       Set_compte_actif( id, FALSE );
     }
    return(retour);
  }
/**********************************************************************************************************/
/* Raz_login_failed: Reset le login failed d'un utilisateur                                               */
/* Entrées: un log, une db et nom d'utilisateur                                                           */
/* Sortie: le nombre de login failed                                                                      */
/**********************************************************************************************************/
 gboolean Raz_login_failed( guint id )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET login_failed = 0 WHERE id=%d",
                NOM_TABLE_UTIL, id );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Raz_login_failed: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/*--------------------------------------------------------------------------------------------------------*/
