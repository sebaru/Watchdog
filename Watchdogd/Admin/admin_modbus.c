/**********************************************************************************************************/
/* Watchdogd/Admin/admin_modbus.c        Gestion des connexions Admin MODBUS au serveur watchdog          */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 05 sept. 2010 12:01:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_modbus.c
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
/* Admin_modbus_reload: Demande le rechargement des conf MODBUS                                           */
/* Entrée: le client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_modbus_reload ( struct CLIENT_ADMIN *client )
  { Partage->com_modbus.Thread_reload = TRUE;
    Write_admin ( client->connexion, " MODBUS Reloading in progress\n" );
    while (Partage->com_modbus.Thread_reload) sched_yield();
    Write_admin ( client->connexion, " MODBUS Reloading done\n" );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_modbus_list ( struct CLIENT_ADMIN *client )
  { GList *liste_modules;
    gchar chaine[512];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des modules MODBUS\n" );
    Write_admin ( client->connexion, chaine );

    pthread_mutex_lock( &Partage->com_modbus.synchro );
    liste_modules = Partage->com_modbus.Modules_MODBUS;
    while ( liste_modules )
     { struct MODULE_MODBUS *module;
       module = (struct MODULE_MODBUS *)liste_modules->data;

       g_snprintf( chaine, sizeof(chaine),
                   "\n"
                   " MODBUS[%02d] -> IP=%s, bit=%d, actif=%d, started=%d, mode=%d, watchdog=%d, \n"
                   "                 min_e_tor=%d, min_e_ana=%d, min_s_tor=%d, min_s_ana=%d\n"
                   "                 trans.=%d, deco.=%d, request=%d, retente=%d, date_next_eana=%d\n",
                   module->modbus.id, module->modbus.ip, module->modbus.bit, module->modbus.actif,
                   module->started, module->mode, module->modbus.watchdog,
                   module->modbus.min_e_tor, module->modbus.min_e_ana,
                   module->modbus.min_s_tor, module->modbus.min_s_ana,
                   module->transaction_id, module->nbr_deconnect, module->request,
                   (int)module->date_retente, (int)module->date_next_eana
                 );
       Write_admin ( client->connexion, chaine );
       liste_modules = liste_modules->next;                                  /* Passage au module suivant */
     }
    pthread_mutex_unlock( &Partage->com_modbus.synchro );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_modbus_start ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " -- Demarrage d'un module MODBUS\n" );
    Write_admin ( client->connexion, chaine );

    while (Partage->com_modbus.admin_start) sched_yield();
    Partage->com_modbus.admin_start = id;

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_start: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=1 WHERE id=%d",
                NOM_TABLE_MODULE_MODBUS, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }
    Libere_DB_SQL( Config.log, &db );

    g_snprintf( chaine, sizeof(chaine), " Module MODBUS %d started\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_modbus_stop ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " -- Arret d'un module MODBUS\n" );
    Write_admin ( client->connexion, chaine );

    while (Partage->com_modbus.admin_stop) sched_yield();
    Partage->com_modbus.admin_stop = id;

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_stop: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=0 WHERE id=%d",
                NOM_TABLE_MODULE_MODBUS, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }
    Libere_DB_SQL( Config.log, &db );

    g_snprintf( chaine, sizeof(chaine), " Module MODBUS %d stopped\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_modbus ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_modbus_start ( client, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_modbus_list ( client );
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_modbus_stop ( client, num );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_modbus_reload(client);
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'MODBUS'\n" );
       Write_admin ( client->connexion,
                     "  start id                               - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  stop id                                - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  list                                   - Liste les modules MODBUS+Borne\n" );
       Write_admin ( client->connexion,
                     "  reload                                 - Recharge la configuration\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown dls command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
