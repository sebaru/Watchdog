/******************************************************************************************************************************/
/* Watchdogd/Dls/plugins.c  -> Gestion des plugins pour DLS                                                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        dim. 02 janv. 2011 19:04:47 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
 /*****************************************************************************************************************************/
/*
 * plugins.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/file.h>                                                                /* Gestion des verrous sur les fichiers */
 #include <sys/wait.h>

/************************************************** Prototypes de fonctions ***************************************************/
 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Dls_foreach_dls_tree: Parcours recursivement l'arbre DLS et execute des commandes en parametres                            */
/* Entrée : le Dls_tree et les fonctions a appliquer                                                                          */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_foreach_dls_tree ( struct DLS_TREE *dls_tree, void *user_data,
                                    void (*do_plugin) (void *user_data, struct PLUGIN_DLS *),
                                    void (*do_tree)   (void *user_data, struct DLS_TREE *) )
  { GSList *liste;
    liste = dls_tree->Liste_dls_tree;
    while (liste)
     { struct DLS_TREE *sub_tree;
       sub_tree = (struct DLS_TREE *)liste->data;
       Dls_foreach_dls_tree( sub_tree, user_data, do_plugin, do_tree );
       liste = liste->next;
     }
    liste = dls_tree->Liste_plugin_dls;
    while(liste && do_plugin)                                                        /* On execute tous les modules un par un */
     { struct PLUGIN_DLS *plugin_actuel;
       plugin_actuel = (struct PLUGIN_DLS *)liste->data;
       do_plugin( user_data, plugin_actuel );
       liste = liste->next;
     }
    if (do_tree) do_tree( user_data, dls_tree );
  }
/******************************************************************************************************************************/
/* Dls_foreach: Parcours l'arbre DLS et execute des commandes en parametres                                                   */
/* Entrée : les fonctions a appliquer                                                                                         */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 void Dls_foreach ( void *user_data, void (*do_plugin) (void *user_data, struct PLUGIN_DLS *),
                                     void (*do_tree)   (void *user_data, struct DLS_TREE *) )
  { if (Partage->com_dls.Dls_tree)
     { pthread_mutex_lock( &Partage->com_dls.synchro );
       Dls_foreach_dls_tree( Partage->com_dls.Dls_tree, user_data, do_plugin, do_tree );
       pthread_mutex_unlock( &Partage->com_dls.synchro );
     }
  }
