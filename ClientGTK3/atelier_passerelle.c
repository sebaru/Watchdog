/******************************************************************************************************************************/
/* Client/atelier_ajout_passerelle.c     Gestion des passerelles pour Watchdog                                                */
/* Projet WatchDog version 1.5     Gestion d'habitat                                             jeu 10 fév 2005 16:25:06 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier_ajout_passerelle.c
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
/******************************************* Définitions des prototypes programme *********************************************/
 #include "protocli.h"

/******************************************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                                          */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Afficher_une_passerelle (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct PAGE_NOTEBOOK *page = user_data;
    struct TRAME_ITEM_PASS *trame_pass;

    if (page->type == TYPE_PAGE_SUPERVISION)
     { struct TYPE_INFO_SUPERVISION *infos=page->infos;
       trame_pass = Trame_ajout_passerelle ( FALSE, infos->Trame, element );
       g_signal_connect( G_OBJECT(trame_pass->item_groupe), "button-press-event",   G_CALLBACK(Supervision_clic_passerelle), trame_pass );
       g_signal_connect( G_OBJECT(trame_pass->item_groupe), "button-release-event", G_CALLBACK(Supervision_clic_passerelle), trame_pass );
     }
    else if (page->type == TYPE_PAGE_ATELIER)
     { struct TYPE_INFO_ATELIER *infos=page->infos;
       trame_pass = Trame_ajout_passerelle ( TRUE, infos->Trame_atelier, element );
       g_signal_connect( G_OBJECT(trame_pass->item_groupe), "button-press-event", G_CALLBACK(Clic_sur_pass), trame_pass );
       g_signal_connect( G_OBJECT(trame_pass->item_groupe), "button-release-event", G_CALLBACK(Clic_sur_pass), trame_pass );
       g_signal_connect( G_OBJECT(trame_pass->item_groupe), "enter-notify-event", G_CALLBACK(Clic_sur_pass), trame_pass );
       g_signal_connect( G_OBJECT(trame_pass->item_groupe), "leave-notify-event", G_CALLBACK(Clic_sur_pass), trame_pass );
       g_signal_connect( G_OBJECT(trame_pass->item_groupe), "motion-notify-event", G_CALLBACK(Clic_sur_pass), trame_pass );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
