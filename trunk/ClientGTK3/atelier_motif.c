/******************************************************************************************************************************/
/* Client/atelier_ajout_motif.c         gestion des ajouts de motifs à la trame                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 08 mai 2004 11:13:34 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier_ajout_motif.c
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

/***************************************** Définitions des prototypes programme ***********************************************/
 #include "protocli.h"

/******************************************************************************************************************************/
/* Atelier_Afficher_un_motif: Ajoute un motif sur la page atelier                                                             */
/* Entrée: les données JSON recu de la requete HTTP                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Afficher_un_motif (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct PAGE_NOTEBOOK *page = user_data;
    struct TRAME_ITEM_MOTIF *trame_motif;

printf("%s : %s:%s\n", __func__, Json_get_string( element, "tech_id" ), Json_get_string ( element, "acronyme" ) );
    if (!page) return;

    if (page->type == TYPE_PAGE_SUPERVISION)
     { struct TYPE_INFO_SUPERVISION *infos=page->infos;
       trame_motif = Trame_ajout_visuel ( FALSE, infos->Trame, element );
       if (!trame_motif)
        { printf("Erreur creation d'un nouveau motif\n");
          return;                                                          /* Ajout d'un test anti seg-fault */
        }
       g_snprintf( trame_motif->color, sizeof(trame_motif->color), "%s", Json_get_string ( element, "def_color" ) );
       trame_motif->mode   = 0;                                                     /* Sauvegarde etat motif */
       trame_motif->cligno = 0;                                                     /* Sauvegarde etat motif */
       g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-press-event",
                         G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item_groupe), "button-release-event",
                         G_CALLBACK(Clic_sur_motif_supervision), trame_motif );
       /*Updater_un_visuel ( trame_motif, element );*/
     }
    else if (page->type == TYPE_PAGE_ATELIER)
     { struct TYPE_INFO_ATELIER *infos=page->infos;
       trame_motif = Trame_ajout_visuel ( TRUE, infos->Trame_atelier, element );
       if (!trame_motif) { return; }                                                                           /* Si probleme */
       g_signal_connect( G_OBJECT(trame_motif->item), "button-press-event",   G_CALLBACK(Clic_sur_motif), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "button-release-event", G_CALLBACK(Clic_sur_motif), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "enter-notify-event",   G_CALLBACK(Clic_sur_motif), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "leave-notify-event",   G_CALLBACK(Clic_sur_motif), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->item), "motion-notify-event",  G_CALLBACK(Clic_sur_motif), trame_motif );

       g_signal_connect( G_OBJECT(trame_motif->select_hg), "button-press-event",   G_CALLBACK(Agrandir_hg), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_hg), "button-release-event", G_CALLBACK(Agrandir_hg), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_hg), "motion-notify-event",  G_CALLBACK(Agrandir_hg), trame_motif );

       g_signal_connect( G_OBJECT(trame_motif->select_hd), "button-press-event",   G_CALLBACK(Agrandir_hd), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_hd), "button-release-event", G_CALLBACK(Agrandir_hd), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_hd), "motion-notify-event",  G_CALLBACK(Agrandir_hd), trame_motif );

       g_signal_connect( G_OBJECT(trame_motif->select_bg), "button-press-event",   G_CALLBACK(Agrandir_bg), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_bg), "button-release-event", G_CALLBACK(Agrandir_bg), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_bg), "motion-notify-event",  G_CALLBACK(Agrandir_bg), trame_motif );

       g_signal_connect( G_OBJECT(trame_motif->select_bd), "button-press-event",   G_CALLBACK(Agrandir_bd), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_bd), "button-release-event", G_CALLBACK(Agrandir_bd), trame_motif );
       g_signal_connect( G_OBJECT(trame_motif->select_bd), "motion-notify-event",  G_CALLBACK(Agrandir_bd), trame_motif );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
