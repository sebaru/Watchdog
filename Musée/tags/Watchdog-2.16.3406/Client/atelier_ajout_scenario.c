/******************************************************************************************************************************/
/* Client/atelier_ajout_scenario.c     Gestion des scenarioaires pour Watchdog                                                */
/* Projet WatchDog version 1.5     Gestion d'habitat                                                      17.07.2017 21:54:51 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * atelier_ajout_scenario.c
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
 #include <string.h>
 #include <stdlib.h>

 #include "Reseaux.h"
 #include "trame.h"
 #include "Config_cli.h"

/******************************************** Définitions des prototypes programme ********************************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                                              /* Configuration generale cliente watchdog */

 static GtkWidget *F_ajout_scenario = NULL;                                     /* Fenetre graphique de choix de scenario */

/******************************************************************************************************************************/
/* Id_vers_trame_motif: Conversion d'un id motif en sa reference TRAME                                                        */
/* Entrée: Un id motif                                                                                                        */
/* sortie: un struct TRAME_ITEM_MOTIF                                                                                         */
/******************************************************************************************************************************/
 static struct TRAME_ITEM_SCENARIO *Id_vers_trame_scenario ( struct TYPE_INFO_ATELIER *infos, gint id )
  { GList *liste;
    liste = infos->Trame_atelier->trame_items;
    while( liste )
     { if ( *(gint *)(liste->data) == TYPE_SCENARIO &&
            ((struct TRAME_ITEM_SCENARIO *)(liste->data))->scenario->id == id ) break;
       else liste = liste->next;
     }
    if (!liste)
     { printf("Id_vers_trame_scenario: item %d non trouvé\n", id );
       return(NULL);
     }
    return( (struct TRAME_ITEM_SCENARIO *)(liste->data) );
  }
/******************************************************************************************************************************/
/* Commenter: Met en route le processus permettant de scenario un synoptique                                                  */
/* Entrée: widget/data                                                                                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Menu_ajouter_scenario ( void )
  { struct TYPE_INFO_ATELIER *infos;
    struct PAGE_NOTEBOOK *page;
    struct CMD_TYPE_SCENARIO add_scenario;

    page = Page_actuelle();                                                                   /* On recupere la page actuelle */
    if (! (page && page->type==TYPE_PAGE_ATELIER) ) return;                                   /* Verification des contraintes */
    infos = (struct TYPE_INFO_ATELIER *)page->infos;                             /* Pointeur sur les infos de la page atelier */

    add_scenario.posx   = TAILLE_SYNOPTIQUE_X/2;
    add_scenario.posy   = TAILLE_SYNOPTIQUE_Y/2;                            
    add_scenario.num    = 0;
    add_scenario.syn_id = infos->syn.id;
    Envoi_serveur( TAG_ATELIER, SSTAG_CLIENT_ATELIER_ADD_SCENARIO,
                   (gchar *)&add_scenario, sizeof(struct CMD_TYPE_SCENARIO) );
  }
/******************************************************************************************************************************/
/* Afficher_un_message: Ajoute un message dans la liste des messages                                                          */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_afficher_un_scenario_atelier( struct CMD_TYPE_SCENARIO *rezo_scenario )
  { struct TRAME_ITEM_SCENARIO *trame_scenario;
    struct TYPE_INFO_ATELIER *infos;
    struct CMD_TYPE_SCENARIO *scenario;
        
    infos = Rechercher_infos_atelier_par_id_syn ( rezo_scenario->syn_id );
    scenario = (struct CMD_TYPE_SCENARIO *)g_try_malloc0( sizeof(struct CMD_TYPE_SCENARIO) );
    if (!scenario)
     { return;
     }

    memcpy ( scenario, rezo_scenario, sizeof(struct CMD_TYPE_SCENARIO) );

    trame_scenario = Trame_ajout_scenario ( TRUE, infos->Trame_atelier, scenario );
    if (!trame_scenario) return;
    trame_scenario->groupe_dpl = Nouveau_groupe();              /* Numéro de groupe pour le deplacement */

    g_signal_connect( G_OBJECT(trame_scenario->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_scenario), trame_scenario );
    g_signal_connect( G_OBJECT(trame_scenario->item_groupe), "button-release-event",
                      G_CALLBACK(Clic_sur_scenario), trame_scenario );
    g_signal_connect( G_OBJECT(trame_scenario->item_groupe), "enter-notify-event",
                      G_CALLBACK(Clic_sur_scenario), trame_scenario );
    g_signal_connect( G_OBJECT(trame_scenario->item_groupe), "leave-notify-event",
                      G_CALLBACK(Clic_sur_scenario), trame_scenario );
    g_signal_connect( G_OBJECT(trame_scenario->item_groupe), "motion-notify-event",
                      G_CALLBACK(Clic_sur_scenario), trame_scenario );
  }
/******************************************************************************************************************************/
/* Cacher_un_message: Enleve un message de la liste des messages                                                              */
/* Entrée: une reference sur le message                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_cacher_un_scenario_atelier( struct CMD_TYPE_SCENARIO *scenario )
  { struct TRAME_ITEM_SCENARIO *trame_scenario;
    struct TYPE_INFO_ATELIER *infos;
        
    infos = Rechercher_infos_atelier_par_id_syn ( scenario->syn_id );
    trame_scenario = Id_vers_trame_scenario( infos, scenario->id );
    printf("Proto_cacher_un_scenario_atelier debut: ID=%d %p\n", scenario->id, trame_scenario );
    if (!trame_scenario) return;
    Deselectionner( infos, (struct TRAME_ITEM *)trame_scenario );                 /* Au cas ou il aurait été selectionné... */
    Trame_del_scenario( trame_scenario );
    g_free(trame_scenario);
    infos->Trame_atelier->trame_items = g_list_remove( infos->Trame_atelier->trame_items, trame_scenario );
    printf("Proto_cacher_un_scenario_atelier fin..\n");
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
