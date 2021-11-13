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

 #include <gnome.h>
 #include <sys/time.h>
 
 #include "Reseaux.h"
 #include "Config_cli.h"
 #include "trame.h"

 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                         /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/**********************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                      */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_comment_supervision( struct CMD_TYPE_COMMENT *rezo_comment )
  { struct TYPE_INFO_SUPERVISION *infos;
    struct CMD_TYPE_COMMENT *comment;

    infos = Rechercher_infos_supervision_par_id_syn ( rezo_comment->syn_id );
    if (!(infos && infos->Trame)) { printf("test\n"); return; }
    comment = (struct CMD_TYPE_COMMENT *)g_try_malloc0( sizeof(struct CMD_TYPE_COMMENT) );
    if (!comment)
     { return;
     }

    memcpy( comment, rezo_comment, sizeof(struct CMD_TYPE_COMMENT) );
    Trame_ajout_commentaire ( TRUE, infos->Trame, comment );
  }
/*--------------------------------------------------------------------------------------------------------*/
