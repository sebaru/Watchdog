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
/* Proto_afficher_un_camera_sup_supervision: Ajoute un camera_sup sur la trame de supervision             */
/* Entrée: une reference sur le camera_sup                                                                */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_camera_sup_supervision( struct CMD_TYPE_CAMERASUP *rezo_camera_sup )
  { struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup;
    struct TYPE_INFO_SUPERVISION *infos;
    struct CMD_TYPE_CAMERASUP *camera_sup;

    infos = Rechercher_infos_supervision_par_id_syn ( rezo_camera_sup->syn_id );
    if (!(infos && infos->Trame)) return;
    camera_sup = (struct CMD_TYPE_CAMERASUP *)g_try_malloc0( sizeof(struct CMD_TYPE_CAMERASUP) );
    if (!camera_sup)
     { return;
     }

    memcpy ( camera_sup, rezo_camera_sup, sizeof(struct CMD_TYPE_CAMERASUP) );

    trame_camera_sup = Trame_ajout_camera_sup ( FALSE, infos->Trame, camera_sup );
    g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_camera_sup_supervision), trame_camera_sup );
    g_signal_connect( G_OBJECT(trame_camera_sup->item_groupe), "button-release-event",
                      G_CALLBACK(Clic_sur_camera_sup_supervision), trame_camera_sup );
  }
/*--------------------------------------------------------------------------------------------------------*/
