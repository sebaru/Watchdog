/**********************************************************************************************************/
/* Watchdogd/Serveur/accept.c        Gestion des connexions securisées                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                      mar 24 jun 2003 12:58:56 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * accept.c
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

 #include <stdio.h>
 #include <openssl/ssl.h>
 #include <openssl/err.h>
 #include <openssl/rand.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Connecter_ssl: Tentative de connexion sécurisée                                                        */
/* Entrée: le pointeur du client                                                                          */
/**********************************************************************************************************/
 void Connecter_ssl( struct CLIENT *client )
  { struct CONNEXION *connexion;
    gint retour;

    connexion = client->connexion;
  
    if (!connexion->ssl)                                                  /* Premier appel de la fonction */
     { connexion->ssl = SSL_new( Cfg_ssrv.Ssl_ctx );                           /* Creation d'une instance */
       if (!connexion->ssl)                                                   /* Si réussite d'allocation */
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                   "Connecter_ssl: unable to create a ssl object" );
          client->mode = DECONNECTE;  
          return;         
        }

       SSL_set_fd( connexion->ssl, connexion->socket );
       SSL_set_accept_state( connexion->ssl );
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                "Connecter_ssl: set_accept_state %d", client->connexion->socket );
     }

    retour = SSL_accept( connexion->ssl );
    if (retour<=0)
     { retour = SSL_get_error( connexion->ssl, retour );
       if (retour == SSL_ERROR_WANT_READ || retour == SSL_ERROR_WANT_WRITE)
        { /*Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                   "Connecter_ssl: need more data", client->connexion->socket );*/
          return;
        }
       
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                "Connecter_ssl: SSL_accept get error %d (%s)",
                 retour, ERR_error_string( retour, NULL ) );
       while ( (retour=ERR_get_error()) )
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                   "Connecter_ssl: SSL_accept error %s",
                    ERR_error_string( retour, NULL ) );
        }
       client->mode = DECONNECTE;
       return;
     }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "Connecter_ssl: Secured Connexion with %s, (%d bits)",
             (gchar *)SSL_get_cipher_name( connexion->ssl ), SSL_get_cipher_bits( connexion->ssl, NULL ) );

          /* Ici, la connexion a été effectuée, il faut maintenant tester les certificats si necessaire ! */ 
    client->certif = SSL_get_peer_certificate( connexion->ssl );      /* On prend le certificat du client */
    if (!client->certif && Cfg_ssrv.ssl_peer_cert)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                "Connecter_ssl: no certificate received and ssl_peer_cert = TRUE in config." );
       client->mode = DECONNECTE;
       return;
     }

    if (client->certif)                                 /* Si nous avons un certificat, nous le verifions */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,  "Connecter_ssl: certificate received" );

       retour = SSL_get_verify_result( connexion->ssl );                    /* Verification du certificat */
       if ( retour != X509_V_OK )                                   /* Si erreur, on se deconnecte presto */
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                   "Connecter_ssl: unauthorized certificate for %s: error %d (%s)",
                    Nom_certif(client->certif), retour, ERR_error_string( retour, NULL ) );
          client->mode = DECONNECTE;
          return;
        }

       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                "Connecter_ssl: Peer = %s, issuer = %s",
                 Nom_certif ( client->certif ), Nom_certif_signataire ( client->certif ) );
     }
    else
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
                "Connecter_ssl: No certificate received" );
     }
    Client_mode ( client, WAIT_FOR_IDENT );
  }
/*--------------------------------------------------------------------------------------------------------*/