/******************************************************************************************************************************/
/* Charger_un_plugin_par_nom: Ouverture d'un plugin dont le nom est en parametre                                              */
/* Entrée: Le plugin D.L.S                                                                                                    */
/* Sortie: FALSE si problème                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Charger_un_plugin ( struct PLUGIN_DLS *dls )
  { gchar nom_fichier_absolu[60];

    if (Partage->com_dls.Compil_at_boot) Compiler_source_dls( FALSE, dls->plugindb.tech_id, NULL, 0 );
    g_snprintf( nom_fichier_absolu, sizeof(nom_fichier_absolu), "Dls/libdls%s.so", dls->plugindb.tech_id );
    strncpy( dls->nom_fichier, nom_fichier_absolu, sizeof(dls->nom_fichier) );                 /* Init des variables communes */

    dls->handle = dlopen( nom_fichier_absolu, RTLD_LOCAL | RTLD_NOW );                      /* Ouverture du fichier librairie */
    if (!dls->handle)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                   "%s: Candidat '%s' dlopen failed (%s)", __func__, dls->plugindb.tech_id, dlerror() );
       Set_compil_status_plugin_dlsDB( dls->plugindb.tech_id, DLS_COMPIL_WARNING_FUNCTION_MISSING, "Function Missing" );
       return(FALSE);
     }

    dls->go = dlsym( dls->handle, "Go" );                                                    /* Recherche de la fonction 'Go' */
    if (!dls->go)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                 "%s: Candidat '%s' failed sur absence GO", __func__, dls->plugindb.tech_id );
       dlclose( dls->handle );
       Set_compil_status_plugin_dlsDB( dls->plugindb.tech_id, DLS_COMPIL_WARNING_FUNCTION_MISSING, "Function Missing" );
       dls->handle = NULL;
       return(FALSE);
     }
    dls->version = dlsym( dls->handle, "version" );                                            /* Recherche de la fonction */
    if (!dls->version)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                "%s: Candidat '%s' does not provide version function", __func__, dls->plugindb.tech_id );
       Set_compil_status_plugin_dlsDB( dls->plugindb.tech_id, DLS_COMPIL_WARNING_FUNCTION_MISSING, "Function Missing" );
     }

    dls->conso = 0.0;
    if (dls->plugindb.on) dls->start_date = time(NULL);
                     else dls->start_date = 0;
    memset ( &dls->vars, 0, sizeof(dls->vars) );                                 /* Mise à zero de tous les bits de remontées */
    dls->vars.debug = dls->plugindb.debug;                         /* Recopie du champ de debug depuis la DB vers la zone RUN */
  /*dls->vars.bit_comm_out = 1;                             /* Par construction, on considere que la comm est HS au démarrage */
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Reseter_all_bit_interne: Met a 0 et decharge tous les bits interne d'un plugin                                             */
/* Entrée: le plugin                                                                                                          */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static void Reseter_all_bit_interne ( struct PLUGIN_DLS *plugin )
  { GSList *liste_bit;
    pthread_mutex_lock( &Partage->com_dls.synchro_data );                                 /* Décharge tous les bits du module */
    liste_bit = Partage->Dls_data_TEMPO;
    while(liste_bit)
     { struct DLS_TEMPO *tempo = liste_bit->data;
       if (!strcmp(tempo->tech_id, plugin->plugindb.tech_id))
        { tempo->status = DLS_TEMPO_NOT_COUNTING;                                           /* La tempo ne compte pas du tout */
          tempo->state  = FALSE;
          tempo->init   = FALSE;
        }
       liste_bit = g_slist_next(liste_bit);
     }
    Charger_confDB_Registre ( plugin->plugindb.tech_id );
    liste_bit = Partage->Dls_data_MSG;                                                /* Decharge tous les messages du module */
    while(liste_bit)
     { struct DLS_MESSAGES *msg = liste_bit->data;
       liste_bit = g_slist_next(liste_bit);
       if (!strcmp(msg->tech_id, plugin->plugindb.tech_id))
        { Dls_data_set_MSG ( &plugin->vars, msg->tech_id, msg->acronyme, (gpointer *)&msg, FALSE, FALSE ); }
     }
    liste_bit = Partage->Dls_data_BOOL;                                               /* Decharge tous les booleens du module */
    while(liste_bit)
     { struct DLS_BOOL *bool = liste_bit->data;
       liste_bit = g_slist_next(liste_bit);
       if (!strcmp(bool->tech_id, plugin->plugindb.tech_id))
        { Dls_data_set_bool ( &plugin->vars, bool->tech_id, bool->acronyme, (gpointer *)&bool, FALSE ); }
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro_data );
  }
/******************************************************************************************************************************/
/* Decharger_plugins: Decharge tous les plugins DLS                                                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static void Reseter_plugin_dls_tree ( struct DLS_TREE *dls_tree, gchar *tech_id )
  { struct PLUGIN_DLS *plugin;
    GSList *liste;
    liste = dls_tree->Liste_plugin_dls;
    while(liste)                                                                            /* Liberation mémoire des modules */
     { plugin = (struct PLUGIN_DLS *)liste->data;
       if ( ! strcasecmp(plugin->plugindb.tech_id, tech_id ) )
        { Reseter_all_bit_interne ( plugin );
          if (plugin->handle)                                   /* Peut etre à 0 si changement de librairie et erreur de link */
           { if (dlclose( plugin->handle ))
              { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: dlclose error '%s' for '%s' (%s)", __func__,
                          dlerror(), plugin->plugindb.tech_id, plugin->plugindb.shortname );
              }
             plugin->handle = NULL;
             Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: plugin '%s' (%s) unloaded", __func__,
                       plugin->plugindb.tech_id, plugin->plugindb.shortname );
           }
          Charger_un_plugin ( plugin );
          plugin->vars.starting = 1;                                               /* au chargement, le bit de start vaut 1 ! */
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: plugin '%s' (%s) loaded", __func__,
                    plugin->plugindb.tech_id, plugin->plugindb.shortname );
          return;
        }
       liste=liste->next;
     }

    liste = dls_tree->Liste_dls_tree;
    while (liste)
     { struct DLS_TREE *sub_dls_tree = liste->data;
       Reseter_plugin_dls_tree ( sub_dls_tree, tech_id );
       liste=liste->next;
     }
  }
