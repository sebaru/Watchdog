/******************************************************************************************************************************/
/* Watchdogd/uiud.c        Fonctions communes autour des UUID                                                                 */
/* Projet Abls-Habitat version 4.4       Gestion d'habitat                                                12.11.2021 22:14:10 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * uuid.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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

 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <fcntl.h>

 #include "watchdogd.h"
/******************************************************************************************************************************/
/* UUID_New: Génère un nouveau UUID dans le buffer en paramètre                                                               */
/* Entrée: le buffer a remplir                                                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void UUID_New ( gchar *target )
  { uuid_t uuid_hex;
    uuid_generate(uuid_hex);
    uuid_unparse_lower(uuid_hex, target );
  }
/******************************************************************************************************************************/
/* UUID_Load: Génère un nouveau UUID pour le thread en parametre et le stocke sur disque                                      */
/* Entrée: le buffer a remplir                                                                                                */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void UUID_Load ( gchar *thread, gchar *target )
  { gchar chaine[256];
    g_snprintf( chaine, sizeof(chaine), "%s.uuid", thread );                               /* Récupère l'UUID sur le FS local */
    gint fd = open ( chaine, O_RDONLY );
    if (fd>0) { read ( fd, target, 36 ); }
    else { UUID_New ( target );
           fd = creat ( chaine, S_IRUSR | S_IWUSR );
           write ( fd, target, 36 );
         }
    close(fd);
 }
/*----------------------------------------------------------------------------------------------------------------------------*/
