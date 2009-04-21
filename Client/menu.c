/**********************************************************************************************************/
/* Client/menu.c          Gestion des callbacks des menus Watchdog v2.0                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 30 oct 2004 14:34:53 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <gnome.h>
 #include "Reseaux.h"
 #include "Config_cli.h"

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
 void Menu_want_plugin_dls ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_PLUGIN_DLS, 0, TRUE )) return;
    Envoi_serveur( TAG_DLS, SSTAG_CLIENT_WANT_PAGE_DLS, NULL, 0 );
  }
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
/* Menu_want_message: l'utilisateur desire editer la base msgs                                            */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_message ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_MESSAGE, 0, TRUE )) return;
    Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_WANT_PAGE_MESSAGE, NULL, 0 );
  }
/**********************************************************************************************************/
/* Menu_want_icone: l'utilisateur desire editer la base icons                                             */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_icone ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_ICONE, 0, TRUE )) return;
    Envoi_serveur( TAG_ICONE, SSTAG_CLIENT_WANT_PAGE_CLASSE, NULL, 0 );
  }
/**********************************************************************************************************/
/* Menu_want_synoptique: l'utilisateur desire editer la base syns                                         */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_synoptique ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_SYNOPTIQUE, 0, TRUE )) return;
    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE, NULL, 0 );
  }
/**********************************************************************************************************/
/* Menu_want_mnemonique: l'utilisateur desire editer la base syns                                         */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_mnemonique ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_MNEMONIQUE, 0, TRUE )) return;
    Envoi_serveur( TAG_MNEMONIQUE, SSTAG_CLIENT_WANT_PAGE_MNEMONIQUE, NULL, 0 );
  }
/**********************************************************************************************************/
/* Menu_want_mnemonique: l'utilisateur desire editer la base syns                                         */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_entreeANA ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_ENTREEANA, 0, TRUE )) return;
    Envoi_serveur( TAG_ENTREEANA, SSTAG_CLIENT_WANT_PAGE_ENTREEANA, NULL, 0 );
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
/* Menu_want_histo_hard: l'utilisateur desire voir l'historique                                           */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_scenario ( void )
  { if (Chercher_page_notebook( TYPE_PAGE_SCENARIO, 0, TRUE )) return;
    Envoi_serveur( TAG_SCENARIO, SSTAG_CLIENT_WANT_PAGE_SCENARIO, NULL, 0 );
    Creer_page_scenario();
    Chercher_page_notebook( TYPE_PAGE_SCENARIO, 0, TRUE );                    /* Affichage de la page */
  }
/**********************************************************************************************************/
/* Menu_want_supervision: l'utilisateur desire voir le synoptique supervision                             */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Menu_want_supervision ( void )
  { struct CMD_ID_SYNOPTIQUE cmd;
    cmd.id = 1;                                                                    /* Synoptique SITE = 1 */
    if (Chercher_page_notebook( TYPE_PAGE_SUPERVISION, cmd.id, TRUE )) return;
    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_WANT_PAGE_SUPERVISION,
                   (gchar *)&cmd, sizeof(struct CMD_ID_SYNOPTIQUE) );
  }
/*--------------------------------------------------------------------------------------------------------*/
