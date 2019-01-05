/**********************************************************************************************************/
/* Watchdogd/ssl.c               Gestion des connexions securisées                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                      mar 24 jun 2003 12:58:56 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ssl.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien LEFEVRE
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
/* Init_SSL: Initialisation de l'environnement SSL                                                        */
/* Entrée: rien                                                                                           */
/* Sortie: un contexte SSL                                                                                */
/**********************************************************************************************************/
 SSL_CTX *Init_ssl ( void )
  { STACK_OF(X509_NAME) *stack;
    SSL_CTX *ssl_ctx;
    gint retour;
    FILE *fd;
    
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG, "Init_ssl SSL init" );
    SSL_load_error_strings();                                                    /* Initialisation de SSL */
    SSL_library_init();                                             /* Init SSL et PRNG: number générator */


    while(!RAND_status())
     { RAND_load_file( "/dev/urandom", 256 );
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG, "Init_ssl: Ajout RandADD" );
     }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO, "Init_ssl: SSL random ok" );

    ssl_ctx = SSL_CTX_new ( TLS_server_method() );                          /* Création d'un contexte SSL */
    if (!ssl_ctx)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                 "Init_ssl : set server method failed (%s)", ERR_error_string( ERR_get_error(), NULL ) );
       return(NULL);
     }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO, "Init_ssl: SSL server method ok" );

    SSL_CTX_set_mode( ssl_ctx, SSL_MODE_AUTO_RETRY );                                /* Mode non bloquant */
    SSL_CTX_set_options( ssl_ctx, SSL_OP_SINGLE_DH_USE );                    /* Options externe à OpenSSL */


    retour = SSL_CTX_load_verify_locations( ssl_ctx, Cfg_ssrv.ssl_file_ca, NULL );
    if (retour != 1)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                 "load verify locations failed (%s), CA file %s",
                 ERR_error_string( ERR_get_error(), NULL ), Cfg_ssrv.ssl_file_ca );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO, "Init_ssl: SSL location ok" );

                                                                  /* Type de verification des certificats */
    if (Cfg_ssrv.ssl_peer_cert == FALSE)
     { SSL_CTX_set_verify( ssl_ctx, SSL_VERIFY_PEER, NULL ); }
    else
     { SSL_CTX_set_verify( ssl_ctx, SSL_VERIFY_PEER | SSL_VERIFY_FAIL_IF_NO_PEER_CERT, NULL ); }

    fd = fopen( Cfg_ssrv.ssl_file_cert, "r" );
    if (!fd)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                "Init_ssl: failed open file %s certif serveur failed (%s)", Cfg_ssrv.ssl_file_cert, strerror(errno) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO, "Init_ssl: open certif server OK (%s)", Cfg_ssrv.ssl_file_cert );
       
    Cfg_ssrv.ssrv_certif = PEM_read_X509( fd, NULL, NULL, NULL );                /* Lecture du certificat */
    fclose(fd);
    if (!Cfg_ssrv.ssrv_certif)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                "Init_ssl: certif loading failed (%s)", Cfg_ssrv.ssl_file_cert );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    retour = SSL_CTX_use_certificate( ssl_ctx, Cfg_ssrv.ssrv_certif );
    if (retour != 1)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                "Init_ssl: use certificate file failed (%s)", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "Init_ssl: use certificate %s", Nom_certif(Cfg_ssrv.ssrv_certif) );

    retour = SSL_CTX_use_RSAPrivateKey_file( ssl_ctx, Cfg_ssrv.ssl_file_key, SSL_FILETYPE_PEM );
    if (retour != 1)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                "Init_ssl: use RSAPrivate key server failed (%s)",
                 ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "Init_ssl: use RSAPrivate key server file %s", Cfg_ssrv.ssl_file_key );

    retour = SSL_CTX_check_private_key( ssl_ctx );                         /* Verification du certif/clef */
    if (retour != 1)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                "Init_ssl: check private key failed (%s)", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    stack = SSL_load_client_CA_file( Cfg_ssrv.ssl_file_ca );
    if (!stack)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                "Init_ssl: load client CA failed (%s)", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO, 
             "Init_ssl: use master ca certificate %s", Cfg_ssrv.ssl_file_ca );
    SSL_CTX_set_client_CA_list( ssl_ctx, stack );
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
             "Init_ssl: SSL initialisation ok" );
    return( ssl_ctx );
  }
/**********************************************************************************************************/
/* Init_SSL: Initialisation de l'environnement SSL                                                        */
/* Entrée: rien                                                                                           */
/* Sortie: un contexte SSL                                                                                */
/**********************************************************************************************************/
 void Liberer_SSL ( void )
  { if (Cfg_ssrv.Ssl_ctx) SSL_CTX_free( Cfg_ssrv.Ssl_ctx );                         /* Libération mémoire */
  } 
/*--------------------------------------------------------------------------------------------------------*/
