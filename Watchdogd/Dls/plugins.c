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
 void Dls_foreach_plugins ( gpointer user_data, void (*do_plugin) (gpointer user_data, struct DLS_PLUGIN *) )
  { GSList *liste;
    pthread_mutex_lock( &Partage->com_dls.synchro );
    liste = Partage->com_dls.Dls_plugins;
    while (liste)
     { struct DLS_PLUGIN *plugin;
       plugin = (struct DLS_PLUGIN *)liste->data;
       do_plugin( user_data, plugin );
       liste = liste->next;
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Dls_foreach_dls_tree: Parcours recursivement l'arbre DLS et execute des commandes en parametres                            */
/* Entrée : le Dls_tree et les fonctions a appliquer                                                                          */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_foreach_syns_reel ( struct DLS_SYN *syn_tree, gpointer user_data, void (*do_syn) (gpointer user_data, struct DLS_SYN *) )
  { GSList *liste;
    liste = syn_tree->Dls_sub_syns;
    while (liste)
     { struct DLS_SYN *sub_tree = liste->data;
       Dls_foreach_syns_reel( sub_tree, user_data, do_syn );
       liste = liste->next;
     }
    do_syn( user_data, syn_tree );
  }
/******************************************************************************************************************************/
/* Dls_foreach: Parcours l'arbre DLS et execute des commandes en parametres                                                   */
/* Entrée : les fonctions a appliquer                                                                                         */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 void Dls_foreach_syns ( gpointer user_data, void (*do_syn)(gpointer user_data, struct DLS_SYN *) )
  { if (Partage->com_dls.Dls_syns)
     { pthread_mutex_lock( &Partage->com_dls.synchro );
       Dls_foreach_syns_reel( Partage->com_dls.Dls_syns, user_data, do_syn );
       pthread_mutex_unlock( &Partage->com_dls.synchro );
     }
  }
/******************************************************************************************************************************/
/* Dls_foreach_dls_tree: Parcours recursivement l'arbre DLS et execute des commandes en parametres                            */
/* Entrée : le Dls_tree et les fonctions a appliquer                                                                          */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static struct DLS_SYN *Dls_search_syn_reel ( struct DLS_SYN *syn_tree, gint id )
  { GSList *liste;
    if (syn_tree->syn_id == id) return(syn_tree);

    liste = syn_tree->Dls_sub_syns;
    while (liste)
     { struct DLS_SYN *sub_tree = liste->data;
       struct DLS_SYN *result = Dls_search_syn_reel( sub_tree, id );
       if (result) return(result);
       liste = liste->next;
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_search_syn: Recherche le synoptique dont l'id est en parametre dans l'arbre des synoptiques                            */
/* Entrée : l'id du synoptique                                                                                                */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 struct DLS_SYN *Dls_search_syn ( gint id )
  { if (!Partage->com_dls.Dls_syns) return(NULL);
    pthread_mutex_lock( &Partage->com_dls.synchro );
    struct DLS_SYN *result = Dls_search_syn_reel( Partage->com_dls.Dls_syns, id );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    return(result);
  }
/******************************************************************************************************************************/
/* Dls_plugin_recalcule_arbre_comm: Calcule l'arbre de communication du module                                                */
/* Entrée: Le plugin D.L.S                                                                                                    */
/* Sortie: FALSE si problème                                                                                                  */
/******************************************************************************************************************************/
 static void Dls_plugin_recalculer_arbre_comm ( gpointer user_data, struct DLS_PLUGIN *dls )
  { gchar chaine[256];

    struct DB *db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: DB connexion failed", __func__ );
       return;
     }

    if (dls->Arbre_Comm)
     { g_slist_free(dls->Arbre_Comm);
       dls->Arbre_Comm = NULL;
     }
/*---------------------------- On recherche tous les tech_id des thread de DigitalInput --------------------------------------*/
    g_snprintf( chaine, sizeof(chaine), "SELECT DISTINCT(thread_tech_id) FROM mappings "
                                        "WHERE tech_id='%s' AND thread_tech_id NOT LIKE '_%%'", dls->tech_id );
    if (!Lancer_requete_SQL ( db, chaine ))
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: DB request failed", __func__ );
       return;
     }

    while ( Recuperer_ligne_SQL ( db ) != NULL )
     { gpointer comm = NULL;
       if (db->row[0])
        { Dls_data_get_MONO ( db->row[0], "COMM", &comm );
          if (comm) dls->Arbre_Comm = g_slist_prepend ( dls->Arbre_Comm, comm );
          else { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: %s:COMM not found !", __func__, db->row[0] ); }
        }
     }
    Libere_DB_SQL ( &db );
  }
/******************************************************************************************************************************/
/* Dls_plugin_recalcule_arbre_comm: Calcule l'arbre de communication du module                                                */
/* Entrée: Le plugin D.L.S                                                                                                    */
/* Sortie: FALSE si problème                                                                                                  */
/******************************************************************************************************************************/
 void Dls_recalculer_arbre_comm ( void )
  { Dls_foreach_plugins ( NULL, Dls_plugin_recalculer_arbre_comm ); }
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_stop_plugin_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = user_data;
    if ( strcasecmp ( tech_id, plugin->tech_id ) ) return;

    plugin->on = FALSE;
    plugin->start_date = 0;
    plugin->conso = 0.0;
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: '%s' stopped (%s)", __func__,
              plugin->tech_id, plugin->name );
  }
/******************************************************************************************************************************/
/* Dls_auto_create_plugin: Créé automatiquement le plugin en parametre (tech_id, nom)                                         */
/* Entrées: le tech_id (unique) et le nom associé                                                                             */
/* Sortie: -1 si pb, id sinon                                                                                                 */
/******************************************************************************************************************************/
 gboolean Dls_auto_create_plugin( gchar *tech_id, gchar *description )
  { JsonNode *RootNode = Json_node_create ();
    if (!RootNode)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for DLS create %s", __func__, tech_id );
       return(FALSE);
     }
    Json_node_add_string ( RootNode, "tech_id", tech_id );
    Json_node_add_string ( RootNode, "description", description );

    JsonNode *api_result = Http_Post_to_global_API ( "/run/dls/create", RootNode );
    if (api_result == NULL || Json_get_int ( api_result, "api_status" ) != SOUP_STATUS_OK)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
                 "%s: API Request for DLS CREATE failed. '%s' not created.", __func__, tech_id );
       Json_node_unref ( RootNode );
       return(FALSE);
     }
    Json_node_unref ( api_result );
    Json_node_unref ( RootNode );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_start_plugin_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = user_data;
    if ( strcasecmp ( tech_id, plugin->tech_id ) ) return;
    if ( plugin->compil_status == FALSE )
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE,
		         "%s: '%s' (%s) could not be started because of CompilStatus Not OK", __func__,
                 plugin->tech_id, plugin->name );
       return;
     }

    plugin->on = TRUE;
    plugin->conso = 0.0;
    plugin->start_date = time(NULL);
    plugin->vars.resetted = FALSE;
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: '%s' started (%s)", __func__, plugin->tech_id, plugin->name );
  }
