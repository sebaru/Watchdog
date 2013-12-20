/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_bit_init.c        Envoi des bits synoptiques initiaux au client                */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mer 01 fév 2006 18:30:05 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Chercher_bit_capteurs: Renvoie 0 si l'element en argument est dans la liste                            */
/* Entrée: L'element                                                                                      */
/* Sortie: 0 si present, 1 sinon                                                                          */
/**********************************************************************************************************/
 gint Chercher_bit_capteurs ( struct CAPTEUR *element, struct CAPTEUR *cherche )
  { if (element->bit_controle == cherche->bit_controle &&
        element->type == cherche->type)
         return 0;
    else return 1;
  }
/**********************************************************************************************************/
/* Envoyer_ixxx_supervision: Envoi des etats initiaux motifs dans la trame supervision                    */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Envoyer_bit_init_supervision_thread ( struct CLIENT *client )
  { struct CMD_ETAT_BIT_CTRL init_etat;
    GList *liste;
    guint bit_controle;

    if (client->bit_init_syn)
     { liste = client->bit_init_syn;                                                    /* Debut de liste */
       while(liste)
        { bit_controle = GPOINTER_TO_INT( liste->data );
    
          if ( ! g_slist_find(client->Liste_bit_syns, GINT_TO_POINTER(bit_controle) ) )
           { client->Liste_bit_syns = g_slist_prepend( client->Liste_bit_syns, GINT_TO_POINTER(bit_controle) );
             Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                      "Envoyer_bit_init_supervision_thread: ajout du bit_syn %03d dans la liste",
                       bit_controle );
           }

          if (bit_controle>=NBR_BIT_CONTROLE) bit_controle=0;                  /* Verification des bornes */

          init_etat.num    = bit_controle;                                  /* Prévoir le champ cligno !! */
          init_etat.etat   = Partage->i[ bit_controle ].etat;
          init_etat.rouge  = Partage->i[ bit_controle ].rouge;
          init_etat.vert   = Partage->i[ bit_controle ].vert;
          init_etat.bleu   = Partage->i[ bit_controle ].bleu;
          init_etat.cligno = Partage->i[ bit_controle ].cligno;

          Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_MOTIF,
                        (gchar *)&init_etat, sizeof(struct CMD_ETAT_BIT_CTRL) );
          liste = liste->next;
        }
       g_list_free( client->bit_init_syn );
       client->bit_init_syn = NULL;
     }

    if (client->bit_init_capteur)
     { liste = client->bit_init_capteur;                                                /* Debut de liste */
       while(liste)
        { struct CAPTEUR *capteur;
          struct CMD_ETAT_BIT_CAPTEUR *init_capteur;

          if ( !g_list_find_custom(client->bit_capteurs, liste->data,
                                   (GCompareFunc) Chercher_bit_capteurs) )
           { capteur = (struct CAPTEUR *)g_try_malloc0( sizeof(struct CAPTEUR) );
             if (capteur) 
              { memcpy( capteur, liste->data, sizeof(struct CAPTEUR) );
                client->bit_capteurs = g_list_append( client->bit_capteurs, capteur );
              }
           } else capteur = (struct CAPTEUR *)liste->data;

          init_capteur = Formater_capteur(capteur);                    /* Formatage de la chaine associée */

          if (init_capteur)                                                               /* envoi client */
           { Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_CAPTEUR,
                           (gchar *)init_capteur, sizeof(struct CMD_ETAT_BIT_CAPTEUR) );
             g_free(init_capteur);                                                /* On libere la mémoire */
           }
          liste = liste->next;
        }
       g_list_foreach( client->bit_init_capteur, (GFunc) g_free, NULL );
       g_list_free( client->bit_init_capteur );
       client->bit_init_capteur = NULL;
     }
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit( NULL );
  }
/*--------------------------------------------------------------------------------------------------------*/

