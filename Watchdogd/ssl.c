/**********************************************************************************************************/
/* Watchdogd/ssl.c               Gestion des connexions securisées                                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 24 jun 2003 12:58:56 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ssl.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
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
 #include "Config.h"
 #include "watchdogd.h"

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 SSL_CTX *Ssl_ctx;                                                 /* Contexte de cryptage des connexions */
/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"

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
    STACK_OF(X509_NAME) *stack;
    DH *dh;

    Info( Config.log, DEBUG_CRYPTO, "MSRV: SSL init" );
    SSL_load_error_strings();                                                    /* Initialisation de SSL */
    SSL_library_init();                                             /* Init SSL et PRNG: number générator */
    while(!RAND_status())
     { RAND_load_file( "/dev/urandom", 256 );
       Info( Config.log, DEBUG_CRYPTO, "Ajout RandADD" );
     }
    Info( Config.log, DEBUG_CRYPTO, "MSRV: SSL random ok" );

    ssl_ctx = SSL_CTX_new ( SSLv23_server_method() );                       /* Création d'un contexte SSL */
    if (!ssl_ctx)
     { Info_c( Config.log, DEBUG_CRYPTO, "set server method", ERR_error_string( ERR_get_error(), NULL ) );
       return(NULL);
     }
    Info( Config.log, DEBUG_CRYPTO, "MSRV: SSL server method ok" );

    SSL_CTX_set_mode( ssl_ctx, SSL_MODE_AUTO_RETRY );                                /* Mode non bloquant */
    retour = SSL_CTX_load_verify_locations( ssl_ctx, FICHIER_CERTIF_CA, NULL );
    if (retour != 1)
     { Info_c( Config.log, DEBUG_CRYPTO, "MSRV: load verify locations",
                                         ERR_error_string( ERR_get_error(), NULL ) );
       Info_c( Config.log, DEBUG_CRYPTO, "MSRV: failed open file certif ca", FICHIER_CERTIF_CA );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info( Config.log, DEBUG_CRYPTO, "MSRV: SSL location ok" );

                                                                  /* Type de verification des certificats */
    SSL_CTX_set_verify( ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL );

    fd = fopen( FICHIER_CERTIF_SERVEUR, "r" );
    if (!fd)
     { Info_c( Config.log, DEBUG_CRYPTO, "MSRV: failed open file certif serveur", FICHIER_CERTIF_SERVEUR );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_c( Config.log, DEBUG_CRYPTO, "MSRV: open certif server", FICHIER_CERTIF_SERVEUR );
       
    certif = PEM_read_X509( fd, NULL, NULL, NULL );                              /* Lecture du certificat */
    fclose(fd);
    if (!certif)
     { Info_c( Config.log, DEBUG_CRYPTO, "MSRV: certif loading failed", FICHIER_CERTIF_SERVEUR );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    retour = SSL_CTX_use_certificate( ssl_ctx, certif );
    if (retour != 1)
     { Info_c( Config.log, DEBUG_CRYPTO, "MSRV: use certificate file", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_c( Config.log, DEBUG_CRYPTO, "MSRV: use certificate", Nom_certif(certif) );

    retour = SSL_CTX_use_RSAPrivateKey_file( ssl_ctx, FICHIER_CERTIF_CLEF_SERVEUR, SSL_FILETYPE_PEM );
    if (retour != 1)
     { Info_c( Config.log, DEBUG_CRYPTO, "MSRV: use RSAPrivate key server",
                           ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_c( Config.log, DEBUG_CRYPTO, "MSRV: use RSAPrivate key server ", FICHIER_CERTIF_CLEF_SERVEUR );

    retour = SSL_CTX_check_private_key( ssl_ctx );                         /* Verification du certif/clef */
    if (retour != 1)
     { Info_c( Config.log, DEBUG_CRYPTO, "MSRV: check private key", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    stack = SSL_load_client_CA_file( FICHIER_CERTIF_CA );
    if (!stack)
     { Info_c( Config.log, DEBUG_CRYPTO, "MSRV: load client CA", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_c( Config.log, DEBUG_CRYPTO, "MSRV: use master ca certificate", FICHIER_CERTIF_CA );
       
    SSL_CTX_set_client_CA_list( ssl_ctx, stack );

    dh = DH_generate_parameters( Config.taille_clef_dh, 5, NULL, NULL );     /* Generation Diffie hellman */
    if (!dh)
     { Info_c( Config.log, DEBUG_CRYPTO, "MSRV: Dh param", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info( Config.log, DEBUG_CRYPTO, "MSRV: creation clef DH ok" );

    retour = SSL_CTX_set_tmp_dh( ssl_ctx, dh );
    if (retour != 1)
     { Info_c( Config.log, DEBUG_CRYPTO, "MSRV: Set DH", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info( Config.log, DEBUG_CRYPTO, "MSRV: activation clef DH ok" );
       
    retour = SSL_CTX_set_tmp_rsa( ssl_ctx, Config.rsa );                 /* Clefs publique et privée RSA */
    if (retour != 1)
     { Info_c( Config.log, DEBUG_CRYPTO, "MSRV: Set RSA", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    Info( Config.log, DEBUG_CRYPTO, "MSRV: SSL initialisation ok" );
    return( ssl_ctx );
  }
/*--------------------------------------------------------------------------------------------------------*/
