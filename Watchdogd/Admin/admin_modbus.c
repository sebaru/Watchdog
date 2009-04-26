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

    pthread_mutex_lock( &Partage->com_modbus.synchro );
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

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }
    Libere_DB_SQL( Config.log, &db );

    g_snprintf( chaine, sizeof(chaine), "Module MODBUS %d started\n", id );
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

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }
    Libere_DB_SQL( Config.log, &db );

    g_snprintf( chaine, sizeof(chaine), "Module MODBUS %d stopped\n", id );
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

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return(-1);
     }
    id = Recuperer_last_ID_SQL ( Config.log, db );
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
 static gint Admin_modbus_add_borne ( struct CLIENT_ADMIN *client, gchar *type,
                                      guint adresse, guint min, guint nbr, guint module )
  { gchar requete[128];
    struct DB *db;
    gint id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_add: impossible d'ouvrir la Base de données",
               Config.db_database );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s(type,adresse,min,nbr,module) VALUES (%d,%d,%d,%d,%d)",
                NOM_TABLE_BORNE_MODBUS, Mode_borne_vers_id(type), adresse, min, nbr, module
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return(-1);
     }
    id = Recuperer_last_ID_SQL ( Config.log, db );
    Libere_DB_SQL( Config.log, &db );

    while (Partage->com_modbus.admin_add_borne) sched_yield();
    Partage->com_modbus.admin_add_borne = id;
    return(id);
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_modbus_del ( struct CLIENT_ADMIN *client, gint id )
  { gchar requete[128], chaine[128];
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

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }

    g_snprintf( requete, sizeof(requete), "DELETE FROM %s WHERE module = %d",
                NOM_TABLE_BORNE_MODBUS, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }

    Libere_DB_SQL( Config.log, &db );
    g_snprintf( chaine, sizeof(chaine), "Module MODBUS %d deleted\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_modbus_del_borne ( struct CLIENT_ADMIN *client, gint id )
  { gchar requete[128], chaine[128];
    struct DB *db;

    while (Partage->com_modbus.admin_del) sched_yield();
    Partage->com_modbus.admin_del_borne = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_modbus_del: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "DELETE FROM %s WHERE id = %d",
                NOM_TABLE_BORNE_MODBUS, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }

    Libere_DB_SQL( Config.log, &db );
    g_snprintf( chaine, sizeof(chaine), "Module MODBUS %d deleted\n", id );
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
    else if ( ! strcmp ( commande, "mbus" ) )
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
    else if ( ! strcmp ( commande, "add" ) )
     { gchar ip[128], chaine[128];
       guint bit, watchdog;
       sscanf ( ligne, "%s %s %d %d", commande, ip, &bit, &watchdog );/* Découpage de la ligne de commande */
       if ( bit >= NBR_BIT_DLS )
        { Write_admin ( client->connexion, " bit should be < NBR_BIT_DLS\n" ); }
       else
        { int id;
          id = Admin_modbus_add ( client, ip, bit, watchdog );
          if (id != -1) { g_snprintf( chaine, sizeof(chaine), "Module MODBUS %d added\n", id ); }
          else          { g_snprintf( chaine, sizeof(chaine), "Module MODBUS NOT added\n" ); }
          Write_admin ( client->connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "addborne" ) )
     { gchar type[128], chaine[128];
       guint adresse, min, nbr, module;
       sscanf ( ligne, "%s %s %d %d %d %d", commande, type, &adresse, &min, &nbr, &module );

       if ( min >= NBR_BIT_DLS )
        { Write_admin ( client->connexion, " min should be < NBR_BIT_DLS\n" ); }
       else if ( module > 255 )
        { Write_admin ( client->connexion, " module should be < 255\n" ); }
       else if ( nbr > 8 )
        { Write_admin ( client->connexion, " nbr should be <= 8\n" ); }
       else
        { int id;
          id = Admin_modbus_add_borne ( client, type, adresse, min, nbr, module );
          if (id != -1) { g_snprintf( chaine, sizeof(chaine), "Module MODBUS %d added\n", id ); }
          else          { g_snprintf( chaine, sizeof(chaine), "Module MODBUS NOT added\n" ); }
          Write_admin ( client->connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "deleteborne" ) )
     { guint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_modbus_del_borne ( client, num );
     }
    else if ( ! strcmp ( commande, "delete" ) )
     { guint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_modbus_del ( client, num );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'MODBUS'\n" );
       Write_admin ( client->connexion,
                     "  addborne type adresse min nbr moduleID - Ajoute une borne a un module\n" );
       Write_admin ( client->connexion,
                     "  deleteborne id                         - Supprime la borne id\n" );
       Write_admin ( client->connexion,
                     "  add ip bit watchdog                    - Ajoute un module MODBUS\n" );
       Write_admin ( client->connexion,
                     "  delete id                              - Supprime le module id\n" );
       Write_admin ( client->connexion,
                     "  start id                               - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  stop id                                - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  mbus                                   - Liste les modules MODBUS+Borne\n" );
       Write_admin ( client->connexion,
                     "  reload                                 - Recharge la configuration\n" );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
