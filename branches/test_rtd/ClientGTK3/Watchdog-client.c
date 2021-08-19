/******************************************************************************************************************************/
/* client/Watchdog-client.c        Le client Watchdog v2.0                                                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           ven 15 fév 2008 18:05:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Watchdog-client.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - sebastien
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

 #include <gtk/gtk.h>
 #include <sys/stat.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <unistd.h>
 #include <time.h>

/**************************************** Définitions des prototypes programme ************************************************/
 #include "protocli.h"
 #include "config.h"
 #include "Erreur.h"
 #include "client.h"
 #include "ClientResources.h"

 #define TITRE_FENETRE "Watchdog ver" VERSION " - GTK3"

/***************************************************** Définition du menu *****************************************************/
#ifdef bouh

/******************************************************************************************************************************/
/* Firefox_exec:Lance un navigateur avec l'URI en pj                                                                          */
/******************************************************************************************************************************/
 void Firefox_exec ( gchar *uri )
  { gchar chaine[256];
    gint pid;
    g_snprintf(chaine, sizeof(chaine), "https://%s.abls-habitat.fr/%s", client.host, uri );
    printf( "Lancement d'un firefox sur %s\n", chaine );
    pid = fork();
    if (pid<0) return;
    else if (!pid)                                                                       /* Lancement de la ligne de commande */
     { execlp( "firefox", "firefox", chaine, NULL );
       printf("Lancement de firefox failed\n");
       _exit(0);
     }
  }
/******************************************************************************************************************************/
/*!Traitement_signaux: Gestion principale des signaux de Watchdog-client
 ******************************************************************************************************************************/
 static void Traitement_signaux ( int num                                                           /*! numéro du signal recu */
                                )
  { switch (num)
     { case SIGINT :
       case SIGTERM: Fermer_client(); break;
       case SIGPIPE: Deconnecter(); break;
       case SIGIO  : Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING, "Signal IO %d !", num );
                     Ecouter_serveur();
                     break;
       default: Info_new( Config_cli.log, Config_cli.log_override, LOG_WARNING, "Signal non gere %d !", num );
     }
    printf("Fin traitement signaux\n");
  }
#endif
/******************************************************************************************************************************/
/*!Fermer_client: Deconnexion du client avant sortir du programme
 ******************************************************************************************************************************/
 static void Fermer_client ( struct CLIENT *client )
  { printf("%s : %p\n", __func__, client);
    Deconnecter(client);
    gtk_widget_destroy(client->window);
    g_free(client);
  }
/******************************************************************************************************************************/
/*!Menu_Recompiler: Lance et recompilation pour etre sur d'etre a la derniere version de prod
 ******************************************************************************************************************************/
 static void Menu_Recompiler (GSimpleAction *simple, GVariant *parameter, gpointer user_data)
  { gint pid;
    pid = fork();
    if (pid<0) return;
    else if (!pid)                                                                       /* Lancement de la ligne de commande */
     { system ("cd; svn co https://svn.abls-habitat.fr/repo/Watchdog/prod SRCRecompile;"
               "cd SRCRecompile; ./autogen.sh; sudo make install; cd ..; rm -rf SRCRecompile; killall Watchdog-client-gtk3");
     }
  }

 static void Menu_Inspector (GSimpleAction *action, GVariant *parameter, gpointer user_data)
  { gtk_window_set_interactive_debugging (TRUE); }

 static void Menu_Quitter (GSimpleAction *simple, GVariant *parameter, gpointer user_data)
  { Fermer_client(user_data); }


 static void Menu_Go_to_wiki (GSimpleAction *simple, GVariant *parameter, gpointer user_data)
  { gint pid;
    pid = fork();
    if (pid<0) return;
    else if (!pid)                                                                       /* Lancement de la ligne de commande */
     { execlp( "firefox", "firefox", "https://wiki.abls-habitat.fr", NULL );
       printf("Lancement de firefox failed\n");
       _exit(0);
     }
  }

 static void Menu_About (GSimpleAction *simple, GVariant *parameter, gpointer user_data)
  { GtkWidget *about_dialog;
    about_dialog = gtk_about_dialog_new ();
    const gchar *authors[] = { "Sebastien Lefevre <sebastien.lefevre@abls-habitat.fr>",
                               "Bruno Lefevre <bruno.lefevre@abls-habitat.fr>", NULL };
    const gchar *documenters[] = {"A vot' bon coeur !", NULL};

    /* Fill in the about_dialog with the desired information */
    gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG (about_dialog), "About Watchdog");
    gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG (about_dialog), "Gnu Public License");
    gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG (about_dialog), authors);
    gtk_about_dialog_set_documenters (GTK_ABOUT_DIALOG (about_dialog), documenters);
    gtk_about_dialog_set_website_label (GTK_ABOUT_DIALOG (about_dialog), "Wiki ABLS-Habitat.fr");
    gtk_about_dialog_set_website (GTK_ABOUT_DIALOG (about_dialog), "https://abls-habitat.fr");

   /* The "response" signal is emitted when the dialog receives a delete event,
    * therefore we connect that signal to the on_close callback function
    * created above.
    */
    g_signal_connect (GTK_DIALOG (about_dialog), "response", G_CALLBACK(gtk_widget_destroy), NULL);

   /* Show the about dialog */
    gtk_widget_show (about_dialog);
  }
