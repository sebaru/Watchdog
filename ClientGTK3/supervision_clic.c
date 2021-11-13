/**********************************************************************************************************/
/* Client/supervision_clic.c        Gestion des clics sur les motifs dans la fenetre supervision          */
/* Projet WatchDog version 3.0       Gestion d'habitat                      jeu 20 mai 2004 13:39:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * supervision_clic.c
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

 #include <gnome.h>
 #include <sys/time.h>

 #include "Reseaux.h"
 #include "client.h"
 #include "Config_cli.h"
 #include "trame.h"

 extern struct CLIENT Client;                                                        /* Identifiant de l'utilisateur en cours */
 extern GList *Liste_pages;                                                       /* Liste des pages ouvertes sur le notebook */
 extern GtkWidget *Notebook;                                                             /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                                              /* Configuration generale cliente watchdog */

 static struct TRAME_ITEM_MOTIF *appui = NULL;

 static GtkWidget *F_set_registre;                                                         /* Widget de l'interface graphique */
 static GtkWidget *Spin_valeur;                                                                         /* Valeur du registre */
/****************************************** Définitions des prototypes programme **********************************************/
 #include "protocli.h"

 extern struct TRAME *Trame_supervision;                               /* La trame de fond de supervision */
/******************************************************************************************************************************/
/* Envoyer_action_immediate: Envoi d'une commande Mxxx au serveur                                                             */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_action_immediate ( struct TRAME_ITEM_MOTIF *trame_motif )
  { struct CMD_SET_BIT_INTERNE bit;
    bit.type = MNEMO_MONOSTABLE;
    if (trame_motif->motif->bit_clic != -1)
     { if (trame_motif->motif->type_gestion == TYPE_BOUTON)
        { bit.num = trame_motif->motif->bit_clic + (trame_motif->num_image / 3); }
       else
        { bit.num = trame_motif->motif->bit_clic; }
       printf("Envoi M%d = 1 au serveur \n", bit.num );
     }
    if(strlen(trame_motif->motif->clic_tech_id)>0)
     { bit.num = -1;
       bit.type= MNEMO_MONOSTABLE;
       g_snprintf( bit.tech_id, sizeof(bit.tech_id), "%s", trame_motif->motif->clic_tech_id );
       g_snprintf( bit.acronyme, sizeof(bit.acronyme), "%s", trame_motif->motif->clic_acronyme );
       printf("Envoi _M %s:%s = 1 au serveur \n", bit.tech_id, bit.acronyme );
     }
    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_SET_BIT_INTERNE,
                   (gchar *)&bit, sizeof(struct CMD_SET_BIT_INTERNE) );
  }
/******************************************************************************************************************************/
/* CB_Envoyer_action_confirme: Callback appelé lors de l'appui sur un des boutons de la fenetre de validation                 */
/* Entrée: la reponse de l'utilisateur et une structure MOTIF                                                                 */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 static gboolean CB_Envoyer_action_confirme ( GtkDialog *dialog, gint reponse, struct TRAME_ITEM_MOTIF *trame_motif )
  { switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoyer_action_immediate( trame_motif );
             }
            break;
       case GTK_RESPONSE_CANCEL:
       default: break;
     }
    gtk_widget_destroy(GTK_WIDGET(dialog));
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Envoyer_action_confirme : Affiche une fenetre de confirmation avant d'envoyer une commande Mxxx au serveur                 */
/* Entrée: Le motif qui vient d'etre cliqué                                                                                   */
/* sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_action_confirme ( struct TRAME_ITEM_MOTIF *trame_motif )
  { GtkWidget *dialog;

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client), GTK_DIALOG_DESTROY_WITH_PARENT,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_OK_CANCEL,
                                      trame_motif->motif->libelle );

    g_signal_connect( dialog, "response", G_CALLBACK(CB_Envoyer_action_confirme), trame_motif );

    gtk_widget_show_all(dialog);                                                         /* Affichage de l'interface complète */
  }
