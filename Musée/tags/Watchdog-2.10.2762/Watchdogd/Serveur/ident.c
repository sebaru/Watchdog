/**********************************************************************************************************/
/* Watchdogd/Serveur/ident.c        Gestion du logon user sur module Client Watchdog                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 19:46:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ident.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 
/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Autoriser_client: Autorise le client à se connecter                                                    */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Autoriser_client ( struct CLIENT *client )
  { struct REZO_SRV_IDENT ident;
    g_snprintf( ident.comment, sizeof(ident.comment), "Serveur Watchdog v%s", VERSION );
    if ( Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_AUTORISE,
                       (gchar *)&ident, sizeof(struct REZO_SRV_IDENT) ) )
     { return; }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "Autoriser_autorisation: RAZ 'login failed' for %d", client->util->id );
    Raz_login_failed( client->util->id );
  }
/**********************************************************************************************************/
/* Proto_set_password: changement de password                                                             */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Proto_set_password ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *util )
  { if (util->id != client->util->id)                                        /* Le client est-il le bon ? */
     { Client_mode ( client, DECONNECTE );
       return;
     }

    if (Modifier_utilisateurDB_set_password( util ))
     { Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_PWDCHANGED, NULL, 0 ); }
    else
     { Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_CANNOTCHANGEPWD, NULL, 0 ); }
    Client_mode ( client, DECONNECTE );                        /* On deconnecte le client tout de suite ! */
  }
/**********************************************************************************************************/
/* Tester_autorisation: envoi de l'autorisation ou non au client                                          */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Tester_autorisation ( struct CLIENT *client, struct REZO_CLI_IDENT *ident )
  { int client_major, client_minor, client_micro;
    int server_major, server_minor, server_micro;
    struct CMD_GTK_MESSAGE gtkmessage;
    gchar *nom;

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Tester_autorisation: Auth in progress for nom='%s', version='%s' (socket %d)",
              ident->nom, ident->version, client->connexion->socket );
    memcpy( &client->ident, ident, sizeof( struct REZO_CLI_IDENT ) );          /* Recopie pour sauvegarde */
            
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
       return;
     }

    if (client->certif) nom = Nom_certif(client->certif);                   /* Recherche par certificat ? */
                   else nom = ident->nom;                                /* Ou par login / mot de passe ? */
    client->util = Rechercher_utilisateurDB_by_name ( nom );
    if (!client->util)
     { g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Unknown User '%s'", nom );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING, gtkmessage.message );
       Client_mode ( client, DECONNECTE );
       return;
     }

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "Tester_autorisation: User %s (id=%d) found in database. Checking parameters.",
              client->util->nom, client->util->id );

/********************************************* Compte du client *******************************************/
    if (!client->util->enable)                             /* Est-ce que son compte est toujours actif ?? */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,  
                "Tester_autorisation: Account disabled for %s(id=%d)", client->util->nom, client->util->id );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_ACCOUNT_DISABLED, NULL, 0 );
       Client_mode (client, DECONNECTE);
       return;
     }

    if (client->util->expire && client->util->date_expire<time(NULL) )   /* Expiration temporel du compte */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING, 
                "Tester_autorisation: Account expired for %s(id=%d)", client->util->nom, client->util->id );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_ACCOUNT_EXPIRED, NULL, 0 );
       Client_mode (client, DECONNECTE);
       return;
     }

/*************************************** Authentification du client par login mot de passe ****************/
    if (!client->certif)                                                        /* si pas de certificat ! */
     { if ( Check_utilisateur_password( client->util, ident->passwd ) == FALSE ) /* Comparaison des codes */
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,  
                  "Tester_autorisation: Password error for '%s'(id=%d)", client->util->nom, client->util->id );
          Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_REFUSE, NULL, 0 );
          Ajouter_one_login_failed( client->util->id, Config.max_login_failed );             /* Dommage ! */
          Client_mode (client, DECONNECTE);
          return;
        }
     }

    if (client->util->mustchangepwd)                    /* L'utilisateur doit-il changer son mot de passe */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,  
                "Tester_autorisation: User '%s'(id=%d) has to change his password",
                 client->util->nom, client->util->id );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_NEEDCHANGEPWD, NULL, 0 );
       Client_mode (client, WAIT_FOR_NEWPWD);
       return;
     }

    Autoriser_client ( client );
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "Tester_autorisation: Autorisation sent for %s(id=%d)", client->util->nom, client->util->id );
                                                           /* Le client est connecté, on en informe D.L.S */
    if (client->util->ssrv_bit_presence) SB(client->util->ssrv_bit_presence, 1);
    Client_mode (client, ENVOI_SYNCHRO);
  }
/*--------------------------------------------------------------------------------------------------------*/
