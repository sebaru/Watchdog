/******************************************************************************************************************************/
/* Client/Watchdog-client.c        Le client Watchdog v2.0                                                                    */
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
 #include "Config_cli.h"
 #include "client.h"
 #include "ClientResources.h"

 #define TITRE_FENETRE "Watchdog ver" VERSION " - GTK3"

 GtkWidget *F_client;                                                                                /* Widget Fenetre Client */

 extern struct CMD_TYPE_UTILISATEUR *Edit_util;                                           /* L'utilisateur en cours d'edition */

 struct CLIENT Client;                                                               /* Identifiant de l'utilisateur en cours */
 struct CONFIG_CLI Config_cli;                                                     /* Configuration generale cliente watchdog */

 static gboolean Arret = FALSE;
/***************************************************** Définition du menu *****************************************************/
#ifdef bouh

/******************************************************************************************************************************/
/* Firefox_exec:Lance un navigateur avec l'URI en pj                                                                          */
/******************************************************************************************************************************/
 void Firefox_exec ( gchar *uri )
  { gchar chaine[256];
    gint pid;
    g_snprintf(chaine, sizeof(chaine), "https://%s.abls-habitat.fr/%s", Client.host, uri );
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
 static void Fermer_client ( void )
  { printf("Fermer_client ! \n");
    Deconnecter();
    gtk_widget_destroy(F_client);
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Curl_progress_callback: Callbakc d'affichage de la progression du download ou upload                                       */
/* Entrée:                                                                                                                    */
/* Sortie: 0                                                                                                                  */
/******************************************************************************************************************************/
 gint Curl_progress_callback ( void *clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
  { Set_progress_ratio( dlnow + ulnow, dltotal + ultotal );
    gtk_main_iteration_do ( TRUE );
    return(0);
  }
/******************************************************************************************************************************/
/* WTD_Curl_init: Envoie une requete au serveur                                                                               */
/* Entrée:                                                                                                                    */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 CURL *WTD_Curl_init ( gchar *erreur )
  { CURL *curl;

    curl = curl_easy_init();                                                                /* Preparation de la requete CURL */
    if (!curl)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR, "%s: cURL init failed", __func__ );
       return(NULL);
     }

/*    g_snprintf( sid, sizeof(sid), "sid=%s", Client.sid );
    curl_easy_setopt(curl, CURLOPT_COOKIE, sid);                           /* Active la gestion des cookies pour la connexion */
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &erreur );
    curl_easy_setopt(curl, CURLOPT_VERBOSE, Config_cli.log_override );
    curl_easy_setopt(curl, CURLOPT_USERAGENT, WATCHDOG_USER_AGENT);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, Curl_progress_callback);
/*     curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0 );*/
/*     curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0 );                                    Warning ! */
/*     curl_easy_setopt(curl, CURLOPT_CAINFO, Cfg_satellite.https_file_ca );
       curl_easy_setopt(curl, CURLOPT_SSLKEY, Cfg_satellite.https_file_key );
       g_snprintf( chaine, sizeof(chaine), "./%s", Cfg_satellite.https_file_cert );
       curl_easy_setopt(curl, CURLOPT_SSLCERT, chaine );*/
    return(curl);
  }
/******************************************************************************************************************************/
/* WTD_Curl_request: Envoie une requete au serveur                                                                            */
/* Entrée:                                                                                                                    */
/* Sortie: FALSE si probleme                                                                                                  */
/******************************************************************************************************************************/
 gboolean WTD_Curl_post_request ( gchar *uri, gint post, gchar *post_data, gint post_length )
  { gchar erreur[CURL_ERROR_SIZE+1];
    struct curl_slist *slist = NULL;
    long http_response;
    gchar url[128];
    CURLcode res;
    CURL *curl;

    http_response = 0;

    curl = WTD_Curl_init ( &erreur[0] );                                               /* Preparation de la requete CURL */
    if (!curl)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR, "%s: cURL init failed for %s", __func__, uri );
       return(FALSE);
     }

    g_snprintf( url, sizeof(url), "http://%s:5560/%s", Client.host, uri );
    curl_easy_setopt(curl, CURLOPT_URL, url );
    if (post == TRUE)
     { curl_easy_setopt(curl, CURLOPT_POST, 1 );
       curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)post_data);
       curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, post_length);
       slist = curl_slist_append(slist, "Content-Type: application/json");
       curl_easy_setopt(curl, CURLOPT_HTTPHEADER, slist);
       curl_easy_setopt(curl, CURLOPT_HEADER, 1);
     }
    else
     { /*curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CB_Receive_gif_data );*/
       curl_easy_setopt(curl, CURLOPT_USERAGENT, WATCHDOG_USER_AGENT);
     }

    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, erreur );
    curl_easy_setopt(curl, CURLOPT_VERBOSE, Config_cli.log_override );
