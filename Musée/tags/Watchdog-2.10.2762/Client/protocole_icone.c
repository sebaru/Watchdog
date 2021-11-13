/**********************************************************************************************************/
/* Client/protocole_icone.c    Gestion du protocole_message pour la connexion au serveur Watchdog       */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_icone.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - Sébastien Lefevre
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
 void Gerer_protocole_icone ( struct CONNEXION *connexion )
  { static GList *Arrivee_classe = NULL;
    static GList *Arrivee_icone = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_ICONE_OK:
             { if (!Tester_page_notebook(TYPE_PAGE_ICONE)) { Creer_page_icone(); }
             }
            break;
       case SSTAG_SERVEUR_ADD_ICONE_WANT_FILE:
             { struct CMD_TYPE_ICONE *ico;
               ico = (struct CMD_TYPE_ICONE *)connexion->donnees;
               Proto_envoyer_gif( ico );
             }
            break;
       case SSTAG_SERVEUR_ADD_ICONE_OK:
             { struct CMD_TYPE_ICONE *ico;
               ico = (struct CMD_TYPE_ICONE *)connexion->donnees;
               Proto_afficher_un_icone( ico );
             }
            break;
       case SSTAG_SERVEUR_DEL_ICONE_OK:
             { struct CMD_TYPE_ICONE *ico;
               ico = (struct CMD_TYPE_ICONE *)connexion->donnees;
               Proto_cacher_un_icone( ico );
             }
            break;
       case SSTAG_SERVEUR_EDIT_ICONE_OK:
             { struct CMD_TYPE_ICONE *ico;
               ico = (struct CMD_TYPE_ICONE *)connexion->donnees;
               Menu_ajouter_editer_icone( ico );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_ICONE_OK:
             { struct CMD_TYPE_ICONE *ico;
               ico = (struct CMD_TYPE_ICONE *)connexion->donnees;
               Proto_rafraichir_un_icone( ico );
             }
            break;
       case SSTAG_SERVEUR_ADD_CLASSE_OK:
             { struct CMD_TYPE_CLASSE *classe;
               classe = (struct CMD_TYPE_CLASSE *)connexion->donnees;
               Proto_afficher_une_classe( classe );
             }
            break;
       case SSTAG_SERVEUR_DEL_CLASSE_OK:
             { struct CMD_TYPE_CLASSE *classe;
               classe = (struct CMD_TYPE_CLASSE *)connexion->donnees;
               Proto_cacher_une_classe( classe );
             }
            break;
       case SSTAG_SERVEUR_EDIT_CLASSE_OK:
             { struct CMD_TYPE_CLASSE *classe;
               classe = (struct CMD_TYPE_CLASSE *)connexion->donnees;
               Menu_ajouter_editer_classe( classe );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_CLASSE_OK:
             { struct CMD_TYPE_CLASSE *classe;
               classe = (struct CMD_TYPE_CLASSE *)connexion->donnees;
               Proto_rafraichir_une_classe( classe );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_CLASSE:
             { struct CMD_TYPE_CLASSE *classe;
               Set_progress_plus(1);

               classe = (struct CMD_TYPE_CLASSE *)g_try_malloc0( sizeof( struct CMD_TYPE_CLASSE ) );
               if (!classe) return; 
               memcpy( classe, connexion->donnees, sizeof(struct CMD_TYPE_CLASSE ) );
               Arrivee_classe = g_list_append( Arrivee_classe, classe );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_CLASSE_FIN:
              { g_list_foreach( Arrivee_classe, (GFunc)Proto_afficher_une_classe, NULL );
                g_list_foreach( Arrivee_classe, (GFunc)g_free, NULL );
                g_list_free( Arrivee_classe );
                Arrivee_classe = NULL;
                Chercher_page_notebook( TYPE_PAGE_ICONE, 0, TRUE );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ICONE:
             { struct CMD_TYPE_ICONE *ico;
               Set_progress_plus(1);

               ico = (struct CMD_TYPE_ICONE *)g_try_malloc0( sizeof( struct CMD_TYPE_ICONE ) );
               if (!ico) return; 
               memcpy( ico, connexion->donnees, sizeof(struct CMD_TYPE_ICONE ) );
               Arrivee_icone = g_list_append( Arrivee_icone, ico );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ICONE_FIN:
             { g_list_foreach( Arrivee_icone, (GFunc)Proto_afficher_un_icone, NULL );
               g_list_foreach( Arrivee_icone, (GFunc)g_free, NULL );
               g_list_free( Arrivee_icone );
               Arrivee_icone = NULL;
               Chercher_page_notebook( TYPE_PAGE_ICONE, 0, TRUE );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
