/**********************************************************************************************************/
/* Include/Reseaux_synoptique.h:   Sous_tag de la gestion synoptiques watchdog 2.0 par lefevre Sebastien  */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 10:44:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_synoptique.h
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

#ifndef _RESEAUX_SYNOPTIQUE_H_
 #define _RESEAUX_SYNOPTIQUE_H_

 #define NBR_CARAC_LIBELLE_SYNOPTIQUE       50
 #define NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8  (2*NBR_CARAC_LIBELLE_SYNOPTIQUE)

 #define NBR_CARAC_PAGE_SYNOPTIQUE          10
 #define NBR_CARAC_PAGE_SYNOPTIQUE_UTF8     (2*NBR_CARAC_PAGE_SYNOPTIQUE)

 struct CMD_TYPE_SYNOPTIQUE
  { guint id;                                                      /* Numero du message dans la structure */
    guint parent_id;
    gchar parent_page[NBR_CARAC_PAGE_SYNOPTIQUE_UTF8+1];
    gchar page[NBR_CARAC_PAGE_SYNOPTIQUE_UTF8+1];
    gchar libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    guint access_level;
  };

/************************************************* Tag de communication ***********************************/
 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE,                      /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FIN,                  /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_CREATE_PAGE_SYNOPTIQUE_OK,                          /* OK pour creer la page synoptique */
    SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE,
    SSTAG_CLIENT_ADD_SYNOPTIQUE,                           /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_SYNOPTIQUE_OK,                                   /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_SYNOPTIQUE,                                    /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_SYNOPTIQUE_OK,                                   /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_SYNOPTIQUE,                              /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_SYNOPTIQUE_OK,          /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_SYNOPTIQUE,                         /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_SYNOPTIQUE_OK,                   /* Le serveur valide les nouvelles données */      

  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

