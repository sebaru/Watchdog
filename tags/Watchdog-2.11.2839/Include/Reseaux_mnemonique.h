/**********************************************************************************************************/
/* Include/Reseaux_mnemonique.h:   Sous_tag de mnemonique pour watchdog 2.0 par lefevre Sebastien         */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_mnemonique.h
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

#ifndef _RESEAUX_MNEMONIQUE_H_
 #define _RESEAUX_MNEMONIQUE_H_

 #define NBR_CARAC_LIBELLE_MNEMONIQUE        80
 #define NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8   (2*NBR_CARAC_LIBELLE_MNEMONIQUE)

 #define NBR_CARAC_ACRONYME_MNEMONIQUE       20
 #define NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8  (2*NBR_CARAC_ACRONYME_MNEMONIQUE)

 #define NBR_CARAC_UNITE_MNEMONIQUE          8
 #define NBR_CARAC_UNITE_MNEMONIQUE_UTF8     (2*NBR_CARAC_UNITE_MNEMONIQUE)

 enum
  { MNEMO_BISTABLE,                                               /* Definitions des types de mnemoniques */
    MNEMO_MONOSTABLE,
    MNEMO_TEMPO,
    MNEMO_ENTREE,
    MNEMO_SORTIE,
    MNEMO_ENTREE_ANA,
    MNEMO_SORTIE_ANA,
    MNEMO_MOTIF,
    MNEMO_CPTH,
    MNEMO_CPT_IMP,
    NBR_TYPE_MNEMO
  };

/********************************************** Base de tous les mnemos ***********************************/
 struct CMD_TYPE_MNEMO_BASE                                 /* Informations partagées par tous les mnémos */
  { guint id;                                                /* ID unique du mnemonique dans la structure */
    guint type;                                                                /* Type du bit interne lié */
    guint num;                                                         /* Numéro du bit lié au mnemonique */
    guint num_plugin;                                                     /* Numéro du plugin DLS associé */
    gchar  groupe[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar  page[NBR_CARAC_PAGE_SYNOPTIQUE_UTF8+1];
    gchar  plugin_dls[NBR_CARAC_PLUGIN_DLS_UTF8+1];
    gchar  acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gchar  libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+1];
    gchar  command_text[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+1];
    gchar  tableau[ NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+1 ];
  };

/******************************************* AddOns pour les Digital Input ********************************/
 struct CMD_TYPE_MNEMO_DI
  { gint num; 
    gboolean furtif;
  };

/******************************************* AddOns pour les Analog Input *********************************/
 #define COURBE_TEMPS_TOP             5                     /* 1 point = 5 secondes sur la grille courbes */
 enum
  { ENTREEANA_NON_INTERP,
    ENTREEANA_4_20_MA_12BITS,
    ENTREEANA_4_20_MA_10BITS,
    ENTREEANA_WAGO_750455,
    ENTREEANA_WAGO_750461,
    NBR_TYPE_ENTREEANA
  };

 struct CMD_TYPE_MNEMO_AI
  { guint  num;                                                                         /* Numero de l'EA */
    gfloat min;
    gfloat max;
    guint  type;                                                               /* Type de gestion de l'EA */
    gchar  unite[NBR_CARAC_UNITE_MNEMONIQUE_UTF8+1];                                      /* Km, h, ° ... */
  };

/******************************************* Pour les compteurs d'impulsions ******************************/
 enum
  { CI_TOTALISATEUR,
    CI_MOYENNEUR_SEC,
    CI_MOYENNEUR_MIN,
    NBR_TYPE_CI
  };

 struct CMD_TYPE_MNEMO_CPT_IMP
  { guint  num;                                                                     /* Numero du compteur */
    gfloat valeur;                                                                  /* Valeur du compteur */
    guint  type;                                                                /* Totalisateur/Moyenneur */
    gfloat multi;                                                                       /* Multiplicateur */
    gchar  unite[NBR_CARAC_UNITE_MNEMONIQUE_UTF8+1];                                      /* Km, h, ° ... */
  };

/******************************************* Pour les temporisations **************************************/
 struct CMD_TYPE_MNEMO_TEMPO
  { guint  num;                                                                     /* Numero de la tempo */
    guint delai_on;                                 /* delai avant mise à un (fixé par option mnémonique) */
    guint delai_off;                              /* delai avant mise à zero (fixé par option mnémonique) */
    guint min_on;        /* Durée minimale pendant laquelle la tempo sera ON (fixé par option mnémonique) */
    guint max_on;        /* Durée maximale pendant laquelle la tempo sera ON (fixé par option mnémonique) */
  };

/*********************************************** Suite des structures *************************************/
 struct CMD_TYPE_MNEMONIQUES
  { guint nbr_mnemos;                                /* Nombre de structure CMD_TYPE_MNEMONIQUE suivantes */
    struct CMD_TYPE_MNEMO_BASE mnemo[];
  };

 struct CMD_TYPE_NUM_MNEMONIQUE
  { guint type;
    guint num;
  };

 struct CMD_TYPE_MNEMO_FULL
  { struct CMD_TYPE_MNEMO_BASE mnemo_base;
    union { struct CMD_TYPE_MNEMO_DI mnemo_di;
            struct CMD_TYPE_MNEMO_AI mnemo_ai;
            struct CMD_TYPE_MNEMO_CPT_IMP mnemo_cptimp;
            struct CMD_TYPE_MNEMO_TEMPO mnemo_tempo;
          };

  };

 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE,                      /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE_FIN,                  /* Ajout d'un groupe dans la liste cliente */
    SSTAG_CLIENT_WANT_PAGE_MNEMONIQUE,                            /* Le client demande la page mnemonique */
    SSTAG_SERVEUR_CREATE_PAGE_MNEMO_OK,              /* Les serveur repond OK pour la creation de la page */
    SSTAG_CLIENT_ADD_MNEMONIQUE,                           /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_MNEMONIQUE_OK,                                   /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_MNEMONIQUE,                                    /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_MNEMONIQUE_OK,                                   /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_MNEMONIQUE,                              /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_MNEMONIQUE_OK,          /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_MNEMONIQUE,                         /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_MNEMONIQUE_OK,                   /* Le serveur valide les nouvelles données */

    SSTAG_CLIENT_WANT_DLS_FOR_MNEMO,                             /* Envoi des synoptiques pour les mnemos */
    SSTAG_SERVEUR_ADDPROGRESS_DLS_FOR_MNEMO,
    SSTAG_SERVEUR_ADDPROGRESS_DLS_FOR_MNEMO_FIN,
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/
