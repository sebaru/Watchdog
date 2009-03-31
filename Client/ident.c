/**********************************************************************************************************/
/* Client/ident.c        Gestion du logon user sur module Client Watchdog                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 07 jun 2003 15:46:05 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ident.c
 * This file is part of <program name>
 *
 * Copyright (C) 2009 - sebastien
 *
 * <program name> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <program name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <program name>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 
 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>
 #include <string.h>
 
 #include "Reseaux.h"
 #include "client.h"
 #include "Config_cli.h"
 #include "Cst_utilisateur.h"
 
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

    memcpy( &ident.nom, &Client_en_cours.nom, sizeof(ident.nom) );
    memcpy( &ident.password, &Client_en_cours.password, sizeof(ident.password) );
    memcpy( &ident.version, VERSION, sizeof(ident.version) );
    
    ident.version_d = Lire_version_donnees( Config_cli.log );

    if ( !Envoi_serveur( TAG_CONNEXION, SSTAG_CLIENT_IDENT, (gchar *)&ident, sizeof(struct REZO_CLI_IDENT) ) )
     { return; }

    Log ( _("Waiting for authorization") );
    Client_en_cours.mode = ATTENTE_AUTORISATION;
    Info_c( Config_cli.log, DEBUG_CONNEXION, "client en mode ATTENTE_AUTORISATION version", ident.version );
  }
/*--------------------------------------------------------------------------------------------------------*/
