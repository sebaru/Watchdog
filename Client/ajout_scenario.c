/**********************************************************************************************************/
/* Client/ajout_scenario.c        Addition/Edition d'un scenario Watchdog2.0                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 10 aoû 2008 11:31:23 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ajout_scenario.c
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
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 static GtkWidget *F_ajout;                                            /* Widget de l'interface graphique */
 static GtkWidget *Spin_heure;                             /* Numéro du scenario en cours d'édition/ajout */
 static GtkWidget *Spin_bitm;                              /* Numéro du scenario en cours d'édition/ajout */
 static GtkWidget *Entry_bitm;                             /* Numéro du scenario en cours d'édition/ajout */
 static GtkWidget *Spin_minute;                            /* Numéro du scenario en cours d'édition/ajout */
 static GtkWidget *Entry_lib;                                                       /* Libelle du scenario */
 static GtkWidget *Check_jours[8];                                                /* valide quels jours ? */
 static GtkWidget *Check_mois[13];                                                 /* valide quels mois ? */
 static GtkWidget *Check_actif;                                  /* Le scenario est-il actif ou inhibe ?? */
 static struct CMD_TYPE_SCENARIO Edit_sce;                                  /* Message en cours d'édition */

/**********************************************************************************************************/
/* CB_ajouter_editer_scenario: Fonction appelée qd on appuie sur un des boutons de l'interface             */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_scenario ( GtkDialog *dialog, gint reponse, gboolean edition )
  { switch(reponse)
     { case GTK_RESPONSE_OK:
             { if (edition)
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
                  Edit_sce.bit_m     = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bitm) );

                  Envoi_serveur( TAG_SCENARIO, SSTAG_CLIENT_VALIDE_EDIT_SCENARIO,
                                (gchar *)&Edit_sce, sizeof( struct CMD_TYPE_SCENARIO ) );
                }
               else
                { struct CMD_TYPE_SCENARIO new_sce;
                  g_snprintf( new_sce.libelle, sizeof(new_sce.libelle),
                              "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
                  new_sce.actif     = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_actif) );
                  new_sce.ts_jour   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[0]) );
                  new_sce.lundi     = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[1]) );
                  new_sce.mardi     = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[2]) );
                  new_sce.mercredi  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[3]) );
                  new_sce.jeudi     = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[4]) );
                  new_sce.vendredi  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[5]) );
                  new_sce.samedi    = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[6]) );
                  new_sce.dimanche  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_jours[7]) );
                  new_sce.ts_mois   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[0]) );
                  new_sce.janvier   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[1]) );
                  new_sce.fevrier   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[2]) );
                  new_sce.mars      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[3]) );
                  new_sce.avril     = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[4]) );
                  new_sce.mai       = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[5]) );
                  new_sce.juin      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[6]) );
                  new_sce.juillet   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[7]) );
                  new_sce.aout      = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[8]) );
                  new_sce.septembre = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[9]) );
                  new_sce.octobre   = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[10]) );
                  new_sce.novembre  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[11]) );
                  new_sce.decembre  = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_mois[12]) );
                  new_sce.heure     = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_heure) );
                  new_sce.minute    = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_minute) );
                  new_sce.bit_m     = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_bitm) );

                  Envoi_serveur( TAG_SCENARIO, SSTAG_CLIENT_ADD_SCENARIO,
                                (gchar *)&new_sce, sizeof( struct CMD_TYPE_SCENARIO ) );
                }
             }
            break;
       case GTK_RESPONSE_CANCEL:
       default:              break;
     }
    gtk_widget_destroy(F_ajout);
    return(TRUE);
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 void Proto_afficher_mnemo_scenario ( struct CMD_TYPE_MNEMONIQUE *mnemo )
  { gchar chaine[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8+10];
    snprintf( chaine, sizeof(chaine), "%s%04d  %s",
              Type_bit_interne_court(mnemo->type), mnemo->num, mnemo->libelle );             /* Formatage */
    gtk_entry_set_text( GTK_ENTRY(Entry_bitm), chaine ); 
  }
/**********************************************************************************************************/
/* Afficher_mnemo: Changement du mnemonique et affichage                                                  */
/* Entre: widget, data.                                                                                   */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Afficher_mnemo_scenario ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE mnemo;
    mnemo.type = MNEMO_MONOSTABLE;
    mnemo.num = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON(Spin_bitm) );
    
    Envoi_serveur( TAG_SCENARIO, SSTAG_CLIENT_TYPE_NUM_MNEMONIQUE,
                   (gchar *)&mnemo, sizeof( struct CMD_TYPE_NUM_MNEMONIQUE ) );
  }
/**********************************************************************************************************/
/* Ajouter_scenario: Ajoute un scenario au systeme                                                          */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_scenario ( struct CMD_TYPE_SCENARIO *edit_sce )
  { GtkWidget *frame, *table, *texte, *hboite;

    if (edit_sce)
     { memcpy( &Edit_sce, edit_sce, sizeof(struct CMD_TYPE_SCENARIO) );    /* Save pour utilisation future */
     }
    else memset (&Edit_sce, 0, sizeof(struct CMD_TYPE_SCENARIO) );                  /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_sce ? _("Edit a scenario") : _("Add a scenario")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_scenario),
                      GINT_TO_POINTER( (edit_sce ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 12, 4, TRUE );
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

    texte = gtk_label_new( _("Monostable") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 10, 11 );
    Spin_bitm = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS-1, 1 );
    g_signal_connect( G_OBJECT(Spin_bitm), "changed",
                      G_CALLBACK(Afficher_mnemo_scenario), NULL );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_bitm, 1, 2, 10, 11 );
    Entry_bitm = gtk_entry_new();
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_bitm, 2, 4, 10, 11 );

    texte = gtk_label_new( _("Libelle") );                                      /* Le scenario en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 11, 12 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_SCENARIO );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, 11, 12 );

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
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_bitm), edit_sce->bit_m );
       gtk_widget_grab_focus( Entry_lib );
     }
    else { gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), TRUE );
           gtk_widget_grab_focus( Entry_lib );
         }
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface complète */
  }
/*--------------------------------------------------------------------------------------------------------*/
