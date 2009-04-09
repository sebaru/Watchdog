/**********************************************************************************************************/
/* Commun/Erreur.c        Gestion des logs systemes                                                       */
/* Projet WatchDog version 1.7       Gestion d'habitat                      jeu 09 avr 2009 22:08:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Erreur.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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

 #include "Erreur.h"

/**********************************************************************************************************/
/* Info_init: Initialisation du traitement d'erreur                                                       */
/* Entrée: Le niveau de debuggage, l'entete, et le fichier log                                            */
/**********************************************************************************************************/
 static void Info_stop( int code_retour, void *log )
  {
    Info( log, DEBUG_INFO, "Fin des logs" );
    if (log) g_free(log);
  }

/**********************************************************************************************************/
/* Info_init: Initialisation du traitement d'erreur                                                       */
/* Entrée: Le niveau de debuggage, l'entete, et le fichier log                                            */
/**********************************************************************************************************/
 struct LOG *Info_init( gchar *entete, guint debug )
  { struct LOG *log;

    log = g_malloc0( sizeof(struct LOG) );
    if (!log) return(NULL);
    
    g_snprintf( log->entete,  sizeof(log->entete), "%s", entete  );
    log->debug_level = debug;
    on_exit( Info_stop, log );
    
    openlog( log->entete, LOG_CONS | LOG_PID | LOG_PERROR, LOG_LOCAL3 );
    return(log);
  }

/**********************************************************************************************************/
/* Info: Log dans un fichier ou sur le terminal l'évolution du systeme                                    */
/* Entrée: Le niveau de debuggage, et le texte a afficher                                                 */
/**********************************************************************************************************/
 void Info( struct LOG *log, guint niveau, gchar *texte )
  { gchar nom[20];
    if (!log) return;

    prctl( PR_GET_NAME, &nom, 0, 0, 0);
    if ( log->debug_level & niveau )
     { syslog( LOG_INFO, "%s %d -> %s\n", nom, niveau, texte );
     }
  }

/**********************************************************************************************************/
/* Info_n: on informe avec une donnée numérique                                                           */
/* Entrée: le niveau, le texte, et la valeur à afficher                                                   */
/**********************************************************************************************************/
 void Info_n( struct LOG *log, guint niveau, gchar *texte, gint valeur )
  { gchar chaine[512];
    g_snprintf( chaine, sizeof(chaine), "%s: %d", texte, valeur );
    Info( log, niveau, chaine );
  }

/**********************************************************************************************************/
/* Info_c: on informe avec une donnée de type chaine                                                      */
/* Entrée: le niveau, le texte, et la chaine à afficher                                                   */
/**********************************************************************************************************/
 void Info_c( struct LOG *log, guint niveau, gchar *texte, gchar *texte2 )
  { gchar chaine[512];
    g_snprintf( chaine, sizeof(chaine), "%s: %s", texte, texte2 );
    Info( log, niveau, chaine );
  }
/*--------------------------------------------------------------------------------------------------------*/
