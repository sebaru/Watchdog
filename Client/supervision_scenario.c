/**********************************************************************************************************/
/* Client/supervision_scenario.c        Configuration des scenarios de Watchdog v2.0                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 10 aoû 2008 11:13:33 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * supervision_scenario.c
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

 GtkWidget *Liste_on;                         /* GtkTreeView pour la gestion des scenarios Watchdog */
                                 /* non static car reutilisable par l'utilitaire d'ajout d'un utilisateur */
 static GtkWidget *F_ajout;                                            /* Widget de l'interface graphique */
 static gboolean En_cours_M;
 static struct CMD_TYPE_MOTIF *Motif;                 /* Motif dont les scenarios sont en cours d'edition */
 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

 enum
  {  COLONNE_ID,
     COLONNE_ACTIF,
     COLONNE_BITM,
     COLONNE_BITM_STRING,
     COLONNE_LIBELLE,
     NBR_COLONNE
  };
 extern GdkColor COULEUR_FOND[];
 extern GdkColor COULEUR_TEXTE[];

 static GtkWidget *F_ajout2;                                           /* Widget de l'interface graphique */
 static GtkWidget *Spin_heure;                             /* Numéro du scenario en cours d'édition/ajout */
 static GtkWidget *Spin_minute;                            /* Numéro du scenario en cours d'édition/ajout */
 static GtkWidget *Entry_lib;                                                      /* Libelle du scenario */
 static GtkWidget *Check_jours[8];                                                /* valide quels jours ? */
 static GtkWidget *Check_mois[13];                                                 /* valide quels mois ? */
 static GtkWidget *Check_actif;                                  /* Le scenario est-il actif ou inhibe ?? */
 static struct CMD_TYPE_SCENARIO Edit_sce;                                  /* Message en cours d'édition */

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 static void Menu_supervision_effacer_scenario ( void );
 static void Menu_supervision_editer_scenario ( void );
 static void Menu_supervision_ajouter_scenario_M ( void );
 static void Menu_supervision_ajouter_scenario_S ( void );

 static GnomeUIInfo Menu_popup_select[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add Start"), NULL, Menu_supervision_ajouter_scenario_M, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Add Stop"), NULL, Menu_supervision_ajouter_scenario_S, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Edit"), NULL, Menu_supervision_editer_scenario, GNOME_STOCK_PIXMAP_OPEN ),
    GNOMEUIINFO_SEPARATOR,
    GNOMEUIINFO_ITEM_STOCK ( N_("Remove"), NULL, Menu_supervision_effacer_scenario, GNOME_STOCK_PIXMAP_CLEAR ),
    GNOMEUIINFO_END
  };

 static GnomeUIInfo Menu_popup_nonselect[]=
  { GNOMEUIINFO_ITEM_STOCK ( N_("Add Start"), NULL, Menu_supervision_ajouter_scenario_M, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_ITEM_STOCK ( N_("Add Stop"), NULL, Menu_supervision_ajouter_scenario_S, GNOME_STOCK_PIXMAP_ADD ),
    GNOMEUIINFO_END
  };

/**********************************************************************************************************/
/* CB_ajouter_editer_scenario: Fonction appelée qd on appuie sur un des boutons de l'interface             */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_scenario ( GtkDialog *dialog, gint reponse, gboolean edition )
  { g_snprintf( Edit_sce.libelle, sizeof(Edit_sce.libelle),
                "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
    Edit_sce.actif     = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_actif) );
    Edit_sce.ts_jour   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[0]) );
    Edit_sce.lundi     = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[1]) );
    Edit_sce.mardi     = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[2]) );
    Edit_sce.mercredi  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[3]) );
    Edit_sce.jeudi     = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[4]) );
    Edit_sce.vendredi  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[5]) );
    Edit_sce.samedi    = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[6]) );
    Edit_sce.dimanche  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[7]) );
    Edit_sce.ts_mois   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[0]) );
    Edit_sce.janvier   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[1]) );
    Edit_sce.fevrier   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[2]) );
    Edit_sce.mars      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[3]) );
    Edit_sce.avril     = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[4]) );
    Edit_sce.mai       = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[5]) );
    Edit_sce.juin      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[6]) );
    Edit_sce.juillet   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[7]) );
    Edit_sce.aout      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[8]) );
    Edit_sce.septembre = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[9]) );
    Edit_sce.octobre   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[10]) );
    Edit_sce.novembre  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[11]) );
    Edit_sce.decembre  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[12]) );
    Edit_sce.heure     = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_heure) );
    Edit_sce.minute    = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_minute) );
    Edit_sce.bit_m     = (En_cours_M ? Motif->bit_clic : Motif->bit_clic2);

    switch(reponse)
     { case GTK_RESPONSE_OK:
             { Envoi_serveur( TAG_SUPERVISION, (edition ? SSTAG_CLIENT_SUP_VALIDE_EDIT_SCENARIO
                                                        : SSTAG_CLIENT_SUP_ADD_SCENARIO),
                                (gchar *)&Edit_sce, sizeof( struct CMD_TYPE_SCENARIO ) );
               break;
             }
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(F_ajout2);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Ajouter_scenario: Ajoute un scenario au systeme                                                        */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_supervision_ajouter_editer_scenario ( struct CMD_TYPE_SCENARIO *edit_sce )
  { GtkWidget *frame, *table, *texte, *hboite;

    if (edit_sce)
     { memcpy( &Edit_sce, edit_sce, sizeof(struct CMD_TYPE_SCENARIO) );   /* Save pour utilisation future */
       if (Edit_sce.bit_m == Motif->bit_clic) En_cours_M = TRUE;
       else En_cours_M = FALSE;
     }
    else memset (&Edit_sce, 0, sizeof(struct CMD_TYPE_SCENARIO) );                 /* Sinon RAZ structure */

    F_ajout2 = gtk_dialog_new_with_buttons( (edit_sce ? _("Edit a scenario") : _("Add a scenario")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout2, "response",
                      G_CALLBACK(CB_ajouter_editer_scenario),
                      GINT_TO_POINTER( (edit_sce ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout2)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 11, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    Check_actif = gtk_check_button_new_with_label( _("Enable") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_actif, 0, 2, 0, 1 );

    Check_jours[0] = gtk_check_button_new_with_label( _("Tous les jours") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_jours[0], 0, 1, 1, 2 );

    Check_jours[1] = gtk_check_button_new_with_label( _("Lundi") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_jours[1], 0, 1, 2, 3 );

    Check_jours[2] = gtk_check_button_new_with_label( _("Mardi") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_jours[2], 0, 1, 3, 4 );

    Check_jours[3] = gtk_check_button_new_with_label( _("Mercredi") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_jours[3], 0, 1, 4, 5 );

    Check_jours[4] = gtk_check_button_new_with_label( _("Jeudi") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_jours[4], 0, 1, 5, 6 );

    Check_jours[5] = gtk_check_button_new_with_label( _("Vendredi") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_jours[5], 0, 1, 6, 7 );

    Check_jours[6] = gtk_check_button_new_with_label( _("Samedi") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_jours[6], 0, 1, 7, 8 );

    Check_jours[7] = gtk_check_button_new_with_label( _("Dimanche") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_jours[7], 0, 1, 8, 9 );

    Check_mois[0] = gtk_check_button_new_with_label( _("Tous les mois") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[0], 2, 4, 1, 2 );

    Check_mois[1] = gtk_check_button_new_with_label( _("Janvier") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[1], 2, 3, 2, 3 );

    Check_mois[2] = gtk_check_button_new_with_label( _("Fevrier") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[2], 2, 3, 3, 4 );

    Check_mois[3] = gtk_check_button_new_with_label( _("Mars") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[3], 2, 3, 4, 5 );

    Check_mois[4] = gtk_check_button_new_with_label( _("Avril") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[4], 2, 3, 5, 6 );

    Check_mois[5] = gtk_check_button_new_with_label( _("Mai") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[5], 2, 3, 6, 7 );

    Check_mois[6] = gtk_check_button_new_with_label( _("Juin") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[6], 2, 3, 7, 8 );

    Check_mois[7] = gtk_check_button_new_with_label( _("Juillet") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[7], 3, 4, 2, 3 );

    Check_mois[8] = gtk_check_button_new_with_label( _("Aout") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[8], 3, 4, 3, 4 );

    Check_mois[9] = gtk_check_button_new_with_label( _("Septembre") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[9], 3, 4, 4, 5 );

    Check_mois[10] = gtk_check_button_new_with_label( _("Octobre") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[10], 3, 4, 5, 6 );

    Check_mois[11] = gtk_check_button_new_with_label( _("Novembre") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[11], 3, 4, 6, 7 );

    Check_mois[12] = gtk_check_button_new_with_label( _("Decembre") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_mois[12], 3, 4, 7, 8 );

    texte = gtk_label_new( _("Heure") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 9, 10 );
    Spin_heure = gtk_spin_button_new_with_range( 0, 23, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_heure, 1, 2, 9, 10 );

    texte = gtk_label_new( _("Minute") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, 9, 10 );
    Spin_minute = gtk_spin_button_new_with_range( 0, 59, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_minute, 3, 4, 9, 10 );

    texte = gtk_label_new( _("Libelle") );                                      /* Le scenario en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 10, 11 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_SCENARIO );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, 10, 11 );

    if (edit_sce)                                                              /* Si edition d'un scenario */
     { /*syn = Rechercher_syn_par_num( Nom_syn, msg->num_synoptique );*/
       Edit_sce.id = edit_sce->id;
       gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_sce->libelle );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), edit_sce->actif );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_jours[0]), edit_sce->ts_jour );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_jours[1]), edit_sce->lundi );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_jours[2]), edit_sce->mardi );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_jours[3]), edit_sce->mercredi );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_jours[4]), edit_sce->jeudi );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_jours[5]), edit_sce->vendredi );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_jours[6]), edit_sce->samedi );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_jours[7]), edit_sce->dimanche );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[0]), edit_sce->ts_mois );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[1]), edit_sce->janvier );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[2]), edit_sce->fevrier );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[3]), edit_sce->mars );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[4]), edit_sce->avril );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[5]), edit_sce->mai );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[6]), edit_sce->juin );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[7]), edit_sce->juillet );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[8]), edit_sce->aout );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[9]), edit_sce->septembre );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[10]), edit_sce->octobre );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[11]), edit_sce->novembre );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_mois[12]), edit_sce->decembre );

       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_heure), edit_sce->heure );
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_minute), edit_sce->minute );
       gtk_widget_grab_focus( Entry_lib );
     }
    else { gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), TRUE );
           gtk_widget_grab_focus( Entry_lib );
         }
    gtk_widget_show_all(F_ajout2);                                   /* Affichage de l'interface complète */
  }
