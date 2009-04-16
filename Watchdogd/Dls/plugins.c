/**********************************************************************************************************/
/* Watchdogd/Dls/plugins.c  -> Gestion des plugins pour DLS                                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 05 avr 2009 19:54:47 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * plugins.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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
 #include <string.h>
 #include <stdio.h>
 #include <dlfcn.h>

 #include "Erreur.h"
 #include "Config.h"
 #include "Dls_DB.h"

 extern GList *Plugins;
 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_dls.h"

/**********************************************************************************************************/
/* Charger_un_plugin_par_nom: Ouverture d'un plugin dont le nom est en parametre                          */
/* Entrée: Le nom de fichier correspondant                                                                */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 static gboolean Charger_un_plugin ( gint id, gint on )
  { struct PLUGIN_DLS_DL *plugin;
    gchar nom_fichier_absolu[80];
    void (*Go)(int);
    void *handle;

    g_snprintf( nom_fichier_absolu, sizeof(nom_fichier_absolu), "%s/libdls%d.so", Config.home, id );

    handle = dlopen( nom_fichier_absolu, RTLD_LAZY );
    if (!handle) { Info_n( Config.log, DEBUG_DLS, "DLS: Candidat rejeté ", id );
                   Info_c( Config.log, DEBUG_DLS, "DLS: -- sur ouverture", dlerror() );
                   return(FALSE);
                 }
    Go = dlsym( handle, "Go" );                                         /* Recherche de la fonction 'Go' */
    if (!Go) { Info_n( Config.log, DEBUG_DLS, "DLS: Candidat rejeté sur absence GO", id ); 
               dlclose( handle );
               return(FALSE);
             }

    plugin = g_malloc0( sizeof( struct PLUGIN_DLS_DL ) );
    if (!plugin)
     { Info_n( Config.log, DEBUG_DLS, "DLS: Plus de mémoire chargement plugin", id );
       dlclose(handle);
       return(FALSE);
     }
    Info_n( Config.log, DEBUG_DLS, "DLS: Charger_un_plugin: handle", GPOINTER_TO_INT(handle) );
    strncpy( plugin->nom_fichier, nom_fichier_absolu, sizeof(plugin->nom_fichier) );
    plugin->handle  = handle;
    plugin->go      = Go;
    plugin->actif   = on;
    plugin->id      = id;
    plugin->start   = 1;
    Plugins = g_list_append( Plugins, plugin );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Retirer_un_plugin: Decharge le plugin dont le numero est en parametre                                  */
/* Entrée: L'identifiant du plugin                                                                        */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Reseter_un_plugin ( gint id )
  { struct PLUGIN_DLS_DL *plugin;
    GList *plugins;
    gboolean actif;
    gint retour;

    Info_n( Config.log, DEBUG_DLS, "DLS: Reseter_un_plugin: Demande de reset plugin", id );

    plugins = Plugins;
    while(plugins)                                                      /* Liberation mémoire des modules */
     { plugin = (struct PLUGIN_DLS_DL *)plugins->data;
       if( plugin->id == id )
        { retour = dlclose( plugin->handle );
          if (retour) Info_n( Config.log, DEBUG_DLS, "DLS: Reseter_un_plugin: dlclose failed", retour );
          Info_n( Config.log, DEBUG_DLS, "DLS: Reseter_un_plugin: plugin dechargé", plugin->id );
          actif = plugin->actif;                                         /* Sauvegarde etat actif/inactif */
          Plugins = g_list_remove( Plugins, plugin );    /* Destruction de l'entete associé dans la GList */
          g_free(plugin);                                                    /* Libération mémoire plugin */
          break;
        }
       plugins = plugins->next;
     }
    if (!plugin) return;                        /* Si le plugin n'a pas été trouvé, on ne le charge pas ! */

    Info_n( Config.log, DEBUG_DLS, "DLS: Reseter_un_plugin: tentative de redemarrage plugin", id );
    if (Charger_un_plugin( id, actif ))                                 /* Chargement en actif ou inactif */
     { Info_n( Config.log, DEBUG_DLS, "DLS: Reseter_un_plugin: Reset OK", id ); }
    else
     { Info_n( Config.log, DEBUG_DLS, "DLS: Reseter_un_plugin: Reset failed", id ); }
  }
/**********************************************************************************************************/
/* Retirer_plugins: Decharge toutes les librairies                                                        */
/* Entrée: Rien                                                                                           */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Retirer_plugins ( void )
  { struct PLUGIN_DLS_DL *plugin;
    GList *plugins;

    plugins = Plugins;
    while(plugins)                                                       /* Liberation mémoire des modules */
     { plugin = (struct PLUGIN_DLS_DL *)plugins->data;
       Info_n( Config.log, DEBUG_DLS, "DLS: Retirer_plugin: tentative dechargement:", plugin->id );
       dlclose( plugin->handle );
       g_free( plugin );
       Info_n( Config.log, DEBUG_DLS, "DLS: Retirer_plugin: Dechargé", plugin->id );
       plugins = plugins->next;
     }
    g_list_free( Plugins );
    Plugins = NULL;
  }
/**********************************************************************************************************/
/* Retirer_plugins: Decharge toutes les librairies                                                        */
/* Entrée: Rien                                                                                           */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Activer_plugins ( gint num, gboolean actif )
  { struct PLUGIN_DLS_DL *plugin;
    GList *plugins;

    plugins = Plugins;
    while(plugins)                                               /* On cherche tous les modules un par un */
     { plugin = (struct PLUGIN_DLS_DL *)plugins->data;

       if ( plugin->id == num )
        { Info_n( Config.log, DEBUG_INFO, "DLS: Activer_plugins: plugin", plugin->id );
          Info_c( Config.log, DEBUG_INFO, "                            " ,(actif ? "enable" : "disable") );
          plugin->actif = actif;
          plugin->start = 1;
          break;
        }
       plugins = plugins->next;
     }
  }
/**********************************************************************************************************/
/* Retirer_plugins: Decharge toutes les librairies                                                        */
/* Entrée: Rien                                                                                           */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Lister_plugins ( void )
  { struct PLUGIN_DLS_DL *plugin;
    GList *plugins;

    plugins = Plugins;
    while(plugins)                                               /* On cherche tous les modules un par un */
     { plugin = (struct PLUGIN_DLS_DL *)plugins->data;

       if ( plugin->actif )
        { Info_n( Config.log, DEBUG_INFO, "DLS: Lister_plugins: plugin", plugin->id );
          Info_c( Config.log, DEBUG_INFO, "                           ", plugin->nom_fichier );
        }
       plugins = plugins->next;
     }
  }
/**********************************************************************************************************/
/* Charger_plugins: Ouverture de toutes les librairies possibles pour le DLS                              */
/* Entrée: Rien                                                                                           */
/* Sortie: Rien                                                                                           */
/**********************************************************************************************************/
 void Charger_plugins ( void )
  { struct DB *Db_watchdog;                                                          /* Database Watchdog */
    SQLHSTMT hquery;
    struct PLUGIN_DLS *dls;
    gint cpt;

    Db_watchdog = ConnexionDB( Config.log, Config.db_database,
                               Config.db_username, Config.db_password );
    if (!Db_watchdog)
     { Info_c( Config.log, DEBUG_DB, "DLS: Charger_plugins: Unable to open database (dsn)", Config.db_database );
       return;
     }

    hquery = Recuperer_plugins_dlsDB( Config.log, Db_watchdog );
    if (!hquery)                                                                      /* Si pas de hquery */
     { DeconnexionDB( Config.log, &Db_watchdog );
       return;
     }
    cpt = 0;
    do
     { dls = Recuperer_plugins_dlsDB_suite( Config.log, Db_watchdog, hquery );
       if (!dls)
        { DeconnexionDB( Config.log, &Db_watchdog );
          Info_n( Config.log, DEBUG_DLS, "DLS: active plugins", cpt );
          return;
        }

       if (Charger_un_plugin( dls->id, dls->on )==TRUE)
        { Info_c( Config.log, DEBUG_DLS, "DLS: Plugin DLS charge", dls->nom ); }
       if (dls->on) cpt++;
       g_free(dls);
     } while ( TRUE );
 }
/*--------------------------------------------------------------------------------------------------------*/
