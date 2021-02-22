/******************************************************************************************************************************/
/* client/connect.c        Gestion du logon user sur module client Watchdog                                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           sam 16 fév 2008 19:19:02 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * connect.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sébastien Lefevre
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

 #include "Reseaux.h"
 #include "client.h"

/******************************************** Définitions des prototypes programme ********************************************/
 #include "config.h"
 #include "protocli.h"

/******************************************************************************************************************************/
/* Deconnecter: libere la mémoire et deconnecte le client                                                                     */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Deconnecter_sale ( struct CLIENT *client )
  { printf("%s : %p\n", __func__, client );
    soup_session_abort (client->connexion);
    client->connexion = NULL;
    Effacer_pages(client);                                                                    /* Efface les pages du notebook */
  }
/******************************************************************************************************************************/
/* Deconnecter_CB: Appeler une fois que la reponse à la requet /connect est recue                                             */
/* Entrée/Sortie: les parametres libsoup                                                                                      */
/******************************************************************************************************************************/
 static void Deconnecter_CB (SoupSession *session, SoupMessage *msg, gpointer user_data)
  { Deconnecter_sale ( user_data ); }
/******************************************************************************************************************************/
/* Deconnecter: libere la mémoire et deconnecte le client                                                                     */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Deconnecter ( struct CLIENT *client )
  { printf("%s : %p\n", __func__, client );
    if (!client->connexion) return;
    Envoi_json_au_serveur ( client, "PUT", NULL, "/api/disconnect", Deconnecter_CB );
    Reset_page_histo( client );
    Log ( client, "Disconnected" );
  }
/******************************************************************************************************************************/
/* Envoi_au_serveur: Envoi une requete web au serveur Watchdogd                                                               */
/* Entrée: des infos sur le paquet à envoyer                                                                                  */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Envoi_json_au_serveur ( struct CLIENT *client, gchar *methode, JsonBuilder *builder, gchar *URI, SoupSessionCallback callback )
  { gchar target[128];
    printf("%s : sending %s\n", __func__, URI );
    g_snprintf( target, sizeof(target), "https://%s:5560%s", client->hostname, URI );
    SoupMessage *msg = soup_message_new ( methode, target );
    client->network_size_sent = 0;
    g_signal_connect ( G_OBJECT(msg), "got-chunk", G_CALLBACK(Update_progress_bar), client );
    if (builder)
     { gsize taille_buf;
       gchar *buf = Json_get_buf (builder, &taille_buf);
       soup_message_set_request ( msg, "application/json; charset=UTF-8", SOUP_MEMORY_TAKE, buf, taille_buf );
       client->network_size_to_send = taille_buf;
       gchar chaine[128];
       g_snprintf ( chaine, sizeof(chaine), "Sending %s %s : %s\n", methode, URI, buf );
       chaine[126]='\n';
       printf(chaine);
     }
    else { printf ( "Sending %s %s\n", methode, URI ); }
    SoupCookie *wtd_session = soup_cookie_new ( "wtd_session", client->wtd_session, "/", NULL, 0 );
    GSList *liste = g_slist_append ( NULL, wtd_session );
    soup_cookies_to_request ( liste, msg );
    g_slist_free(liste);
    if (!msg) { Log( client, "Erreur envoi au serveur"); Deconnecter_sale(client); }
    else soup_session_queue_message (client->connexion, msg, callback, client);
  }
/******************************************************************************************************************************/
/* Envoi_au_serveur: Envoi une requete web au serveur Watchdogd                                                               */
/* Entrée: des infos sur le paquet à envoyer                                                                                  */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Envoi_ws_au_serveur ( struct CLIENT *client, SoupWebsocketConnection *ws, JsonBuilder *builder )
  { gsize taille_buf;
    gchar *buf = Json_get_buf (builder, &taille_buf);
    GBytes *gbytes = g_bytes_new_take ( buf, taille_buf );
    soup_websocket_connection_send_message ( ws, SOUP_WEBSOCKET_DATA_TEXT, gbytes );
    g_bytes_unref( gbytes );
  }
