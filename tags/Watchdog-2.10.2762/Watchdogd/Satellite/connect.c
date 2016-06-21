/**********************************************************************************************************/
/* Watchdogd/Satellite/connect.c        Gestion des connexions au maitre Watchdog v2.0                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 28 sept. 2014 18:05:15 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Satellite.c
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
 
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <string.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/socket.h>
 #include <sys/stat.h>
 #include <netinet/in.h>
 #include <fcntl.h>
 #include <netdb.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Satellite.h"

/**********************************************************************************************************/
/* Satellite_Envoyer_maitre: Envoi d'un paquet au serveur maitre                                          */
/* Entrée: des infos sur le paquet à envoyer                                                              */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Satellite_Envoyer_maitre ( gint tag, gint ss_tag, gchar *buffer, gint taille )
  { if ( Envoyer_reseau( Cfg_satellite.Connexion, tag, ss_tag, buffer, taille ) )
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                "Satellite_Envoyer_maitre: Deconnexion sur erreur envoi au serveur" );
       Satellite_Deconnecter_sale();
     }
  }
/**********************************************************************************************************/
/* Satellite_Deconnecter: Arrete la connexion avec l'instance maitre                                      */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Satellite_Deconnecter_sale ( void )
  { Fermer_connexion(Cfg_satellite.Connexion);
    SSL_CTX_free(Cfg_satellite.Ssl_ctx);                                    /* Libération du contexte SSL */
    Cfg_satellite.Ssl_ctx = NULL;
    Cfg_satellite.Connexion = NULL;
    Cfg_satellite.Mode = SAT_DISCONNECTED;
    Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO,
             "Satellite_Deconnecter_sale: Satellite DISCONNECTED" );
  }
/**********************************************************************************************************/
/* Deconnecter: libere la mémoire et deconnecte le client                                                 */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Satellite_Deconnecter ( void )
  { if (!Cfg_satellite.Connexion) return;
    Envoyer_reseau( Cfg_satellite.Connexion, TAG_CONNEXION, SSTAG_CLIENT_OFF, NULL, 0 );
    Satellite_Deconnecter_sale();
  }
/**********************************************************************************************************/
/* Satellite_Connect: Tentative de connexion à l'instance maitre                                          */
/* Entrée: rien, tout est dans les parametre de configuration                                             */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 gboolean Satellite_Connecter ( void )
  { struct addrinfo *result, *rp;
    struct addrinfo hints;
    gint s;
    gchar service[10];
    int connexion;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    g_snprintf( service, sizeof(service), "%d", Cfg_satellite.master_port );
    s = getaddrinfo( Cfg_satellite.master_host, service, &hints, &result);
    if (s != 0)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING, 
                 "Satellite_Connect: DNS failed %s(%s)", Cfg_satellite.master_host, gai_strerror(s) );
       return(FALSE);
     }

   /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next)
     {
        connexion = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (connexion == -1)
         { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING, 
                    "Satellite_Connect: Socket creation failed" );
           continue;
         }

       if (connect(connexion, rp->ai_addr, rp->ai_addrlen) != -1)
        { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO,
                   "Satellite_Connect: Connect OK to %s (%s) family=%d",
                    Cfg_satellite.master_host, service, rp->ai_family );
          break;                  /* Success */
        }
       else
        { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING, 
                    "Satellite_Connect: connexion refused by server %s (%s) family=%d",
                    Cfg_satellite.master_host, service, rp->ai_family );
        }
       close(connexion);
     }
    freeaddrinfo(result);
    if (rp == NULL) return(FALSE);                                                 /* Erreur de connexion */

    Cfg_satellite.Connexion = Nouvelle_connexion( Config.log, connexion, -1 );/* Creation de la structure */
    if (!Cfg_satellite.Connexion)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR, 
                 "Satellite_Connect: cannot create new connexion" );
       Satellite_Deconnecter_sale();
       return(FALSE);       
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Satellite_Init_SSL: Initialisation de l'environnement SSL                                              */
/* Entrée: rien                                                                                           */
/* Sortie: un contexte SSL                                                                                */
/**********************************************************************************************************/
 static SSL_CTX *Satellite_Init_ssl ( void )
  { SSL_CTX *ssl_ctx;
    X509 *certif;
    gint retour;
    FILE *fd;

    SSL_load_error_strings();                                                    /* Initialisation de SSL */
    SSL_library_init();                                             /* Init SSL et PRNG: number générator */

    ssl_ctx = SSL_CTX_new ( TLSv1_client_method() );                        /* Création d'un contexte SSL */
    if (!ssl_ctx)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR, 
                "Satellite_Init_ssl: Error creation SSL_CTX_new %s", ERR_error_string( ERR_get_error(), NULL ) );
       return( NULL );
     }

    SSL_CTX_set_mode( ssl_ctx, SSL_MODE_AUTO_RETRY );                                /* Mode non bloquant */       
    SSL_CTX_set_options( ssl_ctx, SSL_OP_SINGLE_DH_USE );                    /* Options externe à OpenSSL */

    retour = SSL_CTX_load_verify_locations( ssl_ctx, Cfg_satellite.ssl_file_ca, NULL );
    if (retour != 1)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                "Satellite_Init_ssl: load verify locations error (%s, file %s)",
                 ERR_error_string( ERR_get_error(), NULL ), Cfg_satellite.ssl_file_ca );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    SSL_CTX_set_verify( ssl_ctx, SSL_VERIFY_PEER, NULL );         /* Type de verification des certificats */

    fd = fopen( Cfg_satellite.ssl_file_cert, "r" );
    if (!fd)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                "Satellite_Init_ssl: failed to open file certif %s.", Cfg_satellite.ssl_file_cert );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    certif = PEM_read_X509( fd, NULL, NULL, NULL );                              /* Lecture du certificat */
    fclose(fd);
    if (!certif)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                "Satellite_Init_ssl: Certif loading failed %s", Cfg_satellite.ssl_file_cert );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    retour = SSL_CTX_use_certificate( ssl_ctx, certif );
    if (retour != 1)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO,
                "Satellite_Init_ssl: Use certificate error (%s)", ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO,
             "Satellite_Init_ssl: Use of certificate %s", Nom_certif(certif) );
                                                                                 /* Clef privée du client */
    retour = SSL_CTX_use_RSAPrivateKey_file( ssl_ctx, Cfg_satellite.ssl_file_key, SSL_FILETYPE_PEM );
    if (retour != 1)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                "Satellite_Init_ssl: Error Use RSAPrivate key %s (%s)",
                 Cfg_satellite.ssl_file_key, ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }

    retour = SSL_CTX_check_private_key( ssl_ctx );                         /* Verification du certif/clef */
    if (retour != 1)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                 "Satellite_Init_ssl: check private key failed %s (%s)",
                 Cfg_satellite.ssl_file_key, ERR_error_string( ERR_get_error(), NULL ) );
       SSL_CTX_free(ssl_ctx);
       return(NULL);
     }
    Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO,
             "Satellite_Init_ssl: SSL initialisation ok" );
    return(ssl_ctx);
  }  

