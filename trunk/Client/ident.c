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
     { Deconnecter();
       return;
     }

    Log ( _("Waiting for authorization") );
    Client_en_cours.mode = ATTENTE_AUTORISATION;
    Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
              "Client en mode ATTENTE_AUTORISATION version %d", ident.version );
  }
/**********************************************************************************************************/
/* Envoyer_identification: envoi de l'identification cliente au serveur                                   */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Envoyer_authentification ( struct CMD_TYPE_UTILISATEUR *util )
  { gchar salt[EVP_MAX_MD_SIZE+1], hash[EVP_MAX_MD_SIZE+1];
    gint cpt;
    EVP_MD_CTX *mdctx;
    guint md_len;
    memset( util->hash, 0, sizeof(util->hash) );                                         /* Mise en forme */

    memset( salt, 0, sizeof(salt) );                                                     /* Mise en forme */
    for (cpt=0; cpt<sizeof(util->salt); cpt++)
     { g_snprintf( &salt[2*cpt], 3, "%02X", (guchar) util->salt[cpt] ); }

    memset( hash, 0, sizeof(hash) );                                                     /* Mise en forme */
    for (cpt=0; cpt<sizeof(util->hash); cpt++)
     { g_snprintf( &hash[2*cpt], 3, "%02X", (guchar) util->hash[cpt] ); }
    printf("1- salt:%s\n hash:%s\n", salt, hash );

    mdctx = EVP_MD_CTX_create();
    EVP_DigestInit_ex (mdctx, EVP_sha512(), NULL);
    EVP_DigestUpdate  (mdctx, util->salt, sizeof(util->salt)-1);
    EVP_DigestUpdate  (mdctx, Client_en_cours.password,  strlen(Client_en_cours.password));
    EVP_DigestFinal_ex(mdctx, (guchar *)util->hash, &md_len);
    EVP_MD_CTX_destroy(mdctx);
    printf("2- Hash calculated: longueur %d\n", md_len );

    memset( salt, 0, sizeof(salt) );                                                     /* Mise en forme */
    for (cpt=0; cpt<sizeof(util->salt); cpt++)
     { g_snprintf( &salt[2*cpt], 3, "%02X", (guchar) util->salt[cpt] ); }

    memset( hash, 0, sizeof(hash) );                                                     /* Mise en forme */
    for (cpt=0; cpt<sizeof(util->hash); cpt++)
     { g_snprintf( &hash[2*cpt], 3, "%02X", (guchar) util->hash[cpt] ); }
    printf("3- salt:%s\n hash:%s\n", salt, hash );
    if ( !Envoi_serveur( TAG_CONNEXION, SSTAG_CLIENT_SEND_HASH, (gchar *)util, sizeof(struct CMD_TYPE_UTILISATEUR) ) )
     { Deconnecter();
       return;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
