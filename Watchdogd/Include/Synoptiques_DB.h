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

 struct SYNOPTIQUEDB
  { guint id;                                                      /* Numero du message dans la structure */
    gchar  libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar  mnemo[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];
    guint  groupe;
  };

 struct MOTIFDB
  { guint  id;
    gint   icone_id;                                                        /* Correspond au fichier .gif */
    gint   syn_id;
    gchar  libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                              /* "Vanne gaz chaudière" */
    guint  gid;                                                  /* Nom du groupe d'appartenance du motif */
    gint   bit_controle;                                                                    /* Ixxx, Cxxx */
    gint   bit_clic;                          /* Bit à activer quand on clic avec le bouton gauche souris */
    gint   bit_clic2;                         /* Bit à activer quand on clic avec le bouton gauche souris */
    guint  position_x;                                                        /* en abscisses et ordonnées */
    guint  position_y;
    gfloat largeur;                                               /* Taille de l'image sur le synoptique */
    gfloat hauteur;
    gfloat angle;
    gchar  type_dialog;                          /* Type de la boite de dialogue pour le clic de commande */
    guchar rouge0;
    guchar vert0;
    guchar bleu0;
    gchar  type_gestion;                                                   /* Statique/dynamique/cyclique */
  };
/************************************** Définition d'une passerelle inter-synoptique **********************/
 struct PASSERELLEDB
  { guint  id;
    gchar  libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];                           /* "Vanne gaz chaudière" */
    gint   syn_id;                                                   /* Elle est sur quel synoptique ???? */
    gint   syn_cible_id;                                                    /* Numéro du synoptique cible */
    gint   bit_controle;            /* Numéro du bit interne de controle de l'aspect du bouton passerelle */
    gint   bit_controle_1;          /* Numéro du bit interne de controle de l'aspect du bouton passerelle */
    gint   bit_controle_2;          /* Numéro du bit interne de controle de l'aspect du bouton passerelle */
    guint  position_x;                                                       /* en abscisses et ordonnées */
    guint  position_y;
    gfloat angle;
  };
/************************************** Définition d'un commentaire ***************************************/
 struct COMMENTDB
  { guint   id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                             /* "Vanne gaz chaudière" */
    gchar   font[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                                  /* police de caractère */
    guchar  rouge;
    guchar  vert;
    guchar  bleu;
    guint   position_x;
    guint   position_y;
    gfloat angle;
  };
/************************************** Définition d'un commentaire ***************************************/
 struct PALETTEDB
  { guint   id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gint    syn_cible_id;                                                   /* Numéro du synoptique cible */
    gchar   libelle[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];
    gint    position;                                            /* Position dans l'ensemble des palettes */
  };
/************************************** Définition d'une passerelle inter-synoptique **********************/
 struct CAPTEURDB
  { guint  id;
    gchar  libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                              /* "Vanne gaz chaudière" */
    gint   type;                                                               /* type du bit de controle */
    gint   syn_id;                                                   /* Elle est sur quel synoptique ???? */
    gint   bit_controle;            /* Numéro du bit interne de controle de l'aspect du bouton passerelle */
    guint  position_x;                                                       /* en abscisses et ordonnées */
    guint  position_y;
    gfloat angle;
  };
/*************************************** Définitions des prototypes ***************************************/
 extern gboolean Tester_groupe_synoptique( struct LOG *log, struct DB *db,
                                           struct UTILISATEURDB *util, guint syn_id );
 extern struct SYNOPTIQUEDB *Rechercher_synoptiqueDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Recuperer_synoptiqueDB ( struct LOG *log, struct DB *db );
 extern struct SYNOPTIQUEDB *Recuperer_synoptiqueDB_suite( struct LOG *log, struct DB *db );
 extern gint Ajouter_synoptiqueDB ( struct LOG *log, struct DB *db, struct CMD_ADD_SYNOPTIQUE *syn );
 extern gboolean Retirer_synoptiqueDB ( struct LOG *log, struct DB *db, struct CMD_ID_SYNOPTIQUE *syn );
 extern gboolean Modifier_synoptiqueDB( struct LOG *log, struct DB *db, struct CMD_EDIT_SYNOPTIQUE *syn );

 extern gboolean Retirer_motifDB ( struct LOG *log, struct DB *db, struct CMD_ID_MOTIF *motif );
 extern gint Ajouter_motifDB ( struct LOG *log, struct DB *db, struct CMD_ADD_MOTIF *motif );
 extern SQLHSTMT Recuperer_motifDB ( struct LOG *log, struct DB *db, gint id_syn );
 extern struct MOTIFDB *Recuperer_motifDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery );
 extern struct MOTIFDB *Rechercher_motifDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_motifDB( struct LOG *log, struct DB *db, struct CMD_EDIT_MOTIF *motif );

 extern gboolean Retirer_commentDB ( struct LOG *log, struct DB *db, struct CMD_ID_COMMENT *comment );
 extern gint Ajouter_commentDB ( struct LOG *log, struct DB *db, struct CMD_ADD_COMMENT *comment );
 extern SQLHSTMT Recuperer_commentDB ( struct LOG *log, struct DB *db, gint id_syn );
 extern struct COMMENTDB *Recuperer_commentDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery );
 extern struct COMMENTDB *Rechercher_commentDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_commentDB( struct LOG *log, struct DB *db, struct CMD_EDIT_COMMENT *comment );

 extern gboolean Retirer_passerelleDB ( struct LOG *log, struct DB *db, struct CMD_ID_PASSERELLE *pass );
 extern gint Ajouter_passerelleDB ( struct LOG *log, struct DB *db, struct CMD_ADD_PASSERELLE *pass );
 extern SQLHSTMT Recuperer_passerelleDB ( struct LOG *log, struct DB *db, gint id_syn );
 extern struct PASSERELLEDB *Recuperer_passerelleDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery );
 extern struct PASSERELLEDB *Rechercher_passerelleDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_passerelleDB( struct LOG *log, struct DB *db, struct CMD_EDIT_PASSERELLE *pass );

 extern gboolean Retirer_paletteDB ( struct LOG *log, struct DB *db, struct CMD_ID_PALETTE *pass );
 extern gint Ajouter_paletteDB ( struct LOG *log, struct DB *db, struct CMD_ADD_PALETTE *pass );
 extern SQLHSTMT Recuperer_paletteDB ( struct LOG *log, struct DB *db, gint id_syn );
 extern struct PALETTEDB *Recuperer_paletteDB_suite( struct LOG *log, struct DB *db, SQLHSTMT hquery );
 extern struct PALETTEDB *Rechercher_paletteDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_paletteDB( struct LOG *log, struct DB *db, struct CMD_EDIT_PALETTE *pass );

 extern gboolean Retirer_capteurDB ( struct LOG *log, struct DB *db, struct CMD_ID_CAPTEUR *capteur );
 extern gint Ajouter_capteurDB ( struct LOG *log, struct DB *db, struct CMD_ADD_CAPTEUR *capteur );
 extern gboolean Recuperer_capteurDB ( struct LOG *log, struct DB *db, gint id_syn );
 extern struct CAPTEURDB *Recuperer_capteurDB_suite( struct LOG *log, struct DB *db );
 extern struct CAPTEURDB *Rechercher_capteurDB ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_capteurDB( struct LOG *log, struct DB *db, struct CMD_EDIT_CAPTEUR *capteur );
#endif
/*--------------------------------------------------------------------------------------------------------*/