/******************************************************************************************************************************/
/* Activer_plugin_by_id: Active ou non un plugin by id                                                                        */
/* Entrée: l'ID du plugin                                                                                                     */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_Activer_plugin ( gchar *tech_id, gboolean actif )
  { if (actif) Dls_foreach_plugins ( tech_id, Dls_start_plugin_reel );
          else Dls_foreach_plugins ( tech_id, Dls_stop_plugin_reel );
  }
/******************************************************************************************************************************/
/* Dls_Save_CodeC_to_disk: Enregistre un codec sur le disque pour le tech_id en parametre                                     */
/* Entrée: Le tech_id du DLS a sauver, et le codeC associé                                                                    */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Dls_Save_CodeC_to_disk ( gchar *tech_id, gchar *codec )
  { gchar source_file[128];

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Saving '%s' started", __func__, tech_id );
    g_snprintf( source_file, sizeof(source_file), "Dls/%s.c", tech_id );
    unlink(source_file);
    gint id_fichier = open( source_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (id_fichier<0 || lockf( id_fichier, F_TLOCK, 0 ) )
     { Info_new( Config.log, Config.log_trad, LOG_WARNING, "%s: Open file '%s' for write failed (%s)", __func__,
                 source_file, strerror(errno) );
       close(id_fichier);
       return(FALSE);
     }

    gint taille_codec = strlen(codec);
    gint retour_write = write( id_fichier, codec, taille_codec );
    close(id_fichier);
    if (retour_write<0)
     { Info_new( Config.log, Config.log_trad, LOG_ERR, "%s: Write %d bytes to file '%s' failed (%s)", __func__,
                 taille_codec, source_file, strerror(errno) );
       return(FALSE);
     }
    Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: Write %d bytes to file '%s' OK.", __func__, taille_codec, source_file );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Compiler_source_dls: Compilation de la source DLS en librairie                                                             */
/* Entrée: Le tech_id du DLS a compiler, et le codeC associé                                                                  */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 gboolean Dls_Compiler_source_dls( gchar *tech_id )
  { gchar source_file[128], target_file[128];

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Compilation of '%s' started", __func__, tech_id );
    gint top = Partage->top;
    g_snprintf( source_file, sizeof(source_file), "Dls/%s.c", tech_id );
    g_snprintf( target_file, sizeof(target_file),  "Dls/libdls%s.so", tech_id );
    Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: Starting GCC.", __func__ );

    gint pidgcc = fork();
    if (pidgcc<0)
     { Info_new( Config.log, Config.log_trad, LOG_WARNING, "%s_Fils: envoi erreur Fork GCC '%s'", __func__, tech_id );
       return(FALSE);
     }
    else if (!pidgcc)
     { execlp( "gcc", "gcc", "-I/usr/include/glib-2.0", "-I/usr/lib/glib-2.0/include", "-I/usr/lib64/glib-2.0/include",
               "-I/usr/lib/i386-linux-gnu/glib-2.0/include", "-I/usr/lib/x86_64-linux-gnu/glib-2.0/include",
               "-shared", "--no-gnu-unique", "-Wno-unused-variable", "-ggdb", "-Wall", "-lwatchdog-dls",
               source_file, "-fPIC", "-o", target_file, NULL );
       _exit(0);
     }

    Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: Waiting for gcc to finish pid %d", __func__, pidgcc );
    gint wcode;
    waitpid(pidgcc, &wcode, 0 );
    gint gcc_return_code = WEXITSTATUS(wcode);
    if (gcc_return_code == 1) unlink(target_file);
    Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: gcc pid %d is down with return code %d", __func__, pidgcc, gcc_return_code );
    Info_new( Config.log, Config.log_trad, LOG_INFO, "%s: Compilation of '%s' finished in %05.1fs", __func__, tech_id, (Partage->top - top)/10.0 );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Charger_un_plugin_par_nom: Ouverture d'un plugin dont le nom est en parametre                                              */
/* Entrée: Le plugin D.L.S                                                                                                    */
/* Sortie: FALSE si problème                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Dls_Charger_un_plugin ( struct DLS_PLUGIN *dls )
  { gchar nom_fichier_absolu[60];

    if (Partage->com_dls.Thread_run == FALSE) return(FALSE);          /* si l'instance est en cours d'arret, on sort de suite */
    g_snprintf( nom_fichier_absolu, sizeof(nom_fichier_absolu), "Dls/libdls%s.so", dls->tech_id );
    strncpy( dls->nom_fichier, nom_fichier_absolu, sizeof(dls->nom_fichier) );                 /* Init des variables communes */

    if (dls->handle)
     { dls->go = NULL;                                      /* On empeche DLS de lancer la fonction qui est en cours de decom */
       if (dlclose( dls->handle ))
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: dlclose error '%s' for '%s' (%s)", __func__,
                    dlerror(), dls->tech_id, dls->shortname );
        }
       dls->handle = NULL;
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: plugin '%s' (%s) unloaded", __func__,
                 dls->tech_id, dls->shortname );
     }

    dls->handle = dlopen( nom_fichier_absolu, RTLD_LOCAL | RTLD_NOW );                      /* Ouverture du fichier librairie */
    if (!dls->handle)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                   "%s: Candidat '%s' dlopen failed (%s)", __func__, dls->tech_id, dlerror() );
       return(FALSE);
     }

    dls->go = dlsym( dls->handle, "Go" );                                                    /* Recherche de la fonction 'Go' */
    if (!dls->go)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                 "%s: Candidat '%s' failed sur absence GO", __func__, dls->tech_id );
       dlclose( dls->handle );
       dls->handle = NULL;
       return(FALSE);
     }

    dls->version = dlsym( dls->handle, "version" );                                               /* Recherche de la fonction */
    if (!dls->version)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                "%s: Candidat '%s' does not provide version function", __func__, dls->tech_id );
       dlclose( dls->handle );
       dls->handle = NULL;
       return(FALSE);
     }

    dls->conso = 0.0;
    if (dls->on) dls->start_date = time(NULL);
            else dls->start_date = 0;
    memset ( &dls->vars, 0, sizeof(dls->vars) );                                 /* Mise à zero de tous les bits de remontées */
    dls->vars.debug = dls->debug;                                  /* Recopie du champ de debug depuis la DB vers la zone RUN */
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE,
              "%s: Candidat '%s' loaded (%s)", __func__, dls->tech_id, dls->shortname );

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Reseter_all_bit_interne: Met a 0 et decharge tous les bits interne d'un plugin                                             */
/* Entrée: le plugin                                                                                                          */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static void Reseter_all_bit_interne ( struct DLS_PLUGIN *plugin )
  { GSList *liste_bit;

    pthread_mutex_lock( &Partage->com_dls.synchro_data );                                 /* Décharge tous les bits du module */
    liste_bit = Partage->Dls_data_TEMPO;
    while(liste_bit)
     { struct DLS_TEMPO *tempo = liste_bit->data;
       if (!strcasecmp(tempo->tech_id, plugin->tech_id))
        { tempo->status = DLS_TEMPO_NOT_COUNTING;                                           /* La tempo ne compte pas du tout */
          tempo->state  = FALSE;
          tempo->init   = FALSE;
        }
       liste_bit = g_slist_next(liste_bit);
     }

    liste_bit = Partage->Dls_data_MSG;                                                /* Decharge tous les messages du module */
    while(liste_bit)
     { struct DLS_MESSAGES *msg = liste_bit->data;
       if (!strcasecmp(msg->tech_id, plugin->tech_id))
        { Dls_data_set_MSG ( &plugin->vars, msg->tech_id, msg->acronyme, (gpointer *)&msg, FALSE, FALSE ); }
       liste_bit = g_slist_next(liste_bit);
     }

    liste_bit = Partage->Dls_data_MONO;                                               /* Decharge tous les monoeens du module */
    while(liste_bit)
     { struct DLS_MONO *mono = liste_bit->data;
       if (!strcasecmp(mono->tech_id, plugin->tech_id) && strcasecmp(mono->acronyme, "IO_COMM") )
        { Dls_data_set_MONO ( &plugin->vars, mono->tech_id, mono->acronyme, (gpointer)&mono, FALSE ); }
       liste_bit = g_slist_next(liste_bit);
     }

    liste_bit = Partage->Dls_data_BI;                                               /* Decharge tous les bieens du module */
    while(liste_bit)
     { struct DLS_BI *bi = liste_bit->data;
       if (!strcasecmp(bi->tech_id, plugin->tech_id) && strcasecmp(bi->acronyme, "IO_COMM") )
        { Dls_data_set_BI   ( &plugin->vars, bi->tech_id, bi->acronyme, (gpointer)&bi, FALSE ); }
       liste_bit = g_slist_next(liste_bit);
     }

    liste_bit = Partage->Dls_data_WATCHDOG;                                          /* Decharge tous les watchdogs du module */
    while(liste_bit)
     { struct DLS_WATCHDOG *wtd = liste_bit->data;
       if (!strcasecmp(wtd->tech_id, plugin->tech_id) && strcasecmp(wtd->acronyme, "IO_COMM") )
        { Dls_data_set_WATCHDOG ( &plugin->vars, wtd->tech_id, wtd->acronyme, (gpointer *)&wtd, FALSE ); }
       liste_bit = g_slist_next(liste_bit);
     }

    liste_bit = Partage->Dls_data_VISUEL;                                              /* Decharge tous les visuels du module */
    while(liste_bit)
     { struct DLS_VISUEL *visu = liste_bit->data;
       if (!strcasecmp(visu->tech_id, plugin->tech_id))
        { Dls_data_set_VISUEL ( &plugin->vars, visu->tech_id, visu->acronyme, (gpointer *)&visu, 0, "black", FALSE, "resetted" ); }
       liste_bit = g_slist_next(liste_bit);
     }

    pthread_mutex_unlock( &Partage->com_dls.synchro_data );
  }
