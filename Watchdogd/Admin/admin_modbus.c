/**********************************************************************************************************/
/* Watchdogd/Admin/admin_modbus.c        Gestion des connexions Admin MODBUS au serveur watchdog          */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_modbus.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - sebastien
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

 #include "Admin.h"
 #include "Modbus.h"
 #include "watchdogd.h"

 extern struct CONFIG Config;
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_modbus_list ( struct CLIENT_ADMIN *client )
  { GList *liste_modules, *liste_bornes;
    gchar chaine[128];

    liste_modules = Partage->com_modbus.Modules_MODBUS;
    while ( liste_modules )
     { struct MODULE_MODBUS *module;
       module = (struct MODULE_MODBUS *)liste_modules->data;

       g_snprintf( chaine, sizeof(chaine),
                   "\n MODBUS[%02d] -> IP=%s, bit=%d, started=%d, transaction=%d, "
                   "nbr_deconnect=%d, request=%d, date_retente=%d \n",
                   module->id, module->ip, module->bit, module->started,
                   module->transaction_id, module->nbr_deconnect, module->request,
                   (int)module->date_retente
                 );
       Write_admin ( client->connexion, chaine );

       liste_bornes = module->Bornes;
       while ( liste_bornes )
        { struct BORNE_MODBUS *borne;
          borne = (struct BORNE_MODBUS *)liste_bornes->data;
          g_snprintf( chaine, sizeof(chaine),
                      " - Borne %02d -> type='%s', adresse=%d, min=%d, nbr=%d\n",
                      borne->id, Mode_borne[borne->type], borne->adresse, borne->min, borne->nbr
                    );
          Write_admin ( client->connexion, chaine );
          liste_bornes = liste_bornes->next;
        }
       liste_modules = liste_modules->next;
     }
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_modbus ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                             /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "reload" ) )
     { Partage->com_modbus.reload = TRUE;
       Write_admin ( client->connexion, " MODBUS Reloading in progress\n" );
       while (Partage->com_modbus.reload) sched_yield();
       Write_admin ( client->connexion, " MODBUS Reloading done\n" );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion, "  -- Watchdog ADMIN -- Help du mode 'MODBUS'\n" );
       Write_admin ( client->connexion, "  add                  - Ajoute un module modbus\n" );
       Write_admin ( client->connexion, "  reload               - Recharge la configuration\n" );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
