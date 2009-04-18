/**********************************************************************************************************/
/* Commun/Utilisateur/actif.c    Gestion de l'activité des comptes Watchdog                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 20:30:14 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * actif.c
 * This file is part of <Watchdog>
 *
 * Copyright (C) 2009 - sebastien
 *
 * <Watchdog> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <Watchdog> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <Watchdog>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 
 #include <glib.h>
 #include <stdlib.h>
 #include <sql.h>                                             /* Entetes de gestion de la base de données */
 #include <sqlext.h> 
 #include <sqltypes.h>

 #include "Erreur.h"
 #include "Utilisateur_DB.h"
 
/************************************ Prototypes des fonctions ********************************************/
 #include "proto_util.h"

/**********************************************************************************************************/
/* Set_compte_actif: positionne le flag enable du compte à vrai ou faux                                   */
/* Entrées: un log, une db et un id d'utilisateur et un flag                                              */
/* Sortie: boolean false si probleme                                                                      */
/**********************************************************************************************************/
 gboolean Set_compte_actif( struct LOG *log, struct DB *db, guint id, gboolean enable )
  { gchar requete[200];
    SQLRETURN retour;
    SQLHSTMT hquery;                                                          /* Handle SQL de la requete */

    hquery = NewQueryDB( log, db );                            /* Création d'un nouveau handle de requete */
    if (!hquery)
     { Info_n( log, DEBUG_DB, "Set_compte_actif: recherche failed: query=null", id );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET enable = '%d', login_failed = 0 WHERE id=%d",
                NOM_TABLE_UTIL, enable, id );

    retour = SQLExecDirect( hquery, (guchar *)requete, SQL_NTS );          /* Execution de la requete SQL */
    if ((retour != SQL_SUCCESS) && (retour != SQL_SUCCESS_WITH_INFO))
     { Info_c( log, DEBUG_DB, "Set_compte_actif: update failed", requete );
       PrintErrQueryDB( log, db, hquery );
       EndQueryDB( log, db, hquery );
       return(FALSE);
     }
    else Info_n( log, DEBUG_DB, "Set_compte_actif: update ok", id );
    EndQueryDB( log, db, hquery );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
