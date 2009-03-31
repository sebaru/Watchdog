/**********************************************************************************************************/
/* Client/Ajout_util.c        Addition/Eddition d'un utilisateur Watchdog2.0                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 24 aoû 2003 20:01:18 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <gnome.h>
 
 #include "Reseaux.h"
 
 enum
  {  COLONNE_ID,
     COLONNE_NOM,
     COLONNE_COMMENTAIRE,
     NBR_COLONNE
  };
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */
 
 static GtkWidget *Liste_grps, *Liste_grp_util;                            /* Liste des groupes existants */
 static GtkWidget *Check_expire;                                         /* Le bouton d'expiration ou non */
 static GtkWidget *Calendar;                                 /* Le calendrier pour l'expiration du compte */
 static GtkWidget *Entry_nom;                                                  /* Le nom de l'utilisateur */
 static GtkWidget *Entry_id;                                                     /* L'id de l'utilisateur */
 static GtkWidget *Entry_last;                                                   /* L'id de l'utilisateur */
 static GtkWidget *Entry_comment;                                  /* Commentaire associé à l'utilisateur */
 static GtkWidget *Check_actif;                                   /* Le compte utilisateur est-il actif ? */
 static GtkWidget *Check_cansetpass;                      /* L'utilisateur peut-il changer son password ? */
 static GtkWidget *Check_setpassnow;                                      /* Pour changer le password now */
 static GtkWidget *Check_changepass;   /* L'utilisateur doit-il changer son password au prochain login ?? */
 static GtkWidget *Entry_pass1, *Entry_pass2;                            /* Acquisition des passwords ... */
 static GtkWidget *F_ajout;                                /* Widget visuel de la fenetre d'ajout/edition */
 static struct CMD_EDIT_UTILISATEUR Edit_util;                          /* Utilisateur en cours d'edition */
/**********************************************************************************************************/
/* Visu_groupe: Creation d'un GtkTreeView permettant la visu des données des groupes                      */
/* Entrées: kedal                                                                                         */
/* Sortie: un GtkTreeView                                                                                 */
/**********************************************************************************************************/
 static GtkWidget *Visu_groupe ( void )
  { GtkTreeSelection *selection;
    GtkTreeViewColumn *colonne;
    GtkCellRenderer *renderer;
    GtkTreeModel *store;
    GtkWidget *vue;

    store = (GtkTreeModel *)gtk_list_store_new ( NBR_COLONNE, G_TYPE_UINT,
                                                              G_TYPE_STRING,                    /* Nom */
                                                              G_TYPE_STRING             /* Commentaire */
                                               );

    vue = gtk_tree_view_new_with_model ( store );                                   /* Creation de la vue */
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(vue) );
    gtk_tree_selection_set_mode( selection, GTK_SELECTION_MULTIPLE );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne de l'id de l'utilisateur */
    colonne = gtk_tree_view_column_new_with_attributes ( _("GroupName"), renderer,
                                                         "text", COLONNE_NOM,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_NOM);             /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW(vue), colonne );

    renderer = gtk_cell_renderer_text_new();                          /* Colonne de l'id de l'utilisateur */
    colonne = gtk_tree_view_column_new_with_attributes ( _("Comment"), renderer,
                                                         "text", COLONNE_COMMENTAIRE,
                                                         NULL);
    gtk_tree_view_column_set_sort_column_id(colonne, COLONNE_COMMENTAIRE);            /* On peut la trier */
    gtk_tree_view_append_column ( GTK_TREE_VIEW(vue), colonne );

    gtk_tree_view_set_rules_hint( GTK_TREE_VIEW(vue), TRUE );                          /* Pour faire beau */
    g_object_unref (G_OBJECT (store));                        /* nous n'avons plus besoin de notre modele */

    return( vue );
  }
/**********************************************************************************************************/
/* Recuperer_groupes_util: Parcours la liste visuelle et renvoie un tableau de groupes                    */
/* Entrée: rien                                                                                           */
/* sortie: un tableau de groupes nouvellement alloué                                                      */
/**********************************************************************************************************/
 static guint *Recuperer_groupes_util ( void )
  { static guint gids[NBR_MAX_GROUPE_PAR_UTIL];
    GtkTreeModel *store;
    GtkTreeIter iter;
    gboolean valide;
    guint cpt, id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_grp_util) );
    valide = gtk_tree_model_get_iter_first( store, &iter );
    cpt = 0;
    while ( cpt<NBR_MAX_GROUPE_PAR_UTIL && valide )      /* Parcours de la liste des groupes utilisateurs */
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       if (id) gids[cpt++] = id;                              /* on ne compte pas le groupe TOUT_LE_MONDE */
       valide = gtk_tree_model_iter_next( store, &iter );
     }
    gids[cpt++] = 0;
    return( gids );
  }
