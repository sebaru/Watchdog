/**********************************************************************************************************/
/* Client/Watchdog-client.c        Le client Watchdog v2.0                                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                       ven 15 fév 2008 18:05:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Watchdog-client.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - sebastien
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
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <unistd.h>
 #include <time.h>

 #include "sysconfig.h"
 #include "Erreur.h"
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 GtkWidget *F_client;                                                            /* Widget Fenetre Client */

 GtkWidget *Barre_status;                                                /* Barre d'etat de l'application */

 extern GtkWidget *Label_msg_total;           /* Nombre total de message dans l'historique RAM du serveur */
 extern GtkWidget *Label_msg_def;             /* Nombre total de message dans l'historique RAM du serveur */
 extern GtkWidget *Label_msg_alrm;            /* Nombre total de message dans l'historique RAM du serveur */
 extern GtkWidget *Label_msg_inhib;           /* Nombre total de message dans l'historique RAM du serveur */
 extern GtkWidget *Label_heure;                                       /* pour afficher l'heure du serveur */

 extern struct CMD_EDIT_UTILISATEUR *Edit_util;                       /* L'utilisateur en cours d'edition */

 extern gint Nbr_message;                                                /* Nombre de message de Watchdog */

 struct CLIENT Client_en_cours;                                  /* Identifiant de l'utilisateur en cours */
 struct CONFIG_CLI Config_cli;                                 /* Configuration generale cliente watchdog */
 struct CONNEXION *Connexion = NULL;                                              /* Connexion au serveur */
 SSL_CTX *Ssl_ctx;                                                                        /* Contexte SSL */

 time_t Pulse = -TEMPS_MAX_PULSE;                                    /* Dernier temps de pulse du serveur */
 static void Fermer_client ( void );
 static void A_propos ( GtkWidget *widget, gpointer data );
/****************************** Inclusion des images XPM pour les menus ***********************************/
 #include "boule_rouge.xpm"
 #include "boule_verte.xpm" 
 #include "boule_bleue.xpm" 
 #include "boule_orange.xpm" 
 #include "boule_jaune.xpm" 
 GdkBitmap *Rmask, *Bmask, *Vmask, *Omask, *Jmask;
 GdkPixmap *Rouge, *Bleue, *Verte, *Orange, *Jaune;

 static gboolean Arret = FALSE;
