/**********************************************************************************************************/
/* Client/ident.c        Gestion du logon user sur module Client Watchdog                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 07 jun 2003 15:46:05 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ident.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
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
 #include <bonobo/bonobo-i18n.h>
 #include <string.h>
 
 #include "config.h"
 #include "Reseaux.h"
 #include "client.h"
 #include "Config_cli.h"
 
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern struct CLIENT Client_en_cours;                           /* Identifiant de l'utilisateur en cours */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

/**********************************************************************************************************/
/* Envoyer_identification: envoi de l'identification cliente au serveur                                   */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Envoyer_identification ( void )
  { struct REZO_CLI_IDENT ident;

    memcpy( &ident.nom, &Client_en_cours.util.nom, sizeof(ident.nom) );
    snprintf( ident.version, sizeof(ident.version), "%s", VERSION );
    ident.version_d = Lire_version_donnees( Config_cli.log );

    if ( !Envoi_serveur( TAG_CONNEXION, SSTAG_CLIENT_IDENT, (gchar *)&ident, sizeof(struct REZO_CLI_IDENT) ) )
     { return; }

    Log ( _("Waiting for authorization") );
    Client_en_cours.mode = ATTENTE_AUTORISATION;
    Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
              "Client en mode ATTENTE_AUTORISATION version %d", ident.version );
  }
/*--------------------------------------------------------------------------------------------------------*/