/******************************************************************************************************************************/
/* Decharger_plugins: Decharge tous les plugins DLS                                                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static void Dls_Reseter_plugin_dls_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = user_data;
    if ( ! strcasecmp(plugin->tech_id, tech_id ) )
     { Reseter_all_bit_interne ( plugin );
       Dls_Charger_un_plugin ( plugin );
       Charger_confDB_Registre ( plugin->tech_id );                           /* Chargement conf a faire apres la compilation */
       plugin->vars.resetted = TRUE;                                               /* au chargement, le bit de start vaut 1 ! */
       return;
     }
  }
/******************************************************************************************************************************/
/* Retirer_plugins: Decharge toutes les librairies                                                                            */
/* Entrée: Le numéro du plugin a décharger                                                                                    */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_Reseter_un_plugin ( gchar *tech_id )
  { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: Reset plugin '%s' en cours", __func__, tech_id );
    Dls_foreach_plugins ( tech_id, Dls_Reseter_plugin_dls_reel );
  }
/******************************************************************************************************************************/
/* Decharger_plugins: Decharge tous les plugins DLS                                                                           */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_Decharger_plugins ( void )
  { pthread_mutex_lock( &Partage->com_dls.synchro );
    while(Partage->com_dls.Dls_plugins)                                                     /* Liberation mémoire des modules */
     { struct DLS_PLUGIN *plugin = Partage->com_dls.Dls_plugins->data;
       if (plugin->handle && dlclose( plugin->handle ))
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: dlclose error '%s' for '%s' (%s)", __func__,
                    dlerror(), plugin->tech_id, plugin->shortname );
        }
       Partage->com_dls.Dls_plugins = g_slist_remove( Partage->com_dls.Dls_plugins, plugin );
       if (plugin->Arbre_Comm) g_slist_free(plugin->Arbre_Comm);
                                                                             /* Destruction de l'entete associé dans la GList */
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: plugin '%s' unloaded (%s)", __func__,
                 plugin->tech_id, plugin->name );
       g_free( plugin );
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Dls_Add_plugin_to_dls_syn: Ajoute les dependances du syn en parametre                                                      */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static void Dls_Add_plugin_to_dls_syn ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { struct DLS_SYN *dls_syn = user_data;
    if (plugin->syn_id == dls_syn->syn_id)
     { dls_syn->Dls_plugins = g_slist_append( dls_syn->Dls_plugins, plugin ); }
  }
