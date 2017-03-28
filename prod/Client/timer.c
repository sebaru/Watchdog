/**********************************************************************************************************/
/* Client/timer.c         Gestion des échéances temporelles                                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 28 nov 2004 14:11:04 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * timer.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
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
 #include "Config_cli.h"
 #include "client.h"

 extern GList *Liste_pages;                                   /* Liste des pages ouvertes sur le notebook */  
 extern struct CLIENT Client;                           /* Identifiant de l'utilisateur en cours */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

/**********************************************************************************************************/
/* Timer_motif: gestion des échéances temporelles des motifs.                                             */
/* Entree: trame_motif                                                                                    */
/* Sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static void Timer_pass( struct TRAME_ITEM_PASS *trame_pass, gint cligno )
  {                                                                 /* Premier rectangle de la passerelle */
     if (trame_pass->cligno1 == 1 && !cligno &&                                    /* Gestion clignotement */
        (trame_pass->en_cours_rouge1 != 100 ||
         trame_pass->en_cours_vert1  != 100 ||
         trame_pass->en_cours_bleu1  != 100
        )
       )
     { Trame_peindre_pass_1 ( trame_pass, 100, 100, 100 ); }

    if ( cligno && (trame_pass->en_cours_rouge1 != trame_pass->rouge1 ||
                    trame_pass->en_cours_vert1  != trame_pass->vert1  ||
                    trame_pass->en_cours_bleu1  != trame_pass->bleu1
                   )
       )
     { Trame_peindre_pass_1 ( trame_pass, trame_pass->rouge1,
                                          trame_pass->vert1,
                                          trame_pass->bleu1 );
     }

                                                                   /* Deuxieme rectangle de la passerelle */
    if (trame_pass->cligno2 == 1 && !cligno &&                                     /* Gestion clignotement */
        (trame_pass->en_cours_rouge2 != 100 ||
         trame_pass->en_cours_vert2  != 100 ||
         trame_pass->en_cours_bleu2  != 100
        )
       )
     { Trame_peindre_pass_2 ( trame_pass, 0, 0, 0 ); }

    if ( cligno && (trame_pass->en_cours_rouge2 != trame_pass->rouge2 ||
                    trame_pass->en_cours_vert2  != trame_pass->vert2  ||
                    trame_pass->en_cours_bleu2  != trame_pass->bleu2
                   )
       )
     { Trame_peindre_pass_2 ( trame_pass, trame_pass->rouge2,
                                        trame_pass->vert2,
                                        trame_pass->bleu2 );
     }

/***************************************** Gestion de la vignette Sécurité des Personnes ******************/
    if (trame_pass->cligno3 == 1 && !cligno &&                                     /* Gestion clignotement */
        (trame_pass->en_cours_rouge3 != 100 ||
         trame_pass->en_cours_vert3  != 100 ||
         trame_pass->en_cours_bleu3  != 100
        )
       )
     { Trame_peindre_pass_3 ( trame_pass, 0, 0, 0 ); }

    if ( cligno && (trame_pass->en_cours_rouge3 != trame_pass->rouge3 ||
                    trame_pass->en_cours_vert3  != trame_pass->vert3  ||
                    trame_pass->en_cours_bleu3  != trame_pass->bleu3
                   )
       )
     { Trame_peindre_pass_3 ( trame_pass, trame_pass->rouge3,
                                        trame_pass->vert3,
                                        trame_pass->bleu3 );
     }

  }