/**********************************************************************************************************/
/* Ajouter_scenario: Ajoute un scenario au systeme                                                        */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_supervision_ajouter_scenario_M ( void )
  { En_cours_M = TRUE;
    Menu_supervision_ajouter_editer_scenario ( NULL );
  }
/**********************************************************************************************************/
/* Ajouter_scenario: Ajoute un scenario au systeme                                                        */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_supervision_ajouter_scenario_S ( void )
  { En_cours_M = FALSE;
    Menu_supervision_ajouter_editer_scenario ( NULL );
  }
/**********************************************************************************************************/
/* CB_effacer_scenario: Fonction appelée qd on appuie sur un des boutons de l'interface                    */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_effacer_scenario ( GtkDialog *dialog, gint reponse, gboolean edition )
  { struct CMD_TYPE_SCENARIO rezo_scenario;
    GtkTreeSelection *selection;
    GtkTreeModel *store;
    GList *lignes;
    GtkTreeIter iter;

    switch(reponse)
     { case GTK_RESPONSE_YES:
            selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_on) );
            store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_on) );
            lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
            while ( lignes )
             { gchar *libelle;
               gtk_tree_model_get_iter( store, &iter, lignes->data );  /* Recuperation ligne selectionnée */
               gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_scenario.id, -1 );       /* Recup du id */
               gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

               memcpy( &rezo_scenario.libelle, libelle, sizeof(rezo_scenario.libelle) );
               g_free( libelle );

               Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_SUP_DEL_SCENARIO,
                             (gchar *)&rezo_scenario, sizeof(struct CMD_TYPE_SCENARIO) );
               gtk_tree_selection_unselect_iter( selection, &iter );
               lignes = lignes->next;
             }
            g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
            g_list_free (lignes);                                                   /* Liberation mémoire */
            break;
       default: break;
     }
    gtk_widget_destroy( GTK_WIDGET(dialog) );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Menu_effacer_scenario: Retrait des scenarios selectionnés                                                */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_supervision_effacer_scenario ( void )
  { GtkTreeSelection *selection;
    GtkWidget *dialog;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_on) );
    nbr = gtk_tree_selection_count_selected_rows( selection );

    if (!nbr) return;                                                       /* Si rien n'est selectionné */

    dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                      GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                      GTK_MESSAGE_WARNING, GTK_BUTTONS_YES_NO,
                                      _("Do you want to delete %d scenario%c ?"), nbr, (nbr>1 ? 's' : ' ') );
    g_signal_connect( dialog, "response",
                      G_CALLBACK(CB_effacer_scenario), NULL );
    gtk_widget_show_all( dialog );
  }
