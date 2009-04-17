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
/* Admin_modbus_reload: Demande le rechargement des conf MODBUS                                           */
/* Entrée: le client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_modbus_reload ( struct CLIENT_ADMIN *client )
  { Partage->com_modbus.reload = TRUE;
    Write_admin ( client->connexion, " MODBUS Reloading in progress\n" );
    while (Partage->com_modbus.reload) sched_yield();
    Write_admin ( client->connexion, " MODBUS Reloading done\n" );
  }
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
                   "\n MODBUS[%02d] -> IP=%s, bit=%d, actif=%d, started=%d, trans.=%d, "
                   "deco.=%d, request=%d, retente=%d \n",
                   module->id, module->ip, module->bit, module->actif, module->started,
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
 static void Admin_modbus_start ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    while (Partage->com_modbus.admin_start) sched_yield();
    Partage->com_modbus.admin_start = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_start: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=1 WHERE id=%d",
                NOM_TABLE_MODULE_MODBUS, id
              );

    if ( mysql_query ( db->mysql, requete ) )
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_start: requete failed",
               (char *)mysql_error(db->mysql) );
       Libere_DB_SQL( Config.log, &db );
       return;
     }
    Libere_DB_SQL( Config.log, &db );

    g_snprintf( chaine, sizeof(chaine), "Module MODBUS %d started", id );
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

    while (Partage->com_modbus.admin_stop) sched_yield();
    Partage->com_modbus.admin_stop = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_stop: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=0 WHERE id=%d",
                NOM_TABLE_MODULE_MODBUS, id
              );

    if ( mysql_query ( db->mysql, requete ) )
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_stop: requete failed",
               (char *)mysql_error(db->mysql) );
       Libere_DB_SQL( Config.log, &db );
       return;
     }
    Libere_DB_SQL( Config.log, &db );

    g_snprintf( chaine, sizeof(chaine), "Module MODBUS %d started", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static gint Admin_modbus_add ( struct CLIENT_ADMIN *client, gchar *ip_orig, guint bit, guint watchdog )
  { gchar requete[128], *ip;
    struct DB *db;
    gint id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_add: impossible d'ouvrir la Base de données",
               Config.db_database );
       return(-1);
     }

    ip = Normaliser_chaine ( Config.log, ip_orig );                      /* Formatage correct des chaines */
    if (!ip)
     { Info( Config.log, DEBUG_ADMIN, "Admin_modbus_add: Normalisation impossible" );
       Libere_DB_SQL( Config.log, &db );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s(actif,ip,bit,watchdog) VALUES (FALSE,'%s',%d,%d)",
                NOM_TABLE_MODULE_MODBUS, ip, bit, watchdog
              );
    g_free(ip);

    if ( mysql_query ( db->mysql, requete ) )
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_add: requete failed",
               (char *)mysql_error(db->mysql) );
       Libere_DB_SQL( Config.log, &db );
       return(-1);
     }
    id = mysql_insert_id(db->mysql);
    Libere_DB_SQL( Config.log, &db );
    while (Partage->com_modbus.admin_add) sched_yield();
    Partage->com_modbus.admin_add = id;
    return(id);
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_modbus_del ( struct CLIENT_ADMIN *client, gint id )
  { struct MODULE_MODBUS *module;
    gchar requete[128], chaine[128];
    struct DB *db;

    while (Partage->com_modbus.admin_del) sched_yield();
    Partage->com_modbus.admin_del = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_del: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "DELETE FROM %s WHERE id = %d",
                NOM_TABLE_MODULE_MODBUS, id
              );

    if ( mysql_query ( db->mysql, requete ) )
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_del: requete failed",
               (char *)mysql_error(db->mysql) );
       Libere_DB_SQL( Config.log, &db );
       return;
     }

    g_snprintf( requete, sizeof(requete), "DELETE FROM %s WHERE module = %d",
                NOM_TABLE_BORNE_MODBUS, id
              );

    if ( mysql_query ( db->mysql, requete ) )
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_del: requete failed",
               (char *)mysql_error(db->mysql) );
       Libere_DB_SQL( Config.log, &db );
       return;
     }

    Libere_DB_SQL( Config.log, &db );
    g_snprintf( chaine, sizeof(chaine), "Module MODBUS %d deleted", id );
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
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_modbus_stop ( client, num );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_modbus_reload(client);
     }
    else if ( ! strcmp ( commande, "add" ) )
     { gchar ip[128], chaine[128];
       guint bit, watchdog;
       sscanf ( ligne, "%s %s %d %d", commande, ip, &bit, &watchdog );/* Découpage de la ligne de commande */
       if ( bit >= NBR_BIT_DLS )
        { Write_admin ( client->connexion, " bit should be < NBR_BIT_DLS\n" ); }
       else
        { int id;
          id = Admin_modbus_add ( client, ip, bit, watchdog );
          if (id != -1) { g_snprintf( chaine, sizeof(chaine), "Module MODBUS %d added", id ); }
          else          { g_snprintf( chaine, sizeof(chaine), "Module MODBUS NOT added" ); }
          Write_admin ( client->connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion, "  -- Watchdog ADMIN -- Help du mode 'MODBUS'\n" );
       Write_admin ( client->connexion, "  add ip bit watchdog  - Ajoute un module MODBUS\n" );
       Write_admin ( client->connexion, "  start id             - Demarre le module id\n" );
       Write_admin ( client->connexion, "  stop id              - Demarre le module id\n" );
       Write_admin ( client->connexion, "  reload               - Recharge la configuration\n" );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