/******************************************************************************************************************************/
/* Traiter_reception_websocket_CB: Opere le traitement d'un message recu par la WebSocket MSGS                                  */
/* Entrée: rien                                                                                                               */
/* Sortie: un widget boite                                                                                                    */
/******************************************************************************************************************************/
 void Traiter_reception_ws_CB ( SoupWebsocketConnection *self, gint type, GBytes *message_brut, gpointer user_data )
  { gsize taille;
    struct CLIENT *client = user_data;
    printf("%s: Recu WS: %s %p\n", __func__, g_bytes_get_data ( message_brut, &taille ), client );
    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( message_brut, &taille ) );
    if (!response) return;

    gchar *zmq_tag = Json_get_string( response, "zmq_tag" );
    if (zmq_tag)
     {      if(!strcasecmp(zmq_tag,"DLS_HISTO")) { Updater_histo( client, response ); }
       else if(!strcasecmp(zmq_tag,"PULSE"))     { Set_progress_pulse( client ); }
       else printf("%s: tag '%s' inconnu\n", __func__, zmq_tag );
     }
    json_node_unref(response);
  }
/******************************************************************************************************************************/
/* Traiter_reception_websocket_CB: Opere le traitement d'un message recu par la WebSocket MSGS                                  */
/* Entrée: rien                                                                                                               */
/* Sortie: un widget boite                                                                                                    */
/******************************************************************************************************************************/
 static void Traiter_reception_ws_on_closed ( SoupWebsocketConnection *connexion, gpointer user_data )
  { printf("%s\n", __func__ );
  }
 static void Traiter_reception_ws_on_error  ( SoupWebsocketConnection *connexion, GError *error, gpointer user_data )
  { struct CLIENT *client = user_data;
    printf("%s: WebSocket Error '%s' received !\n", __func__, error->message );
    Log( client, error->message );
  }
/******************************************************************************************************************************/
/* Traiter_connect_ws_CB: Termine la creation de la connexion websocket MSGS et raccorde le signal handler                    */
/* Entrée: les variables traditionnelles de libsous                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Traiter_connect_ws_CB (GObject *source_object, GAsyncResult *res, gpointer user_data )
  { struct CLIENT *client = user_data;
    GError *error = NULL;
    printf("%s\n", __func__ );
    client->websocket = soup_session_websocket_connect_finish ( client->connexion, res, &error );
    if (client->websocket)                                                                   /* No limit on incoming packet ! */
     { g_object_set ( G_OBJECT(client->websocket), "max-incoming-payload-size", G_GINT64_CONSTANT(0), NULL );
       g_signal_connect ( client->websocket, "message", G_CALLBACK(Traiter_reception_ws_CB), client );
       g_signal_connect ( client->websocket, "closed",  G_CALLBACK(Traiter_reception_ws_on_closed), client );
       g_signal_connect ( client->websocket, "error",   G_CALLBACK(Traiter_reception_ws_on_error), client );
     }
    else { printf("%s: Error opening Websocket '%s' !\n", __func__, error->message);
           g_error_free (error);
         }
  }
