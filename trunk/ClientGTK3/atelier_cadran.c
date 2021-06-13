/******************************************************************************************************************************/
/* Client/atelier_ajout_cadran.c     Gestion des cadrans synoptiques pour Watchdog                                            */
/* Projet WatchDog version 1.5     Gestion d'habitat                                             dim 29 jan 2006 16:59:01 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier_ajout_cadran.c
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
/* Afficher_un_cadran: Ajoute un cadran sur la trame                                                                          */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Afficher_un_cadran (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct PAGE_NOTEBOOK *page=user_data;
    struct TRAME_ITEM_CADRAN *trame_cadran;
printf("%s : %s:%s posx %f, posy %f min=%f, max=%f nb_decimal=%d\n",
        __func__, Json_get_string( element, "tech_id" ), Json_get_string ( element, "acronyme" ),
                                        Json_get_double ( element, "posx" ), Json_get_double ( element, "posy" ),
                                        Json_get_double ( element, "minimum" ), Json_get_double ( element, "maximum" ),
                                        Json_get_int ( element, "nb_decimal" )
      );


    if (!page) return;
    if (page->type == TYPE_PAGE_SUPERVISION)
     { struct TYPE_INFO_SUPERVISION *infos=page->infos;
       trame_cadran = Trame_ajout_cadran ( FALSE, infos->Trame, element );
       g_signal_connect( G_OBJECT(trame_cadran->item_groupe), "button-press-event",
                         G_CALLBACK(Clic_sur_cadran_supervision), trame_cadran );
     }
    else if (page->type == TYPE_PAGE_ATELIER)
     { struct TYPE_INFO_ATELIER *infos=page->infos;
       trame_cadran = Trame_ajout_cadran ( TRUE, infos->Trame_atelier, element );
       g_signal_connect( G_OBJECT(trame_cadran->item_groupe), "button-press-event",
                         G_CALLBACK(Clic_sur_cadran), trame_cadran );
       g_signal_connect( G_OBJECT(trame_cadran->item_groupe), "button-release-event",
                         G_CALLBACK(Clic_sur_cadran), trame_cadran );
       g_signal_connect( G_OBJECT(trame_cadran->item_groupe), "enter-notify-event",
                         G_CALLBACK(Clic_sur_cadran), trame_cadran );
       g_signal_connect( G_OBJECT(trame_cadran->item_groupe), "leave-notify-event",
                         G_CALLBACK(Clic_sur_cadran), trame_cadran );
       g_signal_connect( G_OBJECT(trame_cadran->item_groupe), "motion-notify-event",
                         G_CALLBACK(Clic_sur_cadran), trame_cadran );
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