/**********************************************************************************************************/
/* Connecter_ssl: Tentative de connexion sécurisée au serveur                                             */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 gboolean Satellite_Connecter_ssl ( void )
  { gint retour;

    Cfg_satellite.Ssl_ctx = Satellite_Init_ssl();                             /* Creation du contexte SSL */
    if (!Cfg_satellite.Ssl_ctx)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                 "Connecter_ssl : Can't initialise SSL" );
       return(FALSE);
     }

    Cfg_satellite.Connexion->ssl = SSL_new( Cfg_satellite.Ssl_ctx );         /* Instanciation du contexte */
    if (Cfg_satellite.Connexion->ssl)                                         /* Si réussite d'allocation */
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO, "Connecter_ssl: SSL_new OK" );
       SSL_set_fd( Cfg_satellite.Connexion->ssl, Cfg_satellite.Connexion->socket );
       SSL_set_connect_state( Cfg_satellite.Connexion->ssl );                    /* Nous sommes un client */
     }
    else
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO, "Connecter_ssl: SSL_new failed" );
       Satellite_Deconnecter();
       return(FALSE);
     }

encore:
    retour = SSL_connect( Cfg_satellite.Connexion->ssl );
    if (retour<=0)
     { retour = SSL_get_error( Cfg_satellite.Connexion->ssl, retour );
       if (retour == SSL_ERROR_WANT_READ || retour == SSL_ERROR_WANT_WRITE)
        { /*sleep(1);*/
          goto encore;
        }
       
       Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                 "Connecter_ssl: SSL_connect get error %d (%s)",
                 retour, ERR_error_string( ERR_get_error(), NULL ) );
     }
                          /* Ici, la connexion a été effectuée, il faut maintenant tester les certificats */
    Cfg_satellite.master_certif = SSL_get_peer_certificate( Cfg_satellite.Connexion->ssl );
    if (!Cfg_satellite.master_certif)
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING, "Connecter_ssl: no certificate received" );
       Satellite_Deconnecter();
       return(FALSE);
     }
    Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE,
              "Connecter_ssl: certificate received. Algo=%s, keylength=%d",
              (gchar *) SSL_get_cipher_name( Cfg_satellite.Connexion->ssl ),
                        SSL_get_cipher_bits( Cfg_satellite.Connexion->ssl, NULL ) );

    retour = SSL_get_verify_result( Cfg_satellite.Connexion->ssl );         /* Verification du certificat */
    if ( retour != X509_V_OK )                                      /* Si erreur, on se deconnecte presto */
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE, "Connecter_ssl: unauthorized certificate" );
       Satellite_Deconnecter();
       return(FALSE);
     }

    Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE,
              "Connecter_ssl: partenaire %s, signataire %s",
              Nom_certif ( Cfg_satellite.master_certif ),
              Nom_certif_signataire ( Cfg_satellite.master_certif ) );
    Cfg_satellite.Mode = SAT_WAIT_FOR_CONNECTED;
    Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE,
             "Satellite_Connecter_ssl: Satellite WAITING for 'CONNECTED'" );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/