/******************************************************************************************************************************/
/* Connecter_au_serveur_CB: Traite la reponse du serveur a la demande de connexionen                                          */
/* Entrée: les variables traditionnelles de libsous                                                                           */
/* Sortie: l'ihm est mise a jour et les ws sont activées                                                                      */
/******************************************************************************************************************************/
 static void Connecter_au_serveur_CB (SoupSession *session, SoupMessage *msg, gpointer user_data)
  { struct CLIENT *client = user_data;
    GBytes *response_brute;
    gchar *reason_phrase;
    gint status_code;
    gchar chaine[128];
    gsize taille;
    printf("%s\n", __func__ );
    g_object_get ( msg, "status-code", &status_code, "reason-phrase", &reason_phrase, NULL );
    if (status_code != 200)
     { gchar chaine[256];
       g_snprintf(chaine, sizeof(chaine), "Error connecting to server %s: Code %d - %s", client->hostname, status_code, reason_phrase );
       Log(client, chaine);
       Deconnecter_sale(client);
       return;
     }

    GSList *cookies, *liste;
    cookies = soup_cookies_from_response(msg);
    liste = cookies;
    while ( liste )
     { SoupCookie *cookie = liste->data;
       const char *name = soup_cookie_get_name (cookie);
       if (!strcmp(name,"wtd_session"))
        { g_snprintf(client->wtd_session, sizeof(client->wtd_session), "%s", soup_cookie_get_value(cookie) );
          printf("Get session %s\n", client->wtd_session);
          break;
        }
       liste = g_slist_next(liste);
     }
    soup_cookies_free(cookies);

    g_object_get ( msg, "response-body-data", &response_brute, NULL );
    JsonNode *response = Json_get_from_string ( g_bytes_get_data ( response_brute, &taille ) );
    g_snprintf( chaine, sizeof(chaine), "Connected with %s@%s to %s Instance '%s' with %s. Version %s - %s",
                client->username, client->hostname,
                (Json_get_bool(response, "instance_is_master") ? "Master" : "Slave"),
                Json_get_string(response, "instance"),
                (Json_get_bool(response, "ssl") ? "SSL" : "NO SSL"),
                Json_get_string(response, "version"), Json_get_string(response, "message") );
    client->access_level = Json_get_int ( response, "access_level" );
    Log(client, chaine);
    json_node_unref(response);
    Envoi_json_au_serveur( client, "GET", NULL, "/api/histo/alive", Afficher_histo_alive_CB );
    g_snprintf(chaine, sizeof(chaine), "wss://%s:5560/api/live-motifs", client->hostname );
    soup_session_websocket_connect_async ( client->connexion, soup_message_new ( "GET", chaine ),
                                           NULL, NULL, g_cancellable_new(), Traiter_connect_ws_CB, client );
  }
/******************************************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                                               */
/* Entrée: une nom et un password                                                                                             */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                                              */
/******************************************************************************************************************************/
 static void Connecter_au_serveur ( struct CLIENT *client )
  { printf("%s\n", __func__ );
    Log( client, "Trying to connect" );
    client->connexion = soup_session_new();
    g_object_set ( G_OBJECT(client->connexion), "ssl-strict", FALSE, NULL );
    JsonBuilder *builder = Json_create ();
    if (builder == NULL) return;
    Json_add_string ( builder, "username", client->username );
    Json_add_string ( builder, "password", client->password );
    Envoi_json_au_serveur ( client, "POST", builder, "/api/connect", Connecter_au_serveur_CB );
  }