/**********************************************************************************************************/
/* Positionner_groupes_util: Affiche à l'ecran les groupes de l'utilisateur                               */
/* Entrée: le tableau des groupes utilisateurs                                                            */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Positionner_groupes_util ( guint *gids )
  { GtkTreeModel *store, *cible;
    GtkTreeIter iter, iter_cible;
    gchar *nom, *comment;
    gboolean valide;
    guint cpt, id;

    store  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_grps) );
    cible  = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_grp_util) );
    valide = gtk_tree_model_get_iter_first( store, &iter );
    cpt = 0;
    while ( valide )                                     /* Parcours de la liste des groupes utilisateurs */
     { gtk_tree_model_get( store, &iter, COLONNE_ID, &id, -1 );
       cpt=0;
       while ( cpt<NBR_MAX_GROUPE_PAR_UTIL )
        { if (gids[cpt] == id)                                        /* Le groupe fait parti des gids ?? */
           { gtk_tree_model_get( store, &iter, COLONNE_NOM, &nom, -1 );
             gtk_tree_model_get( store, &iter, COLONNE_COMMENTAIRE, &comment, -1 );
             gtk_list_store_append ( GTK_LIST_STORE(cible), &iter_cible );       /* Acquisition iterateur */
             gtk_list_store_set ( GTK_LIST_STORE(cible), &iter_cible,
                                  COLONNE_ID, id,
                                  COLONNE_NOM, nom,
                                  COLONNE_COMMENTAIRE, comment,
                                  -1
                                );
             g_free(nom);
             g_free(comment);
             break;
           }
          if (!gids[cpt]) break;                                                        /* Fin de tableau */
          cpt++;
        }
       valide = gtk_tree_model_iter_next( store, &iter );
     }
  }
/**********************************************************************************************************/
/* Changer_compte_expire: Appelé quand l'utilisateur clique sur le toggle expire                          */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 static void Changer_compte_expire ( void )
  { gtk_widget_set_sensitive( Calendar, GTK_TOGGLE_BUTTON(Check_expire)->active );
  }
/**********************************************************************************************************/
/* Changer_setpassnow: Appelé quand l'utilisateur clique sur le toggle setpasswordnow                     */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 static void Changer_setpassnow ( void )
  { gboolean actif;
    actif = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_setpassnow));
    gtk_widget_set_sensitive( Entry_pass1, actif );
    gtk_widget_set_sensitive( Entry_pass2, actif );
  }
/**********************************************************************************************************/
/* Proto_afficher_un_groupe_existant: ajoute un groupe dans la liste des groupes existants                */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_groupe_existant ( struct CMD_SHOW_GROUPE *groupe )
  { GtkTreeModel *store;
    GtkTreeIter iter;
    store = gtk_tree_view_get_model ( GTK_TREE_VIEW(Liste_grps) );

    gtk_list_store_append ( GTK_LIST_STORE(store), &iter );                      /* Acquisition iterateur */
    gtk_list_store_set ( GTK_LIST_STORE(store), &iter,
                         COLONNE_ID, groupe->id,
                         COLONNE_NOM, groupe->nom,
                         COLONNE_COMMENTAIRE, groupe->commentaire,
                         -1
                       );
  }
/**********************************************************************************************************/
/* Proto_fin_affichage_groupes: Les groupes existants ont été envoyés par le serveur, nous pouvons faire  */
/*                              apparaitre les groupes de l'utilisateur dans l'interface                  */
/* les groupes de l'utilisateur en cours d'edition                                                        */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 void Proto_fin_affichage_groupes_existants ( void )
  { Positionner_groupes_util ( Edit_util.gids );
  }
