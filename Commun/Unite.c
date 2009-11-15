/**********************************************************************************************************/
/* Commun/unite.c        Gestion des unites entreeANAs de Watchdog v2.0                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 15 nov. 2009 13:43:06 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Unite.c
 * This file is part of <program name>
 *
 * Copyright (C) 2009 - 
 *
 * <program name> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <program name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <program name>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 
 #include <glib.h>
 #include <string.h>
 #include "Cst_entreeana.h"

 static gchar *UNITE_EA[NBR_TYPE_UNITE] =
  { "°C",
    "°K",
    "mbar",
    "bar",
    "m",
    "km",
    "l",
    "m/s",
    "km/h",
    "%",
    "% HR",
    "s",
    "h",
    "A",
    "V"
  };

 static gchar *TYPE_EA[NBR_TYPE_ENTREEANA] =
  { "Non Interprete",
    "4/20 mA 12 bits"
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
/**********************************************************************************************************/
/* String_vers_unite: renvoie l'unite en tant que int                                                     */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 gint String_vers_unite ( guchar *unite )
  { gint i;
    for (i=0; i<NBR_TYPE_UNITE; i++)
     { if ( !strcmp( UNITE_EA[i], unite ) ) return(i); }
    return(0);
  }
/**********************************************************************************************************/
/* Unite_vers_string: renvoie l'unite sous forme de chaine de caractere                                   */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 gchar *Type_ea_vers_string ( guint type )
  { if (type<NBR_TYPE_ENTREEANA) return( TYPE_EA[type] );
                            else return ( "Unknown" );
  }
/*--------------------------------------------------------------------------------------------------------*/
