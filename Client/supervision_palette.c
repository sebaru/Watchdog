/**********************************************************************************************************/
/* Client/supervision_palette.c        Affichage du synoptique de supervision                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 29 jan 2006 14:23:01 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <gnome.h>
 #include <sys/time.h>
 
 #include "Reseaux.h"
 #include "Config_cli.h"
 #include "trame.h"
 #include "motifs.h"

 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

/********************************* D�finitions des prototypes programme ***********************************/
 #include "protocli.h"

/**********************************************************************************************************/
/* Changer_vue: Demande au serveur une nouvelle vue                                                       */
/* Entr�e: une reference sur le message                                                                   */
/* Sortie: N�ant                                                                                          */
/**********************************************************************************************************/
 static void Changer_vue( GtkWidget *bouton, gpointer data )
  { struct CMD_ID_SYNOPTIQUE cmd;
    const gchar *libelle;
    gint syn_cible_id;
    syn_cible_id = GPOINTER_TO_INT(data);
printf("----------------------- Changer_vue: cible = %d\n", syn_cible_id );
    if (Chercher_page_notebook( TYPE_PAGE_SUPERVISION, syn_cible_id, TRUE )) return;

    libelle = gtk_button_get_label(GTK_BUTTON(bouton));
    cmd.id = syn_cible_id;
    g_snprintf( cmd.libelle, sizeof(cmd.libelle), "Site/%s", libelle );

    Creer_page_supervision( cmd.libelle, syn_cible_id );                                 /* Creation page */
    Chercher_page_notebook( TYPE_PAGE_SUPERVISION, syn_cible_id, TRUE );          /* Affichage de la page */

    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_WANT_PAGE_SUPERVISION,
                   (gchar *)&cmd, sizeof(struct CMD_ID_SYNOPTIQUE) );
  }
/**********************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                      */
/* Entr�e: une reference sur le message                                                                   */
/* Sortie: N�ant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_une_palette_supervision( struct CMD_SHOW_PALETTE *rezo_palette )
  { struct TYPE_INFO_SUPERVISION *infos;
    GtkWidget *bouton;
        
    infos = Rechercher_infos_supervision_par_id_syn ( rezo_palette->syn_id );
    if (!(infos && infos->Trame)) return;

    bouton = gtk_button_new_with_label( rezo_palette->libelle );
    g_signal_connect( G_OBJECT(bouton), "clicked",
                      G_CALLBACK( Changer_vue ), GINT_TO_POINTER(rezo_palette->syn_cible_id) );
    gtk_box_pack_start( GTK_BOX(infos->Box_palette), bouton, FALSE, FALSE, 0 );

    gtk_widget_show_all(bouton);
  }
/*--------------------------------------------------------------------------------------------------------*/
