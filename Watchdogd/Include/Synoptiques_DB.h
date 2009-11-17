/**********************************************************************************************************/
/* Watchdogd/Include/Synoptiques.h        Déclaration structure internes des synoptiques watchdog         */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 25 sep 2003 16:33:06 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _SYNOPTIQUES_H_
 #define _SYNOPTIQUES_H_

 #include "Reseaux.h"
 #include "Db.h"
 #include "Cst_synoptiques.h"
 #include "Utilisateur_DB.h"

 #define NOM_TABLE_SYNOPTIQUE     "syns"
 #define NOM_TABLE_MOTIF          "syns_motifs"
 #define NOM_TABLE_COMMENT        "syns_comments"
 #define NOM_TABLE_PASSERELLE     "syns_pass"
 #define NOM_TABLE_PALETTE        "syns_palettes"
 #define NOM_TABLE_CAPTEUR        "syns_capteurs"
 #define NOM_TABLE_CAMERASUP      "syns_camerasup"

/*************************************** Définitions des prototypes ***************************************/
 extern gboolean Tester_groupe_synoptique( struct LOG *log, struct DB *db,
                                           struct UTILISATEURDB *util, guint syn_id );
 extern struct CMD_TYPE_SYNOPTIQUE *Rechercher_synoptiqueDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Recuperer_synoptiqueDB ( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_SYNOPTIQUE *Recuperer_synoptiqueDB_suite( struct LOG *log, struct DB *db );
 extern gint Ajouter_synoptiqueDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_SYNOPTIQUE *syn );
 extern gboolean Retirer_synoptiqueDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_SYNOPTIQUE *syn );
 extern gboolean Modifier_synoptiqueDB( struct LOG *log, struct DB *db, struct CMD_TYPE_SYNOPTIQUE *syn );

 extern gboolean Retirer_motifDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_MOTIF *motif );
 extern gint Ajouter_motifDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_MOTIF *motif );
 extern gboolean Recuperer_motifDB ( struct LOG *log, struct DB *db, gint id_syn );
 extern struct CMD_TYPE_MOTIF *Recuperer_motifDB_suite( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_MOTIF *Rechercher_motifDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_motifDB( struct LOG *log, struct DB *db, struct CMD_TYPE_MOTIF *motif );

 extern gboolean Retirer_commentDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_COMMENT *comment );
 extern gint Ajouter_commentDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_COMMENT *comment );
 extern gboolean Recuperer_commentDB ( struct LOG *log, struct DB *db, gint id_syn );
 extern struct CMD_TYPE_COMMENT *Recuperer_commentDB_suite( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_COMMENT *Rechercher_commentDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_commentDB( struct LOG *log, struct DB *db, struct CMD_TYPE_COMMENT *comment );

 extern gboolean Retirer_passerelleDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_PASSERELLE *pass );
 extern gint Ajouter_passerelleDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_PASSERELLE *pass );
 extern gboolean Recuperer_passerelleDB ( struct LOG *log, struct DB *db, gint id_syn );
 extern struct CMD_TYPE_PASSERELLE *Recuperer_passerelleDB_suite( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_PASSERELLE *Rechercher_passerelleDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_passerelleDB( struct LOG *log, struct DB *db, struct CMD_TYPE_PASSERELLE *pass );

 extern gboolean Retirer_paletteDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_PALETTE *pass );
 extern gint Ajouter_paletteDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_PALETTE *pass );
 extern gboolean Recuperer_paletteDB ( struct LOG *log, struct DB *db, gint id_syn );
 extern struct CMD_TYPE_PALETTE *Recuperer_paletteDB_suite( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_PALETTE *Rechercher_paletteDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_paletteDB( struct LOG *log, struct DB *db, struct CMD_TYPE_PALETTE *pass );

 extern gboolean Retirer_capteurDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAPTEUR *capteur );
 extern gint Ajouter_capteurDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAPTEUR *capteur );
 extern gboolean Recuperer_capteurDB ( struct LOG *log, struct DB *db, gint id_syn );
 extern struct CMD_TYPE_CAPTEUR *Recuperer_capteurDB_suite( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_CAPTEUR *Rechercher_capteurDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_capteurDB( struct LOG *log, struct DB *db, struct CMD_TYPE_CAPTEUR *capteur );

 extern gboolean Retirer_camera_supDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA_SUP *camera_sup );
 extern gint Ajouter_camera_supDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA_SUP *camera_sup );
 extern gboolean Recuperer_camera_supDB ( struct LOG *log, struct DB *db, gint id_syn );
 extern struct CMD_TYPE_CAMERA_SUP *Recuperer_camera_supDB_suite( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_CAMERA_SUP *Rechercher_camera_supDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_camera_supDB( struct LOG *log, struct DB *db, struct CMD_TYPE_CAMERA_SUP *camera_sup );
#endif
/*--------------------------------------------------------------------------------------------------------*/
