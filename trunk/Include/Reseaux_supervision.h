/******************************************************************************************************************************/
/* Include/Reseaux_supervision.h:   Sous_tag de supervision pour watchdog 2.0 par lefevre Sebastien                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Reseaux_supervision.h
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

#ifndef _RESEAUX_SUPERVISION_H_
 #define _RESEAUX_SUPERVISION_H_

 struct CMD_ETAT_BIT_CTRL
  { guint   num;
    guchar  etat;
    guchar  rouge;
    guchar  vert;
    guchar  bleu;
    guchar  cligno;
  };
 struct CMD_ETAT_BIT_CADRAN
  { guint   bit_controle;
    guint   type;
    gchar   libelle[25];
  };

 struct CMD_SET_BIT_INTERNE
  { guint type;
    guint num;
    gfloat valeur;
  };

 enum 
  { SSTAG_CLIENT_WANT_PAGE_SUPERVISION,                                  /* Le client desire la page de supervision graphique */
    SSTAG_SERVEUR_AFFICHE_PAGE_SUP,
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_MOTIF,                              /* Le serveur envoi des motifs page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_MOTIF_FIN,                          /* Le serveur envoi des motifs page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_COMMENT,                          /* Le serveur envoi des comments page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_COMMENT_FIN,                      /* Le serveur envoi des comments page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PASS,                          /* Le serveur envoi des passerelles page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PASS_FIN,                      /* Le serveur envoi des passerelles page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PALETTE,                          /* Le serveur envoi des palettes page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PALETTE_FIN,                      /* Le serveur envoi des palettes page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CADRAN,                            /* Le serveur envoi des cadrans page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CADRAN_FIN,                        /* Le serveur envoi des cadrans page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAMERA_SUP,                         /* Le serveur envoi des camera page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAMERA_SUP_FIN,                     /* Le serveur envoi des camera page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_HORLOGE,                                     /* Le serveur envoi des ticks horloges */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_HORLOGE_FIN,                                    /* Fin de l'envoi des ticks horloge */

    SSTAG_SERVEUR_SUPERVISION_CHANGE_MOTIF,                            /* Un motif à changé d'etat, nous l'envoyons au client */
    SSTAG_CLIENT_CHANGE_MOTIF_UNKNOWN,                          /* Reponse si le numero Ixxx n'est pas utilisé dans le client */
    SSTAG_SERVEUR_SUPERVISION_CHANGE_CADRAN,                          /* Un cadran à changé d'etat, nous l'envoyons au client */
    SSTAG_CLIENT_CHANGE_CADRAN_UNKNOWN,                         /* Reponse si le numero Ixxx n'est pas utilisé dans le client */
    SSTAG_CLIENT_ACTION_M,                                                            /* Le client envoie un bit M au serveur */
    SSTAG_CLIENT_SET_BIT_INTERNE,                                               /* Le client envoie un bit interne au serveur */
    SSTAG_SERVEUR_SUPERVISION_SET_SYN_VARS,                                           /* Changement des variables synoptiques */
    SSTAG_CLIENT_SET_SYN_VARS_UNKNOWN,                                                                  /* Synoptique inconnu */
    SSTAG_CLIENT_ACQ_SYN,                                                                 /* Le client acquitte un synoptique */
    SSTAG_CLIENT_WANT_HORLOGES,                                    /* Le client demande la liste des horloges d'un synoptique */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_HORLOGES,                         /* Le serveur envoi des scenario page supervision */
    SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_HORLOGES_FIN,                     /* Le serveur envoi des scenario page supervision */
    SSTAG_CLIENT_WANT_HORLOGE,                                    /* Le client demande les ticks d'une horloge en particulier */
    SSTAG_CLIENT_EDIT_HORLOGE,                                                       /* Le client souhaite editer une horloge */
    SSTAG_SERVEUR_EDIT_HORLOGE_OK,
    SSTAG_CLIENT_VALIDE_EDIT_HORLOGE,
    SSTAG_SERVEUR_VALIDE_EDIT_HORLOGE_OK,
    SSTAG_CLIENT_ADD_HORLOGE,
    SSTAG_SERVEUR_ADD_HORLOGE_OK,
    SSTAG_CLIENT_DEL_HORLOGE,
    SSTAG_SERVEUR_DEL_HORLOGE_OK,
  };

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/

