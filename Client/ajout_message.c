/**********************************************************************************************************/
/* Client/ajout_message.c        Addition/Edition d'un message Watchdog2.0                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 20 nov 2004 13:47:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <gnome.h>
 
 #include "Reseaux.h"
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */

 static GtkWidget *F_ajout;                                            /* Widget de l'interface graphique */
 static GtkWidget *Spin_num;                                /* Numéro du message en cours d'édition/ajout */
 static GtkWidget *Entry_lib;                                                       /* Libelle du message */
 static GtkWidget *Entry_objet;                                                       /* Objet du message */
 static GtkWidget *Option_type;                                                 /* Type actuel du message */
 static GtkWidget *Option_syn;                                                  /* Type actuel du message */
 static GtkWidget *Check_not_inhibe;                              /* Le message est-il actif ou inhibe ?? */
 static GtkWidget *Check_sms;                                     /* Le message est-il actif ou inhibe ?? */
 static struct CMD_EDIT_MESSAGE Edit_msg;                                   /* Message en cours d'édition */

/**********************************************************************************************************/
/* CB_ajouter_editer_message: Fonction appelée qd on appuie sur un des boutons de l'interface             */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_message ( GtkDialog *dialog, gint reponse, gboolean edition )
  { switch(reponse)
     { case GTK_RESPONSE_OK:
             { if (edition)
                { g_snprintf( Edit_msg.libelle, sizeof(Edit_msg.libelle),
                              "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
                  g_snprintf( Edit_msg.objet, sizeof(Edit_msg.objet),
                              "%s", gtk_entry_get_text( GTK_ENTRY(Entry_objet) ) );
                  Edit_msg.type       = gtk_option_menu_get_history( GTK_OPTION_MENU(Option_type) );
                  Edit_msg.not_inhibe = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_not_inhibe) );
                  Edit_msg.sms        = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_sms) );
                  Edit_msg.num        = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_num) );
                  Edit_msg.num_syn    = gtk_option_menu_get_history( GTK_OPTION_MENU(Option_syn) );
                  Edit_msg.num_voc    = 0; /* A voir !!! */
printf("sms=%d\n", Edit_msg.sms );
                  Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_VALIDE_EDIT_MESSAGE,
                                (gchar *)&Edit_msg, sizeof( struct CMD_EDIT_MESSAGE ) );
                }
               else
                { struct CMD_ADD_MESSAGE new_msg;
                  g_snprintf( new_msg.libelle, sizeof(new_msg.libelle),
                              "%s", gtk_entry_get_text( GTK_ENTRY(Entry_lib) ) );
                  g_snprintf( new_msg.objet, sizeof(new_msg.objet),
                              "%s", gtk_entry_get_text( GTK_ENTRY(Entry_objet) ) );
                  new_msg.type       = gtk_option_menu_get_history( GTK_OPTION_MENU(Option_type) );
                  new_msg.not_inhibe = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_not_inhibe) );
                  new_msg.sms        = gtk_toggle_button_get_active( GTK_TOGGLE_BUTTON(Check_sms) );
                  new_msg.num        = gtk_spin_button_get_value_as_int( GTK_SPIN_BUTTON(Spin_num) );
                  new_msg.num_syn    = gtk_option_menu_get_history( GTK_OPTION_MENU(Option_syn) );
                  new_msg.num_voc    = 0; /* A voir !!! */

                  Envoi_serveur( TAG_MESSAGE, SSTAG_CLIENT_ADD_MESSAGE,
                                (gchar *)&new_msg, sizeof( struct CMD_EDIT_MESSAGE ) );
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
/* Ajouter_message: Ajoute un message au systeme                                                          */
/* Entrée: rien                                                                                           */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_message ( struct CMD_EDIT_MESSAGE *edit_msg )
  { GtkWidget *frame, *table, *texte, *hboite, *bouton, *menu;
    gint cpt;

    if (edit_msg)
     { memcpy( &Edit_msg, edit_msg, sizeof(struct CMD_EDIT_MESSAGE) );    /* Save pour utilisation future */
     }
    else memset (&Edit_msg, 0, sizeof(struct CMD_EDIT_MESSAGE) );                  /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_msg ? _("Edit a message") : _("Add a message")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_message),
                      GINT_TO_POINTER( (edit_msg ? TRUE : FALSE) ) );

    frame = gtk_frame_new("Settings");                               /* Création de l'interface graphique */
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_container_set_border_width( GTK_CONTAINER(frame), 6 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), hboite );

    table = gtk_table_new( 8, 4, TRUE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(hboite), table, TRUE, TRUE, 0 );

    Check_not_inhibe = gtk_check_button_new_with_label( _("Enable") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_not_inhibe, 0, 2, 0, 1 );

    Check_sms = gtk_check_button_new_with_label( _("Notification par sms") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_sms, 0, 4, 7, 8 );

    texte = gtk_label_new( _("MsgID") );                 /* Id unique du message en cours d'edition/ajout */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    Spin_num = gtk_spin_button_new_with_range( 0, NBR_BIT_DLS, 1 );
    gtk_table_attach_defaults( GTK_TABLE(table), Spin_num, 1, 2, 1, 2 );

    texte = gtk_label_new( _("Type") );     /* Création de l'option menu pour le choix du type de message */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 2, 3 );
    Option_type = gtk_option_menu_new();
    menu = gtk_menu_new();
    for ( cpt=0; cpt<NBR_TYPE_MSG; cpt++ )
     { gtk_menu_shell_append( GTK_MENU_SHELL(menu),
                              gtk_menu_item_new_with_label( Type_vers_string(cpt) ) );
     }
    gtk_option_menu_set_menu( GTK_OPTION_MENU(Option_type), menu );
    gtk_table_attach_defaults( GTK_TABLE(table), Option_type, 1, 2, 2, 3 );

  /*  Liste = Recuperer_liste_typemsgDB( Config.log, Db_watchdog );
    gtk_combo_set_popdown_strings( GTK_COMBO(Combo_type), Liste );
    g_list_foreach( Liste, (GFunc) g_free, NULL );
    g_list_free(Liste);*/

    texte = gtk_label_new( _("Click to\nchange Audio") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 3, 5 );
    bouton = gtk_button_new_with_label( _("Change") );
    gtk_table_attach_defaults( GTK_TABLE(table), bouton, 1, 2, 3, 4 );
    bouton = gtk_button_new_with_label( _("Disable") );
    gtk_table_attach_defaults( GTK_TABLE(table), bouton, 1, 2, 4, 5 );
    if (!edit_msg) gtk_widget_set_sensitive( bouton, FALSE );

    texte = gtk_label_new( _("Synopt.") );                       /* Choix du synoptique cible du messages */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 5, 6 );
    Option_syn = gtk_option_menu_new();
    menu = gtk_menu_new();
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label("Syn1") );
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label("Syn2") );
    gtk_menu_shell_append( GTK_MENU_SHELL(menu), gtk_menu_item_new_with_label("Syn3") );
    gtk_option_menu_set_menu( GTK_OPTION_MENU(Option_syn), menu );
    gtk_table_attach_defaults( GTK_TABLE(table), Option_syn, 1, 4, 5, 6 );
