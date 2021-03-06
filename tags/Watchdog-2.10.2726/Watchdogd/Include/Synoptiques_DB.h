/**********************************************************************************************************/
/* Watchdogd/Include/Synoptiques_DB.h     Déclaration structure internes des synoptiques watchdog         */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 25 sep 2003 16:33:06 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Synoptiques_DB.h
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
 
#ifndef _SYNOPTIQUES_H_
 #define _SYNOPTIQUES_H_

 #include "Reseaux.h"
 #include "Db.h"
 #include "Utilisateur_DB.h"

 #define NOM_TABLE_SYNOPTIQUE     "syns"
 #define NOM_TABLE_MOTIF          "syns_motifs"
 #define NOM_TABLE_COMMENT        "syns_comments"
 #define NOM_TABLE_PASSERELLE     "syns_pass"
 #define NOM_TABLE_PALETTE        "syns_palettes"
 #define NOM_TABLE_CAPTEUR        "syns_capteurs"
 #define NOM_TABLE_CAMERASUP      "syns_camerasup"

/*************************************** Définitions des prototypes ***************************************/
 extern struct CMD_TYPE_SYNOPTIQUE *Rechercher_synoptiqueDB ( guint id );
 extern gboolean Recuperer_synoptiqueDB ( struct DB **db );
 extern struct CMD_TYPE_SYNOPTIQUE *Recuperer_synoptiqueDB_suite( struct DB **db );
 extern gint Ajouter_synoptiqueDB ( struct CMD_TYPE_SYNOPTIQUE *syn );
 extern gboolean Retirer_synoptiqueDB ( struct CMD_TYPE_SYNOPTIQUE *syn );
 extern gboolean Modifier_synoptiqueDB( struct CMD_TYPE_SYNOPTIQUE *syn );

 extern gboolean Retirer_motifDB ( struct CMD_TYPE_MOTIF *motif );
 extern gint Ajouter_motifDB ( struct CMD_TYPE_MOTIF *motif );
 extern gboolean Recuperer_motifDB ( struct DB **db, gint id_syn );
 extern struct CMD_TYPE_MOTIF *Recuperer_motifDB_suite( struct DB **db );
 extern struct CMD_TYPE_MOTIF *Rechercher_motifDB ( guint id );
 extern gboolean Modifier_motifDB( struct CMD_TYPE_MOTIF *motif );

 extern gboolean Retirer_commentDB ( struct CMD_TYPE_COMMENT *comment );
 extern gint Ajouter_commentDB ( struct CMD_TYPE_COMMENT *comment );
 extern gboolean Recuperer_commentDB ( struct DB **db, gint id_syn );
 extern struct CMD_TYPE_COMMENT *Recuperer_commentDB_suite( struct DB **db );
 extern struct CMD_TYPE_COMMENT *Rechercher_commentDB ( guint id );
 extern gboolean Modifier_commentDB( struct CMD_TYPE_COMMENT *comment );

 extern gboolean Retirer_passerelleDB ( struct CMD_TYPE_PASSERELLE *pass );
 extern gint Ajouter_passerelleDB ( struct CMD_TYPE_PASSERELLE *pass );
 extern gboolean Recuperer_passerelleDB ( struct DB **db, gint id_syn );
 extern struct CMD_TYPE_PASSERELLE *Recuperer_passerelleDB_suite( struct DB **db );
 extern struct CMD_TYPE_PASSERELLE *Rechercher_passerelleDB ( guint id );
 extern gboolean Modifier_passerelleDB( struct CMD_TYPE_PASSERELLE *pass );

 extern gboolean Retirer_paletteDB ( struct CMD_TYPE_PALETTE *pass );
 extern gint Ajouter_paletteDB ( struct CMD_TYPE_PALETTE *pass );
 extern gboolean Recuperer_paletteDB ( struct DB **db, gint id_syn );
 extern struct CMD_TYPE_PALETTE *Recuperer_paletteDB_suite( struct DB **db );
 extern struct CMD_TYPE_PALETTE *Rechercher_paletteDB ( guint id );
 extern gboolean Modifier_paletteDB( struct CMD_TYPE_PALETTE *pass );

 extern gboolean Retirer_capteurDB ( struct CMD_TYPE_CAPTEUR *capteur );
 extern gint Ajouter_capteurDB ( struct CMD_TYPE_CAPTEUR *capteur );
 extern gboolean Recuperer_capteurDB ( struct DB **db, gint id_syn );
 extern struct CMD_TYPE_CAPTEUR *Recuperer_capteurDB_suite( struct DB **db );
 extern struct CMD_TYPE_CAPTEUR *Rechercher_capteurDB ( guint id );
 extern gboolean Modifier_capteurDB( struct CMD_TYPE_CAPTEUR *capteur );

 extern gboolean Retirer_camera_supDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA_SUP *camera_sup );
 extern gint Ajouter_camera_supDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA_SUP *camera_sup );
 extern gboolean Recuperer_camera_supDB ( struct LOG *log, struct DB *db, gint id_syn );
 extern struct CMD_TYPE_CAMERA_SUP *Recuperer_camera_supDB_suite( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_CAMERA_SUP *Rechercher_camera_supDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_camera_supDB( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA_SUP *camera_sup );
#endif
/*--------------------------------------------------------------------------------------------------------*/
