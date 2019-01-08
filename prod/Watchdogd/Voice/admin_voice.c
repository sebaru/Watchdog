/******************************************************************************************************************************/
/* Watchdogd/Voice/admin_voice.c        Gestion de l'administration du thread Voice                                           */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                     29.12.2018 23:00:22*/
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_voice.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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
 
 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include "watchdogd.h"
 #include "Voice.h"

/******************************************************************************************************************************/
/* Admin_voice_reload: Demande le rechargement des conf SMS                                                                     */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_voice_reload ( gchar *response )
  { Cfg_voice.lib->Thread_reload = TRUE;
    response = Admin_write ( response, " | - VOICE Reload done" );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_voice: Gere une commande 'admin voice' depuis une response admin                                                         */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 gchar *Admin_command ( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { 
     }
    else if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, " | -- Watchdog ADMIN -- Help du mode 'VOICE'" );
       response = Admin_write ( response, " | - reload                 - Recharge la configuration" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " | - Unknown VOICE command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