/**********************************************************************************************************/
/* Menu_editer_scenario: Demande d'edition du scenario selectionné                                          */
/* Entrée: rien                                                                                           */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static void Menu_supervision_editer_scenario ( void )
  { GtkTreeSelection *selection;
    struct CMD_TYPE_SCENARIO rezo_scenario;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;
    gchar *libelle;
    guint nbr;

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_on) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_on) );

    nbr = gtk_tree_selection_count_selected_rows( selection );
    if (!nbr) return;                                                        /* Si rien n'est selectionné */


    lignes = gtk_tree_selection_get_selected_rows ( selection, NULL );
    gtk_tree_model_get_iter( store, &iter, lignes->data );             /* Recuperation ligne selectionnée */
    gtk_tree_model_get( store, &iter, COLONNE_ID, &rezo_scenario.id, -1 );                  /* Recup du id */
    gtk_tree_model_get( store, &iter, COLONNE_LIBELLE, &libelle, -1 );

    memcpy( &rezo_scenario.libelle, libelle, sizeof(rezo_scenario.libelle) );
    g_free( libelle );
    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_SUP_EDIT_SCENARIO,
                  (gchar *)&rezo_scenario, sizeof(struct CMD_TYPE_SCENARIO) );
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
  }
/**********************************************************************************************************/
/* Gerer_popup_scenario: Gestion du menu popup quand on clique droite sur la liste des scenarios            */
/* Entrée: la liste(widget), l'evenement bouton, et les data                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static gboolean Gerer_popup_scenario ( GtkWidget *widget, GdkEventButton *event, gpointer data )
  { static GtkWidget *Popup_select=NULL, *Popup_nonselect=NULL;
    GtkTreeSelection *selection;
    gboolean ya_selection;
    GtkTreePath *path;
    gint cellx, celly;
    if (!event) return(FALSE);

    if ( event->button == 3 )                                                         /* Gestion du popup */
     { if (!Popup_select)    Popup_select = gnome_popup_menu_new( Menu_popup_select );
       if (!Popup_nonselect) Popup_nonselect = gnome_popup_menu_new( Menu_popup_nonselect );

       ya_selection = FALSE;
       selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_on) );  /* On recupere selection */
       if (gtk_tree_selection_count_selected_rows(selection) == 0)
        { gtk_tree_view_get_path_at_pos ( GTK_TREE_VIEW(Liste_on), event->x, event->y,
                                          &path, NULL, &cellx, &celly );
          
          if (path)
           { gtk_tree_selection_select_path( selection, path );
             gtk_tree_path_free( path );
             ya_selection = TRUE;
           }
        } else ya_selection = TRUE;                              /* ya bel et bien qqchose de selectionné */

       gnome_popup_menu_do_popup_modal( (ya_selection ? Popup_select : Popup_nonselect),
                                        NULL, NULL, event, NULL, F_client );
       return(TRUE);
     }
    else if (event->type == GDK_2BUTTON_PRESS && event->button == 1 )                   /* Double clic ?? */
     { Menu_supervision_editer_scenario(); }
    return(FALSE);
  }