/**************************************** Définition du menu **********************************************/
 GnomeUIInfo Menu_habilitation[]=
  { GNOMEUIINFO_ITEM_STOCK( N_("_Users"), N_("Edit users"),
                            Menu_want_util, GNOME_STOCK_TEXT_BULLETED_LIST ),
    GNOMEUIINFO_ITEM_STOCK( N_("_Groups"), N_("Edit groups"),
                            Menu_want_groupe, GNOME_STOCK_PIXMAP_ATTACH ),
    GNOMEUIINFO_END
  };

 GnomeUIInfo Menu_view[]=
  { GNOMEUIINFO_ITEM_STOCK( N_("_Histo hard"), N_("histo_hard"),
                            Menu_want_histo_hard, GNOME_STOCK_PIXMAP_BOOK_BLUE ),
    GNOMEUIINFO_ITEM_STOCK( N_("_Courbes"), N_("Show Curves"),
                            Menu_want_courbe, GNOME_STOCK_MENU_CONVERT ),
    GNOMEUIINFO_ITEM_STOCK( N_("Histo Courbes"), N_("Histo Curves"),
                            Menu_want_histo_courbe, GNOME_STOCK_PIXMAP_BOOK_YELLOW ),
    GNOMEUIINFO_END
  };
 GnomeUIInfo Menu_serveur[]=
  { GNOMEUIINFO_ITEM_STOCK( N_("Connect"), N_("Connect to server"),
                            Connecter, GNOME_STOCK_PIXMAP_EXEC ),
    GNOMEUIINFO_ITEM_STOCK( N_("Stop"), N_("Stop the connexion"),
                            Deconnecter, GNOME_STOCK_PIXMAP_CLOSE ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_SUBTREE(N_("_View"), Menu_view),
    GNOMEUIINFO_ITEM_STOCK( N_("Change Password"), N_("Change the password"),
                            Changer_password, GNOME_STOCK_PIXMAP_PROPERTIES ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_SUBTREE(N_("_Habilitations"), Menu_habilitation),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK( N_("_Quit"), N_("Disconnect and quit"), Fermer_client, GNOME_STOCK_PIXMAP_EXIT ),
    GNOMEUIINFO_END
  };
 GnomeUIInfo Menu_option[]=
  { GNOMEUIINFO_END
  };
 GnomeUIInfo Menu_aide[]=
  { GNOMEUIINFO_ITEM_STOCK( N_("A propos.."), N_("Signatures"), A_propos, GNOME_STOCK_PIXMAP_ABOUT ),
    GNOMEUIINFO_END
  };
 GnomeUIInfo Menu_principal[]=
  { GNOMEUIINFO_SUBTREE(N_("_Serveur"), Menu_serveur),
    GNOMEUIINFO_SUBTREE(N_("_Options"), Menu_option),
    GNOMEUIINFO_SUBTREE(N_("_Aide"), Menu_aide),
    GNOMEUIINFO_END
  };

 GnomeUIInfo Barre_outils[]=
  { GNOMEUIINFO_ITEM_STOCK( N_("Connect"), N_("Connect to server"),
                            Connecter, GNOME_STOCK_PIXMAP_EXEC ),
    GNOMEUIINFO_ITEM_STOCK( N_("Stop"), N_("Stop the connexion"),
                            Deconnecter, GNOME_STOCK_PIXMAP_CLOSE ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK( N_("Supervision"), N_("Graphical Supervision"),
                            Menu_want_supervision, GNOME_STOCK_PIXMAP_SEARCH ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK( N_("Courbes"), N_("Curves"),
                            Menu_want_courbe, GNOME_STOCK_MENU_CONVERT ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK( N_("Quit"), N_("Stop and quit"),
                            Fermer_client, GNOME_STOCK_PIXMAP_EXIT ),
    GNOMEUIINFO_END
  };
  
/**********************************************************************************************************/
/* A_propos: Presentation du programme                                                                    */
/* Entrée: widgte, data  (callback inutilisées)                                                           */
/**********************************************************************************************************/
 static void A_propos ( GtkWidget *widget, gpointer data )
  { GtkWidget *F_a_propos;
    const gchar *auteurs[]=
     { "Sébastien Lefevre",
       "Bruno Lefevre", NULL };
    F_a_propos = gnome_about_new ( PROGRAMME, VERSION, "Sous licence GPL",
                                   _("Client side of project Watchdog"),
                                   auteurs, NULL, NULL, NULL );
    gtk_widget_show( F_a_propos );
  }

/**********************************************************************************************************/
/* Traitement_signaux: Gestion principale des signaux de Watchdog config                                  */
/* Entrée: Numero du signal attrapé                                                                       */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Traitement_signaux ( int num )
  { switch (num)
     { case SIGINT :
       case SIGTERM: Fermer_client(); break;
       case SIGPIPE: Deconnecter(); break;
       case SIGIO  : Info_n( Config_cli.log, DEBUG_SIGNAUX, "Signal IO !", num );
                     Ecouter_serveur();
                     break;
       default: Info_n( Config_cli.log, DEBUG_SIGNAUX, "Signal non gere", num );
     }
    printf("Fin traitement signaux\n");
  }
/**********************************************************************************************************/
/* Fermer_client: Arreter le client et sortie du programme                                                */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Fermer_client ( void )
  { printf("Fermer_client ! \n");
    Deconnecter();
    Arret = TRUE;
  }
/**********************************************************************************************************/
/* Main: Fonction principale du programme du client Watchdog                                              */
/* Entrée: argc, argv pour gnome/gtk                                                                      */
/* Sortie: 0                                                                                              */
/**********************************************************************************************************/
 int main ( int argc, char *argv[] )
  { gint help, port, debug_level;
    struct sigaction sig;
    GnomeClient *client;
    GnomeProgram *prg;
    gchar *file;
    struct poptOption Options[]= 
     { { "port", 'p',       POPT_ARG_INT,
         &port,             0, _("Port to listen to"), "PORT" },
       { "debug",'d',       POPT_ARG_INT,
         &debug_level,      0, _("Debug level"), "LEVEL" },
       { "conffile", 'c',   POPT_ARG_STRING,
         &file,             0, _("Configuration file"), "FILE" },
       { "help", 'h',       POPT_ARG_NONE,
         &help,             0, _("Help"), NULL },
       POPT_TABLEEND
     };

    file = NULL;
    port           = -1;
    debug_level    = -1;
    help           = 0;

    prg = gnome_program_init( PROGRAMME, VERSION, LIBGNOMEUI_MODULE, argc, argv,            /* Init gnome */
                              GNOME_PARAM_POPT_TABLE, Options, GNOME_PARAM_NONE );
    client = gnome_master_client();
    g_signal_connect( GTK_OBJECT( client ), "save_yourself", GTK_SIGNAL_FUNC( gtk_main_quit ), NULL );

    F_client = gnome_app_new( PROGRAMME, TITRE_F_CONFIG );                      /* Création de la fenetre */
    g_signal_connect( G_OBJECT( F_client ), "delete_event",
                      G_CALLBACK( Fermer_client ), NULL );
    g_signal_connect( G_OBJECT( F_client ), "destroy",
                      G_CALLBACK( Fermer_client ), NULL );
    gnome_app_create_menus( GNOME_APP(F_client), Menu_principal );
    gnome_app_create_toolbar( GNOME_APP(F_client), Barre_outils );
    Barre_status = gnome_appbar_new( TRUE, TRUE, GNOME_PREFERENCES_USER );
    gnome_app_set_statusbar( GNOME_APP( F_client ), Barre_status );

    gtk_widget_set_size_request (F_client, 1024, 800);
    gtk_widget_realize( F_client );                                     /* Pour pouvoir creer les pixmaps */
    Rouge  = gdk_pixmap_create_from_xpm_d( F_client->window, &Rmask, NULL, boule_rouge_xpm );
    Verte  = gdk_pixmap_create_from_xpm_d( F_client->window, &Vmask, NULL, boule_verte_xpm );
    Bleue  = gdk_pixmap_create_from_xpm_d( F_client->window, &Bmask, NULL, boule_bleue_xpm );
    Orange = gdk_pixmap_create_from_xpm_d( F_client->window, &Omask, NULL, boule_orange_xpm );
    Jaune  = gdk_pixmap_create_from_xpm_d( F_client->window, &Jmask, NULL, boule_jaune_xpm );
    gnome_app_set_contents( GNOME_APP(F_client), Creer_boite_travail() );

    if (chdir( g_get_home_dir() ))                                  /* Positionnement à la racine du home */
     { printf( "Chdir %s failed\n", g_get_home_dir() ); exit(EXIT_ERREUR); }

    if (chdir( REPERTOIR_CONF ))                                    /* Positionnement à la bonne position */
     { mkdir ( REPERTOIR_CONF, 0700 );
       chdir ( REPERTOIR_CONF );
     }

    Lire_config_cli( &Config_cli, file );                           /* Lecture sur le fichier ~/.watchdog */
    if (port!=-1)        Config_cli.port        = port;                /* Priorite à la ligne de commande */
    if (debug_level!=-1) Config_cli.debug_level = debug_level;

    Config_cli.log = Info_init( "Watchdog_client", Config_cli.debug_level );       /* Init msgs d'erreurs */

    Info( Config_cli.log, DEBUG_INFO, _("Start") );
    Print_config_cli( &Config_cli );

    Ssl_ctx = Init_ssl();
    if (!Ssl_ctx)
     { Info( Config_cli.log, DEBUG_CRYPTO, _("Can't initialise SSL") );
     }
    
    sig.sa_handler = Traitement_signaux;
    sigemptyset(&sig.sa_mask);
    sig.sa_flags = 0;
    sigaction( SIGTERM, &sig, NULL );                                       /* Arret Prématuré (logiciel) */
    sigaction( SIGINT,  &sig, NULL );                                       /* Arret Prématuré (logiciel) */
    sigaction( SIGPIPE,  &sig, NULL );                                      /* Arret Prématuré (logiciel) */

    Client_en_cours.gids = NULL;                     /* Initialisation de la structure de client en cours */
    Client_en_cours.id   = 0;
    Client_en_cours.mode = INERTE;

    gtk_widget_show_all( F_client );                               /* Affichage de le fenetre de controle */
    while ( Arret != TRUE )
     { gtk_main_iteration_do ( FALSE );
       if (Client_en_cours.mode >= ENVOI_IDENT) Ecouter_serveur();
       usleep(1000);
     }

    if (Client_en_cours.gids) g_list_free(Client_en_cours.gids);
    SSL_CTX_free(Ssl_ctx);
    Info( Config_cli.log, DEBUG_INFO, _("Stopped") );
    exit(0);
  }
/*--------------------------------------------------------------------------------------------------------*/
