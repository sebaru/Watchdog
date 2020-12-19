/**********************************************************************************************************/
/* client/ihm.c        L'interface du client Watchdog v2.0                                                */
/* Projet WatchDog version 3.0       Gestion d'habitat                      jeu 21 aoû 2003 18:47:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ihm.c
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

 #include <gtk/gtk.h>

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/**********************************************************************************************************/
/* Detruire_page_plugin_dls: Detruit la page du notebook consacrée aux plugin_dlss watchdog               */
/* Entrée: rien                                                                                           */
/* Sortie: un widget boite                                                                                */
/**********************************************************************************************************/
 struct PAGE_NOTEBOOK *Page_actuelle ( struct CLIENT *client )
  { struct PAGE_NOTEBOOK *page;
    GtkWidget *child;
    GSList * liste;
    gint num;

    num = gtk_notebook_get_current_page( GTK_NOTEBOOK(client->Notebook) );
    child = gtk_notebook_get_nth_page( GTK_NOTEBOOK(client->Notebook), num );

    liste = client->Liste_pages;
    while(liste)
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if ( page->child == child ) return(page);                                /* Nous l'avons trouvé !! */
       liste = liste->next;
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Effacer_pages: On efface toutes les pages restantes du notebook                                                            */
/* Entrée: niet                                                                                                               */
/* Sortie: void                                                                                                               */
/******************************************************************************************************************************/
 void Effacer_pages ( struct CLIENT *client )
  { while( client->Liste_pages )
     { struct PAGE_NOTEBOOK *page = client->Liste_pages->data;
       switch(page->type)
        { case TYPE_PAGE_SUPERVISION : Detruire_page_supervision ( page ); break;
          case TYPE_PAGE_ATELIER     : Detruire_page_atelier     ( page ); break;
          case TYPE_PAGE_PLUGIN_DLS  : Detruire_page_plugin_dls  ( page ); break;
          case TYPE_PAGE_SYNOPTIQUE  : Detruire_page_liste_synoptique ( page ); break;
          default : client->Liste_pages = g_slist_remove ( client->Liste_pages, page );
        }
     }
  }
/******************************************************************************************************************************/
/* Bouton : Prepare un bouton avec tooltip et icone                                                                           */
/* Entrée: le libelle, l'icone et le tooltip                                                                                  */
/* Sortie: le bouton                                                                                                          */
/******************************************************************************************************************************/
 GtkWidget *Bouton ( gchar *libelle, gchar *icone, gchar *tooltip )
  { GtkWidget *bouton = gtk_button_new_with_label( libelle );
    gtk_button_set_image ( GTK_BUTTON(bouton), gtk_image_new_from_icon_name ( icone, GTK_ICON_SIZE_LARGE_TOOLBAR ) );
    gtk_button_set_always_show_image( GTK_BUTTON(bouton), TRUE );
    gtk_widget_set_tooltip_text ( bouton, tooltip );
    return(bouton);
  }
/******************************************************************************************************************************/
/* Menu : Prepare un menu icone                                                                                               */
/* Entrée: le libelle, l'icone                                                                                                */
/* Sortie: le menu                                                                                                            */
/******************************************************************************************************************************/
 GtkWidget *Menu ( gchar *libelle, gchar *icone )
  { GtkWidget *item = gtk_menu_item_new();
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_image_new_from_icon_name ( icone, GTK_ICON_SIZE_LARGE_TOOLBAR ), FALSE, FALSE, 0 );
    gtk_box_pack_start ( GTK_BOX(hbox), gtk_label_new(libelle), FALSE, FALSE, 0 );
    gtk_container_add ( GTK_CONTAINER(item), hbox );
    return(item);
  }
#ifdef bouh
/**********************************************************************************************************/
/* Nbr_page_type: Renvoie le nombre de page notebook de type type                                         */
/* Entrée: Le type de page à compter                                                                      */
/* Sortie: Un entier                                                                                      */
/**********************************************************************************************************/
 gint Nbr_page_type ( gint type )
  { struct PAGE_NOTEBOOK *page;
    gint cpt;
    GList *liste;
    liste = client->Liste_pages;
    cpt = 0;
    while( liste )
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if ( page->type == type ) cpt++;                              /* Est-ce bien une page d'atelier ?? */
       liste = liste->next;                                                        /* On passe au suivant */
     }
    return(cpt);
  }
