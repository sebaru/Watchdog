/******************************************************************************************************************************/
/* Include/Reseaux_message.h:   Sous_tag de message pour watchdog 2.0 par lefevre Sebastien                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Reseaux_message.h
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

#ifndef _RESEAUX_MESSAGE_H_
 #define _RESEAUX_MESSAGE_H_


 
 struct CMD_TYPE_MESSAGE_MP3                                                       /* Structure pour l'échange du fichier mp3 */
  { guint num;
    guint taille;                                                     /* Taille des données qui suivent dans le paquet reseau */
  };

 enum 
  { SSTAG_CLIENT_WANT_PAGE_MESSAGE,
    SSTAG_SERVEUR_CREATE_PAGE_MESSAGE_OK,                                                    /* Affichage de la page onduleur */
    SSTAG_SERVEUR_ADDPROGRESS_MESSAGE,                                             /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_MESSAGE_FIN,                                         /* Ajout d'un groupe dans la liste cliente */

    SSTAG_CLIENT_ADD_MESSAGE,                                                  /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_MESSAGE_OK,                                                          /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_MESSAGE,                                                           /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_MESSAGE_OK,                                                          /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_MESSAGE,                                                     /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_MESSAGE_OK,                                 /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_MESSAGE,                                                /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_MESSAGE_OK,                                          /* Le serveur valide les nouvelles données */

    SSTAG_CLIENT_WANT_DLS_FOR_MESSAGE,                                             /* Envoi des synoptiques pour les messages */
    SSTAG_SERVEUR_ADDPROGRESS_DLS_FOR_MESSAGE,
    SSTAG_SERVEUR_ADDPROGRESS_DLS_FOR_MESSAGE_FIN,

    SSTAG_CLIENT_TYPE_NUM_MNEMO_VOC,
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_VOC,
  };

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
