/******************************************************************************************************************************/
/* Include/Reseaux_mnemonique.h:   Sous_tag de mnemonique pour watchdog 2.0 par lefevre Sebastien                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Reseaux_mnemonique.h
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

#ifndef _RESEAUX_MNEMONIQUE_H_
 #define _RESEAUX_MNEMONIQUE_H_

 #define NBR_CARAC_LIBELLE_MNEMONIQUE        80
 #define NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8   (2*NBR_CARAC_LIBELLE_MNEMONIQUE)

 #define NBR_CARAC_ACRONYME_MNEMONIQUE       20
 #define NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8  (2*NBR_CARAC_ACRONYME_MNEMONIQUE)

 #define NBR_CARAC_UNITE_MNEMONIQUE          8
 #define NBR_CARAC_UNITE_MNEMONIQUE_UTF8     (2*NBR_CARAC_UNITE_MNEMONIQUE)

 #define NBR_CARAC_ACRO_SYN_MNEMONIQUE       20
 #define NBR_CARAC_ACRO_SYN_MNEMONIQUE_UTF8  (2*NBR_CARAC_ACRO_SYN_MNEMONIQUE)


/**************************************************** AddOns pour les Analog Input ********************************************/

 struct CMD_TYPE_MNEMO_AI
  { gfloat min;
    gfloat max;
    guint  type;                                                                                   /* Type de gestion de l'EA */
    gchar  unite[NBR_CARAC_UNITE_MNEMONIQUE_UTF8+1];                                                          /* Km, h, ° ... */
  };

/**************************************************** AddOns pour les Registres ***********************************************/
 struct CMD_TYPE_MNEMO_REGISTRE
  { gchar  unite[NBR_CARAC_UNITE_MNEMONIQUE_UTF8+1];                                                          /* Km, h, ° ... */
  };

/****************************************************** Pour les compteurs d'impulsions ***************************************/
 enum
  { CI_TOTALISATEUR,
    CI_MOYENNEUR_SEC,
    CI_MOYENNEUR_MIN,
    NBR_TYPE_CI
  };

 struct CMD_TYPE_MNEMO_CPT_IMP
  { gfloat valeur;                                                                                      /* Valeur du compteur */
    guint  type;                                                                                    /* Totalisateur/Moyenneur */
    gfloat multi;                                                                                           /* Multiplicateur */
    gchar  unite[NBR_CARAC_UNITE_MNEMONIQUE_UTF8+1];                                                          /* Km, h, ° ... */
    gboolean reset;                                                             /* TRUE si le compteur doit etre resetter à 0 */
  };

/************************************************ Pour les compteurs horaires *************************************************/
 struct CMD_TYPE_MNEMO_CPT_H
  { guint  valeur;                                                                                      /* Valeur du compteur */
  };

/******************************************************* Suite des structures *************************************************/

 struct CMD_TYPE_NUM_MNEMONIQUE
  { guint type;
    guint num;
  };


 enum
  { SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE,                                          /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE_FIN,                                      /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_ALL_MNEMONIQUE_FIN,                                  /* Ajout d'un groupe dans la liste cliente */
    SSTAG_CLIENT_WANT_PAGE_MNEMONIQUE,                                                /* Le client demande la page mnemonique */
    SSTAG_CLIENT_WANT_PAGE_ALL_MNEMONIQUE,                                            /* Le client demande la page mnemonique */
    SSTAG_CLIENT_ADD_MNEMONIQUE,                                               /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_MNEMONIQUE_OK,                                                       /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_MNEMONIQUE,                                                        /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_MNEMONIQUE_OK,                                                       /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_MNEMONIQUE,                                                  /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_MNEMONIQUE_OK,                              /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_MNEMONIQUE,                                             /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_MNEMONIQUE_OK,                                       /* Le serveur valide les nouvelles données */
  };

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