#endif
/******************************************************************************************************************************/
/* Set_progress: Positionne la barre de progression de la fenetre                                                             */
/* Entrées: val, max                                                                                                          */
/* Sortie: Kedal                                                                                                              */
/******************************************************************************************************************************/
 void Set_progress_pulse( struct CLIENT *client )
  { gtk_progress_bar_pulse ( GTK_PROGRESS_BAR(client->Barre_pulse) );
  }
/******************************************************************************************************************************/
/* Set_progress: Positionne la barre de progression de la fenetre                                                             */
/* Entrées: val, max                                                                                                          */
/* Sortie: Kedal                                                                                                              */
/******************************************************************************************************************************/
 void Update_progress_bar( SoupMessage *msg, SoupBuffer *chunk, gpointer data )
  { struct CLIENT *client = data;
    SoupMessageHeaders *headers;
    gdouble fraction;
    gchar chaine[20];

    g_object_get ( msg, "response-headers", &headers, NULL );
    client->network_size_to_send = soup_message_headers_get_content_length ( headers );
    printf("Progress bar prepare to %d \n", client->network_size_to_send );

    client->network_size_sent += chunk->length;
    if (client->network_size_sent >= client->network_size_to_send)
     { client->network_size_sent = client->network_size_to_send; }

    fraction = 1.0*client->network_size_sent/client->network_size_to_send;
    gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR (client->Barre_progress), fraction );
    if (fraction==1.0) g_snprintf( chaine, sizeof(chaine), "Ready" );
                  else g_snprintf( chaine, sizeof(chaine), "%3.1f%% downloaded", 100.0*fraction );
    gtk_progress_bar_set_text( GTK_PROGRESS_BAR (client->Barre_progress), chaine );
    printf("Progress bar set to %s\n", chaine );
  }
/**********************************************************************************************************/
/* Log: Afficher un texte dans l'entry status                                                             */
/* Entrée: la chaine de caracteres                                                                        */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Log( struct CLIENT *client, gchar *chaine )
  { gtk_entry_set_text( GTK_ENTRY(client->Entry_status), chaine ); }
/******************************************************************************************************************************/
/* Changer_page_Notebook: Affiche la page du Notebook dont le numero est en parametre                                         */
/* Entrées: widget, data                                                                                                      */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 struct PAGE_NOTEBOOK *Chercher_page_notebook ( struct CLIENT *client, guint type, guint id, gboolean affiche )
  { struct PAGE_NOTEBOOK *page;
    GSList *liste;

printf("searching page type %d id %d\n", type, id );
    liste = client->Liste_pages;
    while(liste)
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if (page->type == type)                                    /* Si la page existe deja, on l'affiche */
        { switch( type )
           { case TYPE_PAGE_ATELIER:
                  if ( Json_get_int ( ((struct TYPE_INFO_ATELIER *)page->infos)->syn , "id" ) != id )
                   { liste = liste->next; continue; }
                  break;
             case TYPE_PAGE_HISTO_MSGS:
                  if ( ((struct TYPE_INFO_HISTO_MSGS *)page->infos)->page_id != id )
                   { liste = liste->next; continue; }
                  break;
             case TYPE_PAGE_SUPERVISION:
                  if ( Json_get_int ( ((struct TYPE_INFO_SUPERVISION *)page->infos)->syn , "id" ) != id )
                   { liste = liste->next; continue; }
                  break;
             case TYPE_PAGE_MNEMONIQUE:
                  if ( ((struct TYPE_INFO_MNEMONIQUE *)page->infos)->id != id )
                   { liste = liste->next; continue; }
                  break;
             case TYPE_PAGE_HORLOGE:
                  if ( ((struct TYPE_INFO_HORLOGE *)page->infos)->id_mnemo != id )
                   { liste = liste->next; continue; }
                  break;
             case TYPE_PAGE_SOURCE_DLS:
                  if ( ((struct TYPE_INFO_SOURCE_DLS *)page->infos)->rezo_dls.id != id )
                   { liste = liste->next; continue; }
                  break;
             default: break;
           }
          if (affiche)
           { gtk_notebook_set_current_page( GTK_NOTEBOOK(client->Notebook),
                                            gtk_notebook_page_num( GTK_NOTEBOOK(client->Notebook), page->child )
                                          );
           }
          return(page);
        }
       liste = liste->next;
     }
