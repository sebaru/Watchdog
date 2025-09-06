/**********************************************************************************************************/
/* Include/Erreur.h      Déclaration des constantes et prototypes de gestion de log                       */
/* Projet Abls-Habitat version 4.5       Gestion d'habitat                  lun 21 avr 2003 22:06:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Erreur.h
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

#ifndef _ERREUR_H_
 #define _ERREUR_H_

 #include <glib.h>
 #include <sys/syslog.h>                                                   /* Pour les niveaux de LOG_xxx */

 #define  fonction_entre(); { printf("Entre dans la fonction %s\n", __FUNCTION__ ); }
 #define  fonction_sort();  { printf("Sort de la fonction %s\n", __FUNCTION__ ); }

 extern void Info_init( guint debug );
 extern void Info_change_log_level( guint debug );
 extern void Info_new( gchar *function, gboolean override, guint level, gchar *format, ... );
 extern gint Info_reset_nbr_log ( void );
#endif
/*--------------------------------------------------------------------------------------------------------*/
