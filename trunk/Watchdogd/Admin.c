/******************************************************************************************************************************/
/* Watchdogd/Admin.c        Gestion des connexions Admin au serveur watchdog                                                  */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

 #include <sys/socket.h>
 #include <sys/un.h>                                                                   /* Description de la structure AF UNIX */
 #include <sys/types.h>
 #include <sys/prctl.h>
 #include <fcntl.h>
 #include <unistd.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Admin_write : Concatene deux chaines de caracteres proprementvvvvvvv                                                       */
/* Entrée : le buffer et la chaine                                                                                            */
/* Sortie: La nouvelle chaine                                                                                                 */
/******************************************************************************************************************************/
 gchar *Admin_write ( gchar *response, gchar *new_ligne )
  { gchar *new;
    if (response == NULL) { response = g_strdup(""); }
    new = g_strconcat( response, new_ligne, "\n", NULL );
    g_free(response);
    return(new);
  }
/******************************************************************************************************************************/
/* Ecouter_admin: Ecoute ce que dis le CLIENT                                                                                 */
/* Entrée: la connexion, le user host d'origine et commande a parser                                                          */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Processer_commande_admin ( gchar *user, gchar *host, gchar *ligne )
  { gchar commande[128], chaine[256];
    gchar *response=NULL;

    if (!(user && host)) return(NULL);

    Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
             "%s: Commande Received from %s@%s : %s", __func__, user, host, ligne );

    g_snprintf( chaine, sizeof(chaine), "At %010.1f, processing '%s' on instance '%s'",
                (gdouble)Partage->top/10.0, ligne, g_get_host_name() );
    response = Admin_write ( NULL, chaine );

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */

            if ( g_str_has_prefix ( commande, "set"       ) ) { response = Admin_set      ( response, ligne + 4);  }
       else if ( g_str_has_prefix ( commande, "get"       ) ) { response = Admin_get      ( response, ligne + 4);  }
    response = Admin_write ( response, " -\n" );
    return(response);                                                                                    /* Fin de la reponse */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
