/**********************************************************************************************************/
/* Include/Reseaux_option_tempo.h:   Sous_tag de gestion des options tempos watchdog par Lefevre Sebastien*/
/* Projet WatchDog version 2.0       Gestion d'habitat                     sam. 09 mars 2013 11:20:49 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_option_tempo.h
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

#ifndef _RESEAUX_OPTION_TEMPO_H_
 #define _RESEAUX_OPTION_TEMPO_H_

 struct CMD_TYPE_OPTION_TEMPO
  {                                                                                     /* Vient du mnemo */
    guint  id_mnemo;                                                        /* Id unique du mnemo associé */
    guint  num;                                                                     /* Numero de la tempo */
    gchar  libelle[ NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+1 ];
    gchar  groupe[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar  page[NBR_CARAC_PAGE_SYNOPTIQUE_UTF8+1];
    gchar  plugin_dls[NBR_CARAC_PLUGIN_DLS_UTF8+1];
                                                            /* Variable composant les options de la tempo */
    guint delai_on;                                 /* delai avant mise à un (fixé par option mnémonique) */
    guint delai_off;                              /* delai avant mise à zero (fixé par option mnémonique) */
    guint min_on;        /* Durée minimale pendant laquelle la tempo sera ON (fixé par option mnémonique) */
    guint max_on;        /* Durée maximale pendant laquelle la tempo sera ON (fixé par option mnémonique) */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

