/******************************************************************************************************************************/
/* Watchdogd/Syn_cadran.c       Ajout/retrait de module cadran dans les synoptiques                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           dim 29 jan 2006 15:09:58 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * cadran.c
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
/* Synoptique_auto_create_CADRAN: Création automatique d'un cadran depuis la compilation DLS                                  */
/* Entrée: le plugin source, la cible, et les parametres du cadran                                                            */
/* Sortie: FALSE si erreu                                                                                                     */
/******************************************************************************************************************************/
 gboolean Synoptique_auto_create_CADRAN ( struct DLS_PLUGIN *plugin, gchar *tech_id, gchar *acronyme, gchar *forme_src,
                                          gdouble min, gdouble max,
                                          gdouble seuil_ntb, gdouble seuil_nb,
                                          gdouble seuil_nh, gdouble seuil_nth,
                                          gint nb_decimal )
  { gchar *acro, *forme;
/******************************************** Préparation de la base du mnemo *************************************************/
    acro       = Normaliser_chaine ( acronyme );                                             /* Formatage correct des chaines */
    if ( !acro )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation acro impossible. Mnemo NOT added nor modified.", __func__ );
       return(FALSE);
     }

    forme      = Normaliser_chaine ( forme_src );                                            /* Formatage correct des chaines */
    if ( !forme )
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "%s: Normalisation forme impossible. Mnemo NOT added nor modified.", __func__ );
       g_free(acro);
       return(FALSE);
     }

    gboolean retour;
    retour = SQL_Write_new
               ("INSERT INTO syns_cadrans SET "
                "dls_id=%d, tech_id='%s', acronyme='%s', forme='%s', minimum='%f', maximum='%f', "
                "seuil_ntb='%f', seuil_nb='%f', seuil_nh='%f', seuil_nth='%f', "
                "nb_decimal='%d', posx='150', posy='150' "
                "ON DUPLICATE KEY UPDATE forme=VALUES(forme), "
                "minimum=VALUES(minimum), maximum=VALUES(maximum), nb_decimal=VALUES(nb_decimal), "
                "seuil_ntb=VALUES(seuil_ntb), seuil_nb=VALUES(seuil_nb), seuil_nh=VALUES(seuil_nh), seuil_nth=VALUES(seuil_nth)",
                plugin->dls_id, tech_id, acro, forme, min, max, seuil_ntb, seuil_nb, seuil_nh, seuil_nth, nb_decimal );

    g_free(forme);
    g_free(acro);
    return (retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
