/******************************************************************************************************************************/
/* Client/protocole.c    Gestion du protocole pour la connexion au serveur Watchdog                                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           sam 16 fév 2008 19:19:36 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * protocole.c
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

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Satellite.h"

/******************************************** Définitions des prototypes programme ********************************************/

/******************************************************************************************************************************/
/* Envoi_serveur: Envoi d'un paquet au serveur                                                                                */
/* Entrée: des infos sur le paquet à envoyer                                                                                  */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Satellite_Envoi_serveur ( gint tag, gint ss_tag, gchar *buffer, gint taille )
  { if ( Envoyer_reseau( Cfg_satellite.Connexion, tag, ss_tag, buffer, taille ) )
     { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                "Satellite_Envoi_serveur: Deconnexion sur erreur envoi au serveur" );
       Satellite_Deconnecter_sale();
     }
  }
/******************************************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                                                 */
/* Entrée: la connexion avec le serveur                                                                                       */
/* Sortie: Kedal                                                                                                              */
/******************************************************************************************************************************/
 static void Satellite_Gerer_protocole ( struct CONNEXION *connexion )
  { 
    switch ( Reseau_tag(connexion) )
     { case TAG_INTERNAL    : if ( Reseau_ss_tag(connexion) == SSTAG_INTERNAL_SSLNEEDED ) 
                               { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO, 
                                          "Satellite_Gerer_protocole: SSL Needed received");
                               }
                              else if ( Reseau_ss_tag(connexion) == SSTAG_INTERNAL_SSLNEEDED_WITH_CERT ) 
                               { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO, 
                                          "Satellite_Gerer_protocole: SSL Needed_with_cert received");
                               }
                              else if (Reseau_ss_tag(connexion) == SSTAG_INTERNAL_END)               /* Fin echange interne ? */
                               { Cfg_satellite.Mode = SAT_ATTENTE_CONNEXION_SSL;
                                 Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_INFO, 
                                          "Satellite_Gerer_protocole: Satellite en mode ATTENTE_CONNEXION_SSL");
                               }
                              return;
      case TAG_GTK_MESSAGE :
            { struct CMD_GTK_MESSAGE *gtkmessage;
              gint type;
              gtkmessage = (struct CMD_GTK_MESSAGE *)connexion->donnees;
              switch (Reseau_ss_tag(connexion))
               { default:
                 case SSTAG_SERVEUR_INFO    : type = LOG_INFO;    break;
                 case SSTAG_SERVEUR_WARNING : type = LOG_WARNING; break;
                 case SSTAG_SERVEUR_ERREUR  : type = LOG_ERR;     break;
               }
              Info_new( Config.log, Cfg_satellite.lib->Thread_debug, type, 
                       "Satellite_Gerer_protocole: GTKMESSAGE : %s", gtkmessage->message );
              break;
            }
      case TAG_SATELLITE:
            { struct CMD_TYPE_MSRV_EVENT *event;
              if ( Reseau_ss_tag(connexion) != SSTAG_SSRV_SAT_SET_INTERNAL ) break;            /* Le SSRV a envoyé un event ? */
              event = (struct CMD_TYPE_MSRV_EVENT *)connexion->donnees;
              Send_Event ( event->instance, event->thread, event->type, event->objet, event->val_float );
              break;
            }
      case TAG_CONNEXION   : Satellite_Gerer_protocole_connexion    ( connexion ); break;
      default : Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR, 
                          "Satellite_Gerer_protocole : protocole inconnu %d", Reseau_tag(connexion) );
    }
  }
/******************************************************************************************************************************/
/* Ecouter_serveur: Gestion des messages de controle du serveur                                                               */
/* Entrées: data, source, type    inutilisé                                                                                   */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Satellite_Ecouter_maitre ( void )
  { gint recu;

    do
     { recu = Recevoir_reseau( Cfg_satellite.Connexion );
       if (recu==RECU_OK)
        { Satellite_Gerer_protocole( Cfg_satellite.Connexion ); }
     } while ( recu == RECU_OK );

    if (recu>=RECU_ERREUR)                                                                      /* Erreur reseau->deconnexion */
     { printf("Recu erreur %d\n", recu);
       switch( recu )
        { case RECU_ERREUR_CONNRESET: Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                                               "Satellite_Ecouter_maitre: Reset connexion" );
                                      break;
        }
       Satellite_Deconnecter();
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