/******************************************************************************************************************************/
/* Dls_recalculer_arbre_syn_for_id: Calcule l'arbre de dependance des synooptiques                                            */
/* Entrée: l'id du synoptique a traiter, a travers le json array                                                              */
/* Sortie: L'arbre DLS est mis à jour                                                                                         */
/******************************************************************************************************************************/
 static void Dls_recalculer_arbre_syn_for_childs ( JsonArray *ids, guint index, JsonNode *element, gpointer data )
  { struct DLS_SYN *dls_syn_parent = data;
    gint syn_id;
    if (!dls_syn_parent) syn_id = 1;
    else syn_id = Json_get_int ( element, "syn_id" );

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Starting for syn_id '%d'", __func__, syn_id );
    struct DLS_SYN *dls_syn = g_try_malloc0( sizeof(struct DLS_SYN) );
    if (!dls_syn)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory Error for syn_id '%s'", __func__, syn_id );
       return;
     }
    dls_syn->syn_id = syn_id;

    Dls_foreach_plugins ( dls_syn, Dls_Add_plugin_to_dls_syn );

    JsonNode *syn_enfants = Json_node_create ();
    if (!syn_enfants)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory Error for syn_id '%s'", __func__, syn_id );
       g_free(dls_syn);
       return;
     }

    SQL_Select_to_json_node ( syn_enfants, "enfants",
                              "SELECT syn_id FROM syns "
                              "WHERE parent_id='%d' AND syn_id!=1",                    /* Pas de bouclage sur le synoptique 1 */
                              syn_id );

    Json_node_foreach_array_element ( syn_enfants, "enfants", Dls_recalculer_arbre_syn_for_childs, dls_syn );
    if (dls_syn_parent)
     { dls_syn_parent->Dls_sub_syns = g_slist_append ( dls_syn_parent->Dls_sub_syns, dls_syn ); }
    else
     { Partage->com_dls.Dls_syns = dls_syn; }
    Json_node_unref(syn_enfants);
  }
