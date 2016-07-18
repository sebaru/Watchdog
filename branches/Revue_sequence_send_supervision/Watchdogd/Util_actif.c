/**********************************************************************************************************/
/* Watchdogd/Utilisateur/actif.c    Gestion de l'activité des comptes Watchdog                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 20:30:14 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * actif.c
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
/* Set_compte_actif: positionne le flag enable du compte à vrai ou faux                                   */
/* Entrées: un log, une db et un id d'utilisateur et un flag                                              */
/* Sortie: boolean false si probleme                                                                      */
/**********************************************************************************************************/
 gboolean Set_compte_actif( guint id, gboolean enable )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    if (id < NBR_UTILISATEUR_RESERVE) 
     { return(TRUE); }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET enable = '%d', login_failed = 0 WHERE id=%d",
                NOM_TABLE_UTIL, enable, id );

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "Set_compte_actif: DB connexion failed" );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                           /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
    return(retour);
  }
/*--------------------------------------------------------------------------------------------------------*/
