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
    struct TYPE_INFO_SUPERVISION *infos_supervision;
    struct TYPE_INFO_ATELIER     *infos_atelier;

printf("%s : %s:%s\n", __func__, Json_get_string( element, "tech_id" ), Json_get_string ( element, "acronyme" ) );
    if (!page) return;

    if (page->type == TYPE_PAGE_SUPERVISION)  { infos_supervision = page->infos; }
    else if (page->type == TYPE_PAGE_ATELIER) { infos_atelier     = page->infos; }
    else return;


    if ( Json_has_member ( element, "ihm_affichage" ) )
     { if (!strcasecmp( Json_get_string ( element, "ihm_affichage" ), "complexe" ) )
        { Trame_ajout_visuel_complexe ( page, element ); }
       else
        { Trame_ajout_visuel_simple   ( page, element ); }
       return;
     }
 
    if (page->type == TYPE_PAGE_SUPERVISION)
     { Trame_ajout_visuel_old ( FALSE, infos_supervision->Trame, element ); }
    else if (page->type == TYPE_PAGE_ATELIER)
     { Trame_ajout_visuel_old ( TRUE, infos_atelier->Trame_atelier, element ); }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
