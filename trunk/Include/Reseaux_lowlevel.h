/******************************************************************************************************************************/
/* Include/Reseaux_camera.h:   Sous_tag de gestion des camera pour watchdog 2.0 par lefevre Sebastien                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                                       dim. 13 sept. 2009 10:44:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Reseaux_camera.h
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

#ifndef _RESEAUX_CAMERA_H_
 #define _RESEAUX_CAMERA_H_

 #define NBR_CARAC_LOCATION_CAMERA      100
 #define NBR_CARAC_LOCATION_CAMERA_UTF8 (2*NBR_CARAC_LOCATION_CAMERA)

/*************************************************** Gestion des camera *******************************************************/
 struct CMD_TYPE_CAMERA
  { guint   id;                                                                 /* ID unique de la camera */
    guint   num;                                                                   /* Numéro de la camera */
    gchar   libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8];                           /* Libelle de la camera */
    gchar   location[NBR_CARAC_LOCATION_CAMERA_UTF8];                            /* Location de la camera */
  };

/******************************************************** Tag de communication ************************************************/
 enum 
  { SSTAG_CLIENT_WANT_PAGE_CAMERA,
    SSTAG_SERVEUR_CREATE_PAGE_CAMERA_OK,                                                       /* Affichage de la page camera */
    SSTAG_SERVEUR_ADDPROGRESS_CAMERA,                                              /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FIN,                                          /* Ajout d'un groupe dans la liste cliente */

    SSTAG_CLIENT_ADD_CAMERA,                                                   /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_CAMERA_OK,                                                           /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_CAMERA,                                                            /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_CAMERA_OK,                                                           /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_CAMERA,                                                      /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_CAMERA_OK,                                  /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_CAMERA,                                                 /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_CAMERA_OK,                                           /* Le serveur valide les nouvelles données */
  };

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/