/******************************************************************************************************************************/
/* Dls_arbre_dls_syn_erase_syn: Supprime le dls_syn en parametre de l'arbre des synoptique                                    */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 static void Dls_arbre_dls_syn_erase_syn ( struct DLS_SYN *dls_syn )
  { if(dls_syn->Dls_plugins) g_slist_free( dls_syn->Dls_plugins );
    while (dls_syn->Dls_sub_syns)
     { struct DLS_SYN *sub_syn = dls_syn->Dls_sub_syns->data;
       dls_syn->Dls_sub_syns = g_slist_remove ( dls_syn->Dls_sub_syns, sub_syn );
       Dls_arbre_dls_syn_erase_syn ( sub_syn );
     }
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Arbre syn '%d' erased", __func__, dls_syn->syn_id );
    g_free(dls_syn);
  }
/******************************************************************************************************************************/
/* Dls_arbre_dls_syn_erase: Efface l'arbre des synoptiques                                                                    */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_arbre_dls_syn_erase ( void )
  { if (Partage->com_dls.Dls_syns) Dls_arbre_dls_syn_erase_syn(Partage->com_dls.Dls_syns);
    Partage->com_dls.Dls_syns = NULL;
    Partage->com_dls.Dls_syns = NULL;
  }
/******************************************************************************************************************************/
/* Dls_recalculer_arbre_dls_syn: Calcule l'arbre des synoptiques                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_recalculer_arbre_dls_syn ( void )
  { Dls_arbre_dls_syn_erase();
    Dls_recalculer_arbre_syn_for_childs (NULL, 0, NULL, NULL);
  }
/******************************************************************************************************************************/
/* Dls_Plugin_load_by_array: Importe un plugin depius l'API dans la liste des plugins                                         */
/* Entrée: les données JSON recu de la requete HTTP                                                                           */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Plugin_load_by_array (JsonArray *array, guint index, JsonNode *element, gpointer user_data)
  { gchar *tech_id = Json_get_string ( element, "tech_id" );
    struct DLS_PLUGIN *dls = g_try_malloc0 ( sizeof (struct DLS_PLUGIN ) );
    if (!dls)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: '%s' Memory error", __func__, tech_id );
       return;
     }

    g_snprintf ( dls->tech_id,   sizeof(dls->tech_id),   "%s", tech_id );
    g_snprintf ( dls->name,       sizeof(dls->name),       "%s", Json_get_string ( element, "name" ) );
    g_snprintf ( dls->shortname, sizeof(dls->shortname), "%s", Json_get_string ( element, "shortname" ) );
    dls->debug = Json_get_bool ( element, "debug" );
    dls->on    = Json_get_bool ( element, "enable" );

    pthread_mutex_lock( &Partage->com_dls.synchro );
    Dls_Charger_un_plugin ( dls );
    Partage->com_dls.Dls_plugins = g_slist_append( Partage->com_dls.Dls_plugins, dls );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Dls_Importer_plugins: Importe tous les plugins depuis l'API                                                                */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_Importer_plugins ( void )
  { JsonNode *api_result = Http_Post_to_global_API ( "/run/dls/plugins", NULL );
    if (api_result == NULL || Json_get_int ( api_result, "api_status" ) != SOUP_STATUS_OK)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: API Request for DLS PLUGIN failed. No plugin loaded.", __func__ );
       return;
     }
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_INFO, "%s: API Request for DLS PLUGINS OK.", __func__ );
    Json_node_foreach_array_element ( api_result, "plugins", Dls_Plugin_load_by_array, NULL );
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: %03d plugins loaded", __func__,
              Json_get_int ( api_result, "nbr_plugins" ) );
    Json_node_unref ( api_result );
  }