/******************************************************************************************************************************/
/* ActivateCB: Fonction d'activation de a fenetre applicative                                                                 */
/******************************************************************************************************************************/
 static void ActivateCB ( GtkApplication *app, gpointer data )
  { GtkToolItem *bouton, *separateur;

    struct CLIENT *client = g_malloc0 ( sizeof(struct CLIENT) );
    client->settings = g_settings_new ( "fr.abls-habitat.watchdog" );
    client->window = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (client->window), TITRE_FENETRE );
    gtk_window_set_default_size (GTK_WINDOW (client->window), 800, 600);
    gtk_window_set_icon_name ( GTK_WINDOW(client->window), "fr.abls-habitat.watchdog" );
    GtkWidget *box = gtk_box_new( GTK_ORIENTATION_VERTICAL, 10 );
    gtk_container_add (GTK_CONTAINER (client->window), box);

    GtkWidget *toolbar = gtk_toolbar_new();
    gtk_box_pack_start ( GTK_BOX(box), toolbar, FALSE, FALSE, 0 );
    gtk_toolbar_set_style ( GTK_TOOLBAR(toolbar), GTK_TOOLBAR_BOTH );

    bouton = gtk_tool_button_new ( gtk_image_new_from_icon_name("system-run", GTK_ICON_SIZE_LARGE_TOOLBAR), "Se connecter" );
    gtk_tool_item_set_tooltip_text ( bouton, "Se connecter au serveur" );
    g_signal_connect_swapped ( bouton, "clicked", G_CALLBACK(Connecter), client );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), bouton, -1 );

    bouton = gtk_tool_button_new ( gtk_image_new_from_icon_name("window-close", GTK_ICON_SIZE_LARGE_TOOLBAR), "Se déconnecter" );
    gtk_tool_item_set_tooltip_text ( bouton, "Se déconnecter du serveur" );
    g_signal_connect_swapped ( bouton, "clicked", G_CALLBACK(Deconnecter), client );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), bouton, -1 );

    separateur = gtk_separator_tool_item_new ();
    gtk_separator_tool_item_set_draw ( GTK_SEPARATOR_TOOL_ITEM(separateur), TRUE );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), separateur, -1 );

    bouton = gtk_tool_button_new ( gtk_image_new_from_icon_name("edit-find", GTK_ICON_SIZE_LARGE_TOOLBAR), "Superviser" );
    gtk_tool_item_set_tooltip_text ( bouton, "Ouvrir le synoptique d'accueil" );
    g_signal_connect_swapped ( bouton, "clicked", G_CALLBACK(Menu_want_supervision_accueil), client );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), bouton, -1 );

    separateur = gtk_separator_tool_item_new ();
    gtk_separator_tool_item_set_draw ( GTK_SEPARATOR_TOOL_ITEM(separateur), TRUE );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), separateur, -1 );

    bouton = gtk_tool_button_new ( gtk_image_new_from_icon_name("preferences-desktop-display", GTK_ICON_SIZE_LARGE_TOOLBAR), "Atelier" );
    gtk_tool_item_set_tooltip_text ( bouton, "Editer les synoptiques" );
    g_signal_connect_swapped ( bouton, "clicked", G_CALLBACK(Menu_want_liste_synoptique), client );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), bouton, -1 );

    separateur = gtk_separator_tool_item_new ();
    gtk_separator_tool_item_set_draw ( GTK_SEPARATOR_TOOL_ITEM(separateur), FALSE );
    gtk_tool_item_set_expand ( separateur, TRUE );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), separateur, -1 );

    bouton = gtk_tool_button_new ( gtk_image_new_from_icon_name("application-exit", GTK_ICON_SIZE_LARGE_TOOLBAR), "Quitter" );
    gtk_tool_item_set_tooltip_text ( bouton, "Sortir de l'application" );
    g_signal_connect_swapped ( bouton, "clicked", G_CALLBACK(Fermer_client), client );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), bouton, -1 );

    gtk_box_pack_start ( GTK_BOX(box), Creer_boite_travail(client), TRUE, TRUE, 0 );

    gtk_widget_show_all(client->window);
  }
/******************************************************************************************************************************/
/*!Main: Fonction principale du programme du client Watchdog
 ******************************************************************************************************************************/
 int main ( int argc,                                                       /*!< nombre d'argument dans la ligne de commande, */
            char *argv[]                                                               /*!< Arguments de la ligne de commande */
          )
  { static GActionEntry app_entries[] =
     { { "About", Menu_About, NULL, NULL, NULL },
       { "Inspector", Menu_Inspector, NULL, NULL, NULL },
       { "Go_to_wiki", Menu_Go_to_wiki, NULL, NULL, NULL },
       { "Quitter", Menu_Quitter, NULL, NULL, NULL },
       { "Recompiler", Menu_Recompiler, NULL, NULL, NULL },
    //  { "new", new_activated, NULL, NULL, NULL }
     };
    int status;

    if (chdir( g_get_home_dir() ))                                                      /* Positionnement à la racine du home */
     { printf( "Chdir %s failed\n", g_get_home_dir() ); exit(-1); }
    else
     { printf( "Chdir %s OK\n", g_get_home_dir() ); }

    if (chdir( REPERTOIR_CONF ))                                                        /* Positionnement à la bonne position */
     { printf ("Chdir %s NOK. Creating new directory\n", REPERTOIR_CONF );
       mkdir ( REPERTOIR_CONF, 0700 );
       chdir ( REPERTOIR_CONF );
     } else printf ("Chdir %s OK\n", REPERTOIR_CONF );


    GtkApplication *app = gtk_application_new ("fr.abls_habitat.watchdog", G_APPLICATION_FLAGS_NONE);
    g_action_map_add_action_entries ( G_ACTION_MAP (app), app_entries, G_N_ELEMENTS (app_entries), app );
    g_signal_connect (app, "activate", G_CALLBACK (ActivateCB), NULL);

    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);
/*    g_resources_unregister(clientResources_get_resource());*/
    return status;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