/**********************************************************************************************************/
/* Ajouter_groupes: Ajout de la selection dans les groupes utilisateurs                                   */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 static void Ajouter_groupes ( void )
  { GtkTreeSelection *selection;
    GtkTreeModel *store, *store_new;
    GtkTreeIter iter, iter_new;
    GList *lignes;                                                            /* Les lignes selectionnées */
    GList *liste;                                                          /* Parcours de la liste lignes */

    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_grps) );
    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_grps) );
    store_new = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_grp_util) );
    lignes    = gtk_tree_selection_get_selected_rows ( selection, NULL );
    if (!(lignes && lignes->data)) return;

    for (liste = lignes; liste; liste=liste->next)
     { gchar *nom, *comment;
       guint id, id_new;
       gboolean valide;
       gtk_tree_model_get_iter( store, &iter, liste->data );     /* Recuperation de la ligne selectionnée */
       gtk_tree_model_get( store, &iter,
                           COLONNE_ID, &id,
                           COLONNE_NOM, &nom,
                           COLONNE_COMMENTAIRE, &comment,
                           -1 );                                                       /* Recup du groupe */

       valide = gtk_tree_model_get_iter_first( store_new, &iter_new );                /* Test de presence */
       while ( valide )          /* Parcours de la liste des groupes utilisateurs pour tester la presence */
        { gtk_tree_model_get( store_new, &iter_new, COLONNE_ID, &id_new, -1 );
          if ( id == id_new ) break;
          valide = gtk_tree_model_iter_next( store_new, &iter_new );
        }
     
       if (!valide)                 /* Si nous sommes en bas de liste, c'est que l'item n'est pas présent */
        { gtk_list_store_append ( GTK_LIST_STORE(store_new), &iter_new );        /* Acquisition iterateur */
          gtk_list_store_set ( GTK_LIST_STORE(store_new), &iter_new,
                               COLONNE_ID, id,
                               COLONNE_NOM, nom,
                               COLONNE_COMMENTAIRE, comment,
                               -1
                             );
        }
       g_free(nom);
       g_free(comment);
     }
    g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (lignes);                                                           /* Liberation mémoire */
    gtk_tree_selection_unselect_all( selection );                      /* On deselectionne tous nos items */
  }
/**********************************************************************************************************/
/* Retirer_groupes: Retrait de la selection dans les groupes utilisateurs                                 */
/* Entrée: rien                                                                                           */
/* sortie: kedal                                                                                          */
/**********************************************************************************************************/
 static void Retirer_groupes ( void )
  { GtkTreeSelection *selection;
    GtkTreeModel *store;
    GtkTreeIter iter;
    GList *lignes;                                                            /* Les lignes selectionnées */

    store     = gtk_tree_view_get_model    ( GTK_TREE_VIEW(Liste_grp_util) );
    selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(Liste_grp_util) );
    while ( (lignes = gtk_tree_selection_get_selected_rows ( selection, NULL )) != NULL )
     { gtk_tree_model_get_iter( store, &iter, lignes->data );    /* Recuperation de la ligne selectionnée */
       gtk_list_store_remove( GTK_LIST_STORE(store), &iter );             /* Retrait de la liste visuelle */

       g_list_foreach (lignes, (GFunc) gtk_tree_path_free, NULL);
       g_list_free (lignes);                                                        /* Liberation mémoire */
     }
  }
