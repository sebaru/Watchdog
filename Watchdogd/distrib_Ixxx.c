/**********************************************************************************************************/
/* Watchdogd/distrib_Ixxx.c        Distribution des changements d'etats motif                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 10 mai 2004 11:31:17 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * distrib_Ixxx.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
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

 #include <glib.h>
 #include <string.h>
 #include <unistd.h>
 #include <time.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_arrive_Ixxx_dls ( void )
  { struct CMD_ETAT_BIT_CTRL *new_motif;
    gint num, reste;

    if (!Partage->com_msrv.liste_i) return;                                   /* Si pas de i, on se barre */

    pthread_mutex_lock( &Partage->com_msrv.synchro );             /* Ajout dans la liste de msg a traiter */
    num = GPOINTER_TO_INT(Partage->com_msrv.liste_i->data);                /* Recuperation du numero de i */
    reste = g_list_length(Partage->com_msrv.liste_i);
    Partage->com_msrv.liste_i = g_list_remove ( Partage->com_msrv.liste_i, GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    Info_new( Config.log, Config.log_all, LOG_DEBUG,
              "Gerer_arrive_Ixxx_dls: Recu I(%03d)=%d, r%03d v%03d, b%03d. Reste a traiter %03d",
              num, Partage->i[num].etat,
              Partage->i[num].rouge, Partage->i[num].vert, Partage->i[num].bleu
            );

/***************************** Création de la structure passée aux clients ********************************/
    new_motif = (struct CMD_ETAT_BIT_CTRL *) g_malloc0( sizeof(struct CMD_ETAT_BIT_CTRL) );
    if (new_motif)
     { guint i;

       new_motif->num    = num;
       new_motif->etat   = Partage->i[num].etat;
       new_motif->rouge  = Partage->i[num].rouge;
       new_motif->vert   = Partage->i[num].vert;
       new_motif->bleu   = Partage->i[num].bleu;
       new_motif->cligno = Partage->i[num].cligno;

       for (i=0; i<Config.max_serveur; i++)
        { struct CMD_ETAT_BIT_CTRL *motif_ssrv;
          if (Partage->Sous_serveur[i].Thread_run == TRUE)
           { motif_ssrv = (struct CMD_ETAT_BIT_CTRL *)g_malloc0( sizeof( struct CMD_ETAT_BIT_CTRL ) );
             if (motif_ssrv)
              { memcpy ( motif_ssrv, new_motif, sizeof(struct CMD_ETAT_BIT_CTRL) );            /* Recopie */
                pthread_mutex_lock( &Partage->Sous_serveur[i].synchro );
                Partage->Sous_serveur[i].new_motif = g_list_append ( Partage->Sous_serveur[i].new_motif,
                                                                     motif_ssrv );
                pthread_mutex_unlock( &Partage->Sous_serveur[i].synchro );
              }
             else
              { Info_new( Config.log, Config.log_all, LOG_ERR, "Gerer_arrive_Ixxx_dls: not enough memory" ); }
           }
        }
       g_free (new_motif);
     }
    else
     { Info_new( Config.log, Config.log_all, LOG_ERR, 
                "Gerer_arrive_Ixxx_dls: Not enough memory to hangle I%03d", num );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
