/******************************************************************************************************************************/
/* Client/supervision_camera.c        Affichage des camera_sups synoptique de supervision                                     */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       sam. 19 sept. 2009 15:54:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

/***************************************** Définitions des prototypes programme ***********************************************/
 #include "protocli.h"
/******************************************************************************************************************************/
/* Clic_sur_camera_supervision: Appelé quand un evenement est capté sur une camera de supervision                             */
/* Entrée: une structure Event                                                                                                */
/* Sortie :rien                                                                                                               */
/******************************************************************************************************************************/
 void Clic_sur_camera_sup_supervision ( GooCanvasItem *widget, GooCanvasItem *target,
                                        GdkEvent *event, struct TRAME_ITEM_CAMERA_SUP *trame_camera_sup )
  { if ( !(event->button.button == 1 &&                                                                     /* clic gauche ?? */
           event->type == GDK_BUTTON_PRESS)
       ) return;

    gint pid = fork();
    if (pid<0) return;
    else if (!pid)                                                                       /* Lancement de la ligne de commande */
     { execlp( "vlc", "vlc", trame_camera_sup->camera_sup->location, NULL );
       _exit(0);
     }
  }

/*--------------------------------------------------------------------------------------------------------*/
