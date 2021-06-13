/**********************************************************************************************************/
/* Client/timer.c         Gestion des échéances temporelles                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                       dim 28 nov 2004 14:11:04 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * timer.c
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
 #include "client.h"
 #include "trame.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/******************************************************************************************************************************/
/* Timer_motif: gestion des échéances temporelles des motifs.                                                                 */
/* Entree: trame_motif                                                                                                        */
/* Sortie: TRUE                                                                                                               */
/******************************************************************************************************************************/
 static void Timer_svg( struct TRAME_ITEM_SVG *trame_svg, gboolean hidden )
  { if (hidden) g_object_set ( trame_svg->item, "visibility", GOO_CANVAS_ITEM_INVISIBLE, NULL );
           else g_object_set ( trame_svg->item, "visibility", GOO_CANVAS_ITEM_VISIBLE, NULL );
  }
/**********************************************************************************************************/
/* Timer_motif: gestion des échéances temporelles des motifs.                                             */
/* Entree: trame_motif                                                                                    */
/* Sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static void Timer_motif( struct TRAME_ITEM_MOTIF *trame_motif, gint cligno )
  { switch( Json_get_int ( trame_motif->visuel, "gestion" ) )
     { case TYPE_INERTE    : return;
       case TYPE_STATIQUE  : break;

       case TYPE_PROGRESSIF:
            if (trame_motif->mode && trame_motif->num_image != trame_motif->nbr_images-1)
             { Trame_choisir_frame ( trame_motif, trame_motif->num_image+1, trame_motif->color );
             }
            else if (!trame_motif->mode && trame_motif->num_image != 0)
             { Trame_choisir_frame ( trame_motif, trame_motif->num_image-1, trame_motif->color );
             }
            break;
       case TYPE_CYCLIQUE_0N:
            if (trame_motif->mode)
             {  if (trame_motif->num_image != trame_motif->nbr_images-1)
                 { Trame_choisir_frame ( trame_motif, trame_motif->num_image+1, trame_motif->color );
                 }
                else
                 { Trame_choisir_frame ( trame_motif, 0, trame_motif->color );
                 }
             }
            else
             { Trame_choisir_frame ( trame_motif, 0, trame_motif->color );
             }
            break;
       case TYPE_CYCLIQUE_1N:
            if (trame_motif->mode)
             {  if (trame_motif->num_image != trame_motif->nbr_images-1)
                 { Trame_choisir_frame ( trame_motif, trame_motif->num_image+1, trame_motif->color );
                 }
                else
                 { Trame_choisir_frame ( trame_motif, 1, trame_motif->color );
                 }
             }
            else
             { Trame_choisir_frame ( trame_motif, 0, trame_motif->color );
             }
            break;
       case TYPE_CYCLIQUE_2N:
            if (trame_motif->mode >= 2)
             {  if (trame_motif->num_image != trame_motif->nbr_images-1)
                 { Trame_choisir_frame ( trame_motif, trame_motif->num_image+1, trame_motif->color );
                 }
                else
                 { Trame_choisir_frame ( trame_motif, 2, trame_motif->color );
                 }
             }
            else if (trame_motif->mode == 1)
             { Trame_choisir_frame ( trame_motif, 1, trame_motif->color );
             }
            else
             { Trame_choisir_frame ( trame_motif, 0, trame_motif->color );
             }
            break;
     }

    if (trame_motif->cligno == 1 && !cligno)
     { Trame_peindre_motif ( trame_motif, "black" ); }

    if ( cligno && (strcmp(trame_motif->color, trame_motif->en_cours_color) ) )
     { Trame_peindre_motif ( trame_motif, trame_motif->color );
     }
  }
/**********************************************************************************************************/
/* Timer: gestion des échéances temporelles. Appelée toutes les 333 millisecondes                         */
/* Entree: data inutile                                                                                   */
/* Sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 gboolean Timer ( gpointer data )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    struct TYPE_INFO_SUPERVISION *infos;
    struct PAGE_NOTEBOOK *page = data;
    GList *liste_motifs;
    GSList *liste_timer;

    infos = (struct TYPE_INFO_SUPERVISION *)page->infos;
    if (! (infos && infos->Trame) ) return(FALSE);

    pthread_mutex_lock ( &infos->Trame );
    liste_motifs = infos->Trame->trame_items;
    while (liste_motifs)
     { switch( *((gint *)liste_motifs->data) )
        { case TYPE_MOTIF      : trame_motif = (struct TRAME_ITEM_MOTIF *)liste_motifs->data;
                                 Timer_motif( trame_motif, infos->timer_hidden );
                                 break;
          case TYPE_COMMENTAIRE:
                                 break;
          case TYPE_CADRAN:      break;
          case TYPE_CAMERA_SUP : break;
          /*default: printf("Timer: type inconnu\n" );*/
        }
       liste_motifs=liste_motifs->next;
     }
    pthread_mutex_unlock ( &infos->Trame );

    liste_timer = infos->Trame->Liste_timer;
    while (liste_timer)
     { struct TRAME_ITEM_SVG *trame_svg = liste_timer->data;
       liste_timer = g_slist_next ( liste_timer );                                             /* On prepare le prochain coup */
       if (trame_svg->cligno)
         { Timer_svg( trame_svg, infos->timer_hidden ); }                                 /* Si cligno, on le fait clignotter */
        else                                                                                  /* Sinon on le vire de la liste */
         { infos->Trame->Liste_timer = g_slist_remove ( infos->Trame->Liste_timer, trame_svg );
           Timer_svg( trame_svg, FALSE );                                                 /* Si cligno, on le fait clignotter */
         }
     }

    if (infos->timer_hidden) infos->timer_hidden=FALSE; else infos->timer_hidden=TRUE;
    return(TRUE);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
