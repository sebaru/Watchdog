/**********************************************************************************************************/
/* Client/protocole.c    Gestion du protocole pour la connexion au serveur Watchdog                       */
/* Projet WatchDog version 3.0       Gestion d'habitat                       sam 16 fév 2008 19:19:36 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sébastien Lefevre
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
 #include <openssl/err.h>
 #include <stdio.h>

 #include <unistd.h>
 #include <libgen.h>                                                                      /* Pour dirname */

 #include "Erreur.h"
 #include "Config_cli.h"
 #include "Reseaux.h"
 #include "client.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *Barre_status;                                         /* Barre d'etat de l'application */
 extern struct CLIENT Client;                                    /* Identifiant de l'utilisateur en cours */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 static void Gerer_protocole ( struct CONNEXION *connexion )
  {
    switch ( Reseau_tag(connexion) )
     { case TAG_GTK_MESSAGE : Gerer_protocole_gtk_message ( connexion ); return;
       case TAG_INTERNAL    : if ( Reseau_ss_tag(connexion) == SSTAG_INTERNAL_SSLNEEDED )
                               { Client.ssl_needed = TRUE;
                                 Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
                                         _("Gerer_protocole: SSL Needed received") );
                               }
                              else if ( Reseau_ss_tag(connexion) == SSTAG_INTERNAL_SSLNEEDED_WITH_CERT )
                               { Client.ssl_needed_with_cert = TRUE;
                                 Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
                                         _("Gerer_protocole: SSL Needed_with_cert received") );
                               }
                              else if (Reseau_ss_tag(connexion) == SSTAG_INTERNAL_END)/* Fin echange interne ? */
                               { if (Client.ssl_needed)
                                  { Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
                                           _("Gerer_protocole: Start SSL connexion") );
                                    if ( ! Connecter_ssl() )                   /* Gere les parametres SSL */
                                     { Deconnecter();
                                       return;
                                     }
                                  }
                                 Envoyer_authentification();         /* Envoi l'identification au serveur */
                               }
                              return;
      case TAG_MNEMONIQUE  : Gerer_protocole_mnemonique   ( connexion ); break;
      case TAG_SYNOPTIQUE  : Gerer_protocole_synoptique   ( connexion ); break;
      case TAG_SUPERVISION : Gerer_protocole_supervision  ( connexion ); break;
      case TAG_ATELIER     : Gerer_protocole_atelier      ( connexion ); break;
      case TAG_LOWLEVEL      : Gerer_protocole_lowlevel     ( connexion ); break;
      case TAG_ADMIN       : Gerer_protocole_admin        ( connexion ); break;
      case TAG_CONNEXION   : Gerer_protocole_connexion    ( connexion ); break;
      default : printf("Gerer_protocole : protocole inconnu %d\n", Reseau_tag(connexion) );
    }
  }
/**********************************************************************************************************/
/* Ecouter_serveur: Gestion des messages de controle du serveur                                           */
/* Entrées: data, source, type    inutilisé                                                               */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Ecouter_serveur ( void )
  { gint recu;

    do
     { recu = Recevoir_reseau( Client.connexion );
       if (recu==RECU_OK)
        { Gerer_protocole( Client.connexion ); }
     } while ( recu == RECU_OK );

    if (recu>=RECU_ERREUR)                                             /* Erreur reseau->deconnexion */
     { printf("Recu erreur %d\n", recu);
       switch( recu )
        { case RECU_ERREUR_CONNRESET: Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING,
                                            "Ecouter_serveur: Reset connexion" );
                                      break;
        }
       Deconnecter();
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
