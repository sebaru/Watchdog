/**********************************************************************************************************/
/* Watchdogd/Admin/admin_dls.c        Gestion des connexions Admin DLS au serveur watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_dls.c
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
 static void Admin_dls_reload ( struct CLIENT_ADMIN *client )
  { Partage->com_dls.reload = TRUE;
    Write_admin ( client->connexion, " DLS Reloading in progress\n" );
    while (Partage->com_dls.reload) sched_yield();
    Write_admin ( client->connexion, " DLS Reloading done\n" );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_dls_list ( struct CLIENT_ADMIN *client )
  { GList *liste_dls;
    gchar chaine[128];

    pthread_mutex_lock( &Partage->com_dls.synchro );
    liste_dls = Partage->com_dls.Plugins;
    while ( liste_dls )
     { struct PLUGIN_DLS *dls;
       dls = (struct PLUGIN_DLS *)liste_dls->data;

       g_snprintf( chaine, sizeof(chaine), " DLS[%03d] -> actif=%d, conso=%f, nom=%s\n",
                           dls->id, dls->on, dls->conso, dls->nom );
       Write_admin ( client->connexion, chaine );
       liste_dls = liste_dls->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_dls_gcc ( struct CLIENT_ADMIN *client, gint id )
  { GList *liste_dls;
    gchar chaine[128];

    pthread_mutex_lock( &Partage->com_dls.synchro );
    liste_dls = Partage->com_dls.Plugins;
    while ( liste_dls )
     { struct PLUGIN_DLS *dls;
       dls = (struct PLUGIN_DLS *)liste_dls->data;

       if ( id == -1 || id == dls->id)
        { g_snprintf( chaine, sizeof(chaine), " Compilation du DLS[%03d] in progress\n", dls->id );
          Write_admin ( client->connexion, chaine );
          Compiler_source_dls ( NULL, id );
          g_snprintf( chaine, sizeof(chaine), " Compilation du DLS[%03d] done\n", dls->id );
          Write_admin ( client->connexion, chaine );
        }
       liste_dls = liste_dls->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_dls_start ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    while (Partage->com_dls.admin_start) sched_yield();
    Partage->com_dls.admin_start = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_dls_start: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=1 WHERE id=%d",
                NOM_TABLE_DLS, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }
    Libere_DB_SQL( Config.log, &db );

    g_snprintf( chaine, sizeof(chaine), "Module DLS %d started\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_dls_stop ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    while (Partage->com_dls.admin_stop) sched_yield();
    Partage->com_dls.admin_stop = id;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_ADMIN, "Admin_dls_stop: impossible d'ouvrir la Base de données",
               Config.db_database );
       return;
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=0 WHERE id=%d",
                NOM_TABLE_DLS, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }
    Libere_DB_SQL( Config.log, &db );

    g_snprintf( chaine, sizeof(chaine), "Module DLS %d stopped\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_dls ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_dls_start ( client, num );
     }
    else if ( ! strcmp ( commande, "gcc" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_dls_gcc ( client, num );
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_dls_stop ( client, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_dls_list ( client );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_dls_reload(client);
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'MODBUS'\n" );
       Write_admin ( client->connexion,
                     "  start id                               - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  stop id                                - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  list                                   - D.L.S. Status\n" );
       Write_admin ( client->connexion,
                     "  gcc id                                 - Compile le plugin id\n" );
       Write_admin ( client->connexion,
                     "  reload                                 - Recharge la configuration\n" );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
