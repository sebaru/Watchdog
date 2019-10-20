/******************************************************************************************************************************/
/* Include/Reseaux_utilisateur.h:   Sous_tag de utilisateur pour watchdog 2.0 par lefevre Sebastien                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Reseaux_utilisateur.h
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

#ifndef _RESEAUX_UTILISATEUR_H_
 #define _RESEAUX_UTILISATEUR_H_

 #define NBR_CARAC_LOGIN             16                           /* Idem pour la longueur max du password */
 #define NBR_CARAC_LOGIN_UTF8        (2*NBR_CARAC_LOGIN)
 #define NBR_CARAC_COMMENTAIRE       40
 #define NBR_CARAC_COMMENTAIRE_UTF8  (2*NBR_CARAC_COMMENTAIRE)

 struct CMD_TYPE_UTILISATEUR
  { guint    id;
    gchar    username[ NBR_CARAC_LOGIN_UTF8+1 ];
    gchar    email[80];
    gchar    hash[255];
    gchar    commentaire[ NBR_CARAC_COMMENTAIRE_UTF8+1 ];
    guint    access_level;
    gboolean enable;
    gchar    date_create[32];
    gchar    date_modif[32];
    gboolean sms_enable;
    gchar    sms_phone[80];
    gboolean sms_allow_cde;
    gboolean imsg_enable;
    gchar    imsg_jabberid[80];
    gboolean imsg_allow_cde;
    gboolean imsg_available;
    guint    ssrv_bit_presence;
  };

 enum 
  { SSTAG_CLIENT_WANT_PAGE_UTIL,
    SSTAG_SERVEUR_CREATE_PAGE_UTIL_OK,                                        /* Affichage de la page DLS */
    SSTAG_SERVEUR_ADDPROGRESS_UTIL,                       /* Ajout d'un utilisateur dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_UTIL_FIN,
    SSTAG_CLIENT_ADD_UTIL,                            /* Le client desire ajouter un utilisateur watchdog */
    SSTAG_SERVEUR_ADD_UTIL_OK,                                    /* L'ajout du utilisateur est un succes */

    SSTAG_CLIENT_DEL_UTIL,                                     /* Le client desire retirer un utilisateur */
    SSTAG_SERVEUR_DEL_UTIL_OK,                                    /* L'ajout du utilisateur est un succes */

    SSTAG_CLIENT_EDIT_UTIL,                               /* Le client demande l'edition d'un utilisateur */
    SSTAG_SERVEUR_EDIT_UTIL_OK,                /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_UTIL,                               /* Le client renvoie les données editées */
    SSTAG_CLIENT_CHANGE_PASSWORD,                  /* Le client demande a modifier le mot de passe actuel */
    SSTAG_SERVEUR_VALIDE_EDIT_UTIL_OK,                         /* Le serveur valide les nouvelles données */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

