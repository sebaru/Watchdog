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

#ifdef bouh
/**********************************************************************************************************/
/* Changer_vue_directe: Demande au serveur une nouvelle vue                                               */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Changer_vue_directe ( guint num_syn )
  { struct CMD_TYPE_SYNOPTIQUE cmd;
    if (Chercher_page_notebook( TYPE_PAGE_SUPERVISION, num_syn, TRUE )) return;

    cmd.id = num_syn;
    Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_WANT_PAGE_SUPERVISION,
                   (gchar *)&cmd, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
  }
/**********************************************************************************************************/
/* Changer_vue: Demande au serveur une nouvelle vue                                                       */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static gboolean Changer_vue (GooCanvasItem *canvasitem, GooCanvasItem *target,
                              GdkEvent          *event, struct CMD_TYPE_PASSERELLE *pass )
  { if ( !(event->button.button == 1 &&                                                 /* clic gauche ?? */
           event->type == GDK_BUTTON_PRESS)
       )
    return(FALSE);

    Changer_vue_directe ( pass->syn_cible_id );
    return(TRUE);
  }
#endif
/******************************************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                                          */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Afficher_une_passerelle (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { struct TYPE_INFO_SUPERVISION *infos=user_data;
    struct TRAME_ITEM_PASS *trame_pass;
    struct CMD_TYPE_PASSERELLE *pass;

    if (!(infos && infos->Trame)) return;
    pass = (struct CMD_TYPE_PASSERELLE *)g_try_malloc0( sizeof(struct CMD_TYPE_PASSERELLE) );
    if (!pass)
     { return;
     }

    pass->position_x   = atoi(Json_get_string ( element, "posx" ));
    pass->position_y   = atoi(Json_get_string ( element, "posy" ));
    pass->angle        = atoi(Json_get_string ( element, "angle" ));
    pass->id           = atoi(Json_get_string ( element, "id" ));
    pass->syn_id       = atoi(Json_get_string ( element, "syn_id" ));
    pass->syn_cible_id = atoi(Json_get_string ( element, "syn_cible_id" ));
    g_snprintf( pass->libelle, sizeof(pass->libelle), "%s", Json_get_string ( element, "page" ));
    //g_snprintf( pass->page,    sizeof(pass->page),    "%s", Json_get_string ( element, "page" ));

    trame_pass = Trame_ajout_passerelle ( FALSE, infos->Trame, pass );
//    g_signal_connect( G_OBJECT(trame_pass->item_groupe), "button-press-event",
//                      G_CALLBACK(Changer_vue), pass );
//    g_signal_connect( G_OBJECT(trame_pass->item_groupe), "button-release-event",
//                      G_CALLBACK(Changer_vue), pass );
  }
/*--------------------------------------------------------------------------------------------------------*/
