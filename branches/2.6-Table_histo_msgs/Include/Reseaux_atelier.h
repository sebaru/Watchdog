/**********************************************************************************************************/
/* Include/Reseaux_atelier.h:   Sous_tag de l'atelier pour watchdog 2.0 par lefevre Sebastien             */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 10:44:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_atelier.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
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

/************************************** Qu'est-ce qu'une camera de supervision ?? *************************/
 #define DEFAULT_CAMERA_LARGEUR        160
 #define DEFAULT_CAMERA_HAUTEUR        120

 struct CMD_TYPE_MOTIF
  { gint    id;                                                               /* Id du motif dans la base */
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gint    icone_id;                                                       /* Correspond au fichier .gif */
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                             /* "Vanne gaz chaudière" */
    guint   gid;                                                 /* Nom du groupe d'appartenance du motif */
    gint    bit_controle;                                                                   /* Ixxx, Cxxx */
    gint    bit_clic;                         /* Bit à activer quand on clic avec le bouton gauche souris */
    gint    bit_clic2;                        /* Bit à activer quand on clic avec le bouton gauche souris */
    gint    rafraich;                                     /* Temps de rafraichissement du motif à l'ecran */
    guint   position_x;                                                      /* en abscisses et ordonnées */
    guint   position_y;
    gfloat  largeur;                                               /* Taille de l'image sur le synoptique */
    gfloat  hauteur;
    gfloat  angle;
    gchar   type_dialog;                         /* Type de la boite de dialogue pour le clic de commande */
    guchar  rouge0;
    guchar  vert0;
    guchar  bleu0;
    gchar   type_gestion;                                                  /* Statique/dynamique/cyclique */
    gint    layer;
  };
 struct CMD_TYPE_MOTIFS
  { guint nbr_motifs;                                     /* Nombre de structure CMD_TYPE_MOTIF suivantes */
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
/*********************************************** Gestion des passerelles **********************************/
 struct CMD_TYPE_PASSERELLE
  { gint    id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gint    syn_cible_id;                                                   /* Numéro du synoptique cible */
    gchar   libelle[NBR_CARAC_PAGE_SYNOPTIQUE_UTF8+1];                     /* Libelle du synoptique cible */
    gint    bit_controle;                                                        /* Ixxx, Cxxx A virer ?? */
    guint   position_x;                                                      /* en abscisses et ordonnées */
    guint   position_y;
    guint   bit_controle_1;                                       /* Numéro Ixxx du premier bit de retour */
    guint   bit_controle_2;                                        /* Numéro Ixxx du second bit de retour */
    guint   bit_controle_3;                                         /* Numéro Ixxx du 3ieme bit de retour */
    gfloat  angle;
  };

/*********************************************** Gestion des passerelles **********************************/
 struct CMD_TYPE_PALETTE
  { gint    id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gint    syn_cible_id;                                                   /* Numéro du synoptique cible */
    gchar   libelle[NBR_CARAC_PAGE_SYNOPTIQUE_UTF8+1];                     /* Libelle du synoptique cible */
    guint   position;                                                        /* en abscisses et ordonnées */
  };
/*********************************************** Gestion des capteurs ***************************************/
 struct CMD_TYPE_CAPTEUR
  { gint    id;
    gint    syn_id;                                                /* Numéro du synoptique ou est l'icone */
    gchar   libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                       /* Libelle du synoptique cible */
    gint    type;                                                              /* type du bit de controle */
    gint    bit_controle;                                                                   /* Ixxx, Cxxx */
    guint   position_x;                                                      /* en abscisses et ordonnées */
    guint   position_y;
    gfloat  angle;
  };

/*********************************************** Gestion des cameras de supervision ***********************/
 struct CMD_TYPE_CAMERA_SUP
  { gint  id;
    gint  syn_id;
    gint  num;                                                       /* Némero (utilisateur) de la camera */
    gchar libelle[NBR_CARAC_LIBELLE_MOTIF_UTF8+1];                                             /* "ChSeb" */
    gint  camera_src_id;
    gchar location[NBR_CARAC_LOCATION_CAMERA_UTF8];                               /* Libelle de la camera */
    gint  position_x;
    gint  position_y;
    gint  type;
    gchar objet[128];
    gint  bit;                        /* Numéro du bistable a positioner en cas de detection de mouvement */

  };
/************************************************* Tag de communication ***********************************/
 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_PROPRIETE_SYNOPTIQUE,  /* Envoi des groupes pour l'edition motif */
    SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_PROPRIETE_SYNOPTIQUE_FIN,            /* Le transfert est terminé */

    SSTAG_CLIENT_ATELIER_SYNOPTIQUE,                 /* Le client desire editer par atelier le synoptique */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF,         /* Le serveur envoi des motifs dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF_FIN,     /* Le serveur envoi des motifs dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT,     /* Le serveur envoi des comments dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT_FIN, /* Le serveur envoi des comments dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS,            /* Le serveur envoi des pass dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS_FIN,        /* Le serveur envoi des pass dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE,     /* Le serveur envoi des palettes dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE_FIN, /* Le serveur envoi des palettes dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAPTEUR,     /* Le serveur envoi des capteurs dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAPTEUR_FIN, /* Le serveur envoi des capteurs dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAMERA_SUP,    /* Le serveur envoi des camera dans l'atelier client */
    SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAMERA_SUP_FIN,/* Le serveur envoi des camera dans l'atelier client */

    SSTAG_CLIENT_ATELIER_DEL_MOTIF,                  /* Le client desire editer par atelier le synoptique */
    SSTAG_SERVEUR_ATELIER_DEL_MOTIF_OK,              /* Le client desire editer par atelier le synoptique */
    SSTAG_CLIENT_ATELIER_ADD_MOTIF,                    /* Le client desire ajouter un motif au synoptique */
    SSTAG_SERVEUR_ATELIER_ADD_MOTIF_OK,                                          /* L'ajout est un succes */
    SSTAG_CLIENT_ATELIER_EDIT_MOTIF,                /* Le client envoi les propriétés du motif au serveur */

    SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE,              /* Le client veut un mnemonique du bit TYPE-NUM (B001) */
    SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE,                                     /* Le serveur repond au client */

    SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_EA,                          /* Le client desire un mnémonique EAxxx */
    SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE_EA,
    
    SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_PASS,                         /* Le client desire un mnémonique Ixxx */
    SSTAG_SERVEUR_TYPE_NUM_MNEMONIQUE_PASS,

    SSTAG_CLIENT_WANT_PAGE_CLASSE_FOR_ATELIER,        /* Le client veut les données classe pour l'atelier */
    SSTAG_SERVEUR_ADDPROGRESS_CLASSE_FOR_ATELIER, /* Le serveur envoie les données classes pour l'atelier */
    SSTAG_SERVEUR_ADDPROGRESS_CLASSE_FOR_ATELIER_FIN,
    SSTAG_CLIENT_WANT_PAGE_ICONE_FOR_ATELIER,          /* Le client veut les données icone pour l'atelier */
    SSTAG_SERVEUR_ADDPROGRESS_ICONE_FOR_ATELIER,   /* Le serveur envoie les données icones pour l'atelier */
    SSTAG_SERVEUR_ADDPROGRESS_ICONE_FOR_ATELIER_FIN,

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

    SSTAG_CLIENT_ATELIER_ADD_CAPTEUR,      /* Le client veut ajouter un capteur au syn en cours d'edition */
    SSTAG_SERVEUR_ATELIER_ADD_CAPTEUR_OK,                       /* Le serveur valide l'ajout d'un capteur */
    SSTAG_CLIENT_ATELIER_DEL_CAPTEUR,                               /* Le client veut detruire un capteur */
    SSTAG_SERVEUR_ATELIER_DEL_CAPTEUR_OK,                                             /* Destruction OKAY */
    SSTAG_CLIENT_ATELIER_EDIT_CAPTEUR,            /* Le serveur envoi les propriétés du capteur au client */

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
/*--------------------------------------------------------------------------------------------------------*/

