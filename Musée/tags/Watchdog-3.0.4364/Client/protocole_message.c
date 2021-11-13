/**********************************************************************************************************/
/* Client/protocole_message.c    Gestion du protocole_message pour la connexion au serveur Watchdog       */
/* Projet WatchDog version 3.0       Gestion d'habitat                       sam 16 fév 2008 19:20:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_message.c
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
 void Gerer_protocole_message ( struct CONNEXION *connexion )
  { static GSList *Arrivee_message = NULL;
    static GSList *Arrivee_dls     = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_MESSAGE_OK:
             { if (!Tester_page_notebook(TYPE_PAGE_MESSAGE)) { Creer_page_message(); }
             }
            break;
       case SSTAG_SERVEUR_ADD_MESSAGE_OK:
             { struct CMD_TYPE_MESSAGE *msg;
               msg = (struct CMD_TYPE_MESSAGE *)connexion->donnees;
               Proto_afficher_un_message( msg );
             }
            break;
       case SSTAG_SERVEUR_DEL_MESSAGE_OK:
             { struct CMD_TYPE_MESSAGE *msg;
               msg = (struct CMD_TYPE_MESSAGE *)connexion->donnees;
               Proto_cacher_un_message( msg );
             }
            break;
       case SSTAG_SERVEUR_EDIT_MESSAGE_OK:
             { struct CMD_TYPE_MESSAGE *msg;
               msg = (struct CMD_TYPE_MESSAGE *)connexion->donnees;
               Menu_ajouter_editer_message( msg );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_MESSAGE_OK:
             { struct CMD_TYPE_MESSAGE *msg;
               msg = (struct CMD_TYPE_MESSAGE *)connexion->donnees;
               Proto_rafraichir_un_message( msg );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MESSAGE:
             { struct CMD_TYPE_MESSAGES *msgs;
               gint i;
               msgs = (struct CMD_TYPE_MESSAGES *)connexion->donnees;
               Set_progress_plus( msgs->nbr_messages );
               for (i=0; i<msgs->nbr_messages; i++)
                { struct CMD_TYPE_MESSAGE *msg;
                  msg = (struct CMD_TYPE_MESSAGE *)g_try_malloc0( sizeof( struct CMD_TYPE_MESSAGE ) );
                  if (!msg) break; 
                  memcpy( msg, &msgs->msg[i], sizeof(struct CMD_TYPE_MESSAGE ) );
                  Arrivee_message = g_slist_append( Arrivee_message, msg );
                }
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MESSAGE_FIN:
             { 
               g_slist_foreach( Arrivee_message, (GFunc)Proto_afficher_un_message, NULL );
               g_slist_foreach( Arrivee_message, (GFunc)g_free, NULL );
               g_slist_free( Arrivee_message );
               Arrivee_message = NULL;
               Chercher_page_notebook( TYPE_PAGE_MESSAGE, 0, TRUE );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_DLS_FOR_MESSAGE:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               Set_progress_plus(1);
               dls = (struct CMD_TYPE_PLUGIN_DLS *)g_try_malloc0( sizeof( struct CMD_TYPE_PLUGIN_DLS ) );
               if (!dls) return; 

               memcpy( dls, connexion->donnees, sizeof(struct CMD_TYPE_PLUGIN_DLS ) );
               Arrivee_dls = g_slist_append( Arrivee_dls, dls );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_DLS_FOR_MESSAGE_FIN:
             { g_slist_foreach( Arrivee_dls, (GFunc)Proto_afficher_un_dls_for_message, NULL );
               g_slist_foreach( Arrivee_dls, (GFunc)g_free, NULL );
               g_slist_free( Arrivee_dls );
               Arrivee_dls = NULL;
             }
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO_VOC:
             { struct CMD_TYPE_MNEMO_BASE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMO_BASE *)connexion->donnees;
               Proto_afficher_mnemo_voc_message( mnemo );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
