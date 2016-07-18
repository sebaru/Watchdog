/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_dls.c    Gestion du protocole_dls pour Watchdog                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 avr 2009 11:19:27 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_dls.c
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
 void Gerer_protocole_dls( struct CLIENT *client )
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
     { case SSTAG_CLIENT_WANT_PAGE_DLS:
             { Envoi_client( client, TAG_DLS, SSTAG_SERVEUR_CREATE_PAGE_DLS_OK, NULL, 0 );
               Ref_client( client, "Send plugins dls" );
               pthread_create( &tid, NULL, (void *)Envoyer_plugins_dls_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_ADD_PLUGIN_DLS:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Proto_ajouter_plugin_dls( client, dls );
             }
            break;
       case SSTAG_CLIENT_EDIT_PLUGIN_DLS:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Proto_editer_plugin_dls( client, dls );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_PLUGIN_DLS:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Proto_valider_editer_plugin_dls( client, dls );
             }
            break;
       case SSTAG_CLIENT_DEL_PLUGIN_DLS:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Proto_effacer_plugin_dls( client, dls );
             }
            break;
       case SSTAG_CLIENT_WANT_SOURCE_DLS:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Proto_editer_source_dls( client, dls );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS_DEB:
             { struct CMD_TYPE_SOURCE_DLS *edit_dls;
               edit_dls = (struct CMD_TYPE_SOURCE_DLS *)connexion->donnees;
               Proto_effacer_fichier_plugin_dls( client, edit_dls );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS:
             { struct CMD_TYPE_SOURCE_DLS *edit_dls;
               edit_dls = (struct CMD_TYPE_SOURCE_DLS *)connexion->donnees;
               Proto_valider_source_dls( client, edit_dls,
                                         (gchar *)edit_dls + sizeof(struct CMD_TYPE_SOURCE_DLS) );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS_FIN:
             { memcpy( &client->dls, (struct CMD_TYPE_SOURCE_DLS *)connexion->donnees,
                       sizeof( client->dls ) );
               Ref_client( client, "Send Compiler D.L.S" );
               pthread_create( &tid, NULL, (void *)Proto_compiler_source_dls_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_WANT_SYN_FOR_PLUGIN_DLS:
             { Ref_client( client, "Send Synoptique pour plugin dls" );
               pthread_create( &tid, NULL, (void *)Envoyer_synoptiques_pour_plugin_dls_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_WANT_TYPE_NUM_MNEMO:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               Proto_envoyer_type_num_mnemo_tag( TAG_DLS, SSTAG_SERVEUR_TYPE_NUM_MNEMO, client, mnemo );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
