/**********************************************************************************************************/
/* Client/Include/motifs.h:   Chargement/Affichage d'un synoptique                                        */
/* Projet WatchDog version 1.7     Gestion d'habitat                         sam 27 sep 2003 15:08:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * motifs.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
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

 #ifndef _MOTIFS_H_
 #define _MOTIFS_H_

 #include "Cst_synoptiques.h"
 #include "Cst_atelier.h"

 enum                                                            /* Différents types de gestion de motifs */
  { TYPE_INERTE,
    TYPE_STATIQUE,
    TYPE_DYNAMIQUE,
    TYPE_CYCLIQUE,
    TYPE_PROGRESSIF,
    TYPE_INDICATEUR,
    TYPE_ANALOGIQUE,
    TYPE_BOUTON,
    TYPE_FOND
  };

 #define TAILLE_FONT                60                  /* Nombre de caractere pour la police d'affichage */
 #define COULEUR_FOND_SYN           "MidnightBlue"
 #define COULEUR_ACTIVE             200

 enum /* Différente action associée à un item */
  { ACTION_SANS,
    ACTION_IMMEDIATE,
    ACTION_PROGRAMME,
    ACTION_NBR_ACTION                                                         /* nombre d'action possible */
  };

/**********************************************************************************************************/
#endif
/*--------------------------------------------------------------------------------------------------------*/
