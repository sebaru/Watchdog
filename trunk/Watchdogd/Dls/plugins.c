/******************************************************************************************************************************/
/* Watchdogd/Dls/plugins.c  -> Gestion des plugins pour DLS                                                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        dim. 02 janv. 2011 19:04:47 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
 /*****************************************************************************************************************************/
/*
 * plugins.c
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
/* Check_action_bit_use: Vérifie que les bits d'actions positionnés par le module sont bien owné par celui-ci                 */
/* Entrée: Le plugin D.L.S                                                                                                    */
/* Sortie: FALSE si problème                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Check_action_bit_use ( struct PLUGIN_DLS *dls )
  { gint cpt, dls_id;
    if(!dls) return(FALSE);
    if(!dls->Get_Tableau_bit) return(TRUE);
    if(!dls->Get_Tableau_num) return(TRUE);
    if(!dls->Get_Tableau_msg) return(TRUE);

    for ( cpt=0; dls->Get_Tableau_bit(cpt) != -1; cpt++ )                                          /* Check des bits internes */
     { struct CMD_TYPE_NUM_MNEMONIQUE critere;
       struct CMD_TYPE_MNEMO_BASE *mnemo;
       critere.type = dls->Get_Tableau_bit(cpt);
       critere.num  = dls->Get_Tableau_num(cpt);
       mnemo = Rechercher_mnemo_baseDB_type_num ( &critere );
       Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                "%s: Test Mnemo %d %d for id %d: mnemo %p", __func__,
                 dls->Get_Tableau_bit(cpt), dls->Get_Tableau_num(cpt), dls->plugindb.id, mnemo ); 
       if (!mnemo) return(FALSE);
       dls_id = mnemo->dls_id;
       g_free(mnemo);
       if (dls_id != dls->plugindb.id) return(FALSE);
     }

    for ( cpt=0; dls->Get_Tableau_msg(cpt) != -1; cpt++ )                                          /* Check des bits internes */
     { struct CMD_TYPE_MESSAGE *message;
       message = Rechercher_messageDB ( dls->Get_Tableau_msg(cpt) );
       Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                "%s: Test MSG %d for id %d: mnemo %p", __func__, dls->Get_Tableau_msg(cpt), dls->plugindb.id, message ); 
       if (!message) return(FALSE);
       dls_id = message->dls_id;
       g_free(message);
       if (dls_id != dls->plugindb.id) return(FALSE);
     }

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Charger_un_plugin_par_nom: Ouverture d'un plugin dont le nom est en parametre                                              */
/* Entrée: Le plugin D.L.S                                                                                                    */
/* Sortie: FALSE si problème                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Charger_un_plugin ( struct PLUGIN_DLS *dls )
  { gchar nom_fichier_absolu[60];
    gboolean retour;

    g_snprintf( nom_fichier_absolu, sizeof(nom_fichier_absolu), "Dls/libdls%06d.so", dls->plugindb.id );
    strncpy( dls->nom_fichier, nom_fichier_absolu, sizeof(dls->nom_fichier) );                 /* Init des variables communes */
    dls->starting = 1;                                                             /* au chargement, le bit de start vaut 1 ! */
    dls->conso    = 0.0;

    retour = FALSE;                                                                          /* Par défaut, on retourne FALSE */
    dls->handle = dlopen( nom_fichier_absolu, RTLD_GLOBAL | RTLD_NOW );                     /* Ouverture du fichier librairie */
    if (!dls->handle)
     { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                "%s: Candidat %06d failed (%s)", __func__, dls->plugindb.id, dlerror() );
     }
    else
     { dls->go = dlsym( dls->handle, "Go" );                                                 /* Recherche de la fonction 'Go' */
       if (!dls->go)
        { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                    "%s: Candidat %06d failed sur absence GO", __func__, dls->plugindb.id ); 
          dlclose( dls->handle );
          dls->handle = NULL;
        }
       if (dls->handle)
        { dls->Get_Tableau_bit = dlsym( dls->handle, "Get_Tableau_bit" );                         /* Recherche de la fonction */
          if (!dls->Get_Tableau_bit)
           { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                      "%s: Candidat %06d does not provide Get_Tableau_bit function", __func__, dls->plugindb.id ); 
             Set_compil_status_plugin_dlsDB( dls->plugindb.id, DLS_COMPIL_WARNING_FUNCTION_MISSING );
           }

          dls->Get_Tableau_num = dlsym( dls->handle, "Get_Tableau_num" );                         /* Recherche de la fonction */
          if (!dls->Get_Tableau_num)
           { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                      "%s: Candidat %06d does not provide Get_Tableau_num function", __func__, dls->plugindb.id ); 
             Set_compil_status_plugin_dlsDB( dls->plugindb.id, DLS_COMPIL_WARNING_FUNCTION_MISSING );
           }

          dls->Get_Tableau_msg = dlsym( dls->handle, "Get_Tableau_msg" );                         /* Recherche de la fonction */
          if (!dls->Get_Tableau_msg)
           { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                      "%s: Candidat %06d does not provide Get_Tableau_msg function", __func__, dls->plugindb.id ); 
             Set_compil_status_plugin_dlsDB( dls->plugindb.id, DLS_COMPIL_WARNING_FUNCTION_MISSING );
           }
         }
       
       Info_new( Config.log, Config.log_dls, LOG_INFO, "%s: plugin %06d loaded (%s) checking bits (start=%d)", __func__,
                 dls->plugindb.id, dls->plugindb.shortname, dls->plugindb.on );
       retour = TRUE;

       if (Check_action_bit_use( dls ) == FALSE )
        { Info_new( Config.log, Config.log_dls, LOG_WARNING,
                   "%s: plugin %06d -> bit(s) set but not owned by itself... Disabling", __func__, dls->plugindb.id ); 
          Set_compil_status_plugin_dlsDB( dls->plugindb.id, DLS_COMPIL_ERROR_BIT_SET_BUT_NOT_OWNED );
          dls->plugindb.on=FALSE;
        }
       else Info_new( Config.log, Config.log_dls, LOG_INFO,
                     "%s: plugin %06d -> bit(s) ownership OK", __func__, dls->plugindb.id ); 

     }
    if (dls->plugindb.on) dls->start_date = time(NULL);
                     else dls->start_date = 0;
    return(retour);
  }
