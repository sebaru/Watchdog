/************************************************************************************************************/
/* Include/Version.h    Gestion de la version des données clientes/serveur                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                        sam 04 oct 2003 12:47:01 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                */
/************************************************************************************************************/
/*
 * Version.h
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

#ifndef _VERSION_H
 #define _VERSION_H
 #include <glib.h>

 #define FICHIER_VERSION "version.wdg"
 #define TAILLE_VERSION  20

 extern time_t Lire_version_donnees( struct LOG *log );
 extern time_t Changer_version_donnees( struct LOG *log, time_t new_version );
#endif
/*----------------------------------------------------------------------------------------------------------*/
