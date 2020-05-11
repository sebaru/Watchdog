/**********************************************************************************************************/
/* Client/supervision_comment.c        Affichage du synoptique de supervision                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                       dim 28 nov 2004 13:05:11 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * supervision_comment.c
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
/* Afficher_un_commentaire: Ajoute un commentaire sur un synoptique                                                           */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Afficher_un_commentaire (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct TYPE_INFO_SUPERVISION *infos=user_data;
    struct CMD_TYPE_COMMENT *comment;

    comment = (struct CMD_TYPE_COMMENT *)g_try_malloc0( sizeof(struct CMD_TYPE_COMMENT) );
    if (!comment)
     { return;
     }

    comment->id           = atoi(Json_get_string ( element, "id" ));
    comment->syn_id       = atoi(Json_get_string ( element, "syn_id" ));
    comment->position_x   = atoi(Json_get_string ( element, "posx" ));
    comment->position_y   = atoi(Json_get_string ( element, "posy" ));
    comment->angle        = atoi(Json_get_string ( element, "angle" ));
    comment->rouge        = atoi(Json_get_string ( element, "rouge" ));
    comment->vert         = atoi(Json_get_string ( element, "vert" ));
    comment->bleu         = atoi(Json_get_string ( element, "bleu" ));
    //comment->font_size    = atoi(Json_get_string ( element, "bleu" ));
    g_snprintf( comment->libelle, sizeof(comment->libelle), "%s", Json_get_string ( element, "libelle" ));
    g_snprintf( comment->font,    sizeof(comment->font),    "%s", Json_get_string ( element, "font" ));
    //g_snprintf( comment->def_color,    sizeof(comment->def_color),    "%s", Json_get_string ( element, "def_color" ));

    Trame_ajout_commentaire ( TRUE, infos->Trame, comment );
  }
/*--------------------------------------------------------------------------------------------------------*/
