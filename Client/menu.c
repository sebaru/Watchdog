/**********************************************************************************************************/
/* Client/menu.c          Gestion des callbacks des menus Watchdog v2.0                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 30 oct 2004 14:34:53 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * menu.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - 
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
 
 
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */
/**********************************************************************************************************/
/* Menu_want_util: l'utilisateur desire editer les bases users                                            */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_util ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_UTIL, 0, TRUE )) return;
    Envoi_serveur( TAG_UTILISATEUR, SSTAG_CLIENT_WANT_PAGE_UTIL, NULL, 0 );
  }
/**********************************************************************************************************/
/* Menu_want_groupe: l'utilisateur desire editer les bases groups                                         */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_groupe ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_GROUPE, 0, TRUE )) return;
    Envoi_serveur( TAG_UTILISATEUR, SSTAG_CLIENT_WANT_PAGE_GROUPE, NULL, 0 );
  }
/**********************************************************************************************************/
/* Menu_want_histo_hard: l'utilisateur desire voir l'historique                                           */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_histo_hard ( void )
  { Creer_page_liste_histo_hard();
  }
/**********************************************************************************************************/
/* Menu_want_histo_hard: l'utilisateur desire voir l'historique                                           */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_courbe ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_COURBE, 0, TRUE )) return;
    Creer_page_courbe( "Courbes" );
    Chercher_page_notebook( TYPE_PAGE_COURBE, 0, TRUE );                          /* Affichage de la page */
  }
/**********************************************************************************************************/
/* Menu_want_histo_hard: l'utilisateur desire voir l'historique                                           */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_histo_courbe ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_HISTO_COURBE, 0, TRUE )) return;
    Creer_page_histo_courbe( "Histo Courbes" );
    Chercher_page_notebook( TYPE_PAGE_HISTO_COURBE, 0, TRUE );                    /* Affichage de la page */
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
