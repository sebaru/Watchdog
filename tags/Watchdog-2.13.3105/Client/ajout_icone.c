/**********************************************************************************************************/
/* Client/ajout_icone.c        Addition/Edition d'un icone Watchdog2.0                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 oct 2003 16:27:00 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_icone.c
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
 
 #include "Reseaux.h"
 #include "trame.h"
 #include "liste_icone.h"
 
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *Liste_classe;                      /* GtkTreeView pour la gestion des classes Watchdog */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 static struct TRAME *Trame;
 static GtkWidget *F_ajout;                                            /* Widget de l'interface graphique */
 static GtkWidget *Entry_id;                                  /* Numéro du icone en cours d'édition/ajout */
 static GtkWidget *Entry_nbr_frame;                           /* Numéro du icone en cours d'édition/ajout */
 static GtkWidget *Entry_libelle;                                                     /* Libelle du icone */
 static GtkWidget *Entry_file;                                                        /* Libelle du icone */
 static GtkWidget *Option_classe;            /* Pour le choix d'appartenance du icone à tel ou tel groupe */
 static struct CMD_TYPE_CLASSE Edit_classe;                                 /* Message en cours d'édition */
 static struct CMD_TYPE_ICONE Edit_icone;                                   /* Message en cours d'édition */
 static GList *GListe_classe;                                           /* Liste des libelles des classes */

 static GtkWidget *Entry_classe_id;                       /* Numéro de la classe en cours d'édition/ajout */
 static GtkWidget *Entry_classe_libelle;                       /* Libelle de la classe en cours d'edition */
 static gchar repertoire_courant[120];

/**********************************************************************************************************/
/* Afficher_matrice: Affiche la matrice correspondant au nom de fichier en parametre                      */
/* Entrée: le numéro de l'icone à visualiser                                                              */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Afficher_matrice ( gchar *nom_fichier )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    static struct CMD_TYPE_MOTIF motif;
    gchar frames[30];
    gint nbr;
         /* On "libere" l'ancien motif, mais vi qu'il est 'static', cela revient à mettre ce champ à NULL */
    if ( (struct TRAME_ITEM_MOTIF *)Trame->trame_items )
     { ((struct TRAME_ITEM_MOTIF *)Trame->trame_items->data)->motif = NULL; }
    Trame_effacer_trame( Trame );                                      /* Effacement de la trame actuelle */

    trame_motif = Trame_new_item();
    if (!trame_motif) return;
    memset( &motif, 0, sizeof(struct CMD_TYPE_MOTIF) );
    motif.position_x = TAILLE_ICONE_X/2;
    motif.position_y = TAILLE_ICONE_Y/2;
    motif.angle   = 0.0;
    trame_motif->motif = &motif;                          /* On applique les paramètres du motif 'static' */

    Charger_pixbuf_file( trame_motif, nom_fichier );                         /* Chargement du fichier GIF */
    trame_motif->motif->largeur = trame_motif->gif_largeur;
    trame_motif->motif->hauteur = trame_motif->gif_hauteur;

    Reduire_en_vignette( &motif );
    Trame_ajout_motif_par_item( Trame, trame_motif );                                   /* Ajout du motif */

    nbr = ((struct TRAME_ITEM_MOTIF *)(Trame->trame_items->data))->nbr_images;
    g_snprintf( frames, sizeof(frames), _("%d frame%c"), nbr, (nbr>1 ? 's' : ' ') );
    gtk_entry_set_text( GTK_ENTRY(Entry_nbr_frame), frames );
  }
/**********************************************************************************************************/
/* Afficher_matrice: Affiche la matrice correspondant au nom de fichier en parametre                      */
/* Entrée: le numéro de l'icone à visualiser                                                              */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Changer_matrice ( void )
  { gchar *filename;
    filename = gnome_file_entry_get_full_path ( GNOME_FILE_ENTRY(Entry_file), TRUE );
    g_snprintf( repertoire_courant, sizeof(repertoire_courant), "%s", filename );
    Afficher_matrice( repertoire_courant );
  }
