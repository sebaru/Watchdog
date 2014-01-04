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
 #include <openssl/rand.h>

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
 void Calcul_password_hash ( gboolean new_salt, gchar *password )
  { gchar hash[EVP_MAX_MD_SIZE];
    EVP_MD_CTX *mdctx;
    guint cpt,md_len;
    memset( Client_en_cours.util.hash, 0, sizeof(Client_en_cours.util.hash) );           /* Mise en forme */
    memset( hash, 0, sizeof(hash) );                                                     /* Mise en forme */

    if (new_salt)
     { gchar salt[EVP_MAX_MD_SIZE];
       memset ( Client_en_cours.util.salt, 0, sizeof(Client_en_cours.util.salt) );
       RAND_pseudo_bytes( (guchar *)salt, sizeof(salt) );               /* Récupération d'un nouveau SALT */
       for (cpt=0; cpt<sizeof(salt); cpt++)                             /* Mise en forme au format HEX */
        { g_snprintf( &Client_en_cours.util.salt[2*cpt], 3, "%02X", (guchar)salt[cpt] ); }
     }
    mdctx = EVP_MD_CTX_create();                                                        /* Calcul du HASH */
    EVP_DigestInit_ex (mdctx, EVP_sha512(), NULL);
    EVP_DigestUpdate  (mdctx, Client_en_cours.util.nom, strlen(Client_en_cours.util.nom) );
    EVP_DigestUpdate  (mdctx, Client_en_cours.util.salt, sizeof(Client_en_cours.util.salt)-1 );
    EVP_DigestUpdate  (mdctx, password,  strlen(password));
    EVP_DigestFinal_ex(mdctx, (guchar *)hash, &md_len);
    EVP_MD_CTX_destroy(mdctx);
    printf("2- Hash calculated: longueur %d for %s\n", md_len, Client_en_cours.util.nom );

    for (cpt=0; cpt<sizeof(hash); cpt++)                                   /* Mise en forme au format HEX */
     { g_snprintf( &Client_en_cours.util.hash[2*cpt], 3, "%02X", (guchar)hash[cpt] ); }
    printf("3- salt:%s\n4- hash:%s\n", Client_en_cours.util.salt, Client_en_cours.util.hash );
  }
/**********************************************************************************************************/
/* Envoyer_identification: envoi de l'identification cliente au serveur                                   */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Envoyer_authentification ( struct CMD_TYPE_UTILISATEUR *util )
  { memcpy ( &Client_en_cours.util, util, sizeof(struct CMD_TYPE_UTILISATEUR) );            /* Sauvegarde */
    Calcul_password_hash ( FALSE, Client_en_cours.password );
    if ( !Envoi_serveur( TAG_CONNEXION, SSTAG_CLIENT_SEND_HASH,
                        (gchar *)&Client_en_cours.util, sizeof(struct CMD_TYPE_UTILISATEUR) ) )
     { Deconnecter();
       return;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
