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

 #include "watchdogd.h"
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

    Info_new( Config.log, Config.log_all, LOG_DEBUG, "Init_ssl SSL init" );
    SSL_load_error_strings();                                                    /* Initialisation de SSL */
    SSL_library_init();                                             /* Init SSL et PRNG: number générator */
    while(!RAND_status())
     { RAND_load_file( "/dev/urandom", 256 );
       Info_new( Config.log, Config.log_all, LOG_DEBUG, "Init_ssl: Ajout RandADD" );
     }
    Info_new( Config.log, Config.log_all, LOG_INFO, "Init_ssl: SSL random ok" );

    ssl_ctx = SSL_CTX_new ( SSLv23_server_method() );                       /* Création d'un contexte SSL */
    if (!ssl_ctx)
     { Info_new( Config.log, Config.log_all, LOG_ERR,
                 "Init_ssl : set server method failed (%s)", ERR_error_string( ERR_get_error(), NULL ) );
       return(NULL);
     }
    Info_new( Config.log, Config.log_all, LOG_INFO, "Init_ssl: SSL server method ok" );

    SSL_CTX_set_mode( ssl_ctx, SSL_MODE_AUTO_RETRY );                                /* Mode non bloquant */
    retour = SSL_CTX_load_verify_locations( ssl_ctx, FICHIER_CERTIF_CA, NULL );
    if (retour != 1)
     { Info_new( Config.log, Config.log_all, LOG_ERR,
                 "load verify locations failed (%s), CA file %s",
                 ERR_error_string( ERR_get_error(), NULL ), FICHIER_CERTIF_CA );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Config.log_all, LOG_INFO, "Init_ssl: SSL location ok" );

                                                                  /* Type de verification des certificats */
    SSL_CTX_set_verify( ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL );

    fd = fopen( FICHIER_CERTIF_SERVEUR, "r" );
    if (!fd)
     { Info_new( Config.log, Config.log_all, LOG_ERR,
                "Init_ssl: failed open file %s certif serveur failed (%s)", FICHIER_CERTIF_SERVEUR, strerror(errno) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Config.log_all, LOG_INFO, "Init_ssl: open certif server OK (%s)", FICHIER_CERTIF_SERVEUR );
       
    certif = PEM_read_X509( fd, NULL, NULL, NULL );                              /* Lecture du certificat */
    fclose(fd);
    if (!certif)
     { Info_new( Config.log, Config.log_all, LOG_ERR, "Init_ssl: certif loading failed (%s)", FICHIER_CERTIF_SERVEUR );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    retour = SSL_CTX_use_certificate( ssl_ctx, certif );
    if (retour != 1)
     { Info_new( Config.log, Config.log_all, LOG_ERR,
                "Init_ssl: use certificate file failed (%s)", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Config.log_all, LOG_INFO, "Init_ssl: use certificate %s", Nom_certif(certif) );

    retour = SSL_CTX_use_RSAPrivateKey_file( ssl_ctx, FICHIER_CERTIF_CLEF_SERVEUR, SSL_FILETYPE_PEM );
    if (retour != 1)
     { Info_new( Config.log, Config.log_all, LOG_ERR,
                "Init_ssl: use RSAPrivate key server failed (%s)",
                 ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Config.log_all, LOG_INFO,
             "Init_ssl: use RSAPrivate key server file %s", FICHIER_CERTIF_CLEF_SERVEUR );

    retour = SSL_CTX_check_private_key( ssl_ctx );                         /* Verification du certif/clef */
    if (retour != 1)
     { Info_new( Config.log, Config.log_all, LOG_ERR,
                "Init_ssl: check private key failed (%s)", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    stack = SSL_load_client_CA_file( FICHIER_CERTIF_CA );
    if (!stack)
     { Info_new( Config.log, Config.log_all, LOG_ERR,
                "Init_ssl: load client CA failed (%s)", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Config.log_all, LOG_INFO, "Init_ssl: use master ca certificate %s", FICHIER_CERTIF_CA );
       
    SSL_CTX_set_client_CA_list( ssl_ctx, stack );

    dh = DH_generate_parameters( Config.taille_clef_dh, 5, NULL, NULL );     /* Generation Diffie hellman */
    if (!dh)
     { Info_new( Config.log, Config.log_all, LOG_ERR,
                "Init_ssl: Dh parameters failed (%s)", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Config.log_all, LOG_INFO, "Init_ssl: creation clef DH ok" );

    retour = SSL_CTX_set_tmp_dh( ssl_ctx, dh );
    if (retour != 1)
     { Info_new( Config.log, Config.log_all, LOG_ERR,
                "Init_ssl: Set DH failed (%s)", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Config.log_all, LOG_INFO, "Init_ssl: activation clef DH ok" );
       
    retour = SSL_CTX_set_tmp_rsa( ssl_ctx, Config.rsa );                 /* Clefs publique et privée RSA */
    if (retour != 1)
     { Info_new( Config.log, Config.log_all, LOG_ERR,
                "Init_ssl: Set RSA failed (%s)", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    Info_new( Config.log, Config.log_all, LOG_NOTICE, "Init_ssl: SSL initialisation ok" );
    return( ssl_ctx );
  }
/*--------------------------------------------------------------------------------------------------------*/
