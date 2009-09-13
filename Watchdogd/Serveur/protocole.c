/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole.c    Gestion du protocole pour la connexion au client Watchdog             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 25 jan 2004 17:35:52 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2007 - Sébastien Lefevre
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
 
 #include <gnome.h>
 #include <stdio.h>
 #include <openssl/err.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <libgen.h>                                                                      /* Pour dirname */
 #include <pthread.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "Reseaux.h"
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 static void Gerer_protocole ( gint Id_serveur, struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( Reseau_tag(connexion) == TAG_CONNEXION && Reseau_ss_tag(connexion) == SSTAG_CLIENT_OFF )
          { Client_mode ( client, DECONNECTE ); }
/********************************************* Client VALIDE **********************************************/
    else if (client->mode == VALIDE )
          { 
/********************************* Client en VALIDE, gestion des groupes **********************************/
            switch ( Reseau_tag(connexion) )
             { case TAG_ICONE       : Gerer_protocole_icone        ( Id_serveur, client ); break;
               case TAG_DLS         : Gerer_protocole_dls          ( Id_serveur, client ); break;
               case TAG_UTILISATEUR : Gerer_protocole_utilisateur  ( Id_serveur, client ); break;
               case TAG_MESSAGE     : Gerer_protocole_message      ( Id_serveur, client ); break;
               case TAG_MNEMONIQUE  : Gerer_protocole_mnemonique   ( Id_serveur, client ); break;
               case TAG_ENTREEANA   : Gerer_protocole_entreeana    ( Id_serveur, client ); break;
               case TAG_SYNOPTIQUE  : Gerer_protocole_synoptique   ( Id_serveur, client ); break;
               case TAG_SUPERVISION : Gerer_protocole_supervision  ( Id_serveur, client ); break;
               case TAG_HISTO       : Gerer_protocole_histo        ( Id_serveur, client ); break;
               case TAG_ATELIER     : Gerer_protocole_atelier      ( Id_serveur, client ); break;
               case TAG_COURBE      : Gerer_protocole_courbe       ( Id_serveur, client ); break;
               case TAG_HISTO_COURBE: Gerer_protocole_histo_courbe ( Id_serveur, client ); break;
               case TAG_SCENARIO    : Gerer_protocole_scenario     ( Id_serveur, client ); break;
               case TAG_CAMERA      : Gerer_protocole_camera       ( Id_serveur, client ); break;
               case TAG_CONNEXION   : if (Reseau_ss_tag(connexion) == SSTAG_CLIENT_SETPASSWORD )
                                       { struct CMD_UTIL_SETPASSWORD *util;
                                         util = (struct CMD_UTIL_SETPASSWORD *)connexion->donnees;
                                         printf("Set password for %d: %s\n", util->id, util->code_en_clair );
                                         Proto_set_password( Id_serveur, client, util );
                                       }
             }
          }
/************************************** Client en attente identification **********************************/
    else if ( client->mode == ATTENTE_IDENT && Reseau_tag(connexion)    == TAG_CONNEXION
                                            && Reseau_ss_tag(connexion) == SSTAG_CLIENT_IDENT )
          { struct REZO_CLI_IDENT *ident;
            ident = (struct REZO_CLI_IDENT *)connexion->donnees;
            Info_n( Config.log, DEBUG_CONNEXION, "Identification du client", connexion->socket );
            Info_c( Config.log, DEBUG_CONNEXION, "Nom de l'utilisateur", ident->nom );
            Info_c( Config.log, DEBUG_CONNEXION, "Version du client   ", ident->version );
            Info_n( Config.log, DEBUG_CONNEXION, "Version donnees     ", ident->version_d );
            memcpy( &client->ident, ident, sizeof( struct REZO_CLI_IDENT ) );  /* Recopie pour sauvegarde */
            
            Client_mode ( client, ENVOI_AUTORISATION );
          }
/************************************** Client en attente nouveau password ********************************/
    else if ( client->mode == ATTENTE_NEW_PASSWORD && Reseau_tag(connexion)    == TAG_CONNEXION
                                                   && Reseau_ss_tag(connexion) == SSTAG_CLIENT_SETPASSWORD )
          { struct CMD_UTIL_SETPASSWORD *util;
            util = (struct CMD_UTIL_SETPASSWORD *)connexion->donnees;
            printf("Set password for %d: %s\n", util->id, util->code_en_clair );
            Proto_set_password( Id_serveur, client, util );
          }
  }
/**********************************************************************************************************/
/* Ecouter_serveur: Gestion des messages de controle du serveur                                           */
/* Entrées: data, source, type    inutilisé                                                               */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Ecouter_client ( gint Id_serveur, struct CLIENT *client )
  { gint recu;

    recu = Recevoir_reseau( Config.log, client->connexion );
    if (recu==RECU_OK)
     { /*switch( client->connexion->entete.destinataire )
        { case W_SERVEUR: */
       Gerer_protocole( Id_serveur, client );
        /* break;
          default: printf("Ecouter_client: destinataire inconnu\n");
        }*/
     }
    else if (recu>=RECU_ERREUR)                                             /* Erreur reseau->deconnexion */
     { printf("Recu erreur\n");
       switch( recu )
        { case RECU_ERREUR_CONNRESET: Info( Config.log, DEBUG_NETWORK,
                                            "Ecouter_client: Reset connexion" );
                                      break;
        }
       client->mode = DECONNECTE;
     }             
  }
/*--------------------------------------------------------------------------------------------------------*/