/**********************************************************************************************************/
/* CB_ajouter_editer_scenario: Fonction appelée qd on appuie sur un des boutons de l'interface            */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_fenetre_scenario ( GtkDialog *dialog, gint reponse, gboolean edition )
  { switch(reponse)
     { case GTK_RESPONSE_CLOSE:
       default:              break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Creer_page_scenario: Creation de la page du notebook consacrée aux scenarios watchdog                  */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Creer_fenetre_scenario( struct CMD_TYPE_MOTIF *motif )
  { GtkWidget *scroll, *hboite, *frame, *boite, *bouton, *separateur;
    GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkListStore *store;

    Motif = motif;
    F_ajout = gtk_dialog_new_with_buttons( Motif->libelle,
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE,
                                           NULL);
    gtk_widget_set_size_request (F_ajout, 800, 600);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_fenetre_scenario), NULL );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );
   
/***************************************** La liste des scenarios ******************************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_box_pack_start( GTK_BOX(hboite), scroll, TRUE, TRUE, 0 );

    store = gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,                                          /* Id */
                                              G_TYPE_BOOLEAN,                                    /* actif */
                                              G_TYPE_UINT,                                       /* bit_m */
                                              G_TYPE_STRING,                              /* bit_m string */
                                              G_TYPE_STRING                                    /* Libelle */
                               );

    Liste_on = gtk_tree_view_new_with_model ( GTK_TREE_MODEL(store) );        /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_on) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );
    gtk_container_add( GTK_CONTAINER(scroll), Liste_on );

    renderer = gtk_cell_renderer_toggle_new();                             /* Colonne de l'id du scenario */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Enable"), renderer,
                                                         "active", COLONNE_ACTIF,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_ACTIF);                  /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_on), colonne );

    renderer = gtk_cell_renderer_text_new();                            /* Colonne du libelle de scenario */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Mode"), renderer,
                                                         "text", COLONNE_BITM_STRING,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_BITM_STRING);            /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_on), colonne );

    renderer = gtk_cell_renderer_text_new();                            /* Colonne du libelle de scenario */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Scenario"), renderer,
                                                         "text", COLONNE_LIBELLE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_LIBELLE);                /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW (Liste_on), colonne );

    /*gtk_tree_view_set_reorderable( GTK_TREE_VIEW(Liste_on), TRUE );*/
    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(Liste_on), TRUE );                /* Pour faire beau */

    g_signal_connect( G_OBJECT(Liste_on), "button_press_event",             /* Gestion du menu popup */
                      G_CALLBACK(Gerer_popup_scenario), Liste_on );
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */
    
