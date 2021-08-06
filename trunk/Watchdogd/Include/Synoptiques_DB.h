/******************************************************************************************************************************/
/* Watchdogd/Include/Synoptiques_DB.h     Déclaration structure internes des synoptiques watchdog                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          jeu 25 sep 2003 16:33:06 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Synoptiques_DB.h
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

#ifndef _SYNOPTIQUES_H_
 #define _SYNOPTIQUES_H_

 #include "Reseaux.h"
 #include "Db.h"

 #define NOM_TABLE_SYNOPTIQUE     "syns"
 #define NOM_TABLE_MOTIF          "syns_visuels"
 #define NOM_TABLE_COMMENT        "syns_comments"
 #define NOM_TABLE_PASSERELLE     "syns_pass"
 #define NOM_TABLE_PALETTE        "syns_palettes"
 #define NOM_TABLE_CADRAN         "syns_cadrans"
 #define NOM_TABLE_CAMERASUP      "syns_camerasup"
 #define NOM_TABLE_SCENARIO       "syns_scenario"

/************************************************ Définitions des prototypes **************************************************/
 extern gboolean Synoptique_auto_create_VISUEL ( struct DLS_PLUGIN *plugin, gchar *target_tech_id_src, gchar *target_acronyme_src );
 extern gboolean Mnemo_auto_create_VISUEL ( struct DLS_PLUGIN *plugin, gchar *acronyme, gchar *libelle_src,
                                            gchar *forme_src, gchar *couleur_src );
 extern void Dls_VISUEL_to_json ( JsonNode *RootNode, struct DLS_VISUEL *bit );

 extern gboolean Synoptique_auto_create_CADRAN ( struct DLS_PLUGIN *plugin, gchar *tech_id, gchar *acronyme, gchar *forme_src,
                                                 gdouble min, gdouble max,
                                                 gdouble seuil_ntb, gdouble seuil_nb,
                                                 gdouble seuil_nh, gdouble seuil_nth,
                                                 gint nb_decimal );

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
