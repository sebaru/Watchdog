/******************************************************************************************************************************/
/* Watchdogd/Serveur/ident.c        Gestion du logon user sur module Client Watchdog                                          */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          ven 03 avr 2009 19:46:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ident.c
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

 #include <glib.h>
 #include <string.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
 extern struct SSRV_CONFIG Cfg_ssrv;
/******************************************************************************************************************************/
/* Autoriser_client: Autorise le client à se connecter                                                                        */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 static void Autoriser_client ( struct CLIENT *client )
  { struct REZO_SRV_IDENT ident;
    g_snprintf( ident.comment, sizeof(ident.comment), "Serveur Watchdog v%s", VERSION );
    if ( Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_AUTORISE,
                       (gchar *)&ident, sizeof(struct REZO_SRV_IDENT) ) )
     { return; }
  }
/******************************************************************************************************************************/
/* Tester_autorisation: envoi de l'autorisation ou non au client                                                              */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 gboolean Tester_autorisation ( struct CLIENT *client, struct REZO_CLI_IDENT *ident )
  { int client_major, client_minor, client_micro;
    int server_major, server_minor, server_micro;
    struct CMD_GTK_MESSAGE gtkmessage;
    gchar *nom;

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "%s: Auth in progress for nom='%s', version='%s' (socket %d)", __func__,
              ident->nom, ident->version, client->connexion->socket );
    memcpy( &client->ident, ident, sizeof( struct REZO_CLI_IDENT ) );                              /* Recopie pour sauvegarde */

                                                                                            /* Vérification du MAJOR et MINOR */
    sscanf ( ident->version, "%d.%d.%d", &client_major, &client_minor, &client_micro );
    sscanf ( VERSION,        "%d.%d.%d", &server_major, &server_minor, &server_micro );

    if ( ! (client_major == server_major && client_minor == server_minor) )
     { g_snprintf( gtkmessage.message, sizeof(gtkmessage.message),
                   "Wrong version number (%s/%s)", ident->version, VERSION );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                    (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING, gtkmessage.message );
       Client_mode ( client, DECONNECTE );
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
             "%s: Auth NOK with wrong version number for nom='%s', version='%s' (socket %d)", __func__,
              ident->nom, ident->version, client->connexion->socket );
       return(FALSE);
     }

    if (client->certif) nom = Nom_certif(client->certif);                                       /* Recherche par certificat ? */
                   else nom = ident->nom;                                                    /* Ou par login / mot de passe ? */
    client->util = Rechercher_utilisateurDB_by_name ( nom );
    if (!client->util)
     { g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Unknown User '%s'", nom );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING, gtkmessage.message );
       Client_mode ( client, DECONNECTE );
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                "%s: User not found in DB for nom='%s', version='%s' (socket %d)", __func__,
                 ident->nom, ident->version, client->connexion->socket );
       return(FALSE);
     }

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "%s: User '%s' (id=%d) found in database. Checking parameters.", __func__,
              client->util->username, client->util->id );

/*********************************************************** Compte du client *************************************************/
    if (!client->util->enable)                                                 /* Est-ce que son compte est toujours actif ?? */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                "%s: Account disabled for %s(id=%d)", __func__, client->util->username, client->util->id );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_ACCOUNT_DISABLED, NULL, 0 );
       Client_mode (client, DECONNECTE);
       return(FALSE);
     }

/*********************************************** Authentification du client par login mot de passe ****************************/
    if (!client->certif)                                                                            /* si pas de certificat ! */
     { if ( Check_utilisateur_password( client->util, ident->passwd ) == FALSE )                     /* Comparaison des codes */
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                  "%s: Password error for '%s'(id=%d)", __func__, client->util->username, client->util->id );
          Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_REFUSE, NULL, 0 );
          Client_mode (client, DECONNECTE);
          return(FALSE);
        }
     }

    Autoriser_client ( client );
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "%s: Autorisation sent for %s(id=%d)", __func__, client->util->username, client->util->id );
                                                                               /* Le client est connecté, on en informe D.L.S */
    if (client->util->ssrv_bit_presence) SB(client->util->ssrv_bit_presence, 1);
    Client_mode (client, VALIDE);
    Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_CLI_VALIDE, NULL, 0 );                     /* Nous prévenons le client */
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
