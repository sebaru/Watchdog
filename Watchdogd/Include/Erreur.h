/**********************************************************************************************************/
/* Include/Erreur.h      Déclaration des constantes et prototypes de gestion de log                       */
/* Projet WatchDog version 1.7       Gestion d'habitat                      lun 21 avr 2003 22:06:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Erreur.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien LEFEVRE
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

 #define TAILLE_ENTETE_LOG   40
 struct LOG { char entete [TAILLE_ENTETE_LOG+1];
              guint log_level;
            };

 #define  fonction_entre(); { printf("Entre dans la fonction %s\n", __FUNCTION__ ); }
 #define  fonction_sort();  { printf("Sort de la fonction %s\n", __FUNCTION__ ); }

 extern struct LOG *Info_init( gchar *entete, guint debug );
 extern void Info_change_log_level( struct LOG *log, guint debug );
 extern void Info_new( gchar *function, gboolean override, guint level, gchar *format, ... );

#endif
/*--------------------------------------------------------------------------------------------------------*/
