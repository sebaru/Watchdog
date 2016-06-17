/**********************************************************************************************************/
/* Client/ihm.c        L'interface du client Watchdog v2.0                                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 21 aoû 2003 18:47:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ihm.c
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

 GtkWidget *Notebook;                                                /* Le Notebook de controle du client */
 GtkWidget *Entry_status;                                                 /* Status de la machine cliente */
 GList *Liste_pages = NULL;                                   /* Liste des pages ouvertes sur le notebook */  

 static gint nbr_enreg = 0, nbr_enreg_max = 0;
 static GtkWidget *Barre_pulse;                                                        /* Barre de pulse  */
 static GtkWidget *Barre_progress;                                                 /* Barre de chargement */
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/**********************************************************************************************************/
/* Detruire_page_plugin_dls: Detruit la page du notebook consacrée aux plugin_dlss watchdog               */
/* Entrée: rien                                                                                           */
/* Sortie: un widget boite                                                                                */
/**********************************************************************************************************/
 void Detruire_page ( struct PAGE_NOTEBOOK *page_a_virer )
  { gint num;
    num = gtk_notebook_page_num( GTK_NOTEBOOK(Notebook), GTK_WIDGET(page_a_virer->child) );
    if (num>=0)
     { switch(page_a_virer->type)
        { case TYPE_PAGE_ATELIER:
               Detruire_page_atelier( page_a_virer );
               break;
          case TYPE_PAGE_SUPERVISION:
               Detruire_page_supervision( page_a_virer );
               break;
          case TYPE_PAGE_COURBE:
               Detruire_page_courbe( page_a_virer );
               break;
          case TYPE_PAGE_HISTO_COURBE:
               Detruire_page_histo_courbe( page_a_virer );
               break;
        }
       gtk_notebook_remove_page( GTK_NOTEBOOK(Notebook), num );
       Liste_pages = g_list_remove( Liste_pages, page_a_virer );
       if (page_a_virer->infos) g_free(page_a_virer->infos);       /* Libération des infos le cas échéant */
       g_free(page_a_virer);
     }
    else printf("Detruire_page: Page non trouvée\n");
  }
/**********************************************************************************************************/
/* Detruire_page_plugin_dls: Detruit la page du notebook consacrée aux plugin_dlss watchdog               */
/* Entrée: rien                                                                                           */
/* Sortie: un widget boite                                                                                */
/**********************************************************************************************************/
 struct PAGE_NOTEBOOK *Page_actuelle ( void )
  { struct PAGE_NOTEBOOK *page;
    GtkWidget *child;
    GList * liste;
    gint num;

    num = gtk_notebook_get_current_page( GTK_NOTEBOOK(Notebook) );
    child = gtk_notebook_get_nth_page( GTK_NOTEBOOK(Notebook), num );

    liste = Liste_pages;
    while(liste)
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if ( page->child == child ) return(page);                                /* Nous l'avons trouvé !! */
       liste = liste->next;
     }
    return(NULL);
  }
/**********************************************************************************************************/
/* Effacer_pages: On efface toutes les pages restantes du notebook                                        */
/* Entrée: niet                                                                                           */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 void Effacer_pages ( void )
  { while(Liste_pages)
     { struct PAGE_NOTEBOOK *page;
       page = (struct PAGE_NOTEBOOK *)Liste_pages->data;
       Detruire_page( page );
       Liste_pages = g_list_remove( Liste_pages, page );
     }
  }
/**********************************************************************************************************/
/* Nbr_page_type: Renvoie le nombre de page notebook de type type                                         */
/* Entrée: Le type de page à compter                                                                      */
/* Sortie: Un entier                                                                                      */
/**********************************************************************************************************/
 gint Nbr_page_type ( gint type )
  { struct PAGE_NOTEBOOK *page;
    gint cpt;
    GList *liste;
    liste = Liste_pages;
    cpt = 0;
    while( liste )
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if ( page->type == type ) cpt++;                              /* Est-ce bien une page d'atelier ?? */
       liste = liste->next;                                                        /* On passe au suivant */
     }
    return(cpt);
  }
/**********************************************************************************************************/
/* Set_progress: Positionne la barre de progression de la fenetre                                         */
/* Entrées: val, max                                                                                      */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Set_progress_pulse( void )
  { gtk_progress_bar_pulse ( GTK_PROGRESS_BAR(Barre_pulse) );
  }
/**********************************************************************************************************/
/* Set_progress: Positionne la barre de progression de la fenetre                                         */
/* Entrées: val, max                                                                                      */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Set_progress_plus( gint plus )
  { gdouble fraction;
    gchar chaine[20];

    nbr_enreg += plus;
    if (nbr_enreg >= nbr_enreg_max)
     { nbr_enreg=0; nbr_enreg_max=0;
       gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR (Barre_progress), 1.0 );
       gtk_progress_bar_set_text( GTK_PROGRESS_BAR (Barre_progress), "100%" );
     }
    else
     { fraction = 1.0*nbr_enreg/nbr_enreg_max;
       gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR (Barre_progress), fraction );
       g_snprintf( chaine, sizeof(chaine), "%3.1f%%", 100.0*fraction );
       gtk_progress_bar_set_text( GTK_PROGRESS_BAR (Barre_progress), chaine );
     }
  }
