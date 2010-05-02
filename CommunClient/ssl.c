/**********************************************************************************************************/
/* CommunClient/ssl.c        Gestion de l'environnement SSL                                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 02 mai 2010 21:01:40 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ssl.c
 * This file is part of <program name>
 *
 * Copyright (C) 2009 - sebastien
 *
 * <program name> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <program name> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <program name>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 
 #include <stdio.h>
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocommunclient.h"

/**********************************************************************************************************/
/* Close_SSL: Initialisation de l'environnement SSL                                                        */
/* Entrée: rien                                                                                           */
/* Sortie: un contexte SSL                                                                                */
/**********************************************************************************************************/
 void Close_ssl ( struct CLIENT *client )
  { SSL_CTX_free(client->ssl_ctx);
    client->ssl_ctx = NULL;
  }

/**********************************************************************************************************/
/* Init_SSL: Initialisation de l'environnement SSL                                                        */
/* Entrée: rien                                                                                           */
/* Sortie: un contexte SSL                                                                                */
/**********************************************************************************************************/
 gboolean Init_ssl ( struct CLIENT *client )
  { SSL_CTX *ssl_ctx;
    X509 *certif;
    gint retour;
    FILE *fd;

    SSL_load_error_strings();                                                    /* Initialisation de SSL */
    SSL_library_init();                                             /* Init SSL et PRNG: number générator */
    while(!RAND_status())
     { RAND_load_file( "/dev/urandom", 256 );
       Info( client->config.log, DEBUG_INFO, "Ajout RandADD" );
     }

    ssl_ctx = SSL_CTX_new ( SSLv23_client_method() );                       /* Création d'un contexte SSL */
    if (!ssl_ctx)
     { Info_c( client->config.log, DEBUG_CRYPTO, "SSL_CTX_new", ERR_error_string( ERR_get_error(), NULL ) );
       return( FALSE );
     }

    SSL_CTX_set_mode( ssl_ctx, SSL_MODE_AUTO_RETRY );                                /* Mode non bloquant */       

    retour = SSL_CTX_load_verify_locations( ssl_ctx, FICHIER_CERTIF_CA, NULL );
    if (retour != 1)
     { Info_c( client->config.log, DEBUG_CRYPTO,
               "load verify locations", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return( FALSE );
     }

    SSL_CTX_set_verify( ssl_ctx, SSL_VERIFY_PEER, NULL );         /* Type de verification des certificats */

                                                                                  /* Certificat du client */
    fd = fopen( FICHIER_CERTIF_CLIENT, "r" );
    if (!fd)
     { Info_c( client->config.log, DEBUG_CRYPTO, "failed open file certif", FICHIER_CERTIF_CLIENT );
       SSL_CTX_free(ssl_ctx);
       return( FALSE );
     }
    certif = PEM_read_X509( fd, NULL, NULL, NULL );                              /* Lecture du certificat */
    fclose(fd);
    if (!certif)
     { Info_c( client->config.log, DEBUG_CRYPTO, "certif loading failed", FICHIER_CERTIF_CLIENT );
       SSL_CTX_free(ssl_ctx);
       return( FALSE );
     }

    retour = SSL_CTX_use_certificate( ssl_ctx, certif );
    if (retour != 1)
     { Info_c( client->config.log, DEBUG_CRYPTO, "use certificate file",
               ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return( FALSE );
     }
    Info_c( client->config.log, DEBUG_CRYPTO, "use certificate", Nom_certif(certif) );
                                                                                 /* Clef privée du client */
    retour = SSL_CTX_use_RSAPrivateKey_file( ssl_ctx, FICHIER_CERTIF_CLEF_CLIENT, SSL_FILETYPE_PEM );
    if (retour != 1)
     { Info_c( client->config.log, DEBUG_CRYPTO, "use RSAPrivate key", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return( FALSE );
     }

    retour = SSL_CTX_check_private_key( ssl_ctx );                         /* Verification du certif/clef */
    if (retour != 1)
     { Info_c( client->config.log, DEBUG_CRYPTO, "check private key", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return( FALSE );
     }

    Info( client->config.log, DEBUG_INFO, "SSL initialisation ok" );
    client->ssl_ctx = ssl_ctx;
    return(TRUE);
  }  
/**********************************************************************************************************/
/* Connecter_ssl: Tentative de connexion sécurisée au serveur                                             */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 gboolean Connecter_ssl ( struct CLIENT *client )
  { gint retour;
    X509 *certif;

    if (!client->connexion->ssl)                                       /* Premier appel de la fonction ?? */
     { client->connexion->ssl = SSL_new( client->ssl_ctx );                  /* Instanciation du contexte */

       if (client->connexion->ssl)                                            /* Si réussite d'allocation */
        { SSL_set_fd( client->connexion->ssl, client->connexion->socket );
          SSL_set_connect_state( client->connexion->ssl );                       /* Nous sommes un client */
        }
       else
        { Info( client->config.log, DEBUG_CRYPTO, "Connecter_ssl: SSL_new failed" );
          return(FALSE);
        }
     }

encore:
    retour = SSL_connect( client->connexion->ssl );
    if (retour<=0)
     { retour = SSL_get_error( client->connexion->ssl, retour );
       if (retour == SSL_ERROR_WANT_READ || retour == SSL_ERROR_WANT_WRITE)
        { /*Info( client->config.log, DEBUG_CRYPTO, "Connecter_ssl: SSL_connect need more data" );*/
          goto encore;
        }
       
       Info_n( client->config.log, DEBUG_CRYPTO, "Connecter_ssl: SSL_connect get error", retour );
       Info_c( client->config.log, DEBUG_CRYPTO, "Connecter_ssl: SSL_connect",
                                             ERR_error_string( ERR_get_error(), NULL ) );
     }
                          /* Ici, la connexion a été effectuée, il faut maintenant tester les certificats */
    certif = SSL_get_peer_certificate( client->connexion->ssl );     /* On prend le certificat du serveur */
    if (!certif)
     { Info( client->config.log, DEBUG_CRYPTO, "Connecter_ssl: no certificate received" );
       return(FALSE);
     }
    Info( client->config.log, DEBUG_CRYPTO, "Connecter_ssl: certificate received" );

    Info_c( client->config.log, DEBUG_CRYPTO, "Connecter_ssl: algo crypto",
                                          (gchar *) SSL_get_cipher_name( client->connexion->ssl ) );
    Info_n( client->config.log, DEBUG_CRYPTO, "Connecter_ssl: longueur clef",
                                          SSL_get_cipher_bits( client->connexion->ssl, NULL ) );

    retour = SSL_get_verify_result( client->connexion->ssl );               /* Verification du certificat */
    if ( retour != X509_V_OK )                                      /* Si erreur, on se deconnecte presto */
     { Info( client->config.log, DEBUG_CRYPTO, "Connecter_ssl: unauthorized certificate" );
       return(FALSE);
     }

    Info_c( client->config.log, DEBUG_CRYPTO, "Connecter_ssl: partenaire", Nom_certif ( certif ) );
    Info_c( client->config.log, DEBUG_CRYPTO, "Connecter_ssl: signataire", Nom_certif_signataire ( certif ) );
    
    client->mode = ENVOI_IDENT;
    Info( client->config.log, DEBUG_CONNEXION, "client en mode ENVOI_IDENT" );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
