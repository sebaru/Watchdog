/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_icone.c    Gestion du protocole_icone pour Watchdog                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 avr 2009 11:12:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_icone.c
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
 void Gerer_protocole_icone( struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_level_util( client->util, ACCESS_LEVEL_ICONE) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_CLASSE:
             { Envoi_client( client, TAG_SYNOPTIQUE, SSTAG_SERVEUR_CREATE_PAGE_ICONE_OK, NULL, 0 );
               Ref_client( client, "Send Classe" );
               pthread_create( &tid, NULL, (void *)Envoyer_classes_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_EDIT_CLASSE: 
             { struct CMD_TYPE_CLASSE *classe;
               classe = (struct CMD_TYPE_CLASSE *)connexion->donnees;
               Proto_editer_classe( client, classe );
             }
            break;
       case SSTAG_CLIENT_ADD_CLASSE:
             { struct CMD_TYPE_CLASSE *classe;
               classe = (struct CMD_TYPE_CLASSE *)connexion->donnees;
               Proto_ajouter_classe( client, classe );
             }
            break;
       case SSTAG_CLIENT_DEL_CLASSE:
             { struct CMD_TYPE_CLASSE *classe;
               classe = (struct CMD_TYPE_CLASSE *)connexion->donnees;
               Proto_effacer_classe( client, classe );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_CLASSE:
             { struct CMD_TYPE_CLASSE *classe;
               classe = (struct CMD_TYPE_CLASSE *)connexion->donnees;
               Proto_valider_editer_classe( client, classe );
             }
            break;
/********************************* Client en VALIDE, gestion des icones ***********************************/
       case SSTAG_CLIENT_WANT_PAGE_ICONE:
            { client->classe_icone = ((struct CMD_TYPE_CLASSE *)connexion->donnees)->id;
              Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_CREATE_PAGE_ICONE_OK, NULL, 0 );
              Ref_client( client, "Send Icone" );
              pthread_create( &tid, NULL, (void *)Envoyer_icones_thread, client );
              pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_EDIT_ICONE:
             { struct CMD_TYPE_ICONE *icone;
               icone = (struct CMD_TYPE_ICONE *)connexion->donnees;
               Proto_editer_icone( client, icone );
             }
            break;
       case SSTAG_CLIENT_ADD_ICONE:
             { struct CMD_TYPE_ICONE *icone;
               icone = (struct CMD_TYPE_ICONE *)connexion->donnees;
               Proto_ajouter_icone( client, icone );
             }
            break;
       case SSTAG_CLIENT_ADD_ICONE_DEB_FILE:
             { struct CMD_TYPE_ICONE *icone;
               icone = (struct CMD_TYPE_ICONE *)connexion->donnees;
               Proto_ajouter_icone_deb_file( client, icone );
             }
            break;
       case SSTAG_CLIENT_ADD_ICONE_FILE:
             { struct CMD_TYPE_ICONE *icone;
               icone = (struct CMD_TYPE_ICONE *)connexion->donnees;
               Proto_ajouter_icone_file( client, icone,
                                         connexion->entete.taille_donnees - sizeof(struct CMD_TYPE_ICONE),
                                         connexion->donnees + sizeof(struct CMD_TYPE_ICONE) );
             }
            break;
       case SSTAG_CLIENT_ADD_ICONE_FIN_FILE:
             { struct CMD_TYPE_ICONE *icone;
               icone = (struct CMD_TYPE_ICONE *)connexion->donnees;
               Proto_ajouter_icone_fin_file( client, icone );
             }
            break;
       case SSTAG_CLIENT_DEL_ICONE:
             { struct CMD_TYPE_ICONE *icone;
               icone = (struct CMD_TYPE_ICONE *)connexion->donnees;
               Proto_effacer_icone( client, icone );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_ICONE:
             { struct CMD_TYPE_ICONE *icone;
               icone = (struct CMD_TYPE_ICONE *)connexion->donnees;
               Proto_valider_editer_icone( client, icone );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
