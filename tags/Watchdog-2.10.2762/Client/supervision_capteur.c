/**********************************************************************************************************/
/* Client/supervision_capteur.c        Affichage des capteurs synoptique de supervision                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mer 01 fév 2006 18:41:37 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * supervision_capteur.c
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
/* Proto_afficher_un_capteur_supervision: Ajoute un capteur sur la trame de supervision                   */
/* Entrée: une reference sur le capteur                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_afficher_un_capteur_supervision( struct CMD_TYPE_CAPTEUR *rezo_capteur )
  { struct TRAME_ITEM_CAPTEUR *trame_capteur;
    struct TYPE_INFO_SUPERVISION *infos;
    struct CMD_TYPE_CAPTEUR *capteur;

    infos = Rechercher_infos_supervision_par_id_syn ( rezo_capteur->syn_id );
    if (!(infos && infos->Trame)) return;
    capteur = (struct CMD_TYPE_CAPTEUR *)g_try_malloc0( sizeof(struct CMD_TYPE_CAPTEUR) );
    if (!capteur)
     { return;
     }

    memcpy ( capteur, rezo_capteur, sizeof( struct CMD_TYPE_CAPTEUR ) );

    trame_capteur = Trame_ajout_capteur ( FALSE, infos->Trame, capteur );
    trame_capteur->groupe_dpl = Nouveau_groupe();                 /* Numéro de groupe pour le deplacement */
    g_signal_connect( G_OBJECT(trame_capteur->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_capteur_supervision), trame_capteur );
  }
/**********************************************************************************************************/
/* Proto_rafrachir_un_message: Rafraichissement du message en parametre                                   */
/* Entrée: une reference sur le message                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_changer_etat_capteur( struct CMD_ETAT_BIT_CAPTEUR *etat_capteur )
  { struct TRAME_ITEM_CAPTEUR *trame_capteur;
    struct TYPE_INFO_SUPERVISION *infos;
    struct PAGE_NOTEBOOK *page;
    GList *liste_capteurs;
    GList *liste;
    gint cpt;

    cpt = 0;                                                 /* Nous n'avons encore rien fait au debut !! */
    liste = Liste_pages;
    while(liste)                                              /* On parcours toutes les pages SUPERVISION */
     { page = (struct PAGE_NOTEBOOK *)liste->data;
       if (page->type != TYPE_PAGE_SUPERVISION) { liste = liste->next; continue; }
       infos = (struct TYPE_INFO_SUPERVISION *)page->infos;

       liste_capteurs = infos->Trame->trame_items;        /* On parcours tous les capteurs de chaque page */
       while (liste_capteurs)
        { switch( *((gint *)liste_capteurs->data) )
           { case TYPE_CAPTEUR    : cpt++;                          /* Nous updatons un capteur de plus ! */ 
                                    trame_capteur = (struct TRAME_ITEM_CAPTEUR *)liste_capteurs->data;
printf("recu %d/%d, capteur=%d/%d\n", etat_capteur->type, etat_capteur->bit_controle,
       trame_capteur->capteur->type, trame_capteur->capteur->bit_controle );
                                    if (etat_capteur->bit_controle == trame_capteur->capteur->bit_controle &&
                                        etat_capteur->type == trame_capteur->capteur->type
                                       )
                                     { printf("Proto_changer_etat_capteur: change %s\n", etat_capteur->libelle );
                                       g_object_set( trame_capteur->item_entry,
                                                     "text", etat_capteur->libelle, NULL );
                                     }
                                    break;
             case TYPE_MOTIF:
             case TYPE_COMMENTAIRE:
             case TYPE_PASSERELLE:
                                    break;
             default: break;
           }
          liste_capteurs=liste_capteurs->next;
        }
       liste = liste->next;
     }
    if (!cpt)             /* Si nous n'avons rien mis à jour, c'est que le bit Ixxx ne nous est pas utile */
     { Envoi_serveur( TAG_SUPERVISION, SSTAG_CLIENT_CHANGE_CAPTEUR_UNKNOWN,
                      (gchar *)etat_capteur, sizeof(struct CMD_ETAT_BIT_CAPTEUR) ); 
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
