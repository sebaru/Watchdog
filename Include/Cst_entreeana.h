/**********************************************************************************************************/
/* Include/Cst_entreeana.h        Déclaration des constantes dediées aux entrees analogiques              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 25 jun 2004 13:42:31 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Cst_entreeana.h
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

#ifndef _CST_ENTREEANA_H_
 #define _CST_ENTREEANA_H_

 #define TAILLEBUF_HISTO_EANA         1600       /* 800 point de controle dans le client et buffer valana */
 #define NBR_VAL_INIT_COURBE          100                   /* Envoyé via 100 points à chaque paquet rezo */

 #define COURBE_TEMPS_TOP             5  /* 1 point = 5 secondes sur la grille courbes */

 enum
  { ENTREEANA_NON_INTERP,
    ENTREEANA_4_20_MA_12BITS,
    ENTREEANA_4_20_MA_10BITS,
    NBR_TYPE_ENTREEANA
  };

 enum
  { UNITE_DEGRE_C,                                                   /* Definitions des types de messages */
    UNITE_DEGRE_K,
    UNITE_MILLIBAR,
    UNITE_BAR,
    UNITE_METRE,
    UNITE_KILOMETRE,
    UNITE_LITRE,
    UNITE_METRE_PAR_SECONDE,
    UNITE_KILOMETRE_PAR_HEURE,
    UNITE_POURCENT,
    UNITE_POURCENT_HR,
    UNITE_SECONDE,
    UNITE_HEURE,
    UNITE_DATE,
    UNITE_VOLTS,
    NBR_TYPE_UNITE
  };

/*---------------------------- Déclarations des prototypes de fonctions ----------------------------------*/

 extern gchar *Unite_vers_string ( guint type );
 extern gchar *Type_ea_vers_string ( guint type );

#endif
/*--------------------------------------------------------------------------------------------------------*/
