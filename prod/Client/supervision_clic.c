/**********************************************************************************************************/
/* Client/supervision_clic.c        Gestion des clics sur les motifs dans la fenetre supervision          */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 20 mai 2004 13:39:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * supervision_clic.c
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

 #include <gnome.h>
 #include <sys/time.h>
 
 #include "Reseaux.h"
 #include "client.h"
 #include "Config_cli.h"
 #include "trame.h"

 extern struct CLIENT Client;                           /* Identifiant de l'utilisateur en cours */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

 static GnomeUIInfo Menu_popup[]=
  { /*GNOMEUIINFO_ITEM_STOCK ( N_("Program"), NULL, Envoyer_action_programme, GNOME_STOCK_PIXMAP_EXEC ),*/
    GNOMEUIINFO_END
  };
 static struct TRAME_ITEM_MOTIF *appui = NULL;
 static struct TRAME_ITEM_CAPTEUR *appui_capteur = NULL;
 static struct TRAME_ITEM_CAMERA_SUP *appui_camera_sup = NULL;

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern struct TRAME *Trame_supervision;                               /* La trame de fond de supervision */
/**********************************************************************************************************/
/* Envoyer_action_immediate: Envoi d'une commande Mxxx au serveur                                         */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 static void Envoyer_action_immediate ( struct TRAME_ITEM_MOTIF *trame_motif )
  { struct CMD_ETAT_BIT_CLIC bit_clic;
    if (trame_motif->motif->type_gestion == TYPE_BOUTON)
     { bit_clic.num = trame_motif->motif->bit_clic + (trame_motif->num_image / 3);
     }
    else
     { bit_clic.num = trame_motif->motif->bit_clic; }

    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_ACTION_M,
                   (gchar *)&bit_clic, sizeof(struct CMD_ETAT_BIT_CLIC) );
    printf("Envoi M%d = 1 au serveur \n", bit_clic.num );
  }
/**********************************************************************************************************/
/* Clic_sur_motif_supervision: Appelé quand un evenement est capté sur un motif de la trame supervision   */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 void Clic_sur_motif_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                   GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif )
  { static GtkWidget *Popup=NULL;
    if (!(trame_motif && event)) return;

    if (event->type == GDK_BUTTON_PRESS)
     { if (trame_motif->motif->type_gestion == TYPE_BOUTON)
        { printf("Appui sur bouton num_image=%d\n", trame_motif->num_image );
          if ( (trame_motif->num_image % 3) == 1 )
           { Trame_choisir_frame( trame_motif, trame_motif->num_image + 1,      /* Frame 2: bouton appuyé */
                                  trame_motif->rouge,
                                  trame_motif->vert,
                                  trame_motif->bleu );
           }
        }
       appui = trame_motif;
     }
    else if (event->type == GDK_BUTTON_RELEASE && appui)
     {
printf("release !\n");
       if ( ((GdkEventButton *)event)->button == 3 &&
             trame_motif->motif->type_dialog == ACTION_PROGRAMME)                     /* Gestion du popup */
        { if (!Popup) Popup = gnome_popup_menu_new( Menu_popup );
          gnome_popup_menu_do_popup_modal( Popup, NULL, NULL, (GdkEventButton *)event, NULL, F_client );
        }

       if (appui->motif->type_gestion == TYPE_BOUTON)                 /* On met la frame 1: bouton relevé */
        { if ( (trame_motif->num_image % 3) == 2 )
           { Trame_choisir_frame( appui, trame_motif->num_image - 1, appui->rouge, appui->vert, appui->bleu );
             Envoyer_action_immediate( trame_motif );
           }
        }
       else if ( ((GdkEventButton *)event)->button == 1)           /* Release sur le motif qui a été appuyé ?? */
        { switch( trame_motif->motif->type_dialog )
           { case ACTION_SANS:      printf("action sans !!\n");
                                    break;
             case ACTION_IMMEDIATE: printf("action immediate !!\n");
                                    Envoyer_action_immediate( trame_motif );
                                    break;
/*             case ACTION_PROGRAMME: printf("action programme !!\n");
                                    Envoyer_action_programme( trame_motif );
                                    break;*/
/*             case ACTION_DIFFERE:
             case ACTION_REPETE:
                                    break;*/
             default: printf("Clic_sur_motif_supervision: type dialog inconnu\n");
           }
        }
       appui = NULL;                          /* L'action est faite, on ne selectionne donc plus le motif */
     }
  }
