/**********************************************************************************************************/
/* Client/atelier_ajout_comment.c     Gestion des commentaires pour Watchdog                              */
/* Projet WatchDog version 1.5     Gestion d'habitat                         jeu 03 fév 2005 16:25:06 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * atelier_ajout_comment.c
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
 #include <string.h>
 #include <stdlib.h>

 #include "Reseaux.h"
 #include "trame.h"
 #include "Config_cli.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

 static GtkWidget *F_ajout_comment = NULL;                   /* Fenetre graphique de choix de commentaire */
 static GtkWidget *Entry_comment;                                           /* Commentaire proprement dit */
 static GtkWidget *Couleur_comment=NULL;                             /* Definit la couleur du commentaire */
 static GtkWidget *Font_comment=NULL;                                 /* Definit la police du commentaire */

 enum
  { STYLE_TITRE, STYLE_SOUS_TITRE, STYLE_ANNOTATION };

/**********************************************************************************************************/
/* Id_vers_trame_motif: Conversion d'un id motif en sa reference TRAME                                    */
/* Entrée: Un id motif                                                                                    */
/* sortie: un struct TRAME_ITEM_MOTIF                                                                     */
/**********************************************************************************************************/
 struct TRAME_ITEM_COMMENT *Id_vers_trame_comment ( struct TYPE_INFO_ATELIER *infos, gint id )
  { GList *liste;
    liste = infos->Trame_atelier->trame_items;
    while( liste )
     { if ( *(gint *)(liste->data) == TYPE_COMMENTAIRE &&
            ((struct TRAME_ITEM_COMMENT *)(liste->data))->comment->id == id ) break;
       else liste = liste->next;
     }
    if (!liste)
     { printf("Id_vers_trame_comment: item %d non trouvé\n", id );
       return(NULL);
     }
    return( (struct TRAME_ITEM_COMMENT *)(liste->data) );
  }
/**********************************************************************************************************/
/* Changer_commentaire: Synchronise le commentaire avec le preview gnome                                  */
/* Entrée: widget/data                                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Changer_commentaire ( GtkWidget *widget, gpointer data )
  { gnome_font_picker_set_preview_text( GNOME_FONT_PICKER(Font_comment), 
                                        gtk_entry_get_text( GTK_ENTRY(Entry_comment) ) );
  }
/**********************************************************************************************************/
/* Mise_en_style_prog: Charge un style préprogrammé dans le code source                                   */
/* Entrée: widget, un numero de style                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Mise_en_style_prog( GtkWidget *widget, gpointer data )
  { int style;
    style = GPOINTER_TO_INT(data);
    switch(style)
     { case STYLE_TITRE:
            gnome_font_picker_set_font_name( GNOME_FONT_PICKER(Font_comment),
                                             "Bitstream Vera Serif Italic 22" );
            gnome_color_picker_set_i8( GNOME_COLOR_PICKER( Couleur_comment ),
                                       0, 0, 0, 0 );                                       /* R G B Alpha */
            break;
       case STYLE_SOUS_TITRE:
            gnome_font_picker_set_font_name( GNOME_FONT_PICKER(Font_comment),
                                             "Bitstream Vera Serif Bold Italic 12" );
            gnome_color_picker_set_i8( GNOME_COLOR_PICKER( Couleur_comment ),
                                       0, 0, 0, 0 );
            break;
       case STYLE_ANNOTATION:
            gnome_font_picker_set_font_name( GNOME_FONT_PICKER(Font_comment),
                                             "Bitstream Vera Serif Italic 10" );
            gnome_color_picker_set_i8( GNOME_COLOR_PICKER( Couleur_comment ),
                                       0, 0, 0, 0 );
            break;
     }
  }
/**********************************************************************************************************/
/* CB_editier_propriete_TOR: Fonction appelée qd on appuie sur un des boutons de l'interface              */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajout_comment ( GtkDialog *dialog, gint reponse )
  { struct CMD_TYPE_COMMENT add_comment;
    struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;

    page = Page_actuelle();                                               /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return(TRUE);         /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;         /* Pointeur sur les infos de la page atelier */

    switch(reponse)
     { case GTK_RESPONSE_OK: g_snprintf( add_comment.libelle, sizeof(add_comment.libelle), "%s",
                                         gtk_entry_get_text (GTK_ENTRY(Entry_comment)) );
                             g_snprintf( add_comment.font, sizeof(add_comment.font), "%s",
                                         gnome_font_picker_get_font_name(GNOME_FONT_PICKER(Font_comment)) );
                             gnome_color_picker_get_i8 ( GNOME_COLOR_PICKER(Couleur_comment),
                                                         &add_comment.rouge,
                                                         &add_comment.vert,
                                                         &add_comment.bleu, NULL );
                             add_comment.position_x = TAILLE_SYNOPTIQUE_X/2;
                             add_comment.position_y = TAILLE_SYNOPTIQUE_Y/2;                            
                             add_comment.syn_id = infos->syn.id;

                             Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_COMMENT,
                                           (gchar *)&add_comment, sizeof(struct CMD_TYPE_COMMENT) );
                             printf("Requete d'ajout de commentaire envoyée au serveur....\n");
                             return(TRUE);                                /* On laisse la fenetre ouverte */
                                 break;
       case GTK_RESPONSE_CLOSE: break;
     }
    gtk_widget_destroy( F_ajout_comment );
    F_ajout_comment = NULL;
    return(TRUE);
  }
