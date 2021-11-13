/******************************************************************************************************************************/
/* Watchdogd/Utilisateur/groupe.c    Gestion de l'interface SQL pour les groupes                                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          ven 03 avr 2009 20:32:27 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * groupe.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 
 #include <unistd.h>
 #include <stdio.h>
 #include <string.h>
 #include <time.h>
 #include <stdlib.h>

/******************************************** Prototypes des fonctions ********************************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Tester_level_util: renvoie true si l'utilisateur fait partie du groupe en parametre                                       */
/* Entrées: une structure UTIL et un id de groupe                                                                             */
/* Sortie: false si pb                                                                                                        */
/******************************************************************************************************************************/
 gboolean Tester_level_util( struct CMD_TYPE_UTILISATEUR *util, guint level )
  { gint cpt;
    if (!util) return(FALSE);
    if ( util->id == UID_ROOT) return(TRUE);                                             /* Le tech est dans tous les groupes */
    if (!util->enable) return(FALSE);
    return( util->access_level >= level );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
