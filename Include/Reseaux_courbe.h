/**********************************************************************************************************/
/* Include/Reseaux_courbe.h:   Sous_tag de courbe utilisé pour watchdog 2.0   par lefevre Sebastien       */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_courbe.h
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

#ifndef _RESEAUX_COURBE_H_
 #define _RESEAUX_COURBE_H_

 #include "Cst_entreeana.h"

 struct CMD_APPEND_COURBE
  { guint  slot_id;                                                                          /* Numero de l'ea */
    guint  type;
    guint  date;
    guint  val;
  };

 struct CMD_TYPE_COURBE
  { guint  slot_id;                                           /* Pour affichage sur le bon slot du client */
    guint  id;                                                         /* Numéro de la courbe en question */
    guint  type;
    guchar libelle [ NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+1 ];
  };

 enum 
  { SSTAG_CLIENT_WANT_PAGE_SOURCE_FOR_COURBE,                        /* Envoi des infos eana pour courbes */
    SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_COURBE,            /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_COURBE_FIN,
    SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_COURBE,                /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_COURBE_FIN,
    SSTAG_CLIENT_ADD_COURBE,                               /* Le client a fait son choix -> ajout demandé */
    SSTAG_SERVEUR_ADD_COURBE_OK,                          /* Le serveur repond OK pour la requete cliente */
    SSTAG_SERVEUR_APPEND_COURBE,                   /* Le serveur envoie periodiquement la nouvelle valeur */

    SSTAG_CLIENT_DEL_COURBE,                                        /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_COURBE_OK,                                       /* L'ajout du groupe est un succes */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