/******************************************************************************************************************************/
/* Retirer_plugins: Decharge toutes les librairies                                                                            */
/* Entrée: Le numéro du plugin a décharger                                                                                    */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Reseter_un_plugin ( gchar *tech_id )
  { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: Reset plugin '%s' en cours", __func__, tech_id );
    pthread_mutex_lock( &Partage->com_dls.synchro );
    Reseter_plugin_dls_tree ( Partage->com_dls.Dls_tree, tech_id );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Decharger_plugins: Decharge tous les plugins DLS                                                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static void Decharger_plugin_by_id_dls_tree ( gint id, struct DLS_TREE *dls_tree )
  { GSList *liste;

    liste = dls_tree->Liste_plugin_dls;
    while(liste)                                                                            /* Liberation mémoire des modules */
     { struct PLUGIN_DLS *plugin = liste->data;
       if (plugin->plugindb.id == id)
        { Reseter_all_bit_interne (plugin);
          if (plugin->handle)
           { if (dlclose( plugin->handle ))
              { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: dlclose error '%s' for '%s' (%s)", __func__,
                          dlerror(), plugin->plugindb.tech_id, plugin->plugindb.shortname );
              }
           }
          dls_tree->Liste_plugin_dls = g_slist_remove( dls_tree->Liste_plugin_dls, plugin );
                                                                            /* Destruction de l'entete associée dans la GList */
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: plugin '%s' unloaded (%s)", __func__,
                    plugin->plugindb.tech_id, plugin->plugindb.nom );
          g_free( plugin );
          return;
        }
       liste=liste->next;
     }

    liste = dls_tree->Liste_dls_tree;
    while (liste)
     { struct DLS_TREE *sub_dls_tree = liste->data;
       Decharger_plugin_by_id_dls_tree ( id, sub_dls_tree );
       liste=liste->next;
     }
  }
/******************************************************************************************************************************/
/* Decharger_plugins: Decharge tous les plugins DLS                                                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Decharger_plugin_by_id ( gint id )
  { pthread_mutex_lock( &Partage->com_dls.synchro );
    Decharger_plugin_by_id_dls_tree ( id, Partage->com_dls.Dls_tree );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Decharger_plugins: Decharge tous les plugins DLS                                                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static void Decharger_plugins_dls_tree ( struct DLS_TREE *dls_tree )
  { struct PLUGIN_DLS *plugin;

    while(dls_tree->Liste_plugin_dls)                                                        /* Liberation mémoire des modules */
     { plugin = (struct PLUGIN_DLS *)dls_tree->Liste_plugin_dls->data;
       if (plugin->handle && dlclose( plugin->handle ))
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: dlclose error '%s' for '%s' (%s)", __func__,
                    dlerror(), plugin->plugindb.tech_id, plugin->plugindb.shortname );
        }
       dls_tree->Liste_plugin_dls = g_slist_remove( dls_tree->Liste_plugin_dls, plugin );
                                                                             /* Destruction de l'entete associé dans la GList */
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: plugin '%s' unloaded (%s)", __func__,
                 plugin->plugindb.tech_id, plugin->plugindb.nom );
       g_free( plugin );
     }

    while (dls_tree->Liste_dls_tree)
     { struct DLS_TREE *sub_dls_tree = dls_tree->Liste_dls_tree->data;
       Decharger_plugins_dls_tree ( sub_dls_tree );
       dls_tree->Liste_dls_tree = g_slist_remove(dls_tree->Liste_dls_tree, sub_dls_tree);
       g_free(sub_dls_tree);
     }
  }
