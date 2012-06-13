/**********************************************************************************************************/
/* Client/ssl.c        Gestion de l'environnement SSL                                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 23 jun 2003 11:30:45 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ssl.c
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

 #include <stdio.h>
 #include <openssl/ssl.h>
 #include <openssl/err.h>
 #include <openssl/rand.h>

 #include "Erreur.h"
 #include "Reseaux.h"
 #include "Config_cli.h"
 #include "client.h"

 extern struct CLIENT Client_en_cours;                           /* Identifiant de l'utilisateur en cours */
 extern struct CONNEXION *Connexion;                                              /* connexion au serveur */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */
 extern SSL_CTX *Ssl_ctx;                                                                 /* Contexte SSL */
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/**********************************************************************************************************/
/* Init_SSL: Initialisation de l'environnement SSL                                                        */
/* Entrée: rien                                                                                           */
/* Sortie: un contexte SSL                                                                                */
/**********************************************************************************************************/
 SSL_CTX *Init_ssl ( void )
  { SSL_CTX *ssl_ctx;
    X509 *certif;
    gint retour;
    FILE *fd;

    SSL_load_error_strings();                                                    /* Initialisation de SSL */
    SSL_library_init();                                             /* Init SSL et PRNG: number générator */
    while(!RAND_status())
     { RAND_load_file( "/dev/urandom", 256 );
       Info( Config_cli.log, DEBUG_INFO, "Ajout RandADD" );
     }

    ssl_ctx = SSL_CTX_new ( SSLv23_client_method() );                       /* Création d'un contexte SSL */
    if (!ssl_ctx)
     { Info_c( Config_cli.log, DEBUG_CRYPTO, "SSL_CTX_new", ERR_error_string( ERR_get_error(), NULL ) );
       return( NULL );
     }

    SSL_CTX_set_mode( ssl_ctx, SSL_MODE_AUTO_RETRY );                                /* Mode non bloquant */       

    retour = SSL_CTX_load_verify_locations( ssl_ctx, FICHIER_CERTIF_CA, NULL );
    if (retour != 1)
     { Info_c( Config_cli.log, DEBUG_CRYPTO, "load verify locations", ERR_error_string( ERR_get_error(), NULL ) );
       Info_c( Config_cli.log, DEBUG_CRYPTO, "failed open file certif ca", FICHIER_CERTIF_CA );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    SSL_CTX_set_verify( ssl_ctx, SSL_VERIFY_PEER, NULL );         /* Type de verification des certificats */

                                                                                  /* Certificat du client */
    fd = fopen( FICHIER_CERTIF_CLIENT, "r" );
    if (!fd)
     { Info_c( Config_cli.log, DEBUG_CRYPTO, "failed open file certif", FICHIER_CERTIF_CLIENT );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    certif = PEM_read_X509( fd, NULL, NULL, NULL );                              /* Lecture du certificat */
    fclose(fd);
    if (!certif)
     { Info_c( Config_cli.log, DEBUG_CRYPTO, "certif loading failed", FICHIER_CERTIF_CLIENT );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    retour = SSL_CTX_use_certificate( ssl_ctx, certif );
    if (retour != 1)
     { Info_c( Config_cli.log, DEBUG_CRYPTO, "use certificate file", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_c( Config_cli.log, DEBUG_CRYPTO, "use certificate", Nom_certif(certif) );
                                                                                 /* Clef privée du client */
    retour = SSL_CTX_use_RSAPrivateKey_file( ssl_ctx, FICHIER_CERTIF_CLEF_CLIENT, SSL_FILETYPE_PEM );
    if (retour != 1)
     { Info_c( Config_cli.log, DEBUG_CRYPTO, "use RSAPrivate key", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    retour = SSL_CTX_check_private_key( ssl_ctx );                         /* Verification du certif/clef */
    if (retour != 1)
     { Info_c( Config_cli.log, DEBUG_CRYPTO, "check private key", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    Info( Config_cli.log, DEBUG_INFO, "SSL initialisation ok" );
    return(ssl_ctx);
  }  
/**********************************************************************************************************/
/* Connecter_ssl: Tentative de connexion sécurisée au serveur                                             */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 gboolean Connecter_ssl ( void )
  { gint retour;
    X509 *certif;

    if (!Connexion->ssl)                                               /* Premier appel de la fonction ?? */
     { Connexion->ssl = SSL_new( Ssl_ctx );                                  /* Instanciation du contexte */

       if (Connexion->ssl)                                                    /* Si réussite d'allocation */
        { Info( Config_cli.log, DEBUG_CRYPTO, "Connecter_ssl: SSL_new OK" );
          SSL_set_fd( Connexion->ssl, Connexion->socket );
          SSL_set_connect_state( Connexion->ssl );                               /* Nous sommes un client */
        }
       else
        { Info( Config_cli.log, DEBUG_CRYPTO, "Connecter_ssl: SSL_new failed" );
          Log( "Impossible de creer le contexte SSL" );
          Deconnecter();
          return(FALSE);
        }
     }
encore:
    retour = SSL_connect( Connexion->ssl );
    if (retour<=0)
     { retour = SSL_get_error( Connexion->ssl, retour );
       if (retour == SSL_ERROR_WANT_READ || retour == SSL_ERROR_WANT_WRITE)
        { Info( Config_cli.log, DEBUG_CRYPTO, "Connecter_ssl: SSL_connect need more data" );
          sleep(1);
          goto encore;
        }
       
       Info_n( Config_cli.log, DEBUG_CRYPTO, "Connecter_ssl: SSL_connect get error", retour );
       Info_c( Config_cli.log, DEBUG_CRYPTO, "Connecter_ssl: SSL_connect",
                                             ERR_error_string( ERR_get_error(), NULL ) );
     }
                          /* Ici, la connexion a été effectuée, il faut maintenant tester les certificats */
    certif = SSL_get_peer_certificate( Connexion->ssl );             /* On prend le certificat du serveur */
    if (!certif)
     { Info( Config_cli.log, DEBUG_CRYPTO, "Connecter_ssl: no certificate received" );
       Log( "Aucun certificat reçu" );
       Deconnecter();
       return(FALSE);
     }
    Info( Config_cli.log, DEBUG_CRYPTO, "Connecter_ssl: certificate received" );

    Info_c( Config_cli.log, DEBUG_CRYPTO, "Connecter_ssl: algo crypto",
                                          (gchar *) SSL_get_cipher_name( Connexion->ssl ) );
    Info_n( Config_cli.log, DEBUG_CRYPTO, "Connecter_ssl: longueur clef",
                                          SSL_get_cipher_bits( Connexion->ssl, NULL ) );

    retour = SSL_get_verify_result( Connexion->ssl );                       /* Verification du certificat */
    if ( retour != X509_V_OK )                                      /* Si erreur, on se deconnecte presto */
     { Info( Config_cli.log, DEBUG_CRYPTO, "Connecter_ssl: unauthorized certificate" );
       Log( "Impossible de vérifier le certificat" );
       Deconnecter();
       return(FALSE);
     }

    Info_c( Config_cli.log, DEBUG_CRYPTO, "Connecter_ssl: partenaire", Nom_certif ( certif ) );
    Info_c( Config_cli.log, DEBUG_CRYPTO, "Connecter_ssl: signataire", Nom_certif_signataire ( certif ) );
        return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
