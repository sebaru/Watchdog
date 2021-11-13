/******************************************************************************************************************************/
/* Watchdogd/Archive/admin_arch.c  Gestion des responses Admin du thread "Archive" de watchdog                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                                                    17.03.2017 08:37:09 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_arch.c
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
 
 #include <unistd.h>                                                                                      /* Pour gethostname */
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Admin_arch_testdb: Test la response vers le serveur de base de données                                                    */
/* Entrée: la response pour sortiee client et la ligne de commande                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_arch_testdb ( gchar *response )
  { gchar chaine[80];
    struct DB*db;
    db = Init_ArchDB_SQL();       
    if (!db)
     { g_snprintf( chaine, sizeof(chaine), " | - Response to DB failed (Host='%s':%d, User='%s' DB='%s')",
                   Partage->com_arch.archdb_host, Partage->com_arch.archdb_port, Partage->com_arch.archdb_username, Partage->com_arch.archdb_database );
       response = Admin_write ( response, chaine );
       return(response);
     }
    g_snprintf( chaine, sizeof(chaine), " | - Response to DB OK (Host=%s:%d, User=%s DB=%s)",
                Partage->com_arch.archdb_host, Partage->com_arch.archdb_port, Partage->com_arch.archdb_username, Partage->com_arch.archdb_database );
    response = Admin_write ( response, chaine );
    Libere_DB_SQL( &db );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_arch_status: Print le statut du thread arch                                                                          */
/* Entrée: la response pour sortiee client et la ligne de commande                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_arch_status ( gchar *response )
  { gchar chaine[128];
    gint save_nbr;

    pthread_mutex_lock( &Partage->com_arch.synchro );                                                        /* lockage futex */
    save_nbr = g_slist_length(Partage->com_arch.liste_arch);
    pthread_mutex_unlock( &Partage->com_arch.synchro );

    g_snprintf( chaine, sizeof(chaine), " | - Length of Arch list : %d", save_nbr );
    response = Admin_write ( response, chaine );
    g_snprintf( chaine, sizeof(chaine), " | - Host     : %s", Partage->com_arch.archdb_host );
    response = Admin_write ( response, chaine );
    g_snprintf( chaine, sizeof(chaine), " | - Port     : %d", Partage->com_arch.archdb_port );
    response = Admin_write ( response, chaine );
    g_snprintf( chaine, sizeof(chaine), " | - Database : %s", Partage->com_arch.archdb_database );
    response = Admin_write ( response, chaine );
    g_snprintf( chaine, sizeof(chaine), " | - Username : %s", Partage->com_arch.archdb_username );
    response = Admin_write ( response, chaine );
    g_snprintf( chaine, sizeof(chaine), " | - Purge    : %d", Partage->com_arch.duree_retention );
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_command: Gere une commande liée au thread arch depuis une response admin                                            */
/* Entrée: le client et la ligne de commande                                                                                  */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_arch ( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */
    if ( ! strcmp ( commande, "status" ) )
     { return(Admin_arch_status ( response )); }
    else if ( ! strcmp ( commande, "testdb" ) )
     { return(Admin_arch_testdb ( response )); }
    else if ( ! strcmp ( commande, "dbcfg" ) )                /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { gboolean retour;
       response = Admin_dbcfg_thread ( response, "arch", ligne+6 );                             /* Si changement de parametre */
       retour = Arch_Lire_config();
       g_snprintf( chaine, sizeof(chaine), " | - Reloading Thread Parameters from Database -> %s",
                   (retour ? "Success" : "Failed") );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "purge" ) )
     { pthread_t tid;
       if (pthread_create( &tid, NULL, (void *)Arch_Update_SQL_Partitions_thread, NULL ))
        { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s: pthread_create failed for Update SQL Partitions", __func__ ); }
        else
        { pthread_detach( tid ); }                                /* On le detache pour qu'il puisse se terminer tout seul */
       g_snprintf( chaine, sizeof(chaine), " | - Purge of old data in progress" );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "clear" ) )
     { gint nbr;
       nbr = Arch_Clear_list ();                                         /* Clear de la list des archives à prendre en compte */
       g_snprintf( chaine, sizeof(chaine), " | - ArchiveList cleared (%d components)", nbr );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'UPS'" );
       response = Admin_write ( response, "  dbcfg ...                              - Get/Set Database Parameters" );
       response = Admin_write ( response, "  status                                 - Get Status of Arch Thread");
       response = Admin_write ( response, "  clear                                  - Clear Archive List" );
       response = Admin_write ( response, "  purge                                  - Purge old data in database" );
       response = Admin_write ( response, "  testdb                                 - Access Test to Database" );
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " | - Unknown command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
   return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
