/**********************************************************************************************************/
/* Commun/Unite.c        Gestion des unites entreeANAs de Watchdog v2.0                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 15 nov. 2009 13:43:06 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Unite.c
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

 #include <glib.h>
 #include <string.h>
 #include "Unite.h"
#ifdef bouh
 #include "Reseaux_mnemonique.h"
#endif

 static gchar *UNITE_EA[NBR_TYPE_UNITE] =
  { "°C",
    "°K",
    "bar",
    "mbar",
    "m",
    "m/s",
    "km",
    "km/h",
    "l",
    "l/s",
    "l/min",
    "dl",
    "dl/s",
    "dl/min",
    "m3",
    "m3/s",
    "m3/min",
    "%",
    "% HR",
    "s",
    "h",
    "A",
    "V"
  };

/**********************************************************************************************************/
/* Unite_vers_string: renvoie l'unite sous forme de chaine de caractere                                   */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 gchar *Unite_vers_string ( guint type )
  { if (type<NBR_TYPE_UNITE) return( UNITE_EA[type] );
                        else return ( "Unknown" );
  }
/*--------------------------------------------------------------------------------------------------------*/