/******************************************************************************************************************************/
/* Decharger_plugins: Decharge tous les plugins DLS                                                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static void Reseter_plugin_by_id_dls_tree ( struct DLS_TREE *dls_tree, gint dls_id )
  { struct PLUGIN_DLS *plugin;
    GSList *liste;
    liste = dls_tree->Liste_plugin_dls;
    while(liste)                                                                            /* Liberation mémoire des modules */
     { plugin = (struct PLUGIN_DLS *)liste->data;
       if ( plugin->plugindb.id == dls_id )
        {
          if (plugin->handle)
           { dlclose( plugin->handle );
             plugin->plugindb.on = TRUE;                                                           /* On tente de l'allumer ! */
             Charger_un_plugin ( plugin );
             Info_new( Config.log, Config.log_dls, LOG_INFO, "%s: plugin %06d reloaded (%s)", __func__,
                       plugin->plugindb.id, plugin->plugindb.shortname );
           }
          return;
        }
       liste=liste->next;
     }

    liste = dls_tree->Liste_dls_tree;
    while (liste)
     { struct DLS_TREE *sub_dls_tree = liste->data;
       Reseter_plugin_by_id_dls_tree ( sub_dls_tree, dls_id );
       liste=liste->next;
     }
  }
/******************************************************************************************************************************/
/* Retirer_plugins: Decharge toutes les librairies                                                                            */
/* Entrée: Le numéro du plugin a décharger                                                                                    */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Reseter_un_plugin ( gint id )
  { Info_new( Config.log, Config.log_dls, LOG_INFO, "%s: Reset plugin %06d", __func__, id );
    pthread_mutex_lock( &Partage->com_dls.synchro );
    Reseter_plugin_by_id_dls_tree ( Partage->com_dls.Dls_tree, id );
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
        { dlclose( plugin->handle );
          dls_tree->Liste_plugin_dls = g_slist_remove( dls_tree->Liste_plugin_dls, plugin );
                                                                             /* Destruction de l'entete associé dans la GList */
          Info_new( Config.log, Config.log_dls, LOG_INFO, "%s: plugin %06d unloaded (%s)", __func__,
                    plugin->plugindb.id, plugin->plugindb.nom );
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
    GSList *liste;

    while(dls_tree->Liste_plugin_dls)                                                        /* Liberation mémoire des modules */
     { plugin = (struct PLUGIN_DLS *)dls_tree->Liste_plugin_dls->data;
       if (plugin->handle) dlclose( plugin->handle );
       dls_tree->Liste_plugin_dls = g_slist_remove( dls_tree->Liste_plugin_dls, plugin );
                                                                             /* Destruction de l'entete associé dans la GList */
       Info_new( Config.log, Config.log_dls, LOG_INFO, "%s: plugin %06d unloaded (%s)", __func__,
                 plugin->plugindb.id, plugin->plugindb.nom );
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
    Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s: Starting for syn id '%d'", __func__, id );

    dls_tree = (struct DLS_TREE *)g_try_malloc0( sizeof(struct DLS_TREE) );
    if (!dls_tree)
     { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: Memory Error for id '%s'", __func__, id );
       return(NULL);
     }
    dls_tree->syn_vars.syn_id = id;

    if ( Recuperer_plugins_dlsDB_by_syn( &db, id ) )
     { struct CMD_TYPE_PLUGIN_DLS *plugindb;
       while ( (plugindb = Recuperer_plugins_dlsDB_suite( &db )) != NULL )
        { struct PLUGIN_DLS *dls;
          Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                    "%s: Loading plugins '%s' for syn '%d' '%s'", __func__, plugindb->shortname, id, plugindb->syn_page );
          dls = (struct PLUGIN_DLS *)g_try_malloc0( sizeof(struct PLUGIN_DLS) );
          if (!dls)
           { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: out of memory", __func__ );
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
        { Info_new( Config.log, Config.log_dls, LOG_DEBUG,
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
    Config.compil = 0;                                                                                   /* fin de traitement */
  }
/******************************************************************************************************************************/
/* Activer_plugin_by_id_dls_tree: Active ou non un plugin by id                                                               */
/* Entrée: l'ID du plugin, start ou stop et le dls_tree support                                                               */
/* Sortie: FALSE si pas trouvé dans le dls_tree en parametre                                                                  */
/******************************************************************************************************************************/
 static gboolean Activer_plugin_by_id_dls_tree ( gint id, gboolean actif, struct DLS_TREE *dls_tree )
  { struct PLUGIN_DLS *plugin;
    GSList *plugins, *liste;

    plugins = dls_tree->Liste_plugin_dls;
    while(plugins)                                                                                  /* Pour chacun des plugin */
     { plugin = (struct PLUGIN_DLS *)plugins->data;
       if ( plugin->plugindb.id == id )
        { if (actif == FALSE)
           { plugin->plugindb.on = FALSE;
             plugin->start_date = 0;
             plugin->conso = 0.0;
             Info_new( Config.log, Config.log_dls, LOG_INFO, "%s: id %06d stopped (%s)", __func__, plugin->plugindb.id, plugin->plugindb.nom );
           }
          else if (Check_action_bit_use( plugin ) == TRUE)
           { plugin->plugindb.on = TRUE;
             plugin->conso = 0.0;
             plugin->starting = 1;
             plugin->start_date = time(NULL);
             Info_new( Config.log, Config.log_dls, LOG_INFO, "%s: id %06d started (%s)", __func__, plugin->plugindb.id, plugin->plugindb.nom );
           }
          else
           { plugin->plugindb.on = 0;
             plugin->start_date = 0;
             plugin->conso = 0.0;
             plugin->starting = 0;
             Info_new( Config.log, Config.log_dls, LOG_WARNING,
                      "%s: Candidat %06d -> bit(s) set but not owned by itself... Disabling", __func__, plugin->plugindb.id ); 
           }
          return(TRUE);
        }
       plugins = plugins->next;
     }

    liste = dls_tree->Liste_dls_tree;                                             /* Si pas trouvé, on cherche dans les sub dls */
    while (liste)
     { if (Activer_plugin_by_id_dls_tree( id, actif, (struct DLS_TREE *)liste->data ) == TRUE ) return(TRUE);
       liste = liste->next;
     }
    return(FALSE);
  }
/******************************************************************************************************************************/
/* Activer_plugin_by_id: Active ou non un plugin by id                                                                        */
/* Entrée: l'ID du plugin                                                                                                     */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Activer_plugin_by_id ( gint id, gboolean actif )
  { pthread_mutex_lock( &Partage->com_dls.synchro );
    Activer_plugin_by_id_dls_tree( id, actif, Partage->com_dls.Dls_tree );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Proto_compiler_source_dls: Compilation de la source DLS                                                                    */
/* Entrée: reset=1 s'il faut resetter le plugin après compil, l'id associé, et le buffer de sortie                            */
/* Sortie: code d'erreur ou 0 si OK                                                                                           */
/******************************************************************************************************************************/
 gint Compiler_source_dls( gboolean reset, gint id, gchar *buffer, gint taille_buffer )
  { gint retour, pidgcc, id_fichier;
    gchar chaine[128];
    gint taille_source;
    gchar *Source;
  
    Info_new( Config.log, Config.log_dls, LOG_NOTICE, "%s: Compilation module DLS %06d", __func__, id );
    if (buffer) memset (buffer, 0, taille_buffer);                                                 /* RAZ du buffer de sortie */

    if ( Get_source_dls_from_DB ( id, &Source, &taille_source ) == FALSE )              /* On récupère le source depuis la DB */
     { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: Cannot get Source DLS for id '%06d'", __func__, id );
       return(DLS_COMPIL_EXPORT_DB_FAILED);
     }
     
    g_snprintf( chaine, sizeof(chaine), "Dls/%06d.dls", id );
    unlink(chaine);
    id_fichier = open( chaine, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (id_fichier<0 || lockf( id_fichier, F_TLOCK, 0 ) )
     { Info_new( Config.log, Config.log_dls, LOG_WARNING, "%s: Open file '%s' for write failed for %06d (%s)", __func__,
                 chaine, id, strerror(errno) );
       close(id_fichier);
       return(DLS_COMPIL_EXPORT_DB_FAILED);
     }
    else
     { if (write( id_fichier, Source, taille_source )<0)
        { Info_new( Config.log, Config.log_dls, LOG_ERR, "%s: Write %d bytes to file '%s' failed for %06d (%s)", __func__, 
                    taille_source, chaine, id, strerror(errno) );
         close(id_fichier);
         return(DLS_COMPIL_EXPORT_DB_FAILED);
        }
       else
        { Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s: Write %d bytes to file '%s' OK for %06d", __func__,
                    taille_source, chaine, id );
          close(id_fichier);
        }
     }

    retour = Traduire_DLS( id );                                                                            /* Traduction DLS */
    Info_new( Config.log, Config.log_dls, LOG_DEBUG,
             "%s: fin traduction %06d : %d", __func__, id, retour );

    if (retour == TRAD_DLS_ERROR_FILE)                                                /* Retour de la traduction D.L.S vers C */
     { Info_new( Config.log, Config.log_dls, LOG_DEBUG,
               "%s: envoi erreur file Traduction D.L.S %06d", id );
      Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_ERROR_LOAD_SOURCE );
      return( DLS_COMPIL_ERROR_LOAD_SOURCE );
     }

    if ( buffer )                             /* Chargement de fichier de log dans le buffer mis à disposition par l'appelant */
     { gint id_fichier;
       gchar log[20];

       Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                "%s: Chargement du fichier de log D.L.S %d", __func__, id );
       g_snprintf( log, sizeof(log), "Dls/%06d.log", id );

       id_fichier = open( log, O_RDONLY, 0 );                /* Ouverture du fichier log et chargement du contenu dans buffer */
       if (id_fichier<0)
        { Info_new( Config.log, Config.log_dls, LOG_ERR,
                "%s: Impossible de charger le fichier de log '%s' : %s", __func__, log, strerror(errno) );
          Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_ERROR_LOAD_LOG );
          return(DLS_COMPIL_ERROR_LOAD_LOG);
        }
       else { int nbr_car, index_buffer_erreur;
              nbr_car = index_buffer_erreur = 0; 
              while ( (nbr_car = read (id_fichier, buffer + index_buffer_erreur,
                                       taille_buffer-1-index_buffer_erreur )) > 0 )
               { index_buffer_erreur+=nbr_car; }
              close(id_fichier);
            }
     }

    if ( retour == TRAD_DLS_ERROR )
     { Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_ERROR_TRAD );
       return ( DLS_COMPIL_ERROR_TRAD );
     }

    pidgcc = fork();
    if (pidgcc<0)
     { Info_new( Config.log, Config.log_dls, LOG_WARNING, "%s_Fils: envoi erreur Fork GCC %06d", __func__, id );
       Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_ERROR_FORK_GCC );
       return(DLS_COMPIL_ERROR_FORK_GCC);
     }
    else if (!pidgcc)
     { gchar source[80], cible[80];
       g_snprintf( source, sizeof(source), "Dls/%06d.c", id );
       g_snprintf( cible,  sizeof(cible),  "Dls/libdls%06d.so", id );
       Info_new( Config.log, Config.log_dls, LOG_DEBUG,
                "%s: GCC start (pid %d) source %s cible %s!",
                 __func__, pidgcc, source, cible );
       execlp( "gcc", "gcc", "-I/usr/include/glib-2.0", "-I/usr/lib/glib-2.0/include", "-I/usr/lib64/glib-2.0/include",
               "-shared", "-o3", "-Wall", "-lwatchdog-dls", source, "-fPIC", "-o", cible, NULL );
       Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s_Fils: lancement GCC failed", __func__ );
       _exit(0);
     }

    Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s: Waiting for gcc to finish pid %d", __func__, pidgcc );
    wait4(pidgcc, NULL, 0, NULL );
    Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s: gcc is down, OK %d", __func__, pidgcc );

    if (reset)                                                                          /* Demande le reset du plugin à D.L.S */
     { Reseter_un_plugin ( id ); }

    Info_new( Config.log, Config.log_dls, LOG_DEBUG, "%s: end of %06d", __func__, id );
    if (retour == TRAD_DLS_WARNING)
     { Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_OK_WITH_WARNINGS );
       return( DLS_COMPIL_OK_WITH_WARNINGS );
     }

    Set_compil_status_plugin_dlsDB( id, DLS_COMPIL_OK );
    return( DLS_COMPIL_OK );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