/******************************************************************************************************************************/
/* Decharger_plugins: Decharge tous les plugins DLS                                                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Decharger_plugins ( void )
  { pthread_mutex_lock( &Partage->com_dls.synchro );
    Decharger_plugins_dls_tree ( Partage->com_dls.Dls_tree );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Creer_dls_activite_securite: Creation du fichier de management des vignettes et etiquettes                                 */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static struct DLS_TREE *Dls_charger_plugins_for_syn ( gint id )
  { struct DLS_TREE *dls_tree = NULL;
    struct DB *db;
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Starting for syn id '%d'", __func__, id );

    dls_tree = (struct DLS_TREE *)g_try_malloc0( sizeof(struct DLS_TREE) );
    if (!dls_tree)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory Error for id '%s'", __func__, id );
       return(NULL);
     }
    dls_tree->syn_vars.syn_id = id;

    if ( Recuperer_plugins_dlsDB_by_syn( &db, id ) )
     { struct CMD_TYPE_PLUGIN_DLS *plugindb;
       while ( (plugindb = Recuperer_plugins_dlsDB_suite( &db )) != NULL )
        { struct PLUGIN_DLS *dls;
          dls = (struct PLUGIN_DLS *)g_try_malloc0( sizeof(struct PLUGIN_DLS) );
          if (!dls)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: out of memory", __func__ );
             g_free(plugindb);
           }
          else { memcpy( &dls->plugindb, plugindb, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
                 g_free(plugindb);
                 Charger_un_plugin( dls );                                                            /* Chargement du plugin */
                 dls_tree->Liste_plugin_dls = g_slist_append( dls_tree->Liste_plugin_dls, dls );
               }
        }
     }

    if ( ! Recuperer_synoptiqueDB_enfant( &db, id ) ) return(dls_tree);                             /* Si pas de connexion DB */
    struct CMD_TYPE_SYNOPTIQUE *syn;
    while ( (syn = Recuperer_synoptiqueDB_suite( &db )) != NULL )
     { if (syn->id != 1)                                                          /* Pas de bouclage sur le synoptique root ! */
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG,
                    "%s: Loading sub plugins for syn '%d' '%s' (parent '%d')", __func__, syn->id, syn->page, id );
          dls_tree->Liste_dls_tree = g_slist_append( dls_tree->Liste_dls_tree, Dls_charger_plugins_for_syn ( syn->id ) );
        }
       g_free(syn);
     }
    return(dls_tree);
  }
/******************************************************************************************************************************/
/* Charger_plugins: Ouverture de toutes les librairies possibles pour le DLS                                                  */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Charger_plugins ( void )
  { pthread_mutex_lock( &Partage->com_dls.synchro );
    Partage->com_dls.Dls_tree = Dls_charger_plugins_for_syn(1);                              /* Chargement du root synoptique */
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    Partage->com_dls.Compil_at_boot = FALSE;/* Apres le chargement initial, on considere que la recompil n'est pas necessaire */
  }
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_start_plugin_dls_tree ( void *user_data, struct PLUGIN_DLS *plugin )
  { gchar *tech_id = (gchar *)user_data;
    if ( ! strcasecmp ( plugin->plugindb.tech_id, tech_id ) )
     { gchar chaine[128];
       plugin->plugindb.on = TRUE;
       plugin->conso = 0.0;
       plugin->start_date = time(NULL);
       plugin->vars.starting = 1;
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: '%s' started (%s)", __func__, plugin->plugindb.tech_id, plugin->plugindb.nom );
       g_snprintf(chaine, sizeof(chaine), "UPDATE dls SET actif='1' WHERE tech_id = '%s'", plugin->plugindb.tech_id );
       SQL_Write ( chaine );
     }
  }
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_stop_plugin_dls_tree ( void *user_data, struct PLUGIN_DLS *plugin )
  { gchar *tech_id = (gchar *)user_data;
    if ( ! strcasecmp ( plugin->plugindb.tech_id, tech_id ) )
     { gchar chaine[128];
       plugin->plugindb.on = FALSE;
       plugin->start_date = 0;
       plugin->conso = 0.0;
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: '%s' stopped (%s)", __func__,
                 plugin->plugindb.tech_id, plugin->plugindb.nom );
       g_snprintf(chaine, sizeof(chaine), "UPDATE dls SET actif='0' WHERE tech_id = '%s'", plugin->plugindb.tech_id );
       SQL_Write ( chaine );
     }
  }
