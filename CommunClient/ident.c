/**********************************************************************************************************/
/* CommunClient/ident.c        Gestion du logon user sur module Client Watchdog                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 02 mai 2010 20:14:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ident.c
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

 #include <sys/types.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <netdb.h>
 #include <fcntl.h>
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocommunclient.h"
 #include "sysconfig.h"

/**********************************************************************************************************/
/* Envoi_serveur: Envoi d'un paquet au serveur                                                            */
/* Entrée: des infos sur le paquet à envoyer                                                              */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 gboolean Envoi_serveur_reel ( struct CLIENT *client, gint tag, gint ss_tag, gchar *buffer, gint taille )
  { if ( Envoyer_reseau( client->config.log, client->connexion, W_SERVEUR, tag, ss_tag, buffer, taille ) )
     { Info( client->config.log, DEBUG_CONNEXION, "Deconnexion sur erreur envoi au serveur" );
       return(FALSE);
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                           */
/* Entrée: une nom et un password                                                                         */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                          */
/**********************************************************************************************************/
 gint Connecter_au_serveur ( struct CLIENT *client )
  { struct sockaddr_in src;                                            /* Données locales: pas le serveur */
    struct hostent *host;
    int connexion;

    if ( !(host = gethostbyname( client->serveur )) )                             /* On veut l'adresse IP */
     { Info_c( client->config.log, DEBUG_CONNEXION,
               _("Connecter_au_serveur: DNS failed"), client->serveur );
       return(-1);
     }

    src.sin_family = host->h_addrtype;
    memcpy( (char*)&src.sin_addr, host->h_addr, host->h_length );                 /* On recopie les infos */
    src.sin_port = htons( client->config.port );

    if ( (connexion = socket( AF_INET, SOCK_STREAM, 0)) == -1)                          /* Protocol = TCP */
     { Info( client->config.log, DEBUG_CONNEXION, _("Connecter_au_serveur: Socket creation failed") );
       return(-2);
     }

    if (connect (connexion, (struct sockaddr *)&src, sizeof(src)) == -1)
     { Info_c( client->config.log, DEBUG_CONNEXION, _("Connecter_au_serveur: connexion refused by server"),
               client->config.serveur );
       close(connexion);
       return(-3);
     }

    client->connexion = Nouvelle_connexion( client->config.log, connexion,
                                            W_CLIENT, client->config.taille_bloc_reseau );
    if (!client->connexion)
     { Info( client->config.log, DEBUG_CONNEXION, _("Connecter_au_serveur: cannot create new connexion") );
       return(-4);       
     }

    client->mode = ATTENTE_CONNEXION_SSL;
    Info( client->config.log, DEBUG_CONNEXION, _("Connecter_au_serveur: client en mode ATTENTE_CONNEXION_SSL") );

    if ( ! Connecter_ssl(client) ) return(-5);                                       /* Gere les parametres SSL */
    Envoyer_identification( client);                                 /* Envoi l'identification au serveur */

    return(0);
  }
/**********************************************************************************************************/
/* Envoyer_identification: envoi de l'identification cliente au serveur                                   */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Envoyer_identification ( struct CLIENT *client )
  { struct REZO_CLI_IDENT ident;

    memcpy( &ident.nom, client->user, sizeof(ident.nom) );
    memcpy( &ident.password, client->password, sizeof(ident.password) );
    snprintf( ident.version, sizeof(ident.version), "%s", VERSION );
    
    ident.version_d = Lire_version_donnees( client->config.log );

    if ( !Envoi_serveur_reel( client, TAG_CONNEXION, SSTAG_CLIENT_IDENT,
                              (gchar *)&ident, sizeof(struct REZO_CLI_IDENT) ) )
     { return; }

    client->mode = ATTENTE_AUTORISATION;
    Info_c( client->config.log, DEBUG_CONNEXION, "client en mode ATTENTE_AUTORISATION version", ident.version );
  }
/*--------------------------------------------------------------------------------------------------------*/
