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
/* Admin_dls_reload: Demande le rechargement des conf DLS                                                 */
/* Entrée: le client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_dls_reload ( struct CLIENT *client, gchar *buffer )
  { Partage->com_dls.Thread_reload = TRUE;
    g_strlcat ( buffer, " DLS Reloading in progress\n", NBR_CARAC_BUFFER_ADMIN );
    while (Partage->com_dls.Thread_reload) sched_yield();
    g_strlcat ( buffer, " DLS Reloading done\n", NBR_CARAC_BUFFER_ADMIN );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_dls_list ( struct CLIENT *client, gchar *buffer )
  { GList *liste_dls;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des modules D.L.S\n" );
    g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
     
    pthread_mutex_lock( &Partage->com_dls.synchro );
    liste_dls = Partage->com_dls.Plugins;
    while ( liste_dls )
     { struct PLUGIN_DLS *dls;
       dls = (struct PLUGIN_DLS *)liste_dls->data;

       g_snprintf( chaine, sizeof(chaine), " DLS[%03d] -> actif=%d, conso=%4.03f, nom=%s\n",
                           dls->plugindb.id, dls->plugindb.on, dls->conso, dls->plugindb.nom );
       g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
       liste_dls = liste_dls->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_dls_gcc ( struct CLIENT *client, gchar *buffer, gint id )
  { GList *liste_dls;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Compilation des plugins D.L.S\n" );
    g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );

    if (id == -1)
     { liste_dls = Partage->com_dls.Plugins;
       while ( liste_dls )
        { struct PLUGIN_DLS *dls;
          dls = (struct PLUGIN_DLS *)liste_dls->data;

          g_snprintf( chaine, sizeof(chaine), " Compilation du DLS[%03d] in progress\n", dls->plugindb.id );
          g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
          Compiler_source_dls ( NULL, dls->plugindb.id );
          g_snprintf( chaine, sizeof(chaine), " Compilation du DLS[%03d] done\n", dls->plugindb.id );
          g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
        }
       liste_dls = liste_dls->next;
     } else
        { g_snprintf( chaine, sizeof(chaine), " Compilation du DLS[%03d] in progress\n", id );
          g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
          Compiler_source_dls ( NULL, id );
          g_snprintf( chaine, sizeof(chaine), " Compilation du DLS[%03d] done\n", id );
          g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
        }
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_dls_start ( struct CLIENT *client, gchar *buffer, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " -- Demarrage d'un plugin D.L.S\n" );
    g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );

    while (Partage->com_dls.admin_start) sched_yield();
    Partage->com_dls.admin_start = id;

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, FALSE, LOG_WARNING,
                 "Admin_dls_start: impossible d'ouvrir la Base de données %s",
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

    g_snprintf( chaine, sizeof(chaine), " Module DLS %d started\n", id );
    g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_dls_stop ( struct CLIENT *client, gchar *buffer, gint id )
  { gchar chaine[128], requete[128];
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " -- Arret d'un plugin D.L.S\n" );
    g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );

    while (Partage->com_dls.admin_stop) sched_yield();
    Partage->com_dls.admin_stop = id;

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, FALSE, LOG_WARNING,
                 "Admin_dls_stop: impossible d'ouvrir la Base de données %s",
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

    g_snprintf( chaine, sizeof(chaine), " Module DLS %d stopped\n", id );
    g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
  }
/**********************************************************************************************************/
/* Admin_dls: Appellée lorsque l'admin envoie une commande en mode dls dans la ligne de commande          */
/* Entrée: La connexion cliente et la ligne de commande, et le buffer de sortie                           */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Admin_dls ( struct CLIENT *client, gchar *buffer, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_dls_start ( client, buffer, num );
     }
    else if ( ! strcmp ( commande, "gcc" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_dls_gcc ( client, buffer, num );
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_dls_stop ( client, buffer, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_dls_list ( client, buffer );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_dls_reload(client, buffer);
     }
    else if ( ! strcmp ( commande, "help" ) )
     { g_strlcat ( buffer, "  -- Watchdog ADMIN -- Help du mode 'D.L.S'\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  start id                               - Demarre le module id\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  stop id                                - Stop le module id\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  list                                   - D.L.S. Status\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  gcc id                                 - Compile le plugin id (-1 for all)\n", NBR_CARAC_BUFFER_ADMIN );
       g_strlcat ( buffer, "  reload                                 - Recharge la configuration\n", NBR_CARAC_BUFFER_ADMIN );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown DLS command : %s\n", ligne );
       g_strlcat ( buffer, chaine, NBR_CARAC_BUFFER_ADMIN );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
