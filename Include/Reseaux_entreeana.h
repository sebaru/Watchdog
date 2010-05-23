/**********************************************************************************************************/
/* Include/Reseaux_entreeana.h:   Sous_tag de entreeana pour watchdog 2.0 par lefevre Sebastien           */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_entreeana.h
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

#ifndef _RESEAUX_ENTREEANA_H_
 #define _RESEAUX_ENTREEANA_H_

 #include "Cst_entreeana.h"

 struct CMD_TYPE_ENTREEANA
  { guint  id_mnemo;                                                        /* Id unique du mnemo associé */
    guint  unite;                                                                         /* Km, h, ° ... */
    gfloat min;
    gfloat max;
    guint  type;                                                               /* Type de gestion de l'EA */
                                                                                        /* Vient du mnemo */
    guint  num;                                                                         /* Numero de l'EA */
    gchar libelle[ NBR_CARAC_LIBELLE_ENTREEANA_UTF8+1 ];
  };

 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA,                       /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FIN,
    SSTAG_SERVEUR_CREATE_PAGE_ENTREEANA_OK,                     /* OK pour affichage de la page entreeANA */
    SSTAG_CLIENT_WANT_PAGE_ENTREEANA,

    SSTAG_CLIENT_EDIT_ENTREEANA,                               /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_ENTREEANA_OK,           /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_ENTREEANA,                          /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_ENTREEANA_OK,                    /* Le serveur valide les nouvelles données */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

