/******************************************************************************************************************************/
/* Watchdogd/Dls/Dls_db       Database DLS (gestion des noms de prgs, ...)                                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        dim. 02 janv. 2011 19:06:19 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Dls_db.c
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

 #include "watchdogd.h"
/******************************************************************************************************************************/
/* Dls_auto_create_plugin: Créé automatiquement le plugin en parametre (tech_id, nom)                                         */
/* Entrées: le tech_id (unique) et le nom associé                                                                             */
/* Sortie: -1 si pb, id sinon                                                                                                 */
/******************************************************************************************************************************/
 gboolean Dls_auto_create_plugin( gchar *tech_id, gchar *nom_src )
  { gchar requete[1024], *nom;
    gboolean retour;
    struct DB *db;
#warning a passer sur API
#ifdef bouh

    nom = Normaliser_chaine ( nom_src );                                                     /* Formatage correct des chaines */
    if (!nom)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING, "%s: Normalisation nom impossible", __func__ );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
               "INSERT INTO dls SET is_thread=1,actif=1,"
               "tech_id=UPPER('%s'),shortname='%s',name='%s',package='custom',"
               "syn_id=1,compil_status=0,sourcecode=' - IO_COMM -> COMM; /* Recopie de la comm IO vers le bit de COMM partagé ! */' "
               "ON DUPLICATE KEY UPDATE tech_id=VALUES(tech_id),shortname=VALUES(shortname),"
               "name=VALUES(name),is_thread=1", tech_id, tech_id, nom );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Creating DLS '%s'", __func__, nom );
    g_free(nom);

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(FALSE);
     }

    retour = Lancer_requete_SQL ( db, requete );                                               /* Execution de la requete SQL */
    Libere_DB_SQL(&db);
#endif
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