/**********************************************************************************************************/
/* Commenter: Met en route le processus permettant de commenter un synoptique                             */
/* Entrée: widget/data                                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Creer_fenetre_ajout_commentaire ( void )
  { GtkWidget *hboite, *table, *texte, *bouton;
    if (F_ajout_comment) return;

    F_ajout_comment = gtk_dialog_new_with_buttons( _("Add a comment"),
                                             GTK_WINDOW(F_client),
                                             GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                             GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                             GTK_STOCK_ADD, GTK_RESPONSE_OK,
                                             NULL);
    g_signal_connect( F_ajout_comment, "response",
                      G_CALLBACK(CB_ajout_comment), NULL );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout_comment)->vbox ), hboite, TRUE, TRUE, 0 );
    
    table = gtk_table_new( 3, 3, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

/******************************* Boutons de mise en style rapide ******************************************/
    bouton = gtk_button_new_with_label("Titre");
    gtk_table_attach_defaults( GTK_TABLE(table), bouton, 0, 1, 0, 1 );
    gtk_signal_connect( GTK_OBJECT(bouton), "clicked",
                        GTK_SIGNAL_FUNC(Mise_en_style_prog), GINT_TO_POINTER(STYLE_TITRE) );

    bouton = gtk_button_new_with_label("Sous-titre");
    gtk_table_attach_defaults( GTK_TABLE(table), bouton, 1, 2, 0, 1 );
    gtk_signal_connect( GTK_OBJECT(bouton), "clicked",
                        GTK_SIGNAL_FUNC(Mise_en_style_prog), GINT_TO_POINTER(STYLE_SOUS_TITRE) );

    bouton = gtk_button_new_with_label("Annotation");
    gtk_table_attach_defaults( GTK_TABLE(table), bouton, 2, 3, 0, 1 );
    gtk_signal_connect( GTK_OBJECT(bouton), "clicked",
                        GTK_SIGNAL_FUNC(Mise_en_style_prog), GINT_TO_POINTER(STYLE_ANNOTATION) );

/************************************* Entrys de commande *************************************************/
    texte = gtk_label_new("Commentaire");
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Entry_comment = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_comment), NBR_CARAC_COMMENTAIRE );

    gtk_entry_select_region( GTK_ENTRY(Entry_comment), 0, NBR_CARAC_COMMENTAIRE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_comment, 1, 3, 1, 2 );
    gtk_signal_connect( GTK_OBJECT(Entry_comment), "changed",
                        GTK_SIGNAL_FUNC(Changer_commentaire), NULL );

    Font_comment = gnome_font_picker_new();
    gnome_font_picker_set_title( GNOME_FONT_PICKER( Font_comment ), "Police du commentaire" );
    gnome_font_picker_set_preview_text( GNOME_FONT_PICKER(Font_comment), "Bonjour !" );
    gtk_table_attach_defaults( GTK_TABLE(table), Font_comment, 1, 2, 2, 3 );

    Couleur_comment = gnome_color_picker_new();
    gtk_table_attach_defaults( GTK_TABLE(table), Couleur_comment, 2, 3, 2, 3 );

    gtk_widget_show_all( F_ajout_comment );
  }
/**********************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                      */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_comment_atelier( struct CMD_TYPE_COMMENT *rezo_comment )
  { struct TRAME_ITEM_COMMENT *trame_comment;
    struct TYPE_INFO_ATELIER *infos;
    struct CMD_TYPE_COMMENT *comment;
        
    infos = Rechercher_infos_atelier_par_id_syn ( rezo_comment->syn_id );
    comment = (struct CMD_TYPE_COMMENT *)g_try_malloc0( sizeof(struct CMD_TYPE_COMMENT) );
    if (!comment)
     { return;
     }

    memcpy( comment, rezo_comment, sizeof(struct CMD_TYPE_COMMENT) );

    trame_comment = Trame_ajout_commentaire ( TRUE, infos->Trame_atelier, comment );
    trame_comment->groupe_dpl = Nouveau_groupe();                 /* Numéro de groupe pour le deplacement */
    g_signal_connect( G_OBJECT(trame_comment->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_comment), trame_comment );
    g_signal_connect( G_OBJECT(trame_comment->item_groupe), "button-release-event",
                      G_CALLBACK(Clic_sur_comment), trame_comment );
    g_signal_connect( G_OBJECT(trame_comment->item_groupe), "enter-notify-event",
                      G_CALLBACK(Clic_sur_comment), trame_comment );
    g_signal_connect( G_OBJECT(trame_comment->item_groupe), "leave-notify-event",
                      G_CALLBACK(Clic_sur_comment), trame_comment );
    g_signal_connect( G_OBJECT(trame_comment->item_groupe), "motion-notify-event",
                      G_CALLBACK(Clic_sur_comment), trame_comment );
  }
/**********************************************************************************************************/
/* Cacher_un_message: Enleve un message de la liste des messages                                          */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_cacher_un_comment_atelier( struct CMD_TYPE_COMMENT *comment )
  { struct TRAME_ITEM_COMMENT *trame_comment;
    struct TYPE_INFO_ATELIER *infos;
        
    infos = Rechercher_infos_atelier_par_id_syn ( comment->syn_id );
    trame_comment = Id_vers_trame_comment( infos, comment->id );
    printf("Proto_cacher_un_comment_atelier debut: ID=%d %p\n", comment->id, trame_comment );
    if (!trame_comment) return;
    Deselectionner( infos, (struct TRAME_ITEM *)trame_comment );/* Au cas ou il aurait été selectionné... */
    goo_canvas_item_remove( trame_comment->item_groupe );
    if (trame_comment->comment) g_free(trame_comment->comment);          /* On libere la mémoire associée */
    g_free(trame_comment);
    infos->Trame_atelier->trame_items = g_list_remove( infos->Trame_atelier->trame_items, trame_comment );
    printf("Proto_cacher_un_comment_atelier fin..\n");
  }
/*--------------------------------------------------------------------------------------------------------*/