/**********************************************************************************************************/
/* Set_progress_text: Positionne le texte de la barre de progression de la fenetre                        */
/* Entrées: un gchar *                                                                                    */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Set_progress_text( gchar *texte, gint max )
  { nbr_enreg_max += max;
  }
/**********************************************************************************************************/
/* Log: Afficher un texte dans l'entry status                                                             */
/* Entrée: la chaine de caracteres                                                                        */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Log( gchar *chaine )
  { gtk_entry_set_text( GTK_ENTRY(Entry_status), chaine ); }
/**********************************************************************************************************/
/* Changer_page_Notebook: Affiche la page du Notebook dont le numero est en parametre                     */
/* Entrées: widget, data                                                                                  */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 struct PAGE_NOTEBOOK *Chercher_page_notebook ( guint type, guint id, gboolean affiche )
  { struct PAGE_NOTEBOOK *page;
    GList *liste;
    liste = Liste_pages;
    while(liste)
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if (page->type == type)                                    /* Si la page existe deja, on l'affiche */
        { switch( type )
           { case TYPE_PAGE_ATELIER:
                  if ( ((struct TYPE_INFO_ATELIER *)page->infos)->syn.id != id )
                   { liste = liste->next; continue; }
                  break;
             case TYPE_PAGE_COURBE:
                  break;
             case TYPE_PAGE_HISTO_COURBE:
                  break;
             case TYPE_PAGE_HISTO_MSGS:
                  if ( ((struct TYPE_INFO_HISTO_MSGS *)page->infos)->page_id != id )
                   { liste = liste->next; continue; }
                  break;
             case TYPE_PAGE_SUPERVISION:
                  if ( ((struct TYPE_INFO_SUPERVISION *)page->infos)->syn_id != id )
                   { liste = liste->next; continue; }
                  break;
             case TYPE_PAGE_SOURCE_DLS:
                  if ( ((struct TYPE_INFO_SOURCE_DLS *)page->infos)->id != id )
                   { liste = liste->next; continue; }
                  break;
             default: break;
           }
          if (affiche)
           { gtk_notebook_set_current_page( GTK_NOTEBOOK(Notebook),
                                            gtk_notebook_page_num( GTK_NOTEBOOK(Notebook), page->child )
                                          );
           }
          return(page);
        }
       liste = liste->next;
     }
    return(NULL);
  }
/**********************************************************************************************************/
/* Changer_page_Notebook: Affiche la page du Notebook dont le numero est en parametre                     */
/* Entrées: widget, data                                                                                  */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 gboolean Tester_page_notebook ( guint type )
  { struct PAGE_NOTEBOOK *page;
    GList *liste;

    liste = Liste_pages;
    while(liste)
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if (page->type == type)                                    /* Si la page existe deja, on l'affiche */
        { return(TRUE); }
       liste = liste->next;
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Creer_boite_travail: Creation de la zone de choix/edition... des mots de passe                         */
/* Entrée: Néant                                                                                          */
/* Sortie: Widget *boite, référençant la boite                                                            */
/**********************************************************************************************************/
 GtkWidget *Creer_boite_travail ( void )
  { GtkWidget *vboite, *hboite, *texte;

    vboite = gtk_vbox_new( FALSE, 6 );
    
    Notebook = gtk_notebook_new();
    gtk_box_pack_start( GTK_BOX(vboite), Notebook, TRUE, TRUE, 0 );
    gtk_container_set_border_width( GTK_CONTAINER(Notebook), 6 );
    gtk_notebook_set_scrollable (GTK_NOTEBOOK(Notebook), TRUE );

    hboite = gtk_hbox_new( FALSE, 6 );
    gtk_container_set_border_width( GTK_CONTAINER(hboite), 6 );
    gtk_box_pack_start( GTK_BOX(vboite), hboite, FALSE, FALSE, 0 );

/********************* Status de la machine cliente:connectée/erreur/loguée... ****************************/
    texte = gtk_label_new( "Status" );
    gtk_box_pack_start( GTK_BOX(hboite), texte, FALSE, FALSE, 0 );
    Entry_status = gtk_entry_new();
    gtk_entry_set_editable( GTK_ENTRY(Entry_status), FALSE );
    gtk_box_pack_start( GTK_BOX(hboite), Entry_status, TRUE, TRUE, 0 );

    Barre_progress = gtk_progress_bar_new ();
    gtk_progress_bar_set_fraction ( GTK_PROGRESS_BAR (Barre_progress), 1.0 );
    gtk_progress_bar_set_text( GTK_PROGRESS_BAR (Barre_progress), "100%" );
    gtk_box_pack_start( GTK_BOX(hboite), Barre_progress, FALSE, FALSE, 0 );

    Barre_pulse = gtk_progress_bar_new ();
    gtk_box_pack_start( GTK_BOX(hboite), Barre_pulse, FALSE, FALSE, 0 );

    return(vboite);
 }
/*--------------------------------------------------------------------------------------------------------*/
