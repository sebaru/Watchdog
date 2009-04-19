/**********************************************************************************************************/
/* Watchdogd/Serveur/accept.c        Gestion des connexions securisées                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 24 jun 2003 12:58:56 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <stdio.h>
 #include <openssl/ssl.h>
 #include <openssl/err.h>
 #include <openssl/rand.h>

 #include "Erreur.h"
 #include "Reseaux.h"
 #include "Config.h"
 /*#include "watchdogd.h"*/

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 extern SSL_CTX *Ssl_ctx;                                          /* Contexte de cryptage des connexions */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Connecter_ssl: Tentative de connexion sécurisée                                                        */
/* Entrée: le pointeur du client                                                                          */
/**********************************************************************************************************/
 void Connecter_ssl( struct CLIENT *client )
  { struct CONNEXION *connexion;
    X509 *certif;
    gint retour;

    connexion = client->connexion;
  
    if (!connexion->ssl)                                                  /* Premier appel de la fonction */
     { connexion->ssl = SSL_new( Ssl_ctx );                                    /* Creation d'une instance */
       if (!connexion->ssl)                                                   /* Si réussite d'allocation */
        { Info( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: unable to create a ssl object" );
          client->mode = DECONNECTE;  
          return;         
        }

       SSL_set_fd( connexion->ssl, connexion->socket );
       SSL_set_accept_state( connexion->ssl );
       Info_n( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: set_accept_state", client->connexion->socket );
     }
    Info_n( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: accept en cours 1", client->connexion->socket );
    retour = SSL_accept( connexion->ssl );
    Info_n( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: accept en cours 2", retour );
    if (retour<=0)
     { retour = SSL_get_error( connexion->ssl, retour );
       if (retour == SSL_ERROR_WANT_READ || retour == SSL_ERROR_WANT_WRITE)
        { Info_n( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: need more data", client->connexion->socket );
          return;
        }
       
       Info_n( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: SSL_accept get error", retour );
       while ( (retour=ERR_get_error()) )
        { Info_c( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: SSL_accept",
                                            ERR_error_string( retour, NULL ) );
        }
     }
                          /* Ici, la connexion a été effectuée, il faut maintenant tester les certificats */ 
    certif = SSL_get_peer_certificate( connexion->ssl );             /* On prend le certificat du serveur */
    if (!certif)
     { Info( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: no certificate received" );
       client->mode = DECONNECTE;
       return;
     }
    Info( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: certificate received" );

    retour = SSL_get_verify_result( connexion->ssl );                       /* Verification du certificat */
    if ( retour != X509_V_OK )                                      /* Si erreur, on se deconnecte presto */
     { Info( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: unauthorized certificate" );
       client->mode = DECONNECTE;
       return;
     }

    Info_c( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: algo crypto",
                                      (gchar *)SSL_get_cipher_name( connexion->ssl ) );
    Info_n( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: longueur clef",
                                      SSL_get_cipher_bits( connexion->ssl, NULL ) );

    Info_c( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: partenaire", Nom_certif ( certif ) );
    Info_c( Config.log, DEBUG_CRYPTO, "SSRV: Connecter_ssl: signataire", Nom_certif_signataire ( certif ) );
    
    Client_mode ( client, ATTENTE_IDENT );
  }
/*--------------------------------------------------------------------------------------------------------*/