/**********************************************************************************************************/
/* Clic_sur_motif_supervision: Appelé quand un evenement est capté sur un motif de la trame supervision   */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 void Clic_sur_camera_sup_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                        GdkEvent *event, struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup )
  { if (!(trame_camera_sup && event)) return;

    if (event->type == GDK_BUTTON_PRESS)
     { appui_camera_sup = trame_camera_sup; }
    else if (event->type == GDK_BUTTON_RELEASE && appui_camera_sup)
     { if ( ((GdkEventButton *)event)->button == 1)           /* Release sur le motif qui a été appuyé ?? */
        { gint pid;

          printf( "Clic_sur_camera_sup_supervision : Lancement d'un Gst %s\n",
                  trame_camera_sup->camera_sup->location );
          pid = fork();
          if (pid<0) return;
          else if (!pid)                                             /* Lancement de la ligne de commande */
           {
#ifdef bouh
             gchar chaine[sizeof(trame_camera_sup->camera_sup->location)+6];
#endif

             execlp( "vlc", "vlc", trame_camera_sup->camera_sup->location, NULL );

#ifdef bouh
             g_snprintf( chaine, sizeof(chaine), "uri=%s", trame_camera_sup->camera_sup->location );
             execlp( "gst-launch-0.10", "gst-launch-0.10", "playbin", chaine, NULL );
#endif
             printf("AUDIO: Lancement gst-launch failed\n");
             _exit(0);
           }
        }
       appui_camera_sup = NULL;               /* L'action est faite, on ne selectionne donc plus le motif */
     }
  }
/**********************************************************************************************************/
/* Clic_sur_capteur_supervision_action: Appelé pour lancer un firefox sur la periode en parametre         */
/* Entrée: période d'affichage                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 static void Clic_capteur_supervision_action ( gchar *period )
  { gint pid;
    printf( "Clic_sur_capteur_supervision : Lancement d'un firefox type=%d, num=%d\n",
             appui_capteur->capteur->type, appui_capteur->capteur->bit_controle );
    pid = fork();
    if (pid<0) return;
    else if (!pid)                                             /* Lancement de la ligne de commande */
     { gchar chaine[256];
       g_snprintf( chaine, sizeof(chaine),
                  "http://%s/watchdog/graph.php?type=%d&num=%d&period=%s",
                   Client.host, appui_capteur->capteur->type, appui_capteur->capteur->bit_controle, period );
       execlp( "firefox", "firefox", chaine, NULL );
       printf("Lancement de firefox failed\n");
       _exit(0);
     }
  }
/**********************************************************************************************************/
/* Clic_sur_capteur_supervision_xxx: Appelé quand le menu last xxx est cliqué                             */
/* Entrée: rien                                                                                           */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 static void Clic_capteur_supervision_hour ( void )
  { Clic_capteur_supervision_action ( "hour" ); }
 static void Clic_capteur_supervision_day ( void )
  { Clic_capteur_supervision_action ( "day" ); }
 static void Clic_capteur_supervision_week ( void )
  { Clic_capteur_supervision_action ( "week" ); }
 static void Clic_capteur_supervision_month ( void )
  { Clic_capteur_supervision_action ( "month" ); }
 static void Clic_capteur_supervision_year ( void )
  { Clic_capteur_supervision_action ( "year" ); }
/**********************************************************************************************************/
/* Clic_sur_capteur_supervision: Appelé quand un evenement est capté sur un motif de la trame supervision   */
/* Entrée: une structure Event                                                                            */
/* Sortie :rien                                                                                           */
/**********************************************************************************************************/
 void Clic_sur_capteur_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                     GdkEvent *event, struct TRAME_ITEM_CAPTEUR *trame_capteur )
  { static GtkWidget *Popup = NULL;
    static GnomeUIInfo Popup_comment[]=
     { GNOMEUIINFO_ITEM_STOCK( N_("Last Hour"),  NULL, Clic_capteur_supervision_hour, GNOME_STOCK_PIXMAP_BOOK_OPEN ),
       GNOMEUIINFO_ITEM_STOCK( N_("Last Day"),   NULL, Clic_capteur_supervision_day, GNOME_STOCK_PIXMAP_BOOK_RED ),
       GNOMEUIINFO_ITEM_STOCK( N_("Last Week"),  NULL, Clic_capteur_supervision_week, GNOME_STOCK_PIXMAP_BOOK_GREEN ),
       GNOMEUIINFO_ITEM_STOCK( N_("Last Month"), NULL, Clic_capteur_supervision_month, GNOME_STOCK_PIXMAP_BOOK_BLUE ),
       GNOMEUIINFO_ITEM_STOCK( N_("Last Year"),  NULL, Clic_capteur_supervision_year, GNOME_STOCK_PIXMAP_BOOK_YELLOW ),
       GNOMEUIINFO_END
     };

    if (!(trame_capteur && event)) return;
    appui_capteur = trame_capteur;

    if (event->type == GDK_BUTTON_PRESS)
     { if ( ((GdkEventButton *)event)->button == 1)           /* Release sur le motif qui a été appuyé ?? */
        { Clic_capteur_supervision_hour ();		 }
       else if (event->button.button == 3)
        { if (!Popup) Popup = gnome_popup_menu_new( Popup_comment );                     /* Creation menu */
          gnome_popup_menu_do_popup_modal( Popup, NULL, NULL, (GdkEventButton *)event, NULL, F_client );
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/

