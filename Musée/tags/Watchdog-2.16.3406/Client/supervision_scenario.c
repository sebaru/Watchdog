/******************************************************************************************************************************/
/* Client/supervision_scenario.c        Affichage des scenarios synoptique de supervision                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    23.07.2017 17:24:07 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * supervision_scenario.c
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

 extern GList *Liste_pages;                                                       /* Liste des pages ouvertes sur le notebook */  
 extern GtkWidget *Notebook;                                                             /* Le Notebook de controle du client */
 extern GtkWidget *F_client;                                                                         /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                                              /* Configuration generale cliente watchdog */

/******************************************* Définitions des prototypes programme *********************************************/
 #include "protocli.h"

/******************************************************************************************************************************/
/* Proto_afficher_un_scenario_supervision: Ajoute un scenario sur la trame de supervision                                     */
/* Entrée: une reference sur le scenario                                                                                      */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_afficher_un_scenario_supervision( struct CMD_TYPE_SCENARIO *rezo_scenario )
  { struct TRAME_ITEM_SCENARIO *trame_scenario;
    struct TYPE_INFO_SUPERVISION *infos;
    struct CMD_TYPE_SCENARIO *scenario;

    infos = Rechercher_infos_supervision_par_id_syn ( rezo_scenario->syn_id );
    if (!(infos && infos->Trame)) return;
    scenario = (struct CMD_TYPE_SCENARIO *)g_try_malloc0( sizeof(struct CMD_TYPE_SCENARIO) );
    if (!scenario)
     { return;
     }
    memcpy ( scenario, rezo_scenario, sizeof( struct CMD_TYPE_SCENARIO ) );

    trame_scenario = Trame_ajout_scenario ( FALSE, infos->Trame, scenario );
    trame_scenario->groupe_dpl = Nouveau_groupe();                                    /* Numéro de groupe pour le deplacement */
    g_signal_connect( G_OBJECT(trame_scenario->item_groupe), "button-press-event",
                      G_CALLBACK(Clic_sur_scenario_supervision), trame_scenario );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
