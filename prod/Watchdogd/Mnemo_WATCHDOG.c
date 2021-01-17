/******************************************************************************************************************************/
/* Watchdogd/Mnemo_WATCHDOG.c        Déclaration des fonctions pour la gestion des Watchdogs                                  */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    25.03.2019 14:16:22 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Mnemo_WATCHDOG.c
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
 gboolean Mnemo_auto_create_WATCHDOG ( gboolean deletable, gchar *tech_id, gchar *acronyme, gchar *libelle_src )
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

    if (deletable==FALSE)
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "INSERT INTO mnemos_WATCHDOG SET deletable='0', tech_id='%s',acronyme='%s',libelle='%s' "
                   " ON DUPLICATE KEY UPDATE deletable='0', libelle=VALUES(libelle)",
                   tech_id, acro, libelle );
     }
    else
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "INSERT INTO mnemos_WATCHDOG SET deletable='1', tech_id='%s',acronyme='%s',libelle='%s' "
                   " ON DUPLICATE KEY UPDATE libelle=VALUES(libelle)",
                   tech_id, acro, libelle );
     }
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
/* Dls_DI_to_json : Formate un bit au format JSON                                                                             */
/* Entrées: le builder et le bit                                                                                              */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_WATCHDOG_to_json ( JsonBuilder *builder, struct DLS_WATCHDOG *bit )
  { Json_add_string ( builder, "tech_id",  bit->tech_id );
    Json_add_string ( builder, "acronyme", bit->acronyme );
    Json_add_bool   ( builder, "etat",     Dls_data_get_WATCHDOG ( bit->tech_id, bit->acronyme, (gpointer)&bit ) );
    gint decompte = bit->top - Partage->top;
    Json_add_int    ( builder, "decompte", (decompte > 0 ? decompte : 0) );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