/************************************ Les boutons de controles ********************************************/
    boite = gtk_vbox_new( FALSE, 6 );
    gtk_box_pack_start( GTK_BOX(hboite), boite, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_OPEN );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_supervision_editer_scenario), NULL );

    bouton = gtk_button_new_with_label( _("Add Start") );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_supervision_ajouter_scenario_M), NULL );

    bouton = gtk_button_new_with_label( _("Add Stop") );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_supervision_ajouter_scenario_S), NULL );

    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(boite), separateur, FALSE, FALSE, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_box_pack_start( GTK_BOX(boite), bouton, FALSE, FALSE, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Menu_supervision_effacer_scenario), NULL );

    gtk_widget_show_all( F_ajout );
  }
/**********************************************************************************************************/
/* Rafraichir_visu_scenario: Rafraichissement d'un scenario la liste à l'écran                              */
/* Entrée: une reference sur le scenario                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rafraichir_visu_scenario( GtkTreeIter *iter, struct CMD_TYPE_SCENARIO *scenario )
  { GtkTreeModel *store;
    gchar chaine[20];

    store = gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_on) );             /* Acquisition du modele */
printf("Rafraichir visu_scenario %d %d:%d\n", scenario->id, scenario->heure, scenario->minute);
    if (scenario->bit_m == Motif->bit_clic)
     { g_snprintf( chaine, sizeof(chaine), "Start %02d:%02d", scenario->heure, scenario->minute );
     }
    else 
     { g_snprintf( chaine, sizeof(chaine), "Stop  %02d:%02d", scenario->heure, scenario->minute );
     }
    gtk_list_store_set ( GTK_LIST_STORE(store), iter,
                         COLONNE_ID, scenario->id,
                         COLONNE_ACTIF, scenario->actif,
                         COLONNE_BITM, scenario->bit_m,
                         COLONNE_BITM_STRING, chaine,
                         COLONNE_LIBELLE, scenario->libelle,
                         -1
                       );
printf("Rafraichir visu_scenario fin\n");
  }
/**********************************************************************************************************/
/* Afficher_un_scenario: Ajoute un scenario dans la liste des scenarios                                   */
/* Entrée: une reference sur le scenario                                                                  */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_supervision_afficher_un_scenario( struct CMD_TYPE_SCENARIO *scenario )
  { GtkListStore *store;
    GtkTreeIter iter;

    if (!F_ajout) return;

    store = GTK_LIST_STORE(gtk_tree_view_get_model( GTK_TREE_VIEW(Liste_on) ));
    gtk_list_store_append ( store, &iter );                                      /* Acquisition iterateur */
    Rafraichir_visu_scenario ( &iter, scenario );
  }
/**********************************************************************************************************/
/* Cacher_un_scenario: Enleve un scenario de la liste des scenarios                                       */
/* Entrée: une reference sur le scenario                                                                  */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_supervision_cacher_un_scenario( struct CMD_TYPE_SCENARIO *scenario )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_on) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == scenario->id )
        { printf("elimination scenario %s\n", scenario->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { gtk_list_store_remove( GTK_LIST_STORE(store), &iter ); }
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_scenario: Rafraichissement du scenario en parametre                                 */
/* Entrée: une reference sur le scenario                                                                  */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_supervision_rafraichir_un_scenario( struct CMD_TYPE_SCENARIO *scenario )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    gint id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_on) );
    valide = gtk_tree_model_get_iter_first( store, &iter );

    while ( valide )
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if ( id == scenario->id )
        { printf("maj scenario %s\n", scenario->libelle );
          break;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }

    if (valide)
     { Rafraichir_visu_scenario( &iter, scenario ); }
  }
/*--------------------------------------------------------------------------------------------------------*/
