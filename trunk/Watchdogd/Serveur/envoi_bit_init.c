/******************************************************************************************************************************/
/* Watchdogd/Serveur/envoi_bit_init.c        Envoi des bits synoptiques initiaux au client                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           mer 01 fév 2006 18:30:05 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * envoi_bit_init.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

/*************************************************** Prototypes de fonctions **************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/******************************************************************************************************************************/
/* Envoyer_ixxx_supervision: Envoi des etats initiaux motifs dans la trame supervision                                        */
/* Entrée: Le client, la liste des bits à envoyer ainsi que les capteurs                                                      */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Envoyer_bit_init_supervision ( struct CLIENT *client, GSList *Liste_Bits_i, GSList *Liste_Capteurs )
  { struct CMD_ETAT_BIT_CTRL init_etat;
    GSList *liste;
    guint bit_controle;

    liste = Liste_Bits_i;
    while(liste)
     { bit_controle = GPOINTER_TO_INT( liste->data );
    
       if (bit_controle>=NBR_BIT_CONTROLE) bit_controle=0;                                         /* Verification des bornes */
       if ( ! g_slist_find(client->Liste_bit_syns, GINT_TO_POINTER(bit_controle) ) )        /* Ajout dans la liste recurrente */
        { client->Liste_bit_syns = g_slist_prepend( client->Liste_bit_syns, GINT_TO_POINTER(bit_controle) );
          Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                   "Envoyer_bit_init_supervision: ajout du bit_syn %03d dans la liste",
                    bit_controle );
        }

       init_etat.num    = bit_controle;                             /* Initialisation de la structure avant envoi au client ! */
       init_etat.etat   = Partage->i[ bit_controle ].etat;
       init_etat.rouge  = Partage->i[ bit_controle ].rouge;
       init_etat.vert   = Partage->i[ bit_controle ].vert;
       init_etat.bleu   = Partage->i[ bit_controle ].bleu;
       init_etat.cligno = Partage->i[ bit_controle ].cligno;

       Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_MOTIF,
                     (gchar *)&init_etat, sizeof(struct CMD_ETAT_BIT_CTRL) );
       liste = liste->next;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
