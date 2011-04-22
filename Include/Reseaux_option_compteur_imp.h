/**********************************************************************************************************/
/* Include/Reseaux_option compteur_imp.h:   Gestion des options des compteurs d'impulsions watchdog 2.0   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_option_compteur_imp.h
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

#ifndef _RESEAUX_CPT_IMP_H_
 #define _RESEAUX_CPT_IMP_H_

 #define NBR_CARAC_UNITE_CPT_IMP       8
 #define NBR_CARAC_UNITE_CPT_IMP_UTF8  (2*NBR_CARAC_UNITE_CPT_IMP)

 enum
  { CI_TOTALISATEUR,
    CI_MOYENNEUR_SEC,
    CI_MOYENNEUR_MIN,
    NBR_TYPE_CI
  };

 struct CMD_TYPE_OPTION_COMPTEUR_IMP
  { guint  id_mnemo;                                                        /* Id unique du mnemo associé */
    gfloat valeur;                                                                  /* Valeur du compteur */
    guint  type;                                                                /* Totalisateur/Moyenneur */
    gfloat multi;                                                                       /* Multiplicateur */
    gchar  unite[NBR_CARAC_UNITE_CPT_IMP_UTF8+1];                                         /* Km, h, ° ... */

                                                                                        /* Vient du mnemo */
    guint  num;                                                                     /* Numero du compteur */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