/**********************************************************************************************************/
/* Timer_motif: gestion des échéances temporelles des motifs.                                             */
/* Entree: trame_motif                                                                                    */
/* Sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 static void Timer_motif( struct TRAME_ITEM_MOTIF *trame_motif, gint cligno )
  { switch( trame_motif->motif->type_gestion )
     { case TYPE_INERTE    : return;
       case TYPE_STATIQUE  : break;
                       
       case TYPE_PROGRESSIF:
            if (trame_motif->etat && trame_motif->num_image != trame_motif->nbr_images-1)
             { Trame_choisir_frame ( trame_motif, trame_motif->num_image+1,
                                                  trame_motif->rouge,
                                                  trame_motif->vert,
                                                  trame_motif->bleu );
             }
            else if (!trame_motif->etat && trame_motif->num_image != 0)
             { Trame_choisir_frame ( trame_motif, trame_motif->num_image-1,
                                                  trame_motif->rouge,
                                                  trame_motif->vert,
                                                  trame_motif->bleu );
             }
            break;
       case TYPE_CYCLIQUE_0N:
            if (trame_motif->etat)
             {  if (trame_motif->num_image != trame_motif->nbr_images-1)
                 { Trame_choisir_frame ( trame_motif, trame_motif->num_image+1,
                                                      trame_motif->rouge,
                                                      trame_motif->vert,
                                                      trame_motif->bleu );
                 }
                else
                 { Trame_choisir_frame ( trame_motif, 0, trame_motif->rouge,
                                                         trame_motif->vert,
                                                         trame_motif->bleu );
                 }
             }
            else
             { Trame_choisir_frame ( trame_motif, 0, trame_motif->rouge,
                                                     trame_motif->vert,
                                                     trame_motif->bleu );
             }
            break;
       case TYPE_CYCLIQUE_1N:
            if (trame_motif->etat)
             {  if (trame_motif->num_image != trame_motif->nbr_images-1)
                 { Trame_choisir_frame ( trame_motif, trame_motif->num_image+1,
                                                      trame_motif->rouge,
                                                      trame_motif->vert,
                                                      trame_motif->bleu );
                 }
                else
                 { Trame_choisir_frame ( trame_motif, 1, trame_motif->rouge,
                                                         trame_motif->vert,
                                                         trame_motif->bleu );
                 }
             }
            else
             { Trame_choisir_frame ( trame_motif, 0, trame_motif->rouge,
                                                     trame_motif->vert,
                                                     trame_motif->bleu );
             }
            break;
       case TYPE_CYCLIQUE_2N:
            if (trame_motif->etat >= 2)
             {  if (trame_motif->num_image != trame_motif->nbr_images-1)
                 { Trame_choisir_frame ( trame_motif, trame_motif->num_image+1,
                                                      trame_motif->rouge,
                                                      trame_motif->vert,
                                                      trame_motif->bleu );
                 }
                else
                 { Trame_choisir_frame ( trame_motif, 2, trame_motif->rouge,
                                                         trame_motif->vert,
                                                         trame_motif->bleu );
                 }
             }
            else if (trame_motif->etat == 1)
             { Trame_choisir_frame ( trame_motif, 1, trame_motif->rouge,
                                                     trame_motif->vert,
                                                     trame_motif->bleu );
             }
            else
             { Trame_choisir_frame ( trame_motif, 0, trame_motif->rouge,
                                                     trame_motif->vert,
                                                     trame_motif->bleu );
             }
            break;
     }

    if (trame_motif->cligno == 1 && !cligno &&                                    /* Gestion clignotement */
        (trame_motif->en_cours_rouge != 100 ||
         trame_motif->en_cours_vert  != 100 ||
         trame_motif->en_cours_bleu  != 100
        )
       )
     { Trame_peindre_motif ( trame_motif, 100, 100, 100 ); }

    if ( cligno && (trame_motif->en_cours_rouge != trame_motif->rouge ||
                    trame_motif->en_cours_vert  != trame_motif->vert  ||
                    trame_motif->en_cours_bleu  != trame_motif->bleu
                   )
       )
     { Trame_peindre_motif ( trame_motif, trame_motif->rouge,
                                          trame_motif->vert,
                                          trame_motif->bleu );
     }
  }
/**********************************************************************************************************/
/* Timer: gestion des échéances temporelles. Appelée toutes les 333 millisecondes                         */
/* Entree: data inutile                                                                                   */
/* Sortie: TRUE                                                                                           */
/**********************************************************************************************************/
 gboolean Timer ( gpointer data )
  { struct TRAME_ITEM_MOTIF *trame_motif;
    struct TRAME_ITEM_PASS *trame_pass;
    struct TYPE_INFO_SUPERVISION *infos;
    struct PAGE_NOTEBOOK *page;
    static gint nbr_cligno = 0;
    GList *liste_motifs;

    if (Client.mode != VALIDE) return(TRUE);

    page = Page_actuelle();
    if (!page) return (TRUE);

    if (page->type != TYPE_PAGE_SUPERVISION) return(TRUE);

    infos = (struct TYPE_INFO_SUPERVISION *)page->infos;
    if (! (infos && infos->Trame) ) return(TRUE);

    nbr_cligno++;                                                                    /* Gestion du cligno */
    if (nbr_cligno==2) nbr_cligno=0;

    liste_motifs = infos->Trame->trame_items;
    while (liste_motifs)
     { switch( *((gint *)liste_motifs->data) )
        { case TYPE_MOTIF      : trame_motif = (struct TRAME_ITEM_MOTIF *)liste_motifs->data;
                                 Timer_motif( trame_motif, (nbr_cligno < 1 ? 1 : 0) );
                                 break;
          case TYPE_COMMENTAIRE:                                
          case TYPE_CADRAN:     break;
          case TYPE_PASSERELLE : trame_pass = (struct TRAME_ITEM_PASS *)liste_motifs->data;
                                 Timer_pass( trame_pass, (nbr_cligno < 1 ? 1 : 0) );
                                 break;
          case TYPE_CAMERA_SUP : break;
          default: printf("Timer: type inconnu\n" );
        }
       liste_motifs=liste_motifs->next;
     }
    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
