/**********************************************************************************************************/
/* Client/supervision_cadran.c        Affichage des cadrans synoptique de supervision                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mer 01 fév 2006 18:41:37 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * supervision_cadran.c
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
/* Proto_afficher_un_cadran_supervision: Ajoute un cadran sur la trame de supervision                     */
/* Entrée: une reference sur le cadran                                                                    */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_cadran_supervision( struct CMD_TYPE_CADRAN *rezo_cadran )
  { struct TRAME_ITEM_CADRAN *trame_cadran;
    struct TYPE_INFO_SUPERVISION *infos;
    struct CMD_TYPE_CADRAN *cadran;

    infos = Rechercher_infos_supervision_par_id_syn ( rezo_cadran->syn_id );
    if (!(infos && infos->Trame)) return;
    cadran = (struct CMD_TYPE_CADRAN *)g_try_malloc0( sizeof(struct CMD_TYPE_CADRAN) );
    if (!cadran)
     { return;
     }
    memcpy ( cadran, rezo_cadran, sizeof( struct CMD_TYPE_CADRAN ) );

    trame_cadran = Trame_ajout_cadran ( FALSE, infos->Trame, cadran );
    trame_cadran->groupe_dpl = Nouveau_groupe();                 /* Numéro de groupe pour le deplacement */
    g_signal_connect( G_OBJECT(trame_cadran->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_cadran_supervision), trame_cadran );
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_message: Rafraichissement du message en parametre                                   */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_changer_etat_cadran( struct CMD_ETAT_BIT_CADRAN *etat_cadran )
  { struct TRAME_ITEM_CADRAN *trame_cadran;
    struct TYPE_INFO_SUPERVISION *infos;
    struct PAGE_NOTEBOOK *page;
    GList *liste_cadrans;
    GList *liste;
    gint cpt;

    cpt = 0;                                                 /* Nous n'avons encore rien fait au debut !! */
    liste = Liste_pages;
    while(liste)                                              /* On parcours toutes les pages SUPERVISION */
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if (page->type != TYPE_PAGE_SUPERVISION) { liste = liste->next; continue; }
       infos = (struct TYPE_INFO_SUPERVISION *)page->infos;

       liste_cadrans = infos->Trame->trame_items;          /* On parcours tous les cadrans de chaque page */
       while (liste_cadrans)
        { switch( *((gint *)liste_cadrans->data) )
           { case TYPE_CADRAN    : cpt++;                            /* Nous updatons un cadran de plus ! */ 
                                    trame_cadran = (struct TRAME_ITEM_CADRAN *)liste_cadrans->data;

                                    if (etat_cadran->bit_controle == trame_cadran->cadran->bit_controle &&
                                        etat_cadran->type == trame_cadran->cadran->type
                                       )
                                     { printf("Proto_changer_etat_cadran: change %s\n", etat_cadran->libelle );
                                       g_object_set( trame_cadran->item_entry,
                                                     "text", etat_cadran->libelle, NULL );
                                     }
                                    break;
             case TYPE_MOTIF:
             case TYPE_COMMENTAIRE:
             case TYPE_PASSERELLE:
                                    break;
             default: break;
           }
          liste_cadrans=liste_cadrans->next;
        }
       liste = liste->next;
     }
    if (!cpt)             /* Si nous n'avons rien mis à jour, c'est que le bit Ixxx ne nous est pas utile */
     { Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_CHANGE_CADRAN_UNKNOWN,
                      (gchar *)etat_cadran, sizeof(struct CMD_ETAT_BIT_CADRAN) ); 
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
