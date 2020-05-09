/**********************************************************************************************************/
/* Client/supervision_camera.c        Affichage des camera_sups synoptique de supervision                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                   sam. 19 sept. 2009 15:54:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * supervision_camera.c
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

 #include "Reseaux.h"
 #include "trame.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/******************************************************************************************************************************/
/* Afficher_une_camera: Ajoute une camera sur un synoptique                                                                   */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Afficher_une_camera (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct TYPE_INFO_SUPERVISION *infos=user_data;
    struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    struct CMD_TYPE_CAMERASUP *camera_sup;

    if (!(infos && infos->Trame)) return;
    camera_sup = (struct CMD_TYPE_CAMERASUP *)g_try_malloc0( sizeof(struct CMD_TYPE_CAMERASUP) );
    if (!camera_sup)
     { return;
     }

    camera_sup->id     = atoi(Json_get_string ( element, "id" ));
    camera_sup->syn_id = atoi(Json_get_string ( element, "syn_id" ));
    camera_sup->posx   = atoi(Json_get_string ( element, "posx" ));
    camera_sup->posy   = atoi(Json_get_string ( element, "posy" ));
    //camera_sup->font_size    = atoi(Json_get_string ( element, "bleu" ));
    g_snprintf( camera_sup->libelle,  sizeof(camera_sup->libelle),  "%s", Json_get_string ( element, "libelle" ));
    g_snprintf( camera_sup->location, sizeof(camera_sup->location), "%s", Json_get_string ( element, "location" ));

    trame_camera_sup = Trame_ajout_camera_sup ( FALSE, infos->Trame, camera_sup );
    //g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "button-press-event",
    //                  G_CALLBACK(Clic_sur_camera_sup_supervision), trame_camera_sup );
    //g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "button-release-event",
    //                  G_CALLBACK(Clic_sur_camera_sup_supervision), trame_camera_sup );
  }
/*--------------------------------------------------------------------------------------------------------*/
