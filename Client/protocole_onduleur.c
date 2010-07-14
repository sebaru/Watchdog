/**********************************************************************************************************/
/* Client/protocole_onduleur.c    Gestion du protocole_onduleur pour Watchdog                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 11:55:51 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_onduleur.c
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
 void Gerer_protocole_onduleur ( struct CONNEXION *connexion )
  { static GList *Arrivee_onduleur = NULL;
           
    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_EDIT_ONDULEUR_OK:
             { struct CMD_TYPE_ONDULEUR *onduleur;
               onduleur = (struct CMD_TYPE_ONDULEUR *)connexion->donnees;
               Menu_ajouter_editer_onduleur( onduleur );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_ONDULEUR_OK:
             { struct CMD_TYPE_ONDULEUR *onduleur;
               onduleur = (struct CMD_TYPE_ONDULEUR *)connexion->donnees;
               Proto_rafraichir_un_onduleur( onduleur );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ONDULEUR:
             { struct CMD_TYPE_ONDULEUR *onduleur;
               Set_progress_plusun();

               onduleur = (struct CMD_TYPE_ONDULEUR *)g_malloc0( sizeof( struct CMD_TYPE_ONDULEUR ) );
               if (!onduleur) return; 
               memcpy( onduleur, connexion->donnees, sizeof(struct CMD_TYPE_ONDULEUR ) );
               Arrivee_onduleur = g_list_append( Arrivee_onduleur, onduleur );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ONDULEUR_FIN:
             { g_list_foreach( Arrivee_onduleur, (GFunc)Proto_afficher_un_onduleur, NULL );
               g_list_foreach( Arrivee_onduleur, (GFunc)g_free, NULL );
               g_list_free( Arrivee_onduleur );
               Arrivee_onduleur = NULL;
               Chercher_page_notebook( TYPE_PAGE_ONDULEUR, 0, TRUE );
             }
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_BIT_COMM:
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_EA_UPS_LOAD:
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_EA_UPS_REAL_POWER:
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_EA_INPUT_VOLTAGE:
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_EA_BATTERY_CHARGE:
             { struct CMD_TYPE_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               Proto_afficher_mnemo_onduleur( Reseau_ss_tag ( connexion ), mnemo );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
