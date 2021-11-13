/**********************************************************************************************************/
/* Client/supervision_palette.c        Affichage du synoptique de supervision                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                       dim 29 jan 2006 14:23:01 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * supervision_palette.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sébastien Lefevre
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
 
 #include "Reseaux.h"
 #include "Config_cli.h"
 #include "trame.h"

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
 static void Changer_vue( GtkWidget *bouton, gpointer data )
  { struct CMD_TYPE_SYNOPTIQUE cmd;
    gint syn_cible_id;
    syn_cible_id = GPOINTER_TO_INT(data);
    if (Chercher_page_notebook( TYPE_PAGE_SUPERVISION, syn_cible_id, TRUE )) return;

    cmd.id = syn_cible_id;

    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_WANT_PAGE_SUPERVISION,
                   (gchar *)&cmd, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
  }
/**********************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                      */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_une_palette_supervision( struct CMD_TYPE_PALETTE *rezo_palette )
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
