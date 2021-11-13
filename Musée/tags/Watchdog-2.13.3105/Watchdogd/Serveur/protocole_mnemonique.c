/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_mnemonique.c    Gestion du protocole_mnemonique pour Watchdog              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 avr 2009 11:13:22 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_mnemonique.c
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

 #include <glib.h>
/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_mnemonique( struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util, GID_DLS) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_MNEMONIQUE:
             { Envoi_client( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_CREATE_PAGE_MNEMO_OK, NULL, 0 );
               Ref_client( client, "Send Mnemonique" );
               pthread_create( &tid, NULL, (void *)Envoyer_mnemoniques_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_EDIT_MNEMONIQUE:
             { struct CMD_TYPE_MNEMO_BASE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMO_BASE *)connexion->donnees;
               Proto_editer_mnemonique( client, mnemo );
             }
            break;
       case SSTAG_CLIENT_ADD_MNEMONIQUE:
             { struct CMD_TYPE_MNEMO_FULL *mnemo;
               mnemo = (struct CMD_TYPE_MNEMO_FULL *)connexion->donnees;
               Proto_ajouter_mnemonique( client, mnemo );
             }
            break;
       case SSTAG_CLIENT_DEL_MNEMONIQUE:
             { struct CMD_TYPE_MNEMO_BASE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMO_BASE *)connexion->donnees;
               Proto_effacer_mnemonique( client, mnemo );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_MNEMONIQUE:
             { struct CMD_TYPE_MNEMO_FULL *mnemo;
               mnemo = (struct CMD_TYPE_MNEMO_FULL *)connexion->donnees;
               Proto_valider_editer_mnemonique( client, mnemo );
             }
            break;
       case SSTAG_CLIENT_WANT_DLS_FOR_MNEMO:
             { Ref_client( client, "Send DLS for mnemo" );
               pthread_create( &tid, NULL, (void *)Envoyer_plugins_dls_pour_mnemo_thread, client );
               pthread_detach( tid );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
