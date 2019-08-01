/**********************************************************************************************************/
/* Client/protocole_histo.c    Gestion du protocole_histo pour Watchdog                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                       sam 16 fév 2008 19:19:20 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_histo.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sébastien Lefevre
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
 void Gerer_protocole_histo ( struct CONNEXION *connexion )
  { static GList *Arrivee_histo = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_SHOW_HISTO:
             { struct CMD_TYPE_HISTO *histo;
               histo = (struct CMD_TYPE_HISTO *)connexion->donnees;
               Proto_afficher_un_histo( histo );
             }
            break;
       case SSTAG_SERVEUR_ACK_HISTO:
             { struct CMD_TYPE_HISTO *histo;
               histo = (struct CMD_TYPE_HISTO *)connexion->donnees;
               Proto_rafraichir_un_histo( histo );
             }
            break;
       case SSTAG_SERVEUR_DEL_HISTO:
             { struct CMD_TYPE_HISTO *histo;
               histo = (struct CMD_TYPE_HISTO *)connexion->donnees;
               Proto_cacher_un_histo( histo );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_HISTO:
             { struct CMD_TYPE_HISTO *histo;
               Set_progress_plus(1);
               histo = (struct CMD_TYPE_HISTO *)g_try_malloc0( sizeof( struct CMD_TYPE_HISTO ) );
               if (!histo) return;
               memcpy( histo, connexion->donnees, sizeof(struct CMD_TYPE_HISTO ) );
               Arrivee_histo = g_list_append( Arrivee_histo, histo );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_HISTO_FIN:
             { g_list_foreach( Arrivee_histo, (GFunc)Proto_afficher_un_histo, NULL );
               g_list_foreach( Arrivee_histo, (GFunc)g_free, NULL );
               g_list_free( Arrivee_histo );
               Arrivee_histo = NULL;
               Chercher_page_notebook( TYPE_PAGE_HISTO, 0, TRUE );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
