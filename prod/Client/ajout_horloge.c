/******************************************************************************************************************************/
/* Client/ajout_horloge.c        Addition/Edition d'un horloge Watchdog2.0                                                    */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    14.08.2018 22:02:49 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * ajout_horloge.c
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
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <unistd.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <sys/wait.h>
 #include <fcntl.h>

 #include "Reseaux.h"
/********************************************** Définitions des prototypes programme *****************************************/
 #include "protocli.h"
 #include "client.h"
 #include "Config_cli.h"

 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                                                      /* Configuration generale watchdog */
 extern struct CLIENT Client;                                                        /* Identifiant de l'utilisateur en cours */

 static GtkWidget *F_ajout;                                                                /* Widget de l'interface graphique */
 static GtkWidget *Spin_heure;                                                  /* Numéro du horloge en cours d'édition/ajout */
 static GtkWidget *Spin_minute;                                                 /* Numéro du horloge en cours d'édition/ajout */
 static struct CMD_TYPE_MNEMO_FULL Horloge;                                                            /* Message en cours d'édition */

/******************************************************************************************************************************/
/* CB_ajouter_editer_horloge: Fonction appelée qd on appuie sur un des boutons de l'interface                                 */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                                                   */
/* sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 static gboolean CB_ajouter_editer_horloge ( GtkDialog *dialog, gint reponse, gboolean edition )
  { gint index;

    Horloge.mnemo_horloge.heure      = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_heure) );
    Horloge.mnemo_horloge.minute     = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_minute) );

    switch(reponse)
     { case GTK_RESPONSE_APPLY:
             { Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_VALIDE_EDIT_HORLOGE,
                              (gchar *)&Horloge, sizeof( struct CMD_TYPE_MNEMO_FULL ) );
             }
            break;
       case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_ADD_HORLOGE,
                              (gchar *)&Horloge, sizeof( struct CMD_TYPE_MNEMO_FULL ) );
             }
            break;
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Menu_ajouter_editer_horloge: Ajoute ou edite un horloge au systeme                                                         */
/* Entrée: La structure du horloge a editer ou a creer                                                                        */
/* sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Menu_ajouter_editer_horloge ( struct CMD_TYPE_MNEMO_FULL *edit_horloge, gint id_mnemo)
  { GtkWidget *frame, *table, *texte, *hboite;
    gint cpt, i;

    if (edit_horloge)
     { memcpy( &Horloge, edit_horloge, sizeof(struct CMD_TYPE_MNEMO_FULL) );                             /* Save pour utilisation future */
       F_ajout = gtk_dialog_new_with_buttons( _("Edit a horloge"), GTK_WINDOW(F_client),
                                              GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_STOCK_COPY, GTK_RESPONSE_OK,
                                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                              GTK_STOCK_APPLY, GTK_RESPONSE_APPLY,
                                              NULL);
     }
    else
     { memset (&Horloge, 0, sizeof(struct CMD_TYPE_MNEMO_FULL) );                                             /* Sinon RAZ structure */
       Horloge.mnemo_base.id = id_mnemo;
       printf("%s: Add horloge for mnemo %d\n", __func__, id_mnemo );
       F_ajout = gtk_dialog_new_with_buttons( _("Add a horloge"), GTK_WINDOW(F_client),
                                              GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                              GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                              GTK_STOCK_ADD, GTK_RESPONSE_OK,
                                              NULL);
     }

    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_horloge),
                      GINT_TO_POINTER( (edit_horloge ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                                                   /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 2, 2, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    i = 0;
    texte = gtk_label_new( _("Heure") );                 /* Id unique du horloge en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_heure = gtk_spin_button_new_with_range( 0, 23, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_heure, 1, 2, i, i+1 );

    i++;
    texte = gtk_label_new( _("Minute") );                 /* Id unique du horloge en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, i, i+1 );
    Spin_minute = gtk_spin_button_new_with_range( 0, 59, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_minute, 1, 2, i, i+1 );

    if (edit_horloge)                                                                              /* Si edition d'un horloge */
     { gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_heure), edit_horloge->mnemo_horloge.heure );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_minute), edit_horloge->mnemo_horloge.minute );
       gtk_widget_grab_focus( Spin_heure );
     }
    else { gtk_widget_grab_focus( Spin_heure );
         }
    gtk_widget_show_all(F_ajout);                                                        /* Affichage de l'interface complète */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