printf("not found\n");
    return(NULL);
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Changer_page_client->Notebook: Affiche la page du client->Notebook dont le numero est en parametre                         */
/* Entrées: widget, data                                                                                                      */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 gboolean Tester_page_notebook ( guint type )
  { struct PAGE_NOTEBOOK *page;
    GList *liste;

    liste = client->Liste_pages;
    while(liste)
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if (page->type == type)                                    /* Si la page existe deja, on l'affiche */
        { return(TRUE); }
       liste = liste->next;
     }
    return(FALSE);
  }
#endif
/******************************************************************************************************************************/
/* Creer_boite_travail: Creation de la zone de choix/edition... des mots de passe                                             */
/* Entrée: Néant                                                                                                              */
/* Sortie: Widget *boite, référençant la boite                                                                                */
/******************************************************************************************************************************/
 GtkWidget *Creer_boite_travail ( struct CLIENT *client )
  { GtkWidget *vboite, *hboite, *texte;

    vboite = gtk_box_new( GTK_ORIENTATION_VERTICAL, 6 );

    client->Notebook = gtk_notebook_new();
    gtk_box_pack_start( GTK_BOX(vboite), client->Notebook, TRUE, TRUE, 0 );
    gtk_container_set_border_width( GTK_CONTAINER(client->Notebook), 6 );
    gtk_notebook_set_scrollable (GTK_NOTEBOOK(client->Notebook), TRUE );
    gtk_notebook_append_page ( GTK_NOTEBOOK(client->Notebook), Creer_page_histo(client), gtk_label_new("Fil de l'eau") );

    hboite = gtk_box_new( GTK_ORIENTATION_HORIZONTAL, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_box_set_spacing ( GTK_BOX(hboite), 6 );
    gtk_box_pack_start( GTK_BOX(vboite), hboite, FALSE, FALSE, 0 );

/********************* Status de la machine cliente:connectée/erreur/loguée... ****************************/
    texte = gtk_label_new( "Status" );
    gtk_box_pack_start( GTK_BOX(hboite), texte, FALSE, FALSE, 0 );
    client->Entry_status = gtk_entry_new();
    gtk_entry_set_icon_from_icon_name ( GTK_ENTRY(client->Entry_status), GTK_ENTRY_ICON_PRIMARY, "user-info");
    g_object_set (client->Entry_status, "editable", FALSE, NULL );
    gtk_box_pack_start( GTK_BOX(hboite), client->Entry_status, TRUE, TRUE, 0 );

    client->Barre_pulse = gtk_progress_bar_new ();
    gtk_progress_bar_set_pulse_step( GTK_PROGRESS_BAR (client->Barre_pulse), 1.0 );
    gtk_box_pack_start( GTK_BOX(hboite), client->Barre_pulse, FALSE, FALSE, 0 );

    client->Barre_progress = gtk_progress_bar_new ();
    gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR (client->Barre_progress), 0.75 );
    gtk_progress_bar_set_show_text( GTK_PROGRESS_BAR (client->Barre_progress), TRUE );
    gtk_progress_bar_set_text( GTK_PROGRESS_BAR (client->Barre_progress), "Ready" );
    gtk_box_pack_start( GTK_BOX(hboite), client->Barre_progress, FALSE, FALSE, 0 );

    return(vboite);
 }
/*--------------------------------------------------------------------------------------------------------*/
