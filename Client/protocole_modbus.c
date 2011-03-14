/**********************************************************************************************************/
/* Client/protocole_modbus.c    Gestion du protocole_modbus pour la connexion au serveur Watchdog         */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 05 sept. 2010 16:46:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_modbus.c
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
 void Gerer_protocole_modbus ( struct CONNEXION *connexion )
  { static GList *Arrivee_modbus = NULL;
    static GList *Arrivee_bornes = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_MODBUS_OK:
             { if (!Tester_page_notebook(TYPE_PAGE_MODBUS)) { Creer_page_modbus(); }
             }
            break;
       case SSTAG_SERVEUR_ADD_MODBUS_OK:
             { struct CMD_TYPE_MODBUS *modbus;
               modbus = (struct CMD_TYPE_MODBUS *)connexion->donnees;
               Proto_afficher_un_modbus( modbus );
             }
            break;
       case SSTAG_SERVEUR_DEL_MODBUS_OK:
             { struct CMD_TYPE_MODBUS *modbus;
               modbus = (struct CMD_TYPE_MODBUS *)connexion->donnees;
               Proto_cacher_un_modbus( modbus );
             }
            break;
       case SSTAG_SERVEUR_EDIT_MODBUS_OK:
             { struct CMD_TYPE_MODBUS *modbus;
               modbus = (struct CMD_TYPE_MODBUS *)connexion->donnees;
               Menu_ajouter_editer_modbus( modbus );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_MODBUS_OK:
             { struct CMD_TYPE_MODBUS *modbus;
               modbus = (struct CMD_TYPE_MODBUS *)connexion->donnees;
               Proto_rafraichir_un_modbus( modbus );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MODBUS:
             { struct CMD_TYPE_MODBUS *modbus;
               Set_progress_plusun();

               modbus = (struct CMD_TYPE_MODBUS *)g_malloc0( sizeof( struct CMD_TYPE_MODBUS ) );
               if (!modbus) return; 

               memcpy( modbus, connexion->donnees, sizeof(struct CMD_TYPE_MODBUS ) );
               Arrivee_modbus = g_list_append( Arrivee_modbus, modbus );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MODBUS_FIN:
             { 
               g_list_foreach( Arrivee_modbus, (GFunc)Proto_afficher_un_modbus, NULL );
               g_list_foreach( Arrivee_modbus, (GFunc)g_free, NULL );
               g_list_free( Arrivee_modbus );
               Arrivee_modbus = NULL;
               Chercher_page_notebook( TYPE_PAGE_MODBUS, 0, TRUE );
             }
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_MODBUS:
             { struct CMD_TYPE_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               Proto_afficher_mnemo_modbus( mnemo );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
