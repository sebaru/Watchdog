/**********************************************************************************************************/
/* Client/protocole_admin.c    Gestion du protocole_admin pour la connexion au serveur Watchdog           */
/* Projet WatchDog version 2.0       Gestion d'habitat                     lun. 24 déc. 2012 13:04:37 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_admin.c
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
 void Gerer_protocole_admin ( struct CONNEXION *connexion )
  { 
    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_ADMIN_OK:
             { if (!Tester_page_notebook(TYPE_PAGE_ADMIN)) { Creer_page_admin(); }
               Chercher_page_notebook( TYPE_PAGE_ADMIN, 0, TRUE );                /* Affichage de la page */
             }
            break;
       case SSTAG_SERVEUR_RESPONSE_START:
            break;
       case SSTAG_SERVEUR_RESPONSE_BUFFER:
             { struct CMD_TYPE_ADMIN *admin;
               admin = (struct CMD_TYPE_ADMIN *)connexion->donnees;
               Proto_afficher_un_admin( admin );
             }
            break;
       case SSTAG_SERVEUR_RESPONSE_STOP:
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
