/**********************************************************************************************************/
/* Client/menu.c          Gestion des callbacks des menus Watchdog v2.0                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                      sam 30 oct 2004 14:34:53 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * menu.c
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

 #include <gnome.h>
 #include "Reseaux.h"
 #include "client.h"
 #include "Config_cli.h"

 extern struct CLIENT Client;                                    /* Identifiant de l'utilisateur en cours */
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"
/**********************************************************************************************************/
/* Menu_want_plugin_dls: l'utilisateur desire editer la base plugin_dls                                   */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_client_leger ( void )
  { Firefox_exec ("/"); }
/**********************************************************************************************************/
/* Menu_want_plugin_dls: l'utilisateur desire editer la base plugin_dls                                   */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_plugin_dls ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_PLUGIN_DLS, 0, TRUE )) return;
    Envoi_serveur( TAG_DLS, SSTAG_CLIENT_WANT_PAGE_DLS, NULL, 0 );
  }
/**********************************************************************************************************/
/* Menu_want_page_admin: l'utilisateur desire envoyer des requetes d'administration                       */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_page_admin ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_ADMIN, 0, TRUE )) return;
    Envoi_serveur( TAG_ADMIN, SSTAG_CLIENT_WANT_PAGE_ADMIN, NULL, 0 );
  }
/**********************************************************************************************************/
/* Menu_want_message: l'utilisateur desire editer la base msgs                                            */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_message ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_MESSAGE, 0, TRUE )) return;
    Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_WANT_PAGE_MESSAGE, NULL, 0 );
  }
/**********************************************************************************************************/
/* Menu_want_message: l'utilisateur desire editer la base msgs                                            */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_camera ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_CAMERA, 0, TRUE )) return;
    Envoi_serveur( TAG_LOWLEVEL, SSTAG_CLIENT_WANT_PAGE_CAMERA, NULL, 0 );
  }
/**********************************************************************************************************/
/* Menu_want_synoptique: l'utilisateur desire editer la base syns                                         */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_synoptique ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_SYNOPTIQUE, 0, TRUE )) return;
    Envoi_serveur( TAG_SYNOPTIQUE, SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE, NULL, 0 );
  }
/**********************************************************************************************************/
/* Menu_want_supervision: l'utilisateur desire voir le synoptique supervision                             */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_supervision ( void )
  { struct CMD_TYPE_SYNOPTIQUE cmd;
    cmd.id = 1;                                                                    /* Synoptique SITE = 1 */
    if (Chercher_page_notebook( TYPE_PAGE_SUPERVISION, cmd.id, TRUE )) return;
    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_WANT_PAGE_SUPERVISION,
                   (gchar *)&cmd, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
  }
/*--------------------------------------------------------------------------------------------------------*/