/******************************************************************************************************************************/
/* Identifier: Affiche la fenetre d'identification de l'utilisateur                                                           */
/* Entrée: rien                                                                                                               */
/* Sortie: kedal                                                                                                              */
/******************************************************************************************************************************/
 void Connecter ( struct CLIENT *client )
  { GtkWidget *table, *texte, *content_area, *hbox, *image;
    GtkWidget *Entry_host, *Entry_nom, *Entry_code;

    if (client->connexion) return;
    GtkWidget *fenetre = gtk_dialog_new_with_buttons ( "Identification Required", GTK_WINDOW(client->window),
                                                       GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                       "Annuler", GTK_RESPONSE_CANCEL, "Connecter", GTK_RESPONSE_OK, NULL );

    content_area = gtk_dialog_get_content_area (GTK_DIALOG (fenetre));

    hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
    gtk_box_pack_start (GTK_BOX (content_area), hbox, FALSE, FALSE, 0);

    image = gtk_image_new_from_icon_name ("dialog-question", GTK_ICON_SIZE_DIALOG);
    gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);

    table = gtk_grid_new();                                                                   /* Table des entrys identifiant */
    gtk_box_pack_start( GTK_BOX(hbox), table, TRUE, TRUE, 0 );
    gtk_grid_set_row_spacing( GTK_GRID(table), 5 );
    gtk_grid_set_column_spacing( GTK_GRID(table), 5 );

    texte = gtk_label_new( "Hostname" );
    gtk_grid_attach( GTK_GRID(table), texte, 0, 0, 1, 1 );
    Entry_host = gtk_entry_new();
    gtk_entry_set_text( GTK_ENTRY(Entry_host), g_settings_get_string ( client->settings, "hostname" ) );
    gtk_entry_set_placeholder_text ( GTK_ENTRY(Entry_host), "Votre serveur" );
    gtk_widget_set_tooltip_text ( Entry_host, "Entrez le nom du serveur distant" );
    gtk_entry_set_icon_from_icon_name ( GTK_ENTRY(Entry_host), GTK_ENTRY_ICON_PRIMARY, "system-run" );
    gtk_grid_attach( GTK_GRID(table), Entry_host, 1, 0, 1, 1 );

    texte = gtk_label_new( "Username" );
    gtk_grid_attach( GTK_GRID(table), texte, 0, 1, 1, 1 );
    Entry_nom = gtk_entry_new();
    gtk_entry_set_text( GTK_ENTRY(Entry_nom), g_settings_get_string ( client->settings, "username" ) );
    gtk_entry_set_placeholder_text ( GTK_ENTRY(Entry_nom), "Votre nom" );
    gtk_widget_set_tooltip_text ( Entry_nom, "Entrez votre login de connexion" );
    gtk_entry_set_icon_from_icon_name ( GTK_ENTRY(Entry_nom), GTK_ENTRY_ICON_PRIMARY, "system-users" );
    gtk_grid_attach( GTK_GRID(table), Entry_nom, 1, 1, 1, 1 );

    texte = gtk_label_new( "Password" );
    gtk_grid_attach( GTK_GRID(table), texte, 0, 2, 1, 1 );
    Entry_code = gtk_entry_new();
    gtk_entry_set_visibility( GTK_ENTRY(Entry_code), FALSE );
    gtk_entry_set_text( GTK_ENTRY(Entry_code), g_settings_get_string ( client->settings, "password" ) );
    gtk_entry_set_placeholder_text ( GTK_ENTRY(Entry_code), "Votre mot de passe" );
    gtk_widget_set_tooltip_text ( Entry_code, "Entrez votre mot de passe de connexion" );
    gtk_entry_set_icon_from_icon_name ( GTK_ENTRY(Entry_code), GTK_ENTRY_ICON_PRIMARY, "dialog-password" );
    gtk_grid_attach( GTK_GRID(table), Entry_code, 1, 2, 1, 1 );

    g_signal_connect_swapped( Entry_host, "activate", (GCallback)gtk_widget_grab_focus, Entry_nom );
    g_signal_connect_swapped( Entry_nom,  "activate", (GCallback)gtk_widget_grab_focus, Entry_code );

    gtk_widget_grab_focus( Entry_nom );
    gtk_widget_show_all( hbox );

    if (gtk_dialog_run( GTK_DIALOG(fenetre) ) == GTK_RESPONSE_OK)                      /* Attente de reponse de l'utilisateur */
     { g_snprintf( client->hostname, sizeof(client->hostname), "%s", gtk_entry_get_text( GTK_ENTRY(Entry_host) ) );
       g_snprintf( client->username, sizeof(client->username), "%s", gtk_entry_get_text( GTK_ENTRY(Entry_nom) ) );
       g_snprintf( client->password, sizeof(client->password), "%s", gtk_entry_get_text( GTK_ENTRY(Entry_code) ) );
       g_settings_set_string ( client->settings, "hostname", client->hostname );
       g_settings_set_string ( client->settings, "username", client->username );
       g_settings_set_string ( client->settings, "password", client->password );
       Connecter_au_serveur(client);                                                /* Essai de connexion au serveur Watchdog */
     }
    gtk_widget_destroy( fenetre );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
