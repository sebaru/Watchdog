/**********************************************************************************************************/
/* Client/supervision_comment.c        Affichage du synoptique de supervision                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 28 nov 2004 13:05:11 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * supervision_comment.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - 
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
 #include <sys/time.h>
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CLIENT Client_en_cours;                           /* Identifiant de l'utilisateur en cours */

/**********************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                      */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_comment_supervision( struct CMD_TYPE_COMMENT *rezo_comment )
  { struct TRAME_ITEM_COMMENT *trame_comment;
    struct TYPE_INFO_SUPERVISION *infos;
    struct COMMENTAIRE *comment;
printf("bouh\n");
    infos = Rechercher_infos_supervision_par_id_syn ( rezo_comment->syn_id );
printf("yo\n");
    if (!(infos && infos->Trame)) { printf("test\n"); return; }
    comment = (struct COMMENTAIRE *)g_malloc0( sizeof(struct COMMENTAIRE) );
    if (!comment)
     { Info( Client_en_cours.config.log, DEBUG_MEM, "Afficher_comment_supervision: not enought memory" );
       return;
     }

    comment->id = rezo_comment->id;                               /* Numero du comment dans la DBWatchdog */
    printf("Libelle comment=%s id %d\n", rezo_comment->libelle, rezo_comment->id );
    memcpy( comment->libelle, rezo_comment->libelle, sizeof(comment->libelle) );
    memcpy( comment->font, rezo_comment->font, sizeof(comment->font) );
    comment->position_x = rezo_comment->position_x;                          /* en abscisses et ordonnées */
    comment->position_y = rezo_comment->position_y;
    comment->rouge      = rezo_comment->rouge;
    comment->vert       = rezo_comment->vert;
    comment->bleu       = rezo_comment->bleu;

    trame_comment = Trame_ajout_commentaire ( TRUE, infos->Trame, comment );
  }
/*--------------------------------------------------------------------------------------------------------*/
