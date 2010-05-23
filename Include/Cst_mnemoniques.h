/**********************************************************************************************************/
/* Include/Cst_mnemoniques.h        Déclaration des constantes dediées aux synoptiques                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 25 sep 2003 16:40:16 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Cst_mnemonique.h
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

#ifndef _CST_MNEMONIQUES_H_
 #define _CST_MNEMONIQUES_H_

 #define NBR_CARAC_LIBELLE_MNEMONIQUE       70
 #define NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8  (6*NBR_CARAC_LIBELLE_MNEMONIQUE)

 #define NBR_CARAC_OBJET_MNEMONIQUE       30
 #define NBR_CARAC_OBJET_MNEMONIQUE_UTF8  (6*NBR_CARAC_OBJET_MNEMONIQUE)
 
 #define NBR_CARAC_ACRONYME_MNEMONIQUE       14
 #define NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8  (6*NBR_CARAC_ACRONYME_MNEMONIQUE)

 enum
  { MNEMO_BISTABLE,                                               /* Definitions des types de mnemoniques */
    MNEMO_MONOSTABLE,
    MNEMO_TEMPO,
    MNEMO_ENTREE,
    MNEMO_SORTIE,
    MNEMO_ENTREE_ANA,
    MNEMO_SORTIE_ANA,
    MNEMO_MOTIF,
    MNEMO_CPTH,
    MNEMO_CAMERA,
    NBR_TYPE_MNEMO
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/
