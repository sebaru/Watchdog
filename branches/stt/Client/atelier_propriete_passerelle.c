/**********************************************************************************************************/
/* Client/atelier_propriete_passerelle.c         gestion des passerelles                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 12 mai 2007 21:01:15 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * atelier_propriete_passerelle.c
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
 #include <string.h>
 #include <stdlib.h>

 #include "Reseaux.h"
 #include "trame.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GdkBitmap *Rmask, *Bmask, *Vmask, *Omask, *Jmask;                         /* Des pitites boules ! */
 extern GdkPixmap *Rouge, *Bleue, *Verte, *Orange, *Jaune;
 extern GtkWidget *F_trame;                       /* C'est bien le widget referencant la trame synoptique */

 static GtkWidget *F_propriete;                                      /* Pour acceder la fenetre graphique */
 static GtkWidget *Spin_ctrl_1 = NULL;                          /* Fenetre graphique de choix de passaire */
 static GtkWidget *Spin_ctrl_2 = NULL;                          /* Fenetre graphique de choix de passaire */
 static GtkWidget *Spin_ctrl_3 = NULL;                          /* Fenetre graphique de choix de passaire */
 static GtkWidget *Entry_ctrl_1 = NULL;                         /* Fenetre graphique de choix de passaire */
 static GtkWidget *Entry_ctrl_2 = NULL;                         /* Fenetre graphique de choix de passaire */
 static GtkWidget *Entry_ctrl_3 = NULL;                         /* Fenetre graphique de choix de passaire */

 static struct TRAME_ITEM_PASS *Trame_pass;

/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 void Proto_afficher_mnemo_atelier_pass_2 ( struct CMD_TYPE_MNEMO_BASE *mnemo )
  { gchar chaine[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+10];
    snprintf( chaine, sizeof(chaine), "%s%04d  %s",
              Type_bit_interne_court(mnemo->type), mnemo->num, mnemo->libelle );             /* Formatage */
    gtk_entry_set_text( GTK_ENTRY(Entry_ctrl_1), chaine );
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 void Proto_afficher_mnemo_atelier_pass ( struct CMD_TYPE_MNEMO_BASE *mnemo )
  { gchar chaine[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+10];
    gint num;

    snprintf( chaine, sizeof(chaine), "%s%04d  %s",
              Type_bit_interne_court(mnemo->type), mnemo->num, mnemo->libelle );             /* Formatage */

    num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_ctrl_1) );
    if (mnemo->num == num)
     { gtk_entry_set_text( GTK_ENTRY(Entry_ctrl_1), chaine ); }

    num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_ctrl_2) );
    if (mnemo->num == num)
     { gtk_entry_set_text( GTK_ENTRY(Entry_ctrl_2), chaine ); }

    num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_ctrl_3) );
    if (mnemo->num == num)
     { gtk_entry_set_text( GTK_ENTRY(Entry_ctrl_3), chaine ); }

  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Demander_mnemo_bit_ctrl_1 ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    mnemo.type = MNEMO_MOTIF;
    mnemo.num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_ctrl_1) );
    
    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_PASS,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Demander_mnemo_bit_ctrl_2 ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    mnemo.type = MNEMO_MOTIF;
    mnemo.num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_ctrl_2) );
    
    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_PASS,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Demander_mnemo_bit_ctrl_3 ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    mnemo.type = MNEMO_MOTIF;
    mnemo.num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_ctrl_3) );
    
    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE_PASS,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* CB_editier_propriete_TOR: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_editer_propriete_pass ( GtkDialog *dialog, gint reponse )
  { struct PAGE_NOTEBOOK *page;
   
    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return(TRUE);         /* Verification des contraintes */

    switch(reponse)
     { case GTK_RESPONSE_OK:
            Trame_pass->pass->vignette_activite      = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_ctrl_1) );
            Trame_pass->pass->vignette_secu_bien     = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_ctrl_2) );
            Trame_pass->pass->vignette_secu_personne = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_ctrl_3) );
            break;
       case GTK_RESPONSE_CLOSE:
            break;
     }
    gtk_widget_destroy( F_propriete );
    F_propriete = NULL;
    return(TRUE);
  }
/**********************************************************************************************************/
/* Creer_fenetre_propriete_TOR: Creation de la fenetre d'edition des proprietes TOR                       */
/* Entrée: niet                                                                                           */
/* Sortie: niet                                                                                           */
/**********************************************************************************************************/
 void Editer_propriete_pass ( struct TRAME_ITEM_PASS *trame_pass )
  { GtkWidget *table;
    gint i;

    F_propriete = gtk_dialog_new_with_buttons( _("Edit a gateway"),
                                               GTK_WINDOW(F_client),
                                               GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                               GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                               GTK_STOCK_OK, GTK_RESPONSE_OK,
                                               NULL);
    g_signal_connect( F_propriete, "response",
                      G_CALLBACK(CB_editer_propriete_pass), FALSE );
/*    gtk_widget_set_usize( F_propriete, 600, 100 );*/
    Trame_pass = trame_pass;
/*********************************** Frame de representation du motif actif *******************************/
    table = gtk_table_new( 3, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_propriete)->vbox ), table, FALSE, FALSE, 0 );

    i=0;
    gtk_table_attach_defaults( GTK_TABLE(table), gtk_label_new( _("Vignette ACTIVITE") ), 0, 1, i, i+1 );

    Spin_ctrl_1 = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    g_signal_connect( G_OBJECT(Spin_ctrl_1), "value-changed",
                      G_CALLBACK(Demander_mnemo_bit_ctrl_1), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_ctrl_1, 1, 2, i, i+1 );

    Entry_ctrl_1 = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_ctrl_1), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_ctrl_1, 2, 4, i, i+1 );

    i++;
    gtk_table_attach_defaults( GTK_TABLE(table), gtk_label_new( _("Vignette SECURITE Bien") ), 0, 1, i, i+1 );

    Spin_ctrl_2 = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    g_signal_connect( G_OBJECT(Spin_ctrl_2), "value-changed",
                      G_CALLBACK(Demander_mnemo_bit_ctrl_2), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_ctrl_2, 1, 2, i, i+1 );

    Entry_ctrl_2 = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_ctrl_2), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_ctrl_2, 2, 4, i, i+1 );

    i++;
    gtk_table_attach_defaults( GTK_TABLE(table), gtk_label_new( _("Vignette SECURITE Personne") ), 0, 1, i, i+1 );

    Spin_ctrl_3 = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    g_signal_connect( G_OBJECT(Spin_ctrl_3), "value-changed",
                      G_CALLBACK(Demander_mnemo_bit_ctrl_3), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_ctrl_3, 1, 2, i, i+1 );

    Entry_ctrl_3 = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_ctrl_3), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_ctrl_3, 2, 4, i, i+1 );


    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_ctrl_1), (gdouble) Trame_pass->pass->vignette_activite );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_ctrl_2), (gdouble) Trame_pass->pass->vignette_secu_bien );
    gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_ctrl_3), (gdouble) Trame_pass->pass->vignette_secu_personne );

    Demander_mnemo_bit_ctrl_1();                                           /* Mise à jour des mnemoniques */
    gtk_widget_show_all( F_propriete );
  }
/*--------------------------------------------------------------------------------------------------------*/