/**********************************************************************************************************/
/* CB_ajouter_editer_icone: Fonction appelée qd on appuie sur un des boutons de l'interface               */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_icone ( GtkDialog *dialog, gint reponse, gboolean edition )
  { gchar *filename;
    g_snprintf( Edit_icone.libelle, sizeof(Edit_icone.libelle),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_libelle) ) );

    if ( ! edition )
     { filename = gnome_file_entry_get_full_path ( GNOME_FILE_ENTRY(Entry_file), TRUE );
       if (!filename)
        { printf("CB_AJouter_editer_icone : file not exist.. abort\n");
          reponse = GTK_RESPONSE_CANCEL;
        }
       else if (strlen (filename) >= sizeof(Edit_icone.nom_fichier) )
        { printf("CB_AJouter_editer_icone : filename too long... abort\n");
          reponse = GTK_RESPONSE_CANCEL;
          g_free(filename);
        }
       else { g_snprintf( Edit_icone.nom_fichier, sizeof(Edit_icone.nom_fichier), "%s", filename );
              g_free(filename);
            }
     }
                 
    switch(reponse)
     { case GTK_RESPONSE_OK:
             { guint classe_icone;
               gchar *classe_choisie;
               GList *choix_utilisateur;
               choix_utilisateur = g_list_nth( GListe_classe,
                                               gtk_option_menu_get_history( GTK_OPTION_MENU(Option_classe) )
                                             );
               if (choix_utilisateur)
                { GtkTreeModel *store;
                  GtkTreeIter iter;
                  gboolean valide;
                  classe_choisie = (gchar *)choix_utilisateur->data;
                  store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_classe) );
                  valide = gtk_tree_model_get_iter_first( store, &iter );
                  while ( valide )
                   { gchar *libelle;
                     gtk_tree_model_get( store, &iter, COLONNE_CLASSE_LIBELLE, &libelle, -1 );
                     if (!strcmp( libelle, classe_choisie ))                              /* Si touvée !! */
                      { gtk_tree_model_get( store, &iter, COLONNE_CLASSE_ID, &classe_icone, -1 );
                        break;
                      }
                     valide = gtk_tree_model_iter_next( store, &iter );
                   }
                }
               else classe_icone = 0;
               Edit_icone.id_classe = classe_icone;
               Envoi_serveur( TAG_ICONE, (edition ? SSTAG_CLIENT_VALIDE_EDIT_ICONE
                                                  : SSTAG_CLIENT_ADD_ICONE),
                                (gchar *)&Edit_icone, sizeof( struct CMD_TYPE_ICONE ) );
             }
            break;
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    g_list_foreach( GListe_classe, (GFunc)g_free, NULL );
    g_list_free( GListe_classe );
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_icone: Ajoute un icone au systeme                                                              */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_icone ( struct CMD_TYPE_ICONE *edit_icone )
  { GtkWidget *frame, *table, *texte, *hboite, *menu;
    GList *item;
    GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    if (edit_icone)
     { memcpy( &Edit_icone, edit_icone, sizeof(struct CMD_TYPE_ICONE) );  /* Save pour utilisation future */
     }
    else memset (&Edit_icone, 0, sizeof(struct CMD_TYPE_ICONE) );                  /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_icone ? _("Edit a icon") : _("Add a icon")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_icone),
                      GINT_TO_POINTER( (edit_icone ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 6, 3, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    texte = gtk_label_new( _("IconID") );                  /* Id unique du icone en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );
    Entry_id = gtk_entry_new();
    gtk_editable_set_editable( GTK_EDITABLE(Entry_id), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_id, 1, 2, 0, 1 );

    texte = gtk_label_new( _("Nbr frames") );              /* Id unique du icone en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Entry_nbr_frame = gtk_entry_new();
    gtk_editable_set_editable( GTK_EDITABLE(Entry_nbr_frame), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_nbr_frame, 1, 2, 1, 2 );

    texte = gtk_label_new( _("IconName") );   /* Création de l'option menu pour le choix du type de icone */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 3, 4 );
    Entry_libelle = gtk_entry_new();
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_libelle, 1, 3, 3, 4 );

    texte = gtk_label_new( _("Class") );      /* Création de l'option menu pour le choix du type de icone */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 4, 5 );
    Option_classe = gtk_option_menu_new();
    gtk_table_attach_defaults( GTK_TABLE(table), Option_classe, 1, 3, 4, 5 );
    menu = gtk_menu_new();
    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_classe) );
    valide = gtk_tree_model_get_iter_first( store, &iter );
    GListe_classe = NULL;
    while ( valide )
     { gchar *libelle;
       gtk_tree_model_get( store, &iter, COLONNE_CLASSE_LIBELLE, &libelle, -1 );
       GListe_classe = g_list_append( GListe_classe, libelle );
       valide = gtk_tree_model_iter_next( store, &iter );
     }
    GListe_classe = g_list_sort( GListe_classe, (GCompareFunc)strcmp );
    item = GListe_classe;
    while(item)
     { gtk_menu_shell_append( GTK_MENU_SHELL(menu),
                              gtk_menu_item_new_with_label( item->data ) );
       item = item->next;
     }
    gtk_option_menu_set_menu( GTK_OPTION_MENU(Option_classe), menu );

    Trame = Trame_creer_trame( TAILLE_ICONE_X, TAILLE_ICONE_Y, "darkgray", 0 );
    gtk_widget_set_usize( Trame->trame_widget, TAILLE_ICONE_X, TAILLE_ICONE_Y );
    gtk_table_attach_defaults( GTK_TABLE(table), Trame->trame_widget, 2, 3, 0, 3 );

    texte = gtk_label_new( _("Filename") );   /* Création de l'option menu pour le choix du type de icone */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 5, 6 );
    Entry_file = gnome_file_entry_new("IconFilename", _("Select a file for icon") );
    gnome_file_entry_set_modal( GNOME_FILE_ENTRY(Entry_file), TRUE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_file, 1, 3, 5, 6 );
    g_signal_connect( Entry_file, "changed", Changer_matrice, NULL );

    if (edit_icone)                                                              /* Si edition d'un icone */
     { GtkTreeModel *store;
       GtkTreeIter iter;
       gboolean valide;
       guint classe;
       gchar chaine[80], *libelle;
       GList *liste;

       gtk_entry_set_text( GTK_ENTRY(Entry_libelle), edit_icone->libelle );
       g_snprintf( chaine, sizeof(chaine), "%d", edit_icone->id );
       gtk_entry_set_text( GTK_ENTRY(Entry_id), chaine );
       gtk_widget_set_sensitive( Entry_file, FALSE );
       g_snprintf( chaine, sizeof(chaine), "%d.gif", edit_icone->id );
       printf("fichier à lire: %s\n", chaine );
       Afficher_matrice( chaine );

       store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_classe) );
       valide = gtk_tree_model_get_iter_first( store, &iter );
       classe = 0;
       libelle = NULL;
       while ( valide )          /* Récupération du libelle de la classe associé à l'ID classe de l'icone */
        { gtk_tree_model_get( store, &iter, COLONNE_CLASSE_ID, &classe, -1 );
          if ( classe == edit_icone->id_classe )                                         /* Si touvée !! */
           { gtk_tree_model_get( store, &iter, COLONNE_CLASSE_LIBELLE, &libelle, -1 );
             break;
           }
          g_free( libelle );
          valide = gtk_tree_model_iter_next( store, &iter );
        }
       classe = 0;
       if (libelle)
        { liste = GListe_classe;       /* Récupération de la position dans l'optionmenu, selon le libelle */
          while (liste)
           { if (!strcmp( liste->data, libelle )) break;
             classe++;
             liste = liste->next;
           }
          g_free(libelle);
        }
       gtk_option_menu_set_history( GTK_OPTION_MENU(Option_classe), classe );
     }
    else { GList *liste;
           guint classe;
           liste = GListe_classe;       /* Récupération de la position dans l'optionmenu, selon le libelle */
           classe = 0;
           while (liste)
            { if (!strcmp( liste->data, _("Default") )) break;
              classe++;
              liste = liste->next;
            }
           gtk_option_menu_set_history( GTK_OPTION_MENU(Option_classe), classe );
           gtk_entry_set_text( GTK_ENTRY(Entry_id), _("?") );
           gtk_widget_set_sensitive( Entry_id, FALSE );
           gnome_file_entry_set_filename ( GNOME_FILE_ENTRY(Entry_file), repertoire_courant );
         }
    gtk_widget_show_all( F_ajout );
  }