/******************************************************************************************************************************/
/* Dls_Debug_plugin_reel: Active le debug d'un plugin                                                                         */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Debug_plugin_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = (gchar *)user_data;
    if ( ! strcasecmp ( plugin->tech_id, tech_id ) )
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: '%s' debug started ('%s')", __func__,
                 plugin->tech_id, plugin->name );
       plugin->debug = plugin->vars.debug = TRUE;
     }
  }
/******************************************************************************************************************************/
/* Dls_Undebug_plugin_reel: Désactive le debug d'un plugin                                                                    */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Undebug_plugin_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = (gchar *)user_data;
    if ( ! strcasecmp ( plugin->tech_id, tech_id ) )
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: '%s' debug stopped ('%s')", __func__,
                 plugin->tech_id, plugin->name );
       plugin->debug = plugin->vars.debug = FALSE;
     }
  }
/******************************************************************************************************************************/
/* Dls_Debug_plugin: Active ou non le debug d'un plugin                                                                       */
/* Entrée: le tech_id et le choix actif ou non                                                                                */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_Debug_plugin ( gchar *tech_id, gboolean actif )
  { if (actif) Dls_foreach_plugins ( tech_id, Dls_Debug_plugin_reel );
          else Dls_foreach_plugins ( tech_id, Dls_Undebug_plugin_reel );
  }
