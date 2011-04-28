/**********************************************************************************************************/
/* Include/Reseaux_histo_courbe.h:   Sous_tag de histo_courbe utilisé pour watchdog 2.0                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 nov 2007 11:28:58 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_histo_courbe.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2007 - Sébastien Lefevre
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

#ifndef _RESEAUX_HISTO_COURBE_H_
 #define _RESEAUX_HISTO_COURBE_H_

 #include "Reseaux_courbe.h"

 struct CMD_HISTO_COURBE                         /* Structure pour travailler sur les historiques courbes */
  { guint date_first;
    guint date_last;
  };

 enum 
  { SSTAG_CLIENT_WANT_PAGE_SOURCE_FOR_HISTO_COURBE,                  /* Envoi des infos eana pour courbes */
    SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_HISTO_COURBE,      /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_HISTO_COURBE_FIN,
    SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_HISTO_COURBE,          /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_HISTO_COURBE_FIN,
    SSTAG_CLIENT_SET_DATE,                                     /* Le client a fait son choix dans la date */
    SSTAG_CLIENT_ADD_HISTO_COURBE,                         /* Le client a fait son choix -> ajout demandé */
    SSTAG_SERVEUR_ADD_HISTO_COURBE_OK,                    /* Le serveur repond OK pour la requete cliente */
    SSTAG_SERVEUR_START_HISTO_COURBE,              /* Le serveur envoie periodiquement la nouvelle valeur */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