/*    liste = Recuperer_liste_typemsgDB( Config.log, Db_watchdog );

    for (liste = NULL, nom_syns=Nom_syn; nom_syns; nom_syns=nom_syns->next)
     { liste = g_list_append( liste, ((struct NOM_SYN *)nom_syns->data)->libelle );
       printf("Saisie: ajout syn %s\n", ((struct NOM_SYN *)nom_syns->data)->libelle);
     }
    liste = g_list_sort( liste, (GCompareFunc)strcmp );
    gtk_combo_set_popdown_strings( GTK_COMBO(Combo_syn), liste );*/

    texte = gtk_label_new( _("Message") );                                      /* Le message en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 6, 7 );
    Entry_lib = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_lib), NBR_CARAC_LIBELLE_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_lib, 1, 4, 6, 7 );

    texte = gtk_label_new( _("Object") );                                       /* Le message en lui-meme */
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, 1, 2 );
    Entry_objet = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_objet), NBR_CARAC_OBJET_MSG );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_objet, 3, 4, 1, 2 );

    if (edit_msg)                                                              /* Si edition d'un message */
     { /*syn = Rechercher_syn_par_num( Nom_syn, msg->num_synoptique );*/
       Edit_msg.id = edit_msg->id;
       gtk_entry_set_text( GTK_ENTRY(Entry_lib), edit_msg->libelle );
       gtk_entry_set_text( GTK_ENTRY(Entry_objet), edit_msg->objet );
       gtk_option_menu_set_history( GTK_OPTION_MENU(Option_type), edit_msg->type );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_not_inhibe), edit_msg->not_inhibe );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_sms), edit_msg->sms );

       /*gtk_entry_set_text( GTK_ENTRY(GTK_COMBO(Combo_syn)->entry), syn->libelle );*/
       gtk_spin_button_set_value( GTK_SPIN_BUTTON(Spin_num), edit_msg->num );
       gtk_widget_grab_focus( Entry_lib );
       /*for (cpt=0; cpt<MAX_HP; cpt++)
        { gtk_toggle_button_set_active ( GTK_TOGGLE_BUTTON(Check_hp[cpt]), (msg.hps & (1<<cpt)) ); }*/
     }
    else { gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_not_inhibe), TRUE );
           gtk_widget_grab_focus( Spin_num );
         }
    gtk_widget_show_all(F_ajout);                                    /* Affichage de l'interface complète */
  }
/*--------------------------------------------------------------------------------------------------------*/
