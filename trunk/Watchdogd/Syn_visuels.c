/******************************************************************************************************************************/
/* Watchdogd/Synoptiques/motifs.c       Ajout/retrait de motifs dans les motifs                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          mer 05 mai 2004 12:11:21 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * motifs.c
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
/* Synoptique_auto_create_VISUEL: Création automatique d'un visuel depuis la compilation DLS                                  */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Mnemo_auto_create_VISUEL ( struct DLS_PLUGIN *plugin, gchar *acronyme, gchar *libelle_src, gchar *forme_src )
  { gchar *acro, *libelle, *forme;
    gboolean retour;

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

    forme      = Normaliser_chaine ( forme_src );                                            /* Formatage correct des chaines */
    if ( !forme )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation forme impossible. Mnemo NOT added nor modified.", __func__ );
       g_free(acro);
       g_free(libelle);
       return(FALSE);
     }

    retour = SQL_Write_new( "INSERT INTO mnemos_VISUEL SET "
                            "tech_id='%s', acronyme='%s', forme='%s', libelle='%s' /*access_level=0,*/ "
                            "ON DUPLICATE KEY UPDATE forme=VALUES(forme), libelle=VALUES(libelle)",
                            plugin->tech_id, acro, forme, libelle );
    g_free(forme);
    g_free(libelle);
    g_free(acro);

    return (retour);
  }
/******************************************************************************************************************************/
/* Synoptique_auto_create_VISUEL: Création automatique d'un visuel depuis la compilation DLS                                  */
/* Entrée: un mnemo, et un flag d'edition ou d'ajout                                                                          */
/* Sortie: -1 si erreur, ou le nouvel id si ajout, ou 0 si modification OK                                                    */
/******************************************************************************************************************************/
 gboolean Synoptique_auto_create_VISUEL ( struct DLS_PLUGIN *plugin, gchar *target_tech_id_src, gchar *target_acronyme_src )
  { gchar *target_tech_id, *target_acro;
    gboolean retour;

/******************************************** Préparation de la base du mnemo *************************************************/
    target_acro = Normaliser_chaine ( target_acronyme_src );                                 /* Formatage correct des chaines */
    if ( !target_acro )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation acro impossible. Mnemo NOT added nor modified.", __func__ );
       return(FALSE);
     }

    target_tech_id = Normaliser_chaine ( target_tech_id_src );                               /* Formatage correct des chaines */
    if ( !target_tech_id )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation tech_id impossible. Mnemo NOT added nor modified.", __func__ );
       g_free(target_acro);
       return(FALSE);
     }

    retour = SQL_Write_new ( "INSERT INTO syns_visuels SET "
                             "dls_id='%d', mnemo_id=(SELECT id FROM mnemos_VISUEL WHERE tech_id='%s' AND acronyme='%s'), "
                             "posx='150', posy='150', angle='0', scale='1' "
                             "ON DUPLICATE KEY UPDATE mnemo_id=mnemo_id",
                             plugin->id, target_tech_id, target_acro );
    g_free(target_tech_id);
    g_free(target_acro);
    return (retour);
  }
/******************************************************************************************************************************/
/* Dls_VISUEL_to_json : Formate un bit au format JSON                                                                         */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_VISUEL_to_json ( JsonNode *RootNode, struct DLS_VISUEL *bit )
  { Json_node_add_string ( RootNode, "tech_id",   bit->tech_id );
    Json_node_add_string ( RootNode, "acronyme",  bit->acronyme );
    Json_node_add_string ( RootNode, "mode",      bit->mode  );
    Json_node_add_string ( RootNode, "color",     bit->color );
    Json_node_add_bool   ( RootNode, "cligno",    bit->cligno );
    Json_node_add_string ( RootNode, "libelle",   bit->libelle );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