/**********************************************************************************************************/
/* CB_ajouter_editer_utilisateur: Fonction appelée qd on appuie sur un des boutons de l'interface         */
/* Entrée: la reponse de l'utilisateur et un flag precisant l'edition/ajout                               */
/* sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static gboolean CB_ajouter_editer_utilisateur ( GtkDialog *dialog, gint reponse,
                                                 gboolean edition )
  { switch(reponse)
     { case GTK_RESPONSE_OK:
            if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_setpassnow)) &&
                g_utf8_collate( gtk_entry_get_text(GTK_ENTRY(Entry_pass1) ),
                                gtk_entry_get_text(GTK_ENTRY(Entry_pass2) )
                              )
               )
             { GtkWidget *dialog;
               dialog = gtk_message_dialog_new ( GTK_WINDOW(F_ajout),
                                                 GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_ERROR,
                                                 GTK_BUTTONS_CLOSE, _("Passwords do not match") );
               g_signal_connect_swapped( dialog, "response",
                                         G_CALLBACK(gtk_widget_destroy), dialog );
               return(TRUE);
             }
            if (edition)                                                             /* Si mode edition */
             { guint *gids;

               Edit_util.cansetpass = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_cansetpass));
               Edit_util.setpassnow = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_setpassnow));
               g_snprintf( Edit_util.commentaire, sizeof(Edit_util.commentaire),
                           "%s", gtk_entry_get_text(GTK_ENTRY(Entry_comment) ) );
               g_snprintf( Edit_util.code_en_clair, sizeof(Edit_util.code_en_clair),
                           "%s", gtk_entry_get_text(GTK_ENTRY(Entry_pass1) ) );

               Edit_util.date_expire = gnome_date_edit_get_time( GNOME_DATE_EDIT(Calendar) );
               Edit_util.expire      = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_expire));
               Edit_util.changepass  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_changepass));
               Edit_util.actif       = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_actif));

               gids = Recuperer_groupes_util();
               memcpy( Edit_util.gids, gids, sizeof(Edit_util.gids) );
               Envoi_serveur( TAG_UTILISATEUR, SSTAG_CLIENT_VALIDE_EDIT_UTIL,
                              (gchar *)&Edit_util, sizeof(struct CMD_EDIT_UTILISATEUR) );
             }
            else                                                             /* Si ajout d'un utilisateur */
             { struct CMD_ADD_UTILISATEUR new_util;
               guint *gids;

               g_snprintf( new_util.nom, sizeof(new_util.nom),
                           "%s", gtk_entry_get_text(GTK_ENTRY(Entry_nom) ) );
               g_snprintf( new_util.commentaire, sizeof(new_util.commentaire),
                           "%s", gtk_entry_get_text(GTK_ENTRY(Entry_comment) ) );
               g_snprintf( new_util.code_en_clair, sizeof(new_util.code_en_clair),
                           "%s", gtk_entry_get_text(GTK_ENTRY(Entry_pass1) ) );
               new_util.date_expire = gnome_date_edit_get_time( GNOME_DATE_EDIT(Calendar) );
               new_util.expire      = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_expire));
               new_util.changepass  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_changepass));
               new_util.actif       = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_actif));
               new_util.cansetpass  = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(Check_cansetpass));

               gids = Recuperer_groupes_util();
               memcpy( new_util.gids, gids, sizeof(new_util.gids) );
               Envoi_serveur( TAG_UTILISATEUR, SSTAG_CLIENT_ADD_UTIL,
                              (gchar *)&new_util, sizeof(struct CMD_ADD_UTILISATEUR) );
              }
            break;

       case GTK_RESPONSE_CANCEL:
       default                 : break;
     }

    gtk_widget_destroy( F_ajout );                                           /* Destruction de la fenetre */
    return(TRUE);                                      /* Pour eviter la propagation du signal 'response' */
  }
/**********************************************************************************************************/
/* Menu_ajouter_editer_utilisateur: Met a jour ou ajout un utilisateur au systeme                         */
/* Entrée: l'id et le nom de l'utilisateur                                                                */
/* sortie: rien                                                                                           */
/**********************************************************************************************************/
 void Menu_ajouter_editer_utilisateur ( struct CMD_EDIT_UTILISATEUR *edit_util )
  { GtkWidget *bouton, *frame, *vboite, *table, *texte, *scroll, *separateur;

    if (edit_util)
     { memcpy( &Edit_util, edit_util, sizeof(struct CMD_EDIT_UTILISATEUR) );/*Save pour utilisation future*/
     }
    else memset (&Edit_util, 0, sizeof(struct CMD_EDIT_UTILISATEUR) );             /* Sinon RAZ structure */

    F_ajout = gtk_dialog_new_with_buttons( (edit_util ? _("Edit a user") : _("Add a user")),
                                           GTK_WINDOW(F_client),
                                           GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                           GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                           GTK_STOCK_OK, GTK_RESPONSE_OK,
                                           NULL);
    g_signal_connect( F_ajout, "response",
                      G_CALLBACK(CB_ajouter_editer_utilisateur),
                      GINT_TO_POINTER( (edit_util ? TRUE : FALSE) ) );

    frame = gtk_frame_new( _("Settings") );
    gtk_frame_set_label_align( GTK_FRAME(frame), 0.5, 0.5 );
    gtk_box_pack_start( GTK_BOX( GTK_DIALOG(F_ajout)->vbox ), frame, TRUE, TRUE, 0 );

    vboite = gtk_vbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(vboite), 6 );
    gtk_container_add( GTK_CONTAINER(frame), vboite );

