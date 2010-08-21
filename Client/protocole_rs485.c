/**********************************************************************************************************/
/* Client/protocole_rs485.c    Gestion du protocole_rs485 pour la connexion au serveur Watchdog           */
/* Projet WatchDog version 2.0       Gestion d'habitat                     jeu. 19 août 2010 19:54:11 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_rs485.c
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

 #include <glib.h>
 #include "Erreur.h"
 #include "Reseaux.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_rs485 ( struct CONNEXION *connexion )
  { static GList *Arrivee_rs485 = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_RS485_OK:
             { if (!Tester_page_notebook(TYPE_PAGE_RS485)) { Creer_page_rs485(); }
             }
            break;
       case SSTAG_SERVEUR_ADD_RS485_OK:
             { struct CMD_TYPE_RS485 *rs485;
               rs485 = (struct CMD_TYPE_RS485 *)connexion->donnees;
               Proto_afficher_un_rs485( rs485 );
             }
            break;
       case SSTAG_SERVEUR_DEL_RS485_OK:
             { struct CMD_TYPE_RS485 *rs485;
               rs485 = (struct CMD_TYPE_RS485 *)connexion->donnees;
               Proto_cacher_un_rs485( rs485 );
             }
            break;
       case SSTAG_SERVEUR_EDIT_RS485_OK:
             { struct CMD_TYPE_RS485 *rs485;
               rs485 = (struct CMD_TYPE_RS485 *)connexion->donnees;
               Menu_ajouter_editer_rs485( rs485 );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_RS485_OK:
             { struct CMD_TYPE_RS485 *rs485;
               rs485 = (struct CMD_TYPE_RS485 *)connexion->donnees;
               Proto_rafraichir_un_rs485( rs485 );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_RS485:
             { struct CMD_TYPE_RS485 *rs485;
               Set_progress_plusun();

               rs485 = (struct CMD_TYPE_RS485 *)g_malloc0( sizeof( struct CMD_TYPE_RS485 ) );
               if (!rs485) return; 

               memcpy( rs485, connexion->donnees, sizeof(struct CMD_TYPE_RS485 ) );
               Arrivee_rs485 = g_list_append( Arrivee_rs485, rs485 );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_RS485_FIN:
             { 
               g_list_foreach( Arrivee_rs485, (GFunc)Proto_afficher_un_rs485, NULL );
               g_list_foreach( Arrivee_rs485, (GFunc)g_free, NULL );
               g_list_free( Arrivee_rs485 );
               Arrivee_rs485 = NULL;
               Chercher_page_notebook( TYPE_PAGE_RS485, 0, TRUE );
             }
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_RS485:
             { struct CMD_TYPE_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               Proto_afficher_mnemo_rs485( mnemo );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
