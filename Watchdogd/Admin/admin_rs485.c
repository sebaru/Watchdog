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
  { Partage->com_rs485.reload = TRUE;
    Write_admin ( client->connexion, " RS485 Reloading in progress\n" );
    while (Partage->com_rs485.reload) sched_yield();
    Write_admin ( client->connexion, " RS485 Reloading done\n" );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_rs485_list ( struct CLIENT_ADMIN *client )
  { GList *liste_modules;
    gchar chaine[128];

    liste_modules = Partage->com_rs485.Modules_RS485;
    while ( liste_modules )
     { struct MODULE_RS485 *module;
       module = (struct MODULE_RS485 *)liste_modules->data;

       g_snprintf( chaine, sizeof(chaine),
                   " RS485[%02d] -> num=%d,actif=%d,bit=%d,ea=%03d-%03d,e=%03d-%03d,s=%03d-%03d,"
                   "sa=%03d-%03d,req=%d,ret=%d,ana=%d\n",
                   module->rs485.id, module->rs485.num, module->rs485.actif, module->rs485.bit_comm,
                   module->rs485.ea_min, module->rs485.ea_max,
                   module->rs485.e_min, module->rs485.e_max,
                   module->rs485.s_min, module->rs485.s_max, module->rs485.sa_min, module->rs485.sa_max,
                   (gint)module->date_requete, (gint)module->date_retente, (gint)module->date_ana
                 );
       Write_admin ( client->connexion, chaine );
       liste_modules = liste_modules->next;
     }
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_rs485_start ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    while (Partage->com_rs485.admin_start) sched_yield();
    Partage->com_rs485.admin_start = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_rs485_start: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=1 WHERE id=%d",
                NOM_TABLE_MODULE_RS485, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }
    Libere_DB_SQL( Config.log, &db );

    g_snprintf( chaine, sizeof(chaine), "Module RS485 %d started\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_rs485_stop ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    while (Partage->com_rs485.admin_stop) sched_yield();
    Partage->com_rs485.admin_stop = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_rs485_stop: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=0 WHERE id=%d",
                NOM_TABLE_MODULE_RS485, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }
    Libere_DB_SQL( Config.log, &db );

    g_snprintf( chaine, sizeof(chaine), "Module RS485 %d stopped\n", id );
    Write_admin ( client->connexion, chaine );
  }
#ifdef bouh
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static gint Admin_rs485_add ( struct CLIENT_ADMIN *client, gint id, gint ea_min, gint ea_max,
                               gint e_min, gint e_max, gint ec_min, gint ec_max,gint s_min, gint s_max,
                               gint sa_min,gint sa_max )
  { gchar requete[128];
    struct DB *db;
    gint retour;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_rs485_add: impossible d'ouvrir la Base de données",
               Config.db_database );
       return(-1);
     }


    while (Partage->com_rs485.admin_add) sched_yield();
    Partage->com_rs485.admin_add = id;
    return(retour);
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_rs485_del ( struct CLIENT_ADMIN *client, gint id )
  { gchar requete[128], chaine[128];
    struct DB *db;

    while (Partage->com_rs485.admin_del) sched_yield();
    Partage->com_rs485.admin_del = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_rs485_del: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "DELETE FROM %s WHERE id = %d",
                NOM_TABLE_MODULE_RS485, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }

    Libere_DB_SQL( Config.log, &db );
    g_snprintf( chaine, sizeof(chaine), "Module RS485 %d deleted\n", id );
    Write_admin ( client->connexion, chaine );
  }
#endif
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_rs485 ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
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
                     "  start id                               - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  stop id                                - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  list                                   - Affiche les status des equipements RS485\n" );
       Write_admin ( client->connexion,
                     "  reload                                 - Recharge la configuration\n" );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
