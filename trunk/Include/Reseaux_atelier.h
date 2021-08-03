/******************************************************************************************************************************/
/* Include/Reseaux_atelier.h:   Sous_tag de l'atelier pour watchdog 2.0 par Lefevre Sebastien                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                                       dim. 13 sept. 2009 10:44:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Reseaux_atelier.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien LEFEVRE
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

#ifndef _RESEAUX_ATELIER_H_
 #define _RESEAUX_ATELIER_H_

 #define NBR_CARAC_LIBELLE_MOTIF       50                               /* taille max du libelle du motif */
 #define NBR_CARAC_LIBELLE_MOTIF_UTF8  (2*NBR_CARAC_LIBELLE_MOTIF)

 #define NBR_CARAC_LIBELLE_COMMENT     50                               /* taille max du libelle du motif */
 #define NBR_CARAC_LIBELLE_COMMENT_UTF8  (2*NBR_CARAC_LIBELLE_COMMENT)

 struct CMD_TYPE_MOTIF
  { gint    id;                                                                                   /* Id du motif dans la base */
    gint    syn_id;                                                                    /* Numéro du synoptique ou est l'icone */
    gint    icone_id;                                                                           /* Correspond au fichier .gif */
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                                                 /* "Vanne gaz chaudière" */
    guint   access_level;                                                            /* Nom du groupe d'appartenance du motif */
    gchar   clic_tech_id[32];                                                  /* tech id du dls a positionner si clic gauche */
    gchar   clic_acronyme[64];                                                /* acronyme du dls a positionner si clic gauche */
    gint    rafraich;                                                         /* Temps de rafraichissement du motif à l'ecran */
    guint   position_x;                                                                          /* en abscisses et ordonnées */
    guint   position_y;
    gint    largeur;                                                                   /* Taille de l'image sur le synoptique */
    gint    hauteur;
    gint    angle;
    gchar   def_color[16];                                                    /* acronyme du dls a positionner si clic gauche */
    gchar   type_dialog;                                             /* Type de la boite de dialogue pour le clic de commande */
    gchar   type_gestion;                                                                      /* Statique/dynamique/cyclique */
    gint    layer;
    gchar   tech_id[32];                                                                         /* Visuel Acronyme du cadran */
    gchar   acronyme[64];                                                                        /* Visuel Acronyme du cadran */
  };
 struct CMD_TYPE_MOTIFS
  { guint nbr_motifs;                                                         /* Nombre de structure CMD_TYPE_MOTIF suivantes */
    struct CMD_TYPE_MOTIF motif[];
  };

