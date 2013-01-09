/**********************************************************************************************************/
/* Client/protocole.c    Gestion du protocole pour la connexion au serveur Watchdog                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 16 fév 2008 19:19:36 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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
 extern struct CLIENT Client_en_cours;                           /* Identifiant de l'utilisateur en cours */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

 extern struct CONNEXION *Connexion;                                              /* connexion au serveur */
/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 static void Gerer_protocole ( struct CONNEXION *connexion )
  { GtkWidget *dialog;

    switch ( Reseau_tag(connexion) )
     { case TAG_GTK_MESSAGE : Gerer_protocole_gtk_message ( connexion ); return;
       case TAG_INTERNAL    : if ( Reseau_ss_tag(connexion) == SSTAG_INTERNAL_SSLNEEDED ) 
                               { Client_en_cours.mode = ATTENTE_CONNEXION_SSL;
                                 Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO, 
                                        _("Gerer_protocole: client en mode ATTENTE_CONNEXION_SSL") );
                                 if ( ! Connecter_ssl() )                      /* Gere les parametres SSL */
                                  { Deconnecter();
                                    Log( "SSL connexion failed..." );
                                  }
                               }
                             else if (Reseau_ss_tag(connexion) == SSTAG_INTERNAL_END)/* Fin echange interne ? */
                               { Client_en_cours.mode = ENVOI_IDENT;
                                 Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO, 
                                        _("Gerer_protocole: client en mode ENVOI_IDENT") );
                                 Envoyer_identification();           /* Envoi l'identification au serveur */
                               }
                              break;
       case TAG_CONNEXION: 
            switch ( Reseau_ss_tag ( connexion ) )
             { case SSTAG_SERVEUR_OFF:
                    { printf("Recu SSTAG_SERVEUR_OFF\n");
                      dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                        GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
                                                        _("Server is DOWN") );
                      g_signal_connect_swapped( dialog, "response",
                                                        G_CALLBACK(gtk_widget_destroy), dialog );
                      gtk_widget_show_all(dialog);
                      Deconnecter();
                      Log ( _("Disconnected by server shutdown") );
                      break;
                    }
               default : /*printf("recu SSTAG %d\n", Reseau_ss_tag ( connexion ) );*/
                         break;
             }
            break;
     }
/*************************************** Client en mode VALIDE ********************************************/
    if ( Client_en_cours.mode >= VALIDE )                                       /* Le client valide  */
     {
       switch ( Reseau_tag(connexion) )
        { case TAG_ICONE       : Gerer_protocole_icone        ( connexion ); break;
          case TAG_DLS         : Gerer_protocole_dls          ( connexion ); break;
          case TAG_UTILISATEUR : Gerer_protocole_utilisateur  ( connexion ); break;
          case TAG_MESSAGE     : Gerer_protocole_message      ( connexion ); break;
          case TAG_MNEMONIQUE  : Gerer_protocole_mnemonique   ( connexion ); break;
          case TAG_SYNOPTIQUE  : Gerer_protocole_synoptique   ( connexion ); break;
          case TAG_SUPERVISION : Gerer_protocole_supervision  ( connexion ); break;
          case TAG_HISTO       : Gerer_protocole_histo        ( connexion ); break;
          case TAG_ATELIER     : Gerer_protocole_atelier      ( connexion ); break;
          case TAG_COURBE      : Gerer_protocole_courbe       ( connexion ); break;
          case TAG_HISTO_COURBE: Gerer_protocole_histo_courbe ( connexion ); break;
          case TAG_SCENARIO    : Gerer_protocole_scenario     ( connexion ); break;
          case TAG_CAMERA      : Gerer_protocole_camera       ( connexion ); break;
          case TAG_ADMIN       : Gerer_protocole_admin        ( connexion ); break;
          case TAG_CONNEXION   : if ( Reseau_ss_tag( connexion ) == SSTAG_SERVEUR_PULSE ) 
                                  { Set_progress_pulse(); }
                                 break;
          default : printf("Gerer_protocole : protocole inconnu %d\n", Reseau_tag(connexion) );
        }
     }
    else if ( Client_en_cours.mode == CONNECTE )
     {
       switch ( Reseau_tag(connexion) )
        { case TAG_HISTO    : Gerer_protocole_histo_connecte ( connexion ); break;
          case TAG_CONNEXION: if (Reseau_ss_tag(connexion) == SSTAG_SERVEUR_CLI_VALIDE)
                               { Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
                                           "Gerer_protocole : Client en mode VALIDE" );
                                 Client_en_cours.mode = VALIDE;
                               }
                              break;
          case TAG_FICHIER  : Gerer_protocole_fichier_connecte ( connexion ); break;
        }

     }
/*************************************** Client en attente d'etre connecte ********************************/
    else if ( Client_en_cours.mode == ATTENTE_AUTORISATION )
     { Gerer_protocole_connexion( connexion ); }
  }
/**********************************************************************************************************/
/* Ecouter_serveur: Gestion des messages de controle du serveur                                           */
/* Entrées: data, source, type    inutilisé                                                               */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Ecouter_serveur ( void )
  { gint recu;

    do
     { recu = Recevoir_reseau( Connexion );
       if (recu==RECU_OK)
        { Gerer_protocole( Connexion ); }
     } while ( recu == RECU_EN_COURS || recu == RECU_OK );

    if (recu>=RECU_ERREUR)                                             /* Erreur reseau->deconnexion */
     { printf("Recu erreur\n");
       switch( recu )
        { case RECU_ERREUR_CONNRESET: Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING,
                                            "Ecouter_serveur: Reset connexion" );
                                      break;
        }
       Deconnecter();
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