/*       curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0 );*/
/*     curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0 );                                    Warning ! */
/*       curl_easy_setopt(curl, CURLOPT_CAINFO, Cfg_satellite.https_file_ca );
       curl_easy_setopt(curl, CURLOPT_SSLKEY, Cfg_satellite.https_file_key );
       g_snprintf( chaine, sizeof(chaine), "./%s", Cfg_satellite.https_file_cert );
       curl_easy_setopt(curl, CURLOPT_SSLCERT, chaine );*/

    res = curl_easy_perform(curl);
    if (res)
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                "%s : Error : Could not connect to %s : %s", __func__, uri, erreur );
       curl_easy_cleanup(curl);
       /*if (Gif_received_buffer) { g_free(Gif_received_buffer); }*/
       return(FALSE);
     }
    if (curl_easy_getinfo( curl, CURLINFO_RESPONSE_CODE, &http_response ) != CURLE_OK) http_response = 401;
    curl_easy_cleanup(curl);
    curl_slist_free_all(slist);

    if (http_response != 200)                                                                /* HTTP 200 OK ? */
     { Info_new( Config_cli.log, Config_cli.log_override, LOG_DEBUG,
                "%s : URL %s not received (HTTP_CODE = %d)!", __func__, url, http_response );
       /*if (Gif_received_buffer) { g_free(Gif_received_buffer); }*/
       return(FALSE);
     }
/*    else
     { gchar nom_fichier[80];
       gint fd;
       if (mode) g_snprintf( nom_fichier, sizeof(nom_fichier), "%d.gif.%02d", id, mode );
            else g_snprintf( nom_fichier, sizeof(nom_fichier), "%d.gif", id );
       Info_new( Config_cli.log, Config_cli.log_override, LOG_DEBUG,
                "Download_gif : Saving GIF id %d, mode %d, size %d -> %s", id, mode, Gif_received_size, nom_fichier );
       unlink(nom_fichier);
       fd = open( nom_fichier, O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR );
       if (fd>0)
        { write( fd, Gif_received_buffer, Gif_received_size );
          close (fd);
        }
       else
        { Info_new( Config_cli.log, Config_cli.log_override, LOG_DEBUG,
                   "Download_gif : Unable to save file %s", nom_fichier );
        }
       g_free(Gif_received_buffer);
       Gif_received_buffer = FALSE;
       if (fd<=0) return(FALSE);
     }
*/
    return(TRUE);
  }
