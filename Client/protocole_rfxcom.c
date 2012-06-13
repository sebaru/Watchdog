/**********************************************************************************************************/
/* Client/protocole_rfxcom.c    Gestion du protocole_rfxcom pour la connexion au serveur Watchdog         */
/* Projet WatchDog version 2.0       Gestion d'habitat                    mer. 13 juin 2012 18:05:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_rfxcom.c
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
 void Gerer_protocole_rfxcom ( struct CONNEXION *connexion )
  { static GList *Arrivee_rfxcom = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_RFXCOM_OK:
             { if (!Tester_page_notebook(TYPE_PAGE_RFXCOM)) { Creer_page_rfxcom(); }
             }
            break;
       case SSTAG_SERVEUR_ADD_RFXCOM_OK:
             { struct CMD_TYPE_RFXCOM *rfxcom;
               rfxcom = (struct CMD_TYPE_RFXCOM *)connexion->donnees;
               Proto_afficher_un_rfxcom( rfxcom );
             }
            break;
       case SSTAG_SERVEUR_DEL_RFXCOM_OK:
             { struct CMD_TYPE_RFXCOM *rfxcom;
               rfxcom = (struct CMD_TYPE_RFXCOM *)connexion->donnees;
               Proto_cacher_un_rfxcom( rfxcom );
             }
            break;
       case SSTAG_SERVEUR_EDIT_RFXCOM_OK:
             { struct CMD_TYPE_RFXCOM *rfxcom;
               rfxcom = (struct CMD_TYPE_RFXCOM *)connexion->donnees;
               Menu_ajouter_editer_rfxcom( rfxcom );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_RFXCOM_OK:
             { struct CMD_TYPE_RFXCOM *rfxcom;
               rfxcom = (struct CMD_TYPE_RFXCOM *)connexion->donnees;
               Proto_rafraichir_un_rfxcom( rfxcom );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_RFXCOM:
             { struct CMD_TYPE_RFXCOM *rfxcom;
               Set_progress_plusun();

               rfxcom = (struct CMD_TYPE_RFXCOM *)g_malloc0( sizeof( struct CMD_TYPE_RFXCOM ) );
               if (!rfxcom) return; 

               memcpy( rfxcom, connexion->donnees, sizeof(struct CMD_TYPE_RFXCOM ) );
               Arrivee_rfxcom = g_list_append( Arrivee_rfxcom, rfxcom );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_RFXCOM_FIN:
             { 
               g_list_foreach( Arrivee_rfxcom, (GFunc)Proto_afficher_un_rfxcom, NULL );
               g_list_foreach( Arrivee_rfxcom, (GFunc)g_free, NULL );
               g_list_free( Arrivee_rfxcom );
               Arrivee_rfxcom = NULL;
               Chercher_page_notebook( TYPE_PAGE_RFXCOM, 0, TRUE );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