/**********************************************************************************************************/
/* CB_ajouter_editer_icone: Fonction appelée qd on appuie sur un des boutons de l'interface               */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_classe ( GtkDialog *dialog, gint reponse, gboolean edition )
  { g_snprintf( Edit_classe.libelle, sizeof(Edit_classe.libelle),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_classe_libelle) ) );
                  
    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_ICONE, (edition ? SSTAG_CLIENT_VALIDE_EDIT_CLASSE
                                                  : SSTAG_CLIENT_ADD_CLASSE),
                                (gchar *)&Edit_classe, sizeof( struct CMD_TYPE_CLASSE ) );
               break;
             }
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_icone: Ajoute un icone au systeme                                                              */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_classe ( struct CMD_TYPE_CLASSE *edit_classe )
  { GtkWidget *frame, *table, *texte, *hboite;
    if (edit_classe)
     { memcpy( &Edit_classe, edit_classe, sizeof(struct CMD_TYPE_CLASSE) );/* Save pour utilisation future */
     }
    else memset (&Edit_classe, 0, sizeof(struct CMD_TYPE_CLASSE) );                 /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_classe ? _("Edit a class") : _("Add a class")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_classe),
                      GINT_TO_POINTER( (edit_classe ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 3, 3, FALSE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    texte = gtk_label_new( _("ClassID") );                  /* Id unique du icone en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );
    Entry_classe_id = gtk_entry_new();
    gtk_editable_set_editable( GTK_EDITABLE(Entry_classe_id), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_classe_id, 1, 2, 0, 1 );

    texte = gtk_label_new( _("Name") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Entry_classe_libelle = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_classe_libelle), NBR_CARAC_LIBELLE_ICONE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_classe_libelle, 1, 3, 1, 2 );

    if (edit_classe)                                                           /* Si edition d'une classe */
     { gchar chaine[10];
       gtk_entry_set_text( GTK_ENTRY(Entry_classe_libelle), edit_classe->libelle );
       g_snprintf( chaine, sizeof(chaine), "%d", edit_classe->id );
       gtk_entry_set_text( GTK_ENTRY(Entry_classe_id), chaine );
     }
    else { gtk_entry_set_text( GTK_ENTRY(Entry_classe_id), _("?") );
           gtk_widget_set_sensitive( Entry_classe_id, FALSE );
         }
    gtk_widget_show_all( F_ajout );
  }
/*--------------------------------------------------------------------------------------------------------*/