/*********************************************** Gestion des commentaires *********************************/
 struct CMD_TYPE_COMMENT
  { gint    id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gchar   libelle[NBR_CARAC_LIBELLE_COMMENT_UTF8+1];                           /* "Vanne gaz chaudière" */
    gchar   font[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                                  /* police de caractère */
    guchar  rouge;
    guchar  vert;
    guchar  bleu;
    guint   position_x;
    guint   position_y;
    gfloat  angle;
  };
/**************************************************** Gestion des passerelles *************************************************/
 struct CMD_TYPE_PASSERELLE
  { gint    id;
    gint    syn_id;                                                                    /* Numéro du synoptique ou est l'icone */
    gint    syn_cible_id;                                                                       /* Numéro du synoptique cible */
    gchar   libelle[NBR_CARAC_PAGE_SYNOPTIQUE_UTF8+1];                                         /* Libelle du synoptique cible */
    guint   position_x;                                                                          /* en abscisses et ordonnées */
    guint   position_y;
    gfloat  angle;
  };

/******************************************************* Gestion des palettes *************************************************/
 struct CMD_TYPE_PALETTE
  { gint    id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gint    syn_cible_id;                                                   /* Numéro du synoptique cible */
    gchar   libelle[NBR_CARAC_PAGE_SYNOPTIQUE_UTF8+1];                     /* Libelle du synoptique cible */
    guint   position;                                                        /* en abscisses et ordonnées */
  };
/****************************************************** Gestion des cadrans ***************************************************/
 struct CMD_TYPE_CADRAN
  { gint    id;
    gchar   tech_id[32];                                                                         /* Visuel Acronyme du cadran */
    gchar   acronyme[64];                                                                        /* Visuel Acronyme du cadran */
    gint    syn_id;                                                                    /* Numéro du synoptique ou est l'icone */
    gint    type;                                                                                  /* type du bit de controle */
    guint   position_x;                                                                          /* en abscisses et ordonnées */
    guint   position_y;
    gint    angle;
    gint    nb_decimal;                                                                       /* Nombre de decimal a afficher */
  };

/************************************************* Gestion des Scenario *******************************************************/
 struct CMD_TYPE_SCENARIO
  { gint  id;
    gint  syn_id;
    gint  num;
    gint  posx;
    gint  posy;
  };

 struct CMD_TYPE_SYN_VARS
  { gint syn_id;
    gboolean bit_comm;
    gboolean bit_defaut;
    gboolean bit_defaut_fixe;
    gboolean bit_alarme;
    gboolean bit_alarme_fixe;
    gboolean bit_veille_partielle;
    gboolean bit_veille_totale;
    gboolean bit_alerte;
    gboolean bit_alerte_fixe;
    gboolean bit_alerte_fugitive;
    gboolean bit_derangement;
    gboolean bit_derangement_fixe;
    gboolean bit_danger;
    gboolean bit_danger_fixe;
  };

/******************************************************* Tag de communication *************************************************/
 enum
  { SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_PROPRIETE_SYNOPTIQUE,                      /* Envoi des groupes pour l'edition motif */
    SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_PROPRIETE_SYNOPTIQUE_FIN,                                /* Le transfert est terminé */

    SSTAG_CLIENT_ATELIER_SYNOPTIQUE,                                     /* Le client desire editer par atelier le synoptique */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF,                             /* Le serveur envoi des motifs dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF_FIN,                         /* Le serveur envoi des motifs dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT,                         /* Le serveur envoi des comments dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT_FIN,                     /* Le serveur envoi des comments dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS,                                /* Le serveur envoi des pass dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS_FIN,                            /* Le serveur envoi des pass dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE,                         /* Le serveur envoi des palettes dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE_FIN,                     /* Le serveur envoi des palettes dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CADRAN,                           /* Le serveur envoi des cadrans dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CADRAN_FIN,                       /* Le serveur envoi des cadrans dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAMERA_SUP,                        /* Le serveur envoi des camera dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAMERA_SUP_FIN,                    /* Le serveur envoi des camera dans l'atelier client */

    SSTAG_CLIENT_ATELIER_DEL_MOTIF,                  /* Le client desire editer par atelier le synoptique */
    SSTAG_SERVEUR_ATELIER_DEL_MOTIF_OK,              /* Le client desire editer par atelier le synoptique */
    SSTAG_CLIENT_ATELIER_ADD_MOTIF,                    /* Le client desire ajouter un motif au synoptique */
    SSTAG_SERVEUR_ATELIER_ADD_MOTIF_OK,                                          /* L'ajout est un succes */
    SSTAG_CLIENT_ATELIER_EDIT_MOTIF,                /* Le client envoi les propriétés du motif au serveur */

    SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE,              /* Le client veut un mnemonique du bit TYPE-NUM (B001) */
    SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE,                                     /* Le serveur repond au client */

    SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_EA,                          /* Le client desire un mnémonique EAxxx */
    SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE_EA,

    SSTAG_CLIENT_ATELIER_ADD_COMMENT,  /* Le client veut ajouter un commentaire au syn en cours d'edition */
    SSTAG_SERVEUR_ATELIER_ADD_COMMENT_OK,                   /* Le serveur valide l'ajout d'un commentaire */
    SSTAG_CLIENT_ATELIER_DEL_COMMENT,                           /* Le client veut detruire un commentaire */
    SSTAG_SERVEUR_ATELIER_DEL_COMMENT_OK,            /* Le client desire editer par atelier le synoptique */
    SSTAG_CLIENT_ATELIER_EDIT_COMMENT,            /* Le client envoi les propriétés du comment au serveur */

    SSTAG_CLIENT_ATELIER_ADD_PASS,            /* Le client veut ajouter un pass au syn en cours d'edition */
    SSTAG_SERVEUR_ATELIER_ADD_PASS_OK,                             /* Le serveur valide l'ajout d'un pass */
    SSTAG_CLIENT_ATELIER_DEL_PASS,                                     /* Le client veut detruire un pass */
    SSTAG_SERVEUR_ATELIER_DEL_PASS_OK,               /* Le client desire editer par atelier le synoptique */
    SSTAG_CLIENT_ATELIER_EDIT_PASS,                  /* Le client envoi les propriétés du pass au serveur */

    SSTAG_CLIENT_ATELIER_ADD_PALETTE,      /* Le client veut ajouter un palette au syn en cours d'edition */
    SSTAG_SERVEUR_ATELIER_ADD_PALETTE_OK,                       /* Le serveur valide l'ajout d'un palette */
    SSTAG_CLIENT_ATELIER_DEL_PALETTE,                               /* Le client veut detruire un palette */
    SSTAG_SERVEUR_ATELIER_DEL_PALETTE_OK,            /* Le client desire editer par atelier le synoptique */
    SSTAG_CLIENT_ATELIER_EDIT_PALETTE,            /* Le client envoi les propriétés du palette au serveur */

    SSTAG_CLIENT_ATELIER_ADD_CADRAN,        /* Le client veut ajouter un cadran au syn en cours d'edition */
    SSTAG_SERVEUR_ATELIER_ADD_CADRAN_OK,                         /* Le serveur valide l'ajout d'un cadran */
    SSTAG_CLIENT_ATELIER_DEL_CADRAN,                                 /* Le client veut detruire un cadran */
    SSTAG_SERVEUR_ATELIER_DEL_CADRAN_OK,                                              /* Destruction OKAY */
    SSTAG_CLIENT_ATELIER_EDIT_CADRAN,              /* Le serveur envoi les propriétés du cadran au client */

    SSTAG_CLIENT_ATELIER_ADD_CAMERA_SUP,/* Le client veut ajouter une camera_sup au syn en cours d'edition */
    SSTAG_SERVEUR_ATELIER_ADD_CAMERA_SUP_OK,                /* Le serveur valide l'ajout d'une camera_sup */
    SSTAG_CLIENT_ATELIER_DEL_CAMERA_SUP,                        /* Le client veut detruire une camera_sup */
    SSTAG_SERVEUR_ATELIER_DEL_CAMERA_SUP_OK,                                          /* Destruction OKAY */
    SSTAG_CLIENT_ATELIER_EDIT_CAMERA_SUP,

    SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE_FOR_ATELIER,    /* Le client veut les données classe pour l'atelier */
    SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER,
    SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_FIN,

    SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE_FOR_ATELIER_PALETTE,     /* noms des syn pour les choix de palettes */
    SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_PALETTE,
    SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_PALETTE_FIN,

    SSTAG_CLIENT_WANT_PAGE_CAMERA_FOR_ATELIER,                 /* noms des syn pour les choix de palettes */
    SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FOR_ATELIER,
    SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FOR_ATELIER_FIN,

    SSTAG_CLIENT_TYPE_NUM_MNEMO_CLIC,                    /* Envoi des informations mnemonique à l'atelier */
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC,                   /* Envoi des informations mnemonique à l'atelier */
    SSTAG_CLIENT_TYPE_NUM_MNEMO_CTRL,
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_CTRL,
    SSTAG_CLIENT_TYPE_NUM_MNEMO_CLIC2,
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_CLIC2,
  };

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
