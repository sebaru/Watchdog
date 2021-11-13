/******************************************************************************************************************************/
/* Client/atelier_ajout_comment.c     Gestion des commentaires pour Watchdog                                                  */
/* Projet WatchDog version 3.7     Gestion d'habitat                                             jeu 03 fév 2005 16:25:06 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier_ajout_comment.c
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
/* Afficher_un_commentaire: Ajoute un commentaire sur un synoptique                                                           */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Afficher_un_commentaire (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct PAGE_NOTEBOOK *page=user_data;
    struct TRAME_ITEM_COMMENT *trame_comment;
printf("%s: %s\n", __func__, Json_get_string ( element, "libelle" ) );
    if (page->type == TYPE_PAGE_SUPERVISION)
     { struct TYPE_INFO_SUPERVISION *infos=page->infos;
       Trame_ajout_commentaire ( FALSE, infos->Trame, element );
     }
    else if (page->type == TYPE_PAGE_ATELIER)
     { struct TYPE_INFO_ATELIER *infos=page->infos;
       trame_comment = Trame_ajout_commentaire ( TRUE, infos->Trame_atelier, element );
       g_signal_connect( G_OBJECT(trame_comment->item_groupe), "button-press-event",
                         G_CALLBACK(Clic_sur_comment), trame_comment );
       g_signal_connect( G_OBJECT(trame_comment->item_groupe), "button-release-event",
                         G_CALLBACK(Clic_sur_comment), trame_comment );
       g_signal_connect( G_OBJECT(trame_comment->item_groupe), "enter-notify-event",
                         G_CALLBACK(Clic_sur_comment), trame_comment );
       g_signal_connect( G_OBJECT(trame_comment->item_groupe), "leave-notify-event",
                         G_CALLBACK(Clic_sur_comment), trame_comment );
       g_signal_connect( G_OBJECT(trame_comment->item_groupe), "motion-notify-event",
                         G_CALLBACK(Clic_sur_comment), trame_comment );
       /*"Bitstream Vera Serif Italic 22" );
         "Bitstream Vera Serif Bold Italic 12" );
         "Bitstream Vera Serif Italic 10" );*/
    }

  }
/*--------------------------------------------------------------------------------------------------------*/
