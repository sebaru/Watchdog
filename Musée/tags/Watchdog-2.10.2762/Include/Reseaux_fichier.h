/**********************************************************************************************************/
/* Include/Reseaux_fichier.h:   Sous_tag de l'fichier pour watchdog 2.0 par lefevre Sebastien             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_fichier.h
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

#ifndef _RESEAUX_FICHIER_H_
 #define _RESEAUX_FICHIER_H_

 struct CMD_FICHIER
  { gchar nom[80];
  };

 enum 
  { SSTAG_SERVEUR_VERSION,
    SSTAG_SERVEUR_DEL_FICHIER,
    SSTAG_SERVEUR_APPEND_FICHIER,
    
  };
#endif
/*--------------------------------------------------------------------------------------------------------*/

