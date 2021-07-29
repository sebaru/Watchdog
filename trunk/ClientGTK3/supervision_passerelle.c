/******************************************************************************************************************************/
/* Client/supervision_passerelle.c        Affichage du synoptique de supervision                                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 04 avr 2009 12:29:01 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * supervision_passerelle.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - sebastien
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

 #include "Reseaux.h"
 #include "trame.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/******************************************************************************************************************************/
/* Changer_vue: Demande au serveur une nouvelle vue                                                                           */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Supervision_clic_passerelle (GooCanvasItem *canvasitem, GooCanvasItem *target,
                                       GdkEvent *event, struct TRAME_ITEM_PASS *trame_pass )
  { if ( !(event->button.button == 1 &&                                                                     /* clic gauche ?? */
           event->type == GDK_BUTTON_PRESS)
       ) return(FALSE);

    Demander_synoptique_supervision ( trame_pass->page->client, Json_get_int ( trame_pass->pass, "syn_cible_id" ) );
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