/******************************************************************************************************************************/
/* Dls_acquitter_plugin_reel: Acquitte le synoptique si il est en parametre                                                   */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_acquitter_plugin_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = user_data;
    if ( ! strcasecmp ( plugin->tech_id, tech_id ) )
     { Info_new( Config.log, plugin->vars.debug, LOG_NOTICE,
                 "%s: '%s' acquitté ('%s')", __func__, plugin->tech_id, plugin->shortname );
       Envoyer_commande_dls_data ( plugin->tech_id, "OSYN_ACQUIT" );
     }
  }
/******************************************************************************************************************************/
/* Activer_plugin_by_id: Active ou non un plugin by id                                                                        */
/* Entrée: l'ID du plugin                                                                                                     */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_acquitter_plugin ( gchar *tech_id )
  { Dls_foreach_plugins ( tech_id, Dls_acquitter_plugin_reel ); }
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_acquitter_synoptique_reel ( gpointer user_data, struct DLS_SYN *syn )
  { GSList *plugins = syn->Dls_plugins;
    gint syn_id = GPOINTER_TO_INT(user_data);
    if (syn_id != syn->syn_id) return;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       Info_new( Config.log, plugin->vars.debug, LOG_NOTICE,
                 "%s: '%s' acquitté ('%s')", __func__, plugin->tech_id, plugin->shortname );
       Envoyer_commande_dls_data ( plugin->tech_id, "OSYN_ACQUIT" );
       plugins=g_slist_next(plugins);
     }
  }
/******************************************************************************************************************************/
/* Activer_plugin_by_id: Active ou non un plugin by id                                                                        */
/* Entrée: l'ID du plugin                                                                                                     */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_acquitter_synoptique ( gint syn_id )
  { Dls_foreach_syns ( GINT_TO_POINTER(syn_id), Dls_acquitter_synoptique_reel ); }
/*----------------------------------------------------------------------------------------------------------------------------*/