/******************************************** Paramètres de l'utilisateur *********************************/
    table = gtk_table_new( 6, 4, FALSE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(vboite), table, FALSE, FALSE, 0 );

    texte = gtk_label_new( _("UserID") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 0, 1 );
    gtk_misc_set_alignment( GTK_MISC(texte), 1.0, 0.5 );
    Entry_id = gtk_entry_new();
    gtk_editable_set_editable( GTK_EDITABLE(Entry_id), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_id, 1, 2, 0, 1 );
    gtk_widget_set_sensitive( Entry_id, FALSE );

    texte = gtk_label_new( _("Username") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, 0, 1 );
    gtk_misc_set_alignment( GTK_MISC(texte), 1.0, 0.5 );
    Entry_nom = gtk_entry_new();
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_nom, 3, 4, 0, 1 );
    gtk_entry_set_max_length( GTK_ENTRY(Entry_nom), NBR_CARAC_LOGIN );

    texte = gtk_label_new( _("Last modification") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 0, 1, 1, 2 );
    gtk_misc_set_alignment( GTK_MISC(texte), 1.0, 0.5 );
    Entry_last = gtk_entry_new();
    gtk_editable_set_editable( GTK_EDITABLE(Entry_last), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_last, 1, 2, 1, 2 );
    gtk_widget_set_sensitive( Entry_last, FALSE );

    texte = gtk_label_new( _("Comment") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, 1, 2 );
    gtk_misc_set_alignment( GTK_MISC(texte), 1.0, 0.5 );
    Entry_comment = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_comment), NBR_CARAC_COMMENTAIRE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_comment, 3, 4, 1, 2 );

    Check_setpassnow = gtk_check_button_new_with_label ( _("Set password now") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_setpassnow, 2, 4, 2, 3 );
    g_signal_connect( G_OBJECT(Check_setpassnow), "clicked",
                      G_CALLBACK(Changer_setpassnow), NULL );

    texte = gtk_label_new( _("Password") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, 3, 4 );
    gtk_misc_set_alignment( GTK_MISC(texte), 1.0, 0.5 );
    Entry_pass1 = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_pass1), NBR_CARAC_LOGIN );
    gtk_entry_set_visibility( GTK_ENTRY(Entry_pass1), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_pass1, 3, 4, 3, 4 );
    gtk_widget_set_sensitive( Entry_pass1, FALSE );

    texte = gtk_label_new( _("Password (again)") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 2, 3, 4, 5 );
    gtk_misc_set_alignment( GTK_MISC(texte), 1.0, 0.5 );
    Entry_pass2 = gtk_entry_new();
    gtk_entry_set_max_length( GTK_ENTRY(Entry_pass2), NBR_CARAC_LOGIN );
    gtk_entry_set_visibility( GTK_ENTRY(Entry_pass2), FALSE );
    gtk_table_attach_defaults( GTK_TABLE(table), Entry_pass2, 3, 4, 4, 5 );
    gtk_widget_set_sensitive( Entry_pass2, FALSE );

    Check_actif = gtk_check_button_new_with_label ( _("Account enabled") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_actif, 0, 2, 2, 3 );

    Check_changepass = gtk_check_button_new_with_label ( _("Have to change his password at logon") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_changepass, 0, 2, 3, 4 );

    Check_cansetpass = gtk_check_button_new_with_label ( _("Can change his password") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_cansetpass, 0, 2, 4, 5 );

    Check_expire = gtk_check_button_new_with_label ( _("Account expire") );
    gtk_table_attach_defaults( GTK_TABLE(table), Check_expire, 0, 1, 5, 6 );
    g_signal_connect( G_OBJECT(Check_expire), "clicked",
                      G_CALLBACK(Changer_compte_expire), NULL );

    texte = gtk_label_new( _("Expiration date") );
    gtk_table_attach_defaults( GTK_TABLE(table), texte, 1, 2, 5, 6 );
    gtk_misc_set_alignment( GTK_MISC(texte), 1.0, 0.5 );

    Calendar = gnome_date_edit_new ((time_t) 0, TRUE, TRUE);
    gtk_table_attach_defaults( GTK_TABLE(table), Calendar, 2, 4, 5, 6 );

