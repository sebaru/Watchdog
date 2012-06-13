/**********************************************************************************************************/
/* Include/Reseaux_rfxcom.h:   Sous_tag de gestion des rfxcom pour watchdog 2.0 par lefevre Sebastien     */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 15 août 2010 14:08:21 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_rfxcom.h
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

#ifndef _RESEAUX_RFXCOM_H_
 #define _RESEAUX_RFXCOM_H_

/******************************************** Gestion des rfxcom ******************************************/
 struct CMD_TYPE_RFXCOM
  { guint id;                                                                   /* ID unique de la rfxcom */
    guchar type;                                                                   /* Numéro de la rfxcom */
    guchar canal;                                                                  /* Numéro de la rfxcom */
    gint e_min, ea_min, a_min;
    gchar libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8];                              /* Libelle de la rfxcom */
  };

/************************************************* Tag de communication ***********************************/
 enum 
  { SSTAG_CLIENT_WANT_PAGE_RFXCOM,
    SSTAG_SERVEUR_CREATE_PAGE_RFXCOM_OK,                                     /* Affichage de la page rfxcom */
    SSTAG_SERVEUR_ADDPROGRESS_RFXCOM,                           /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_RFXCOM_FIN,                       /* Ajout d'un groupe dans la liste cliente */

    SSTAG_CLIENT_ADD_RFXCOM,                                /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_RFXCOM_OK,                                        /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_RFXCOM,                                         /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_RFXCOM_OK,                                        /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_RFXCOM,                                   /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_RFXCOM_OK,               /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_RFXCOM,                              /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_RFXCOM_OK,                        /* Le serveur valide les nouvelles données */

    SSTAG_CLIENT_TYPE_NUM_MNEMO_RFXCOM,                     /* Le client souhaite le mnemonique bit_motion */
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_RFXCOM,                           /* Le serveur répond avec le mnemonique */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