/******************************************************************************************************************************/
/* Activer_plugin_by_id: Active ou non un plugin by id                                                                        */
/* Entrée: l'ID du plugin                                                                                                     */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Activer_plugin ( gchar *tech_id, gboolean actif )
  { if (actif) Dls_foreach ( tech_id, Dls_start_plugin_dls_tree, NULL );
          else Dls_foreach ( tech_id, Dls_stop_plugin_dls_tree, NULL );
  }
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_debug_plugin_dls_tree ( void *user_data, struct PLUGIN_DLS *plugin )
  { gchar *tech_id = (gchar *)user_data;
    if ( ! strcasecmp ( plugin->plugindb.tech_id, tech_id ) )
     { gchar chaine[128];
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: '%s' debug started ('%s')", __func__,
                 plugin->plugindb.tech_id, plugin->plugindb.nom );
       plugin->vars.debug = TRUE;
       g_snprintf(chaine, sizeof(chaine), "UPDATE dls SET debug='1' WHERE tech_id = '%s'", plugin->plugindb.tech_id );
       SQL_Write ( chaine );
     }
  }
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_undebug_plugin_dls_tree ( void *user_data, struct PLUGIN_DLS *plugin )
  { gchar *tech_id = (gchar *)user_data;
    if ( ! strcasecmp ( plugin->plugindb.tech_id, tech_id ) )
     { gchar chaine[128];
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: '%s' debug stopped ('%s')", __func__,
                 plugin->plugindb.tech_id, plugin->plugindb.nom );
       plugin->vars.debug = FALSE;
       g_snprintf(chaine, sizeof(chaine), "UPDATE dls SET debug='0' WHERE tech_id = '%s'", plugin->plugindb.tech_id );
       SQL_Write ( chaine );
     }
  }
/******************************************************************************************************************************/
/* Activer_plugin_by_id: Active ou non un plugin by id                                                                        */
/* Entrée: l'ID du plugin                                                                                                     */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Debug_plugin ( gchar *tech_id, gboolean actif )
  { if (actif) Dls_foreach ( tech_id, Dls_debug_plugin_dls_tree, NULL );
          else Dls_foreach ( tech_id, Dls_undebug_plugin_dls_tree, NULL );
  }
