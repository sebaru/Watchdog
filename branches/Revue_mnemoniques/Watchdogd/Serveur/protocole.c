/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole.c    Gestion du protocole pour la connexion au client Watchdog             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 25 jan 2004 17:35:52 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole.c
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
 
 #include <stdio.h>
 #include <openssl/err.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <libgen.h>                                                                      /* Pour dirname */
 #include <pthread.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entr�e: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 static void Gerer_protocole ( struct CLIENT *client )
  { struct CONNEXION *connexion;
    connexion = client->connexion;

    if ( Reseau_tag(connexion) == TAG_CONNEXION && Reseau_ss_tag(connexion) == SSTAG_CLIENT_OFF )
          { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                     "Gerer_protocol: Deconnexion sur demande cliente" );
            Client_mode ( client, DECONNECTE );
          }
/********************************************* Client VALIDE **********************************************/
    else if (client->mode == VALIDE )
          { 
/********************************* Client en VALIDE, gestion des groupes **********************************/
            switch ( Reseau_tag(connexion) )
             { case TAG_ICONE       : Gerer_protocole_icone        ( client ); break;
               case TAG_DLS         : Gerer_protocole_dls          ( client ); break;
               case TAG_UTILISATEUR : Gerer_protocole_utilisateur  ( client ); break;
               case TAG_MESSAGE     : Gerer_protocole_message      ( client ); break;
               case TAG_MNEMONIQUE  : Gerer_protocole_mnemonique   ( client ); break;
               case TAG_SYNOPTIQUE  : Gerer_protocole_synoptique   ( client ); break;
               case TAG_SUPERVISION : Gerer_protocole_supervision  ( client ); break;
               case TAG_HISTO       : Gerer_protocole_histo        ( client ); break;
               case TAG_ATELIER     : Gerer_protocole_atelier      ( client ); break;
               case TAG_COURBE      : Gerer_protocole_courbe       ( client ); break;
               case TAG_HISTO_COURBE: Gerer_protocole_histo_courbe ( client ); break;
               case TAG_CAMERA      : Gerer_protocole_camera       ( client ); break;
               case TAG_ADMIN       : Gerer_protocole_admin        ( client ); break;
               case TAG_SATELLITE   : Gerer_protocole_satellite    ( client ); break;
               case TAG_CONNEXION   : if (Reseau_ss_tag(connexion) == SSTAG_CLIENT_SETPASSWORD )
                                       { struct CMD_TYPE_UTILISATEUR *util;
                                         util = (struct CMD_TYPE_UTILISATEUR *)connexion->donnees;
                                         Proto_set_password( client, util );
                                       }
             }
          }
/************************************** Client en attente identification **********************************/
    else if ( client->mode == WAIT_FOR_IDENT && Reseau_tag(connexion)    == TAG_CONNEXION
                                             && Reseau_ss_tag(connexion) == SSTAG_CLIENT_IDENT )
          { struct REZO_CLI_IDENT *ident;

            ident = (struct REZO_CLI_IDENT *)connexion->donnees;
            if (Tester_autorisation ( client, ident ) == TRUE)               /* Test l'authent cliente (login/code ou certif) */
             { pthread_t tid;
               Ref_client( client, "Send Histo" );
               pthread_create( &tid, NULL, (void *)Envoyer_histo_thread, client );
               pthread_detach( tid );
             }
          }
/************************************** Client en attente nouveau password ********************************/
    else if ( client->mode == WAIT_FOR_NEWPWD && Reseau_tag(connexion)    == TAG_CONNEXION
                                              && Reseau_ss_tag(connexion) == SSTAG_CLIENT_SETPASSWORD )
          { struct CMD_TYPE_UTILISATEUR *util;
            util = (struct CMD_TYPE_UTILISATEUR *)connexion->donnees;
            Proto_set_password( client, util );
          }
  }
/**********************************************************************************************************/
/* Ecouter_serveur: Gestion des messages de controle du serveur                                           */
/* Entr�es: data, source, type    inutilis�                                                               */
/* Sortie: N�ant                                                                                          */
/**********************************************************************************************************/
 void Ecouter_client ( struct CLIENT *client )
  { gint recu;

    recu = Recevoir_reseau( client->connexion );
    if (recu==RECU_OK)
     { /*switch( client->connexion->entete.destinataire )
        { case W_SERVEUR: */
       Gerer_protocole( client );
        /* break;
          default: printf("Ecouter_client: destinataire inconnu\n");
        }*/
     }
    else if (recu>=RECU_ERREUR)                                             /* Erreur reseau->deconnexion */
     { switch( recu )
        { case RECU_ERREUR_CONNRESET: Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                                               "Ecouter_client: Reset connexion" );
                                      break;
        }
       Client_mode ( client, DECONNECTE );
     }             
  }
/*--------------------------------------------------------------------------------------------------------*/
