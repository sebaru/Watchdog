/**********************************************************************************************************/
/* Include/Reseaux_rs485.h:   Sous_tag de gestion des rs485 pour watchdog 2.0 par lefevre Sebastien       */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 15 août 2010 14:08:21 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_rs485.h
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

#ifndef _RESEAUX_RS485_H_
 #define _RESEAUX_RS485_H_

/********************************************* Gestion des rs485 ******************************************/
 struct CMD_TYPE_RS485
  { guint id;                                                                    /* ID unique de la rs485 */
    guint num;                                                                      /* Numéro de la rs485 */
    guint bit_comm;                         /* Bit bistable correspondant au bon fonctionnement du module */
    gboolean actif;
    gint ea_min, ea_max;
    gint e_min, e_max;
    gint s_min, s_max;
    gint sa_min, sa_max;
    gchar libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8];                              /* Libelle de la rs485 */
  };

/************************************************* Tag de communication ***********************************/
 enum 
  { SSTAG_CLIENT_WANT_PAGE_RS485,
    SSTAG_SERVEUR_CREATE_PAGE_RS485_OK,                                     /* Affichage de la page rs485 */
    SSTAG_SERVEUR_ADDPROGRESS_RS485,                           /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_RS485_FIN,                       /* Ajout d'un groupe dans la liste cliente */

    SSTAG_CLIENT_ADD_RS485,                                /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_RS485_OK,                                        /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_RS485,                                         /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_RS485_OK,                                        /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_RS485,                                   /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_RS485_OK,               /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_RS485,                              /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_RS485_OK,                        /* Le serveur valide les nouvelles données */

    SSTAG_CLIENT_TYPE_NUM_MNEMO_RS485,                     /* Le client souhaite le mnemonique bit_motion */
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_RS485,                           /* Le serveur répond avec le mnemonique */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

