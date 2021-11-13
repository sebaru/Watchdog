/******************************************************************************************************************************/
/* Include/Reseaux_dls.h:   Sous_tag de dls utilisé pour watchdog 2.0   par Lefevre Sebastien                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         sam. 31 déc. 2011 17:34:34 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Reseaux_dls.h
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

#ifndef _RESEAUX_DLS_H_
 #define _RESEAUX_DLS_H_

 #define NBR_CARAC_PLUGIN_DLS       40                                                                 /* Nom d'un plugin DLS */
 #define NBR_CARAC_PLUGIN_DLS_UTF8  (2*NBR_CARAC_PLUGIN_DLS)

 #define NBR_BIT_DLS           10000

 #define NBR_ENTRE_TOR         512
 #define NBR_ENTRE_ANA         128
 #define NBR_SORTIE_TOR        512
 #define NBR_SORTIE_ANA        128
 #define NBR_BIT_CONTROLE      NBR_BIT_DLS  /* Ixxx */
 #define NBR_BIT_BISTABLE      NBR_BIT_DLS  /* Bxxx */
 #define NBR_BIT_MONOSTABLE    NBR_BIT_DLS  /* Mxxx */
 #define NBR_TEMPO             512
 #define NBR_MNEMONIQUE        NBR_BIT_DLS
 #define NBR_MESSAGE_ECRITS    NBR_BIT_DLS
 #define NBR_COMPTEUR_H        100
 #define NBR_COMPTEUR_IMP      100
 #define NBR_SCENARIO          100
 #define NBR_CAMERA            16

 struct CMD_TYPE_PLUGIN_DLS
  { gchar nom[ NBR_CARAC_PLUGIN_DLS_UTF8 + 1 ];
    gchar groupe[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar page[NBR_CARAC_PAGE_SYNOPTIQUE_UTF8+1];
    guint num_syn;                                          /* Numéro du fichier syn correspondant(pas l'index dans la table) */
    guint id;
    guint on;
    guint type;                                                                           /* Module, Sous groupe, groupe, ... */
    guint compil_date;                                                                        /* Date de derniere compilation */
    guint compil_status;                                                                    /* Statut de derniere compilation */
  };
 
 struct CMD_TYPE_SOURCE_DLS
  { guint id;
    guint taille;                                                     /* Taille des données qui suivent dans le paquet reseau */
  };

 enum                                                                                  /* Code retour de la compilation D.L.S */
  { DLS_COMPIL_NEVER_COMPILED_YET,
    DLS_COMPIL_ERROR_LOAD_SOURCE,
    DLS_COMPIL_ERROR_LOAD_LOG,
    DLS_COMPIL_ERROR_TRAD,
    DLS_COMPIL_ERROR_FORK_GCC,
    DLS_COMPIL_OK_WITH_WARNINGS,
    DLS_COMPIL_OK,
    NBR_DLS_COMPIL_STATUS
  };

 enum
  { PLUGIN_MODULE,                                                                         /* Definitions des types de plugin */
    PLUGIN_SSGROUPE,
    PLUGIN_GROUPE,
    PLUGIN_TOPLEVEL,
    NBR_TYPE_PLUGIN
  };

 enum 
  { SSTAG_CLIENT_WANT_PAGE_DLS,
    SSTAG_SERVEUR_CREATE_PAGE_DLS_OK,                                                             /* Affichage de la page DLS */
    SSTAG_SERVEUR_ADDPROGRESS_PLUGIN_DLS,                                                                /* Envoi des plugins */
    SSTAG_SERVEUR_ADDPROGRESS_PLUGIN_DLS_FIN,
    SSTAG_CLIENT_ADD_PLUGIN_DLS,                                          /* Le client desire ajouter un utilisateur watchdog */
    SSTAG_SERVEUR_ADD_PLUGIN_DLS_OK,                                                  /* L'ajout du utilisateur est un succes */

    SSTAG_CLIENT_DEL_PLUGIN_DLS,                                                   /* Le client desire retirer un utilisateur */
    SSTAG_SERVEUR_DEL_PLUGIN_DLS_OK,                                                  /* L'ajout du utilisateur est un succes */

    SSTAG_CLIENT_EDIT_PLUGIN_DLS,                                                  /* Le client demande l'edition d'un plugin */
    SSTAG_SERVEUR_EDIT_PLUGIN_DLS_OK,                              /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_PLUGIN_DLS,                                             /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_PLUGIN_DLS_OK,                                       /* Le serveur valide les nouvelles données */

    SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS_DEB,                                         /* Le client renvoie les données editées */
    SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS,                                             /* Le client renvoie les données editées */
    SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS_FIN,                                         /* Le client renvoie les données editées */

    SSTAG_CLIENT_WANT_SOURCE_DLS,                                     /* Le client demande de recupérer un fichier DLS source */
    SSTAG_SERVEUR_SOURCE_DLS_START,
    SSTAG_SERVEUR_SOURCE_DLS,
    SSTAG_SERVEUR_SOURCE_DLS_END,

    SSTAG_SERVEUR_DLS_COMPIL_STATUS,                                                           /* Résultat de ocmpilation DLS */
            
    SSTAG_CLIENT_WANT_SYN_FOR_PLUGIN_DLS,                                           /* Envoi des synoptiques pour les plugins */
    SSTAG_SERVEUR_ADDPROGRESS_SYN_FOR_PLUGIN_DLS,
    SSTAG_SERVEUR_ADDPROGRESS_SYN_FOR_PLUGIN_DLS_FIN,

    SSTAG_CLIENT_WANT_TYPE_NUM_MNEMO,
    SSTAG_SERVEUR_TYPE_NUM_MNEMO,
  };

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/