/******************************************************************************************************************************/
/* Clic_sur_motif_supervision: Appelé quand un evenement est capté sur un motif de la trame supervision                       */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Clic_sur_motif_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                   GdkEvent *event, struct TRAME_ITEM_MOTIF *trame_motif )
  { if (!(trame_motif && event)) return;

    if (event->type == GDK_BUTTON_PRESS)
     { appui = trame_motif;                                                      /* Sauvegarde en attendant la release button */
       if (trame_motif->motif->type_gestion == TYPE_BOUTON && (trame_motif->last_clic + 1 <= time(NULL)) )
        { printf("Appui sur bouton num_image=%d\n", trame_motif->num_image );
          if ( trame_motif->num_image == 1 )
           { Trame_choisir_frame( trame_motif, trame_motif->num_image + 1,                          /* Frame 2: bouton appuyé */
                                  trame_motif->rouge,
                                  trame_motif->vert,
                                  trame_motif->bleu );
           }
          time(&appui->last_clic);                                                         /* Mémorisation de la date de clic */
        }
     }
    else if (event->type == GDK_BUTTON_RELEASE && appui)
     { if (appui->motif->type_gestion == TYPE_BOUTON)                                     /* On met la frame 1: bouton relevé */
        { if ( trame_motif->num_image == 2 )
           { Trame_choisir_frame( appui, trame_motif->num_image - 1, appui->rouge, appui->vert, appui->bleu );
             switch ( trame_motif->motif->type_dialog )
              { case ACTION_IMMEDIATE: Envoyer_action_immediate( trame_motif ); break;
                case ACTION_CONFIRME : Envoyer_action_confirme( trame_motif );  break;
                default: break;
              }
           }
        }
       else if ( ((GdkEventButton *)event)->button == 1)                          /* Release sur le motif qui a été appuyé ?? */
        { switch( trame_motif->motif->type_dialog )
           { case ACTION_SANS:      printf("action sans !!\n");
                                    break;
             case ACTION_IMMEDIATE: printf("action immediate !!\n");
                                    Envoyer_action_immediate( trame_motif );
                                    break;
/*             case ACTION_CONFIRME: printf("action programme !!\n");
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
/******************************************************************************************************************************/
/* CB_Cadran_Set_registre: Fonction appelée qd on appuie sur un des boutons de l'interface de modification de consigne        */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                                                   */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 static gboolean CB_Cadran_Set_registre ( GtkDialog *dialog, gint reponse, gint regnum )
  { struct CMD_SET_BIT_INTERNE Set_bit;
    Set_bit.type = MNEMO_REGISTRE;
    Set_bit.num = regnum;
    Set_bit.valeur = gtk_spin_button_get_value_as_float( GTK_SPIN_BUTTON(Spin_valeur) );

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_SET_BIT_INTERNE,
                              (gchar *)&Set_bit, sizeof( struct CMD_SET_BIT_INTERNE ) );
             }
            break;
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(F_set_registre);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Clic_cadran_set_registre: Appelé quand l'utilisateur souhaite entrer une consigne                                          */
/* Entrée: le cadran, au travers de son lien trame_cadran                                                                     */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 static void Clic_cadran_set_registre ( struct TRAME_ITEM_CADRAN *trame_cadran )
  { GtkWidget *frame, *table, *texte, *hboite;
    gint i;
    F_set_registre = gtk_dialog_new_with_buttons( _("Changer un registre"), GTK_WINDOW(F_client),
                                                  GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                  GTK_STOCK_OK, GTK_RESPONSE_OK,
                                                  NULL);
/*    g_signal_connect( F_set_registre, "response",
                      G_CALLBACK(CB_Cadran_Set_registre),
                      GINT_TO_POINTER(trame_cadran->cadran->bit_controle) );*/

    frame = gtk_frame_new("Settings");                                                   /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_set_registre)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 1, 2, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    i = 0;
    texte = gtk_label_new( "Nouvelle consigne" );                                   /* Création du champ de nouvelle consigne */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );

    Spin_valeur = gtk_spin_button_new_with_range( -1000, +1000, 0.1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_valeur, 1, 2, i, i+1 );
    gtk_widget_show_all(F_set_registre);                                                 /* Affichage de l'interface complète */
  }
/******************************************************************************************************************************/
/* Clic_sur_cadran_supervision: Appelé quand un evenement est capté sur un motif de la trame supervision                      */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Clic_sur_cadran_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                     GdkEvent *event, struct TRAME_ITEM_CADRAN *trame_cadran )
  { static GtkWidget *Popup = NULL;

    if (!(trame_cadran && event)) return;

    if (event->type == GDK_BUTTON_PRESS)
     { if ( ((GdkEventButton *)event)->button == 1 )                              /* Release sur le motif qui a été appuyé ?? */
        { if (trame_cadran->cadran->type == MNEMO_REGISTRE)
           { Clic_cadran_set_registre ( trame_cadran ); }
        }
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