#endif
 static void Menu_Connecter (GSimpleAction *simple, GVariant *parameter, gpointer user_data)
  { Connecter(); }

 static void Menu_Deconnecter (GSimpleAction *simple, GVariant *parameter, gpointer user_data)
  { Deconnecter(); }

 static void Menu_Quitter (GSimpleAction *simple, GVariant *parameter, gpointer user_data)
  { Fermer_client(); }

 static void Menu_about (GSimpleAction *simple, GVariant *parameter, gpointer user_data)
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
 static void ActivateCB ( GtkApplication *app, gpointer user_data)
  { GtkToolItem *bouton, *separateur;
    static GActionEntry app_entries[] =
     { { "about", Menu_about, NULL, NULL, NULL },
       { "Connecter", Menu_Connecter, NULL, NULL, NULL },
       { "Deconnecter", Menu_Deconnecter, NULL, NULL, NULL },
       { "Quitter", Menu_Quitter, NULL, NULL, NULL },
    //  { "new", new_activated, NULL, NULL, NULL }
     };

    F_client = gtk_application_window_new (app);
    gtk_window_set_title (GTK_WINDOW (F_client), TITRE_FENETRE );
    gtk_window_set_default_size (GTK_WINDOW (F_client), 400, 400);
    gtk_window_set_icon_name ( GTK_WINDOW(F_client), "fr.abls-habitat.watchdog" );
    g_action_map_add_action_entries (G_ACTION_MAP (app), app_entries, G_N_ELEMENTS (app_entries), app);

    GtkWidget *box = gtk_box_new( GTK_ORIENTATION_VERTICAL, 10 );
    gtk_container_add (GTK_CONTAINER (F_client), box);

    GtkWidget *toolbar = gtk_toolbar_new();
    gtk_box_pack_start ( GTK_BOX(box), toolbar, FALSE, FALSE, 0 );

    bouton = gtk_tool_button_new ( gtk_image_new_from_icon_name("system-run", GTK_ICON_SIZE_LARGE_TOOLBAR), "Se connecter" );
    gtk_tool_item_set_tooltip_text ( bouton, "Se connecter au serveur" );
    g_signal_connect ( bouton, "clicked", G_CALLBACK(Connecter), NULL );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), bouton, -1 );

    bouton = gtk_tool_button_new ( gtk_image_new_from_icon_name("window-close", GTK_ICON_SIZE_LARGE_TOOLBAR), "Se déconnecter" );
    gtk_tool_item_set_tooltip_text ( bouton, "Se déconnecter du serveur" );
    g_signal_connect ( bouton, "clicked", G_CALLBACK(Deconnecter), NULL );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), bouton, -1 );

    separateur = gtk_separator_tool_item_new ();
    gtk_separator_tool_item_set_draw ( GTK_SEPARATOR_TOOL_ITEM(separateur), TRUE );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), separateur, -1 );

    bouton = gtk_tool_button_new ( gtk_image_new_from_icon_name("edit-find", GTK_ICON_SIZE_LARGE_TOOLBAR), "Superviser" );
    gtk_tool_item_set_tooltip_text ( bouton, "Ouvrir le synoptique d'accueil" );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), bouton, -1 );

    separateur = gtk_separator_tool_item_new ();
    gtk_separator_tool_item_set_draw ( GTK_SEPARATOR_TOOL_ITEM(separateur), FALSE );
    gtk_tool_item_set_expand ( separateur, TRUE );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), separateur, -1 );

    bouton = gtk_tool_button_new ( gtk_image_new_from_icon_name("application-exit", GTK_ICON_SIZE_LARGE_TOOLBAR), "Quitter" );
    gtk_tool_item_set_tooltip_text ( bouton, "Sortir de l'application" );
    g_signal_connect_swapped ( bouton, "clicked", G_CALLBACK(Fermer_client), NULL );
    gtk_toolbar_insert (GTK_TOOLBAR(toolbar), bouton, -1 );

    gtk_box_pack_start ( GTK_BOX(box), Creer_boite_travail(), TRUE, TRUE, 0 );

    gtk_widget_show_all(F_client);
  }
/******************************************************************************************************************************/
/*!Main: Fonction principale du programme du client Watchdog
 ******************************************************************************************************************************/
 int main ( int argc,                                                       /*!< nombre d'argument dans la ligne de commande, */
            char *argv[]                                                               /*!< Arguments de la ligne de commande */
          )
  { struct sigaction sig;
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

    Config_cli.log = Info_init( "Watchdog_client", LOG_DEBUG );                                        /* Init msgs d'erreurs */
    Info_change_log_level( Config_cli.log, Config_cli.log_level );
    Lire_config_cli ( &Config_cli, "watchdog-client.conf" );

    GtkApplication *app = gtk_application_new ("fr.abls_habitat.watchdog", G_APPLICATION_FLAGS_NONE);
    g_signal_connect (app, "activate", G_CALLBACK (ActivateCB), NULL);

    status = g_application_run (G_APPLICATION (app), argc, argv);
    g_object_unref (app);

/*    g_resources_unregister(ClientResources_get_resource());*/
    return status;
#ifdef bouh

    sig.sa_handler = Traitement_signaux;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;
    sigaction( SIGTERM, &sig, NULL );                                                           /* Arret Prématuré (logiciel) */
    sigaction( SIGINT,  &sig, NULL );                                                           /* Arret Prématuré (logiciel) */
    sigaction( SIGPIPE,  &sig, NULL );                                                          /* Arret Prématuré (logiciel) */

    Client.gids = NULL;                                                  /* Initialisation de la structure de client en cours */

    gtk_widget_show_all( F_client );                                                   /* Affichage de le fenetre de controle */
    if (Config_cli.gui_tech == FALSE)
     { memcpy( Client.ident.nom,    Config_cli.user,   sizeof(Client.ident.nom) );
       memcpy( Client.ident.passwd, Config_cli.passwd, sizeof(Client.ident.passwd) );
       memcpy( Client.host,         Config_cli.host,   sizeof(Client.host) );
       Connecter_au_serveur();
     }

    while ( Arret != TRUE )
     { gtk_main_iteration_do ( FALSE );
       if (Client.connexion) Ecouter_serveur();
       usleep(1000);
     }

    if (Client.gids) g_list_free(Client.gids);
    Info_new( Config_cli.log, Config_cli.log_override, LOG_NOTICE, _("Main : Stopped") );
    exit(0);
#endif
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
