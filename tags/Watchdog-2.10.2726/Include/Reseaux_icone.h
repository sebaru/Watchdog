/**********************************************************************************************************/
/* Include/Reseaux_icone.h:   Sous_tag de icone utilisé pour watchdog 2.0   par lefevre Sebastien         */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_icone.h
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

#ifndef _RESEAUX_ICONE_H_
 #define _RESEAUX_ICONE_H_

 #define NBR_CARAC_LIBELLE_ICONE       50
 #define NBR_CARAC_LIBELLE_ICONE_UTF8  (2*NBR_CARAC_LIBELLE_ICONE)

 #define NBR_CARAC_CLASSE_ICONE        40
 #define NBR_CARAC_CLASSE_ICONE_UTF8   (2*NBR_CARAC_CLASSE_ICONE)

 struct CMD_TYPE_CLASSE
  { guint id;                                                      /* Numero du message dans la structure */
    gchar libelle[ NBR_CARAC_CLASSE_ICONE_UTF8+1 ];
  };

 struct CMD_TYPE_ICONE
  { guint id;                                                      /* Numero du message dans la structure */
    gchar libelle[ NBR_CARAC_LIBELLE_ICONE_UTF8+1 ];
    guint id_classe;
    gchar nom_fichier[ 256 ];
  };

 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_ICONE,                           /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_ICONE_FIN,                       /* Ajout d'un groupe dans la liste cliente */
    SSTAG_CLIENT_WANT_PAGE_ICONE,                                         /* Le client veut la page Icone */
    SSTAG_SERVEUR_CREATE_PAGE_ICONE_OK,                        /* Le serveur repond OK pour creer la page */
    SSTAG_CLIENT_ADD_ICONE,                                /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_ICONE_WANT_FILE,       /* Le serveur a ajouté l'icone, il lui manque le fichier gif */
    SSTAG_CLIENT_ADD_ICONE_DEB_FILE,                       /* Le client commance à envoyer le fichier gif */
    SSTAG_CLIENT_ADD_ICONE_FILE,                               /* Le client continue d'envoyer le fichier */
    SSTAG_CLIENT_ADD_ICONE_FIN_FILE,                        /* Le client a terminé l'envoi du fichier gif */
    SSTAG_SERVEUR_ADD_ICONE_OK,                                       /* L'ajout de l'icone est un succes */

    SSTAG_CLIENT_DEL_ICONE,                                         /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_ICONE_OK,                                        /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_ICONE,                                   /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_ICONE_OK,               /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_ICONE,                              /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_ICONE_OK,                        /* Le serveur valide les nouvelles données */

    SSTAG_SERVEUR_ADDPROGRESS_CLASSE,                                          /* Envoi des classes icone */
    SSTAG_SERVEUR_ADDPROGRESS_CLASSE_FIN,                                      /* Envoi des classes icone */
    SSTAG_CLIENT_WANT_PAGE_CLASSE,
    SSTAG_CLIENT_ADD_CLASSE,                               /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_CLASSE_OK,                                       /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_CLASSE,                                        /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_CLASSE_OK,                                       /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_CLASSE,                                  /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_CLASSE_OK,              /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_CLASSE,                             /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_CLASSE_OK                        /* Le serveur valide les nouvelles données */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