/***************************************** Gestion des groupes ********************************************/
    separateur = gtk_hseparator_new();
    gtk_box_pack_start( GTK_BOX(vboite), separateur, FALSE, FALSE, 0 );

    table = gtk_table_new( 3, 3, FALSE );
    gtk_table_set_row_spacings( GTK_TABLE(table), 5 );
    gtk_table_set_col_spacings( GTK_TABLE(table), 5 );
    gtk_box_pack_start( GTK_BOX(vboite), table, TRUE, TRUE, 0 );

    texte = gtk_label_new( _("User groups") );
    gtk_table_attach( GTK_TABLE(table), texte, 0, 1, 0, 1, 0, 0, 0, 0 );

    texte = gtk_label_new( _("All groups") );
    gtk_table_attach( GTK_TABLE(table), texte, 2, 3, 0, 1, 0, 0, 0, 0 );

    bouton = gtk_button_new_from_stock( GTK_STOCK_ADD );
    gtk_table_attach( GTK_TABLE(table), bouton, 1, 2, 1, 2, 0, 0, 0, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Ajouter_groupes), NULL );

    bouton = gtk_button_new_from_stock( GTK_STOCK_REMOVE );
    gtk_table_attach( GTK_TABLE(table), bouton, 1, 2, 2, 3, 0, 0, 0, 0 );
    g_signal_connect_swapped( G_OBJECT(bouton), "clicked",
                              G_CALLBACK(Retirer_groupes), NULL );

/***************************************** Gestion des groupes existants **********************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_table_attach_defaults( GTK_TABLE(table), scroll, 2, 3, 1, 3 );

    Liste_grps = Visu_groupe ();
    gtk_container_add( GTK_CONTAINER(scroll), Liste_grps );

/***************************************** Gestion des groupes de l'utilisateur ***************************/
    scroll = gtk_scrolled_window_new( NULL, NULL );
    gtk_scrolled_window_set_policy( GTK_SCROLLED_WINDOW(scroll), GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS );
    gtk_table_attach_defaults( GTK_TABLE(table), scroll, 0, 1, 1, 3 );

    Liste_grp_util = Visu_groupe();                                                 /* Creation de la vue */
    gtk_container_add( GTK_CONTAINER(scroll), Liste_grp_util );

    if (edit_util)                                                           /* Est-on en mode edition ?? */
     { gchar chaine[80], *date;
       gtk_entry_set_text( GTK_ENTRY(Entry_nom), edit_util->nom );
       gtk_editable_set_editable( GTK_EDITABLE( Entry_nom ), FALSE );

       gtk_entry_set_text( GTK_ENTRY(Entry_comment), edit_util->commentaire );
       g_snprintf( chaine, sizeof(chaine), "%d", edit_util->id );
       gtk_entry_set_text( GTK_ENTRY(Entry_id), chaine );
       gtk_entry_set_text( GTK_ENTRY(Entry_comment), edit_util->commentaire );

       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_actif), edit_util->actif );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_changepass), edit_util->changepass );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_cansetpass), edit_util->cansetpass );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON(Check_expire), edit_util->expire );

       strftime( chaine, sizeof(chaine), "%c", localtime(&edit_util->date_modif) );
       date = g_locale_to_utf8( chaine, -1, NULL, NULL, 0 ); 
       gtk_entry_set_text( GTK_ENTRY(Entry_last), date );
       g_free(date);
       gnome_date_edit_set_time( GNOME_DATE_EDIT(Calendar), edit_util->date_expire );
     }
    else
     { gtk_entry_set_text( GTK_ENTRY(Entry_id), "?" );
       gtk_entry_set_text( GTK_ENTRY(Entry_last), "?" );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( Check_actif ), TRUE );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( Check_changepass ), TRUE );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( Check_cansetpass ), TRUE );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( Check_expire ), TRUE );
       gtk_toggle_button_set_active( GTK_TOGGLE_BUTTON( Check_setpassnow ), TRUE );
       gnome_date_edit_set_time( GNOME_DATE_EDIT(Calendar), time(NULL) + 86400*31*2 );  /* Valable 2 mois */
     }

    gtk_widget_show_all( F_ajout );
  }
/*--------------------------------------------------------------------------------------------------------*/
