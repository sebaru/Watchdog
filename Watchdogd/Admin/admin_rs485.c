/**********************************************************************************************************/
/* Watchdogd/Admin/admin_rs485.c        Gestion des connexions Admin RS485 au serveur watchdog           */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_rs485.c
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
 
 #include <glib.h>
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Admin_rs485_reload: Demande le rechargement des conf RS485                                             */
/* Entrée: le client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_rs485_reload ( struct CLIENT_ADMIN *client )
  { Partage->com_rs485.Thread_reload = TRUE;
    Write_admin ( client->connexion, " RS485 Reloading in progress\n" );
    while (Partage->com_rs485.Thread_reload) sched_yield();
    Write_admin ( client->connexion, " RS485 Reloading done\n" );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_rs485_list ( struct CLIENT_ADMIN *client )
  { GList *liste_modules;
    gchar chaine[256];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des modules RS485\n" );
    Write_admin ( client->connexion, chaine );

    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );
       
    liste_modules = Partage->com_rs485.Modules_RS485;
    while ( liste_modules )
     { struct MODULE_RS485 *module;
       module = (struct MODULE_RS485 *)liste_modules->data;

       g_snprintf( chaine, sizeof(chaine),
                   " RS485[%02d] -> num=%d,enable=%s,bit=%d,ea=%03d-%03d,e=%03d-%03d,s=%03d-%03d,sa=%03d-%03d\n"
                   "              started=%s,req=%d,ret=%d,next_get_ana=%d\n",
                   module->rs485.id, module->rs485.num,
                   (module->rs485.enable ? "TRUE" : "FALSE"),
                   module->rs485.bit_comm,
                   module->rs485.ea_min, module->rs485.ea_max,
                   module->rs485.e_min, module->rs485.e_max,
                   module->rs485.s_min, module->rs485.s_max, module->rs485.sa_min, module->rs485.sa_max,
                   (module->started      ? "TRUE" : "FALSE"),
                   (gint)module->date_requete, (gint)module->date_retente, (gint)module->date_next_get_ana
                 );
       Write_admin ( client->connexion, chaine );
       liste_modules = liste_modules->next;
     }
  }
/**********************************************************************************************************/
/* Admin_rs485_del: Retire le capteur/module rs485 dont l'id est en parametre                           */
/* Entrée: le client et l'id                                                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rs485_del ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Suppression du module rs485 %d\n", id );
    Write_admin ( client->connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );

    if (Partage->com_rs485.Retirer_rs485DB)
     { if (Partage->com_rs485.Retirer_rs485DB( id ))
        { g_snprintf( chaine, sizeof(chaine), " Module erased.\n You should reload configuration...\n" ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT erased.\n" ); }
       Write_admin ( client->connexion, chaine );
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error, thread not loaded.\n" );
       Write_admin ( client->connexion, chaine );
     }
  }
/**********************************************************************************************************/
/* Admin_rs485_add: Ajoute un capteur/module RS485                                                      */
/* Entrée: le client et la structure de reference du capteur                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rs485_add ( struct CLIENT_ADMIN *client, struct RS485DB *rs485 )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Ajout d'un module rs485\n" );
    Write_admin ( client->connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );

    if (Partage->com_rs485.Ajouter_rs485DB)
     { gint last_id;
       last_id = Partage->com_rs485.Ajouter_rs485DB( rs485 );
       if ( last_id != -1 )
        { g_snprintf( chaine, sizeof(chaine), " Module added. New ID=%d.\n You should reload configuration...\n", last_id ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT added.\n" ); }
       Write_admin ( client->connexion, chaine );
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error, thread not loaded.\n" );
       Write_admin ( client->connexion, chaine );
     }
  }
/**********************************************************************************************************/
/* Admin_rs485_change: Modifie la configuration d'un capteur RS485                                        */
/* Entrée: le client et la structure de reference du capteur                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rs485_change ( struct CLIENT_ADMIN *client, struct RS485DB *rs485 )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Modification d'un module rs485\n" );
    Write_admin ( client->connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );

    if (Partage->com_rs485.Modifier_rs485DB)
     { if ( Partage->com_rs485.Modifier_rs485DB( rs485 ) )
        { g_snprintf( chaine, sizeof(chaine), " Module %d changed.\n You should reload configuration...\n", rs485->id ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT changed.\n" ); }
       Write_admin ( client->connexion, chaine );
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error, thread not loaded.\n" );
       Write_admin ( client->connexion, chaine );
     }
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_rs485_start ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Demarrage d'un module RS485\n" );
    Write_admin ( client->connexion, chaine );

    while (Partage->com_rs485.admin_start) sched_yield();
    Partage->com_rs485.admin_start = id;

    g_snprintf( chaine, sizeof(chaine), " Module RS485 %d started\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_rs485_stop ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Arret d'un module RS485\n" );
    Write_admin ( client->connexion, chaine );

    while (Partage->com_rs485.admin_stop) sched_yield();
    Partage->com_rs485.admin_stop = id;

    g_snprintf( chaine, sizeof(chaine), " Module RS485 %d stopped\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_rs485 ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "add" ) )
     { struct RS485DB rs485;
       memset( &rs485, 0, sizeof(struct RS485DB) );
       sscanf ( ligne, "%s %d %d %d %d %d %d %d %d %d %d %d %s", commande,/* Découpage de la ligne de commande */
                &rs485.num, &rs485.bit_comm, (gint *)&rs485.enable,
                &rs485.ea_min, &rs485.ea_max,
                &rs485.e_min, &rs485.e_max,
                &rs485.s_min, &rs485.s_max,
                &rs485.sa_min, &rs485.sa_max,
                rs485.libelle );
       Admin_rs485_add ( client, &rs485 );
     }
    else if ( ! strcmp ( commande, "del" ) )
     { gint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_rs485_del ( client, num );
     }
    else if ( ! strcmp ( commande, "change" ) )
     { struct RS485DB rs485;
       memset( &rs485, 0, sizeof(struct RS485DB) );
       sscanf ( ligne, "%s %d %d %d %d %d %d %d %d %d %d %d %d %s", commande,/* Découpage de la ligne de commande */
                &rs485.id, &rs485.num, &rs485.bit_comm, (gint *)&rs485.enable,
                &rs485.ea_min, &rs485.ea_max,
                &rs485.e_min, &rs485.e_max,
                &rs485.s_min, &rs485.s_max,
                &rs485.sa_min, &rs485.sa_max,
                rs485.libelle );
       Admin_rs485_change ( client, &rs485 );
     }
    else if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_rs485_start ( client, num );
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_rs485_stop ( client, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_rs485_list ( client );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_rs485_reload(client);
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'RS485'\n" );
       Write_admin ( client->connexion,
                     "  add num bit_comm enable ea_min ea_max e_min e_max s_min s_max sa_min sa_max libelle\n" );
       Write_admin ( client->connexion,
                     "                                         - Ajoute un module RS485\n" );
       Write_admin ( client->connexion,
                     "  del id                                 - Retire le module id\n" );
       Write_admin ( client->connexion,
                     "  change id num bit_comm enable ea_min ea_max e_min e_max s_min s_max sa_min sa_max libelle\n" );
       Write_admin ( client->connexion,
                     "                                         - Modifie le module id\n" );
       Write_admin ( client->connexion,
                     "  start id                               - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  stop id                                - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  list                                   - Affiche les status des equipements RS485\n" );
       Write_admin ( client->connexion,
                     "  reload                                 - Recharge la configuration\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown RS485 command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
