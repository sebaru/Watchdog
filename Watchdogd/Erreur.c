/******************************************************************************************************************************/
/* Commun/Erreur.c        Gestion des logs systemes                                                                           */
/* Projet WatchDog version 1.7       Gestion d'habitat                                          jeu 09 avr 2009 22:08:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Erreur.c
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

 #include <glib.h>
 #include <stdio.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <syslog.h>
 #include <string.h>
 #include <time.h>
 #include <sys/prctl.h>
 #include <stdarg.h>

 #include "watchdogd.h"

 static gchar *level_to_string[] =
  { "-EMERG-",
    "-ALERT-",
    "-CRIT--",
    "-ERROR-",
    "WARNING",
    "-NOTICE",
    "-INFO--",
    "-DEBUG-"
  };

/******************************************************************************************************************************/
/* Info_init: Initialisation du traitement d'erreur                                                                           */
/* Entrée: Le niveau de debuggage, l'entete, et le fichier log                                                                */
/******************************************************************************************************************************/
 static void Info_stop( int code_retour, void *arg )
  { Info_new( __func__, TRUE, LOG_NOTICE, "End of logs" ); }
/******************************************************************************************************************************/
/* Info_init: Initialisation du traitement d'erreur                                                                           */
/* Entrée: Le niveau de debuggage, l'entete, et le fichier log                                                                */
/******************************************************************************************************************************/
 void Info_init( guint debug )
  { Config.log_level = (debug ? debug : LOG_INFO);
    on_exit( Info_stop, NULL );
    openlog( NULL, LOG_CONS | LOG_PID, LOG_USER );
  }
/******************************************************************************************************************************/
/* Info_init: Initialisation du traitement d'erreur                                                                           */
/* Entrée: Le niveau de debuggage, l'entete, et le fichier log                                                                */
/******************************************************************************************************************************/
 void Info_change_log_level( guint new_log_level )
  { Config.log_level = (new_log_level ? new_log_level : LOG_INFO);
    Info_new( __func__, TRUE, LOG_NOTICE, "log_level set to %d (%s)", Config.log_level,  level_to_string[Config.log_level] );
  }
/******************************************************************************************************************************/
/* Info_new: on informe le sous systeme syslog en affichant un nombre aléatoire de paramètres                                 */
/* Entrée: le niveau, le texte, et la chaine à afficher                                                                       */
/******************************************************************************************************************************/
 void Info_new( gchar *function, gboolean override, guint level, gchar *format, ... )
  { gchar chaine[512], nom_thread[32];
    va_list ap;

    if ( override == TRUE || (level <= Config.log_level) )                                        /* LOG_EMERG = 0, DEBUG = 7 */
     { prctl( PR_GET_NAME, &nom_thread, 0, 0, 0 );
       g_snprintf( chaine, sizeof(chaine), "{ \"level\": \"%s\", \"thread\": \"%s\", \"function\": \"%s\", \"message\": \"%s\" }",
                   level_to_string[level], nom_thread, function, format );

       va_start( ap, format );
       vsyslog ( level, chaine, ap );
       va_end ( ap );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