/******************************************************************************************************************************/
/* Proto_compiler_source_dls: Compilation de la source DLS                                                                    */
/* Entrée: reset=1 s'il faut resetter le plugin aprÃ¨s compil, l'id associé, et le buffer de sortie                            */
/* Sortie: code d'erreur ou 0 si OK                                                                                           */
/******************************************************************************************************************************/
 gint Compiler_source_dls( gboolean reset, gchar *tech_id, gchar *buffer, gint taille_buffer )
  { gint retour, pidgcc, id_fichier;
    gchar log_buffer[1024], log_file[20];
    gchar chaine[128];
    gint taille_source;
    gchar *Source;

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Compilation module DLS '%s'", __func__, tech_id );
    if (buffer) memset (buffer, 0, taille_buffer);                                                 /* RAZ du buffer de sortie */

    if ( Get_source_dls_from_DB ( tech_id, &Source, &taille_source ) == FALSE )         /* On récupère le source depuis la DB */
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Cannot get Source DLS for id '%s'", __func__, tech_id );
       return(DLS_COMPIL_EXPORT_DB_FAILED);
     }

    g_snprintf( chaine, sizeof(chaine), "Dls/%s.dls", tech_id );
    unlink(chaine);
    id_fichier = open( chaine, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (id_fichier<0 || lockf( id_fichier, F_TLOCK, 0 ) )
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING, "%s: Open file '%s' for write failed (%s)", __func__,
                 chaine, strerror(errno) );
       close(id_fichier);
       return(DLS_COMPIL_EXPORT_DB_FAILED);
     }
    else
     { if (write( id_fichier, Source, taille_source )<0)
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Write %d bytes to file '%s' failed (%s)", __func__,
                    taille_source, chaine, strerror(errno) );
         close(id_fichier);
         return(DLS_COMPIL_EXPORT_DB_FAILED);
        }
       else
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Write %d bytes to file '%s' OK", __func__,
                    taille_source, chaine);
          close(id_fichier);
        }
     }

    retour = Traduire_DLS( tech_id );                                                                       /* Traduction DLS */
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG,
             "%s: fin traduction '%s'. Code retour '%d'", __func__, tech_id, retour );

    if (retour == TRAD_DLS_ERROR_NO_FILE)                                             /* Retour de la traduction D.L.S vers C */
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: envoi erreur file Traduction D.L.S '%s'", tech_id );
       Set_compil_status_plugin_dlsDB( tech_id, DLS_COMPIL_ERROR_LOAD_SOURCE, "Erreur chargement source" );
       return( DLS_COMPIL_ERROR_LOAD_SOURCE );
     }

    memset ( log_buffer, 0, sizeof(log_buffer) );                            /* Chargement de fichier de log dans la database */

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Chargement du fichier de log D.L.S '%s'", __func__, tech_id );
    g_snprintf( log_file, sizeof(log_file), "Dls/%s.log", tech_id );

    id_fichier = open( log_file, O_RDONLY, 0 );              /* Ouverture du fichier log et chargement du contenu dans buffer */
    if (id_fichier<0)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
                "%s: Impossible de charger le fichier de log '%s' : %s", __func__, log_file, strerror(errno) );
       Set_compil_status_plugin_dlsDB( tech_id, DLS_COMPIL_ERROR_LOAD_LOG, "Erreur chargement log" );
       return(DLS_COMPIL_ERROR_LOAD_LOG);
     }
    else
     { int nbr_car, nbr_car_lu;
       nbr_car = nbr_car_lu = 0;
       while ( (nbr_car = read (id_fichier, log_buffer + nbr_car_lu, sizeof(log_buffer)-1-nbr_car_lu )) > 0 )
        { nbr_car_lu+=nbr_car; }
       close(id_fichier);
       if (buffer) memcpy ( buffer, log_buffer, nbr_car_lu );
     }

    if ( retour == TRAD_DLS_SYNTAX_ERROR )
     { Set_compil_status_plugin_dlsDB( tech_id, DLS_COMPIL_SYNTAX_ERROR, log_buffer );
       return ( DLS_COMPIL_SYNTAX_ERROR );
     }

    pidgcc = fork();
    if (pidgcc<0)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING, "%s_Fils: envoi erreur Fork GCC '%s'", __func__, tech_id );
       Set_compil_status_plugin_dlsDB( tech_id, DLS_COMPIL_ERROR_FORK_GCC, "Erreur Fork GCC" );
       return(DLS_COMPIL_ERROR_FORK_GCC);
     }
    else if (!pidgcc)
     { gchar source[80], cible[80];
       g_snprintf( source, sizeof(source), "Dls/%s.c", tech_id );
       g_snprintf( cible,  sizeof(cible),  "Dls/libdls%s.so", tech_id );
       execlp( "gcc", "gcc", "-I/usr/include/glib-2.0", "-I/usr/lib/glib-2.0/include", "-I/usr/lib64/glib-2.0/include",
               "-I/usr/lib/i386-linux-gnu/glib-2.0/include", "-I/usr/lib/x86_64-linux-gnu/glib-2.0/include",
               "-shared", "--no-gnu-unique", "-ggdb", "-Wall", "-lwatchdog-dls", source, "-fPIC", "-o", cible, NULL );
       _exit(0);
     }

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Waiting for gcc to finish pid %d", __func__, pidgcc );
    wait4(pidgcc, NULL, 0, NULL );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: gcc is down, OK %d", __func__, pidgcc );

    if (reset)                                                                          /* Demande le reset du plugin à D.L.S */
     { Reseter_un_plugin ( tech_id ); }

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: end of '%s'", __func__, tech_id );
    if (retour == TRAD_DLS_WARNING)
     { Set_compil_status_plugin_dlsDB( tech_id, DLS_COMPIL_OK_WITH_WARNINGS, log_buffer );
       return( DLS_COMPIL_OK_WITH_WARNINGS );
     }

    Set_compil_status_plugin_dlsDB( tech_id, DLS_COMPIL_OK, "Pas d'erreur !" );
    return( DLS_COMPIL_OK );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
