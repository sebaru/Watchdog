/**********************************************************************************************************/
/* Client/protocole_histo_courbe.c    Gestion du protocole_courbe pour la connexion au serveur Watchdog   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 nov 2007 11:34:05 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_histo_courbe.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2007 - Sébastien Lefevre
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
 void Gerer_protocole_histo_courbe ( struct CONNEXION *connexion )
  { static GList *Arrivee_eana = NULL;
    static GList *Arrivee_mnemo = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_ADD_HISTO_COURBE_OK:
             { struct CMD_TYPE_COURBE *courbe;
               courbe = (struct CMD_TYPE_COURBE *)connexion->donnees;
               printf("proto ajouter courbe !!\n");
               Proto_ajouter_histo_courbe( courbe );
             }
            break;
   /*    case SSTAG_SERVEUR_INIT_COURBE:
             { struct CMD_APPEND_COURBE *courbe, *test;
               gint cpt;
               courbe = (struct CMD_APPEND_COURBE *)connexion->donnees;
               for ( cpt=0; cpt<(connexion->entete.taille_donnees/sizeof(struct CMD_APPEND_COURBE)); cpt++ )
                { test = &courbe[cpt];
                  Proto_append_courbe( courbe );
                }
             }
            break;*/
       case SSTAG_SERVEUR_START_HISTO_COURBE:
             { struct CMD_START_COURBE *courbe;
               courbe = (struct CMD_START_COURBE *)connexion->donnees;
               Proto_start_histo_courbe( courbe );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_HISTO_COURBE:
             { struct CMD_TYPE_MNEMO_AI *eana;
               Set_progress_plus(1);

               eana = (struct CMD_TYPE_MNEMO_AI *)g_try_malloc0( sizeof( struct CMD_TYPE_MNEMO_AI ) );
               if (!eana) return; 
               memcpy( eana, connexion->donnees, sizeof(struct CMD_TYPE_MNEMO_AI ) );
               Arrivee_eana = g_list_append( Arrivee_eana, eana );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_HISTO_COURBE_FIN:
             { 
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_HISTO_COURBE:
             { struct CMD_TYPE_MNEMO_BASE *mnemo;
               Set_progress_plus(1);

               mnemo = (struct CMD_TYPE_MNEMO_BASE *)g_try_malloc0( sizeof( struct CMD_TYPE_MNEMO_BASE ) );
               if (!mnemo) return; 
               memcpy( mnemo, connexion->donnees, sizeof(struct CMD_TYPE_MNEMO_BASE ) );
               Arrivee_mnemo = g_list_append( Arrivee_mnemo, mnemo );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_HISTO_COURBE_FIN:
             { g_list_foreach( Arrivee_eana, (GFunc)Proto_afficher_une_source_EA_for_histo_courbe, NULL );
               g_list_foreach( Arrivee_eana, (GFunc)g_free, NULL );
               g_list_free( Arrivee_eana );
               Arrivee_eana = NULL;

               g_list_foreach( Arrivee_mnemo, (GFunc)Proto_afficher_une_source_for_histo_courbe, NULL );
               g_list_foreach( Arrivee_mnemo, (GFunc)g_free, NULL );
               g_list_free( Arrivee_mnemo );
               Arrivee_mnemo = NULL;
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
