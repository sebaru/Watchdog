/**********************************************************************************************************/
/* Watchdogd/Admin/admin_onduleur.c        Gestion des connexions Admin ONDULEUR au serveur watchdog      */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 11 nov. 2009 11:28:29 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_onduleur.c
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

/**********************************************************************************************************/
/* Admin_onduleur_reload: Demande le rechargement des conf ONDULEUR                                       */
/* Entrée: le client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_onduleur_reload ( struct CLIENT_ADMIN *client )
  { Partage->com_onduleur.reload = TRUE;
    Write_admin ( client->connexion, " ONDULEUR Reloading in progress\n" );
    while (Partage->com_onduleur.reload) sched_yield();
    Write_admin ( client->connexion, " ONDULEUR Reloading done\n" );
  }
/**********************************************************************************************************/
/* Admin_onduleur_list : L'utilisateur admin lance la commande "list" en mode onduleur                    */
/* Entrée: La connexion client ADMIN                                                                      */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 void Admin_onduleur_list ( struct CLIENT_ADMIN *client )
  { GList *liste_modules;
    gchar chaine[512];

    pthread_mutex_lock( &Partage->com_onduleur.synchro );
    liste_modules = Partage->com_onduleur.Modules_ONDULEUR;
    while ( liste_modules )
     { struct MODULE_ONDULEUR *module;
       module = (struct MODULE_ONDULEUR *)liste_modules->data;

       g_snprintf( chaine, sizeof(chaine), "\n"
                   " ONDULEUR[%02d] -> Host=%s, UPS=%s, actif=%d, started=%d nbr_deconnect=%d date_retente=%d\n"
                   "                   bit_comm=%d, ea_ups_load=%d, ea_ups_real_power=%d\n"
                   "                   ea_battery_charge=%d, ea_input_voltage=%d\n",
                   module->id, module->host, module->ups, module->actif, module->started,
                   module->nbr_deconnect, (int)module->date_retente, module->bit_comm,
                   module->ea_ups_load, module->ea_ups_real_power,
                   module->ea_battery_charge, module->ea_input_voltage 
                 );
       Write_admin ( client->connexion, chaine );
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock( &Partage->com_onduleur.synchro );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_onduleur_start ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    while (Partage->com_onduleur.admin_start) sched_yield();
    Partage->com_onduleur.admin_start = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_onduleur_start: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=1 WHERE id=%d",
                NOM_TABLE_MODULE_ONDULEUR, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }
    Libere_DB_SQL( Config.log, &db );

    g_snprintf( chaine, sizeof(chaine), "Module ONDULEUR %d started\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_onduleur_stop ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    while (Partage->com_onduleur.admin_stop) sched_yield();
    Partage->com_onduleur.admin_stop = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_onduleur_stop: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=0 WHERE id=%d",
                NOM_TABLE_MODULE_ONDULEUR, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }
    Libere_DB_SQL( Config.log, &db );

    g_snprintf( chaine, sizeof(chaine), "Module ONDULEUR %d stopped\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static gint Admin_onduleur_add ( struct CLIENT_ADMIN *client, gchar *host_orig, gchar *ups_orig,
                                  guint bit_comm, guint ea_ups_load, guint ea_ups_realpower,
                                  guint ea_battery_charge, guint ea_input_voltage )
  { gchar requete[256], *host, *ups;
    struct DB *db;
    gint id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_onduleur_add: impossible d'ouvrir la Base de données",
               Config.db_database );
       return(-1);
     }

    host = Normaliser_chaine ( Config.log, host_orig );                  /* Formatage correct des chaines */
    if (!host)
     { Info( Config.log, DEBUG_ADMIN, "Admin_onduleur_add: Normalisation impossible" );
       Libere_DB_SQL( Config.log, &db );
       return(-1);
     }

    ups = Normaliser_chaine ( Config.log, ups_orig );                    /* Formatage correct des chaines */
    if (!host)
     { Info( Config.log, DEBUG_ADMIN, "Admin_onduleur_add: Normalisation impossible" );
       g_free(host);
       Libere_DB_SQL( Config.log, &db );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s(host,ups,bit_comm,actif,ea_ups_load,ea_ups_realpower,ea_battery_charge,ea_input_voltage) "
                "VALUES ('%s','%s',%d,%d,%d,%d,%d,%d)",
                NOM_TABLE_MODULE_ONDULEUR, host, ups, bit_comm, 0,
                ea_ups_load,ea_ups_realpower,ea_battery_charge,ea_input_voltage
              );
    g_free(host);
    g_free(ups);

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return(-1);
     }
    id = Recuperer_last_ID_SQL ( Config.log, db );
    Libere_DB_SQL( Config.log, &db );

    while (Partage->com_onduleur.admin_add) sched_yield();
    Partage->com_onduleur.admin_add = id;
    return(id);
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_onduleur_del ( struct CLIENT_ADMIN *client, gint id )
  { gchar requete[128], chaine[128];
    struct DB *db;

    while (Partage->com_onduleur.admin_del) sched_yield();
    Partage->com_onduleur.admin_del = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_onduleur_del: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "DELETE FROM %s WHERE id = %d",
                NOM_TABLE_MODULE_ONDULEUR, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }

    Libere_DB_SQL( Config.log, &db );
    g_snprintf( chaine, sizeof(chaine), "Module ONDULEUR %d deleted\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_onduleur ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_onduleur_start ( client, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_onduleur_list ( client );
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_onduleur_stop ( client, num );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_onduleur_reload(client);
     }
    else if ( ! strcmp ( commande, "add" ) )
     { guint bit_comm, ea_ups_load, ea_ups_realpower, ea_battery_charge, ea_input_voltage;
       gchar host[128], ups[128], chaine[128];
       sscanf ( ligne, "%s %s %s %d %d %d %d %d", commande, host, ups, 
                &bit_comm, &ea_ups_load, &ea_ups_realpower, &ea_battery_charge, &ea_input_voltage
              );                                                     /* Découpage de la ligne de commande */

       if ( bit_comm >= NBR_BIT_DLS )
        { Write_admin ( client->connexion, " bit_comm should be < NBR_BIT_DLS\n" ); }
       else
        { int id;
          id = Admin_onduleur_add ( client, host, ups, bit_comm,
                                    ea_ups_load, ea_ups_realpower, ea_battery_charge, ea_input_voltage
                                  );
          if (id != -1) { g_snprintf( chaine, sizeof(chaine), "Module ONDULEUR %d added\n", id ); }
          else          { g_snprintf( chaine, sizeof(chaine), "Module ONDULEUR NOT added\n" ); }
          Write_admin ( client->connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "delete" ) )
     { guint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_onduleur_del ( client, num );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'ONDULEUR'\n" );
       Write_admin ( client->connexion,
                     "  add host ups bit_com ea_ups_load ea_ups_realpower ea_battery_charge ea_input_voltage\n"
                     "                                         - Ajoute un module ONDULEUR\n" );
       Write_admin ( client->connexion,
                     "  delete id                              - Supprime le module id\n" );
       Write_admin ( client->connexion,
                     "  start id                               - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  stop id                                - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  list                                   - Liste les modules ONDULEUR\n" );
       Write_admin ( client->connexion,
                     "  reload                                 - Recharge la configuration\n" );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
