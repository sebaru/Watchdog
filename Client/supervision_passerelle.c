/**********************************************************************************************************/
/* Client/supervision_passerelle.c        Affichage du synoptique de supervision                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 15 mai 2005 20:25:04 CEST */
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

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/**********************************************************************************************************/
/* Changer_vue: Demande au serveur une nouvelle vue                                                       */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static gboolean Changer_vue (GooCanvasItem *canvasitem, GooCanvasItem *target,
                              GdkEvent          *event, struct PASSERELLE *pass )
  { struct CMD_ID_SYNOPTIQUE cmd;
printf("Changer_vue -> %d\n", pass->syn_cible_id );

    if ( !(event->button.button == 1 &&                                                 /* clic gauche ?? */
           event->type == GDK_BUTTON_PRESS)
       )
    return(FALSE);

    if (Chercher_page_notebook( TYPE_PAGE_SUPERVISION, pass->syn_cible_id, TRUE )) return(TRUE);
printf("Changer_vue -> creation\n" );

    cmd.id = pass->syn_cible_id;
    g_snprintf( cmd.libelle, sizeof(cmd.libelle), "Site/%s", pass->libelle );

    Creer_page_supervision( cmd.libelle, pass->syn_cible_id );      /* Creation de la page de supervision */
    Chercher_page_notebook( TYPE_PAGE_SUPERVISION, pass->syn_cible_id, TRUE );    /* Affichage de la page */

    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_WANT_PAGE_SUPERVISION,
                   (gchar *)&cmd, sizeof(struct CMD_ID_SYNOPTIQUE) );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                      */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_une_passerelle_supervision( struct CMD_SHOW_PASSERELLE *rezo_pass )
  { struct TRAME_ITEM_PASS *trame_pass;
    struct TYPE_INFO_SUPERVISION *infos;
    struct PASSERELLE *pass;
        
    infos = Rechercher_infos_supervision_par_id_syn ( rezo_pass->syn_id );
    if (!(infos && infos->Trame)) return;
    pass = (struct PASSERELLE *)g_malloc0( sizeof(struct PASSERELLE) );
    if (!pass)
     { Info( Config_cli.log, DEBUG_MEM, "Afficher_pass_supervision: not enought memory" );
       return;
     }

    pass->id = rezo_pass->id;                               /* Numero du pass dans la DBWatchdog */
    printf("Libelle pass=%s id %d id_cible %d bit_ctrl1 %d \n", rezo_pass->libelle, rezo_pass->id, rezo_pass->syn_cible_id, rezo_pass->bit_controle_1 );
    memcpy( pass->libelle, rezo_pass->libelle, sizeof(pass->libelle) );
    pass->syn_cible_id = rezo_pass->syn_cible_id;                            /* en abscisses et ordonnées */
    pass->position_x   = rezo_pass->position_x;                              /* en abscisses et ordonnées */
    pass->position_y   = rezo_pass->position_y;
    pass->bit_controle = rezo_pass->bit_controle;
    pass->bit_controle_1 = rezo_pass->bit_controle_1;
    pass->bit_controle_2 = rezo_pass->bit_controle_2;

    trame_pass = Trame_ajout_passerelle ( FALSE, infos->Trame, pass );
    g_signal_connect( G_OBJECT(trame_pass->item_groupe), "button-press-event",
                      G_CALLBACK(Changer_vue), pass );
    g_signal_connect( G_OBJECT(trame_pass->item_groupe), "button-release-event",
                      G_CALLBACK(Changer_vue), pass );
  }
/*--------------------------------------------------------------------------------------------------------*/
