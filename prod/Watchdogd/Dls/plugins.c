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
    g_snprintf( chaine, sizeof(chaine), "SELECT DISTINCT(map_tech_id) FROM mnemos_DI "
                                        "INNER JOIN thread_classe AS tc ON map_thread = tc.thread "
                                        "WHERE tech_id='%s' AND tc.classe='I/O'", dls->tech_id );
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
              plugin->tech_id, plugin->nom );
    SQL_Write_new ( "UPDATE dls SET actif='0' WHERE tech_id = '%s'", plugin->tech_id );
  }
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_start_plugin_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = user_data;
    if ( strcasecmp ( tech_id, plugin->tech_id ) ) return;
    if ( plugin->compil_status < DLS_COMPIL_OK )
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE,
		         "%s: '%s' (%s) could not be started because of CompilStatus Not OK", __func__,
                 plugin->tech_id, plugin->nom );
       return;
     }

    plugin->on = TRUE;
    plugin->conso = 0.0;
    plugin->start_date = time(NULL);
    plugin->vars.resetted = FALSE;
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: '%s' started (%s)", __func__, plugin->tech_id, plugin->nom );
    SQL_Write_new ( "UPDATE dls SET actif='1' WHERE tech_id = '%s'", plugin->tech_id );
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
/* Proto_compiler_source_dls: Compilation de la source DLS                                                                    */
/* Entrée: Le tech_id du DLS a compiler                                                                                       */
/* Sortie: code de compilation DLS_COMPIL_xxx                                                                                 */
/******************************************************************************************************************************/
 static gint Compiler_source_dls( gchar *tech_id )
  { gint retour, pidgcc, id_fichier;
    gchar log_buffer[1024], log_file[64];
    gchar chaine[128];
    gint taille_source;
    gchar *Source;

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: Compilation module DLS '%s'", __func__, tech_id );

    if ( Get_source_dls_from_DB ( tech_id, &Source, &taille_source ) == FALSE )         /* On récupère le source depuis la DB */
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Cannot get Source DLS for id '%s'", __func__, tech_id );
       return(DLS_COMPIL_EXPORT_DB_FAILED);
     }

    g_snprintf( chaine, sizeof(chaine), "Dls/%s.dls", tech_id );
    unlink(chaine);
    id_fichier = open( chaine, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    if (id_fichier<0 || lockf( id_fichier, F_TLOCK, 0 ) )
     { Info_new( Config.log, Config.log_trad, LOG_WARNING, "%s: Open file '%s' for write failed (%s)", __func__,
                 chaine, strerror(errno) );
       close(id_fichier);
       return(DLS_COMPIL_EXPORT_DB_FAILED);
     }
    else
     { if (write( id_fichier, Source, taille_source )<0)
        { Info_new( Config.log, Config.log_trad, LOG_ERR, "%s: Write %d bytes to file '%s' failed (%s)", __func__,
                    taille_source, chaine, strerror(errno) );
         close(id_fichier);
         return(DLS_COMPIL_EXPORT_DB_FAILED);
        }
       else
        { Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: Write %d bytes to file '%s' OK", __func__,
                    taille_source, chaine);
          close(id_fichier);
        }
     }

    retour = Traduire_DLS( tech_id );                                                                       /* Traduction DLS */
    Info_new( Config.log, Config.log_trad, LOG_DEBUG,
             "%s: fin traduction '%s'. Code retour '%d'", __func__, tech_id, retour );

    if (retour == TRAD_DLS_ERROR_NO_FILE)                                             /* Retour de la traduction D.L.S vers C */
     { Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: envoi erreur file Traduction D.L.S '%s'", tech_id );
       Set_compil_status_plugin_dlsDB( tech_id, DLS_COMPIL_ERROR_LOAD_SOURCE, "Erreur chargement source" );
       return( DLS_COMPIL_ERROR_LOAD_SOURCE );
     }

    memset ( log_buffer, 0, sizeof(log_buffer) );                            /* Chargement de fichier de log dans la database */

    Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: Chargement du fichier de log D.L.S '%s'", __func__, tech_id );
    g_snprintf( log_file, sizeof(log_file), "Dls/%s.log", tech_id );

    id_fichier = open( log_file, O_RDONLY, 0 );              /* Ouverture du fichier log et chargement du contenu dans buffer */
    if (id_fichier<0)
     { Info_new( Config.log, Config.log_trad, LOG_ERR,
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
     }

    if ( retour == TRAD_DLS_SYNTAX_ERROR )
     { Set_compil_status_plugin_dlsDB( tech_id, DLS_COMPIL_SYNTAX_ERROR, log_buffer );
       return ( DLS_COMPIL_SYNTAX_ERROR );
     }

    pidgcc = fork();
    if (pidgcc<0)
     { Info_new( Config.log, Config.log_trad, LOG_WARNING, "%s_Fils: envoi erreur Fork GCC '%s'", __func__, tech_id );
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

    Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: Waiting for gcc to finish pid %d", __func__, pidgcc );
    wait4(pidgcc, NULL, 0, NULL );
    Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: gcc is down, OK %d", __func__, pidgcc );

    Info_new( Config.log, Config.log_trad, LOG_DEBUG, "%s: end of '%s'", __func__, tech_id );
    if (retour == TRAD_DLS_WARNING)
     { Set_compil_status_plugin_dlsDB( tech_id, DLS_COMPIL_OK_WITH_WARNINGS, log_buffer );
       return( DLS_COMPIL_OK_WITH_WARNINGS );
     }

    Set_compil_status_plugin_dlsDB( tech_id, DLS_COMPIL_OK, "Pas d'erreur !" );
    return( DLS_COMPIL_OK );
  }
/******************************************************************************************************************************/
/* Charger_un_plugin_par_nom: Ouverture d'un plugin dont le nom est en parametre                                              */
/* Entrée: Le plugin D.L.S                                                                                                    */
/* Sortie: FALSE si problème                                                                                                  */
/******************************************************************************************************************************/
 static gboolean Dls_Charger_un_plugin ( struct DLS_PLUGIN *dls, gboolean compil )
  { gchar nom_fichier_absolu[60];

    if (compil)
     { dls->compil_status = Compiler_source_dls( dls->tech_id );
       if (dls->compil_status<DLS_COMPIL_OK)
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                   "%s: Candidat '%s' CompilStatus failed. Keeping old .so, if it exists.", __func__, dls->tech_id );
        }
     }

    g_snprintf( nom_fichier_absolu, sizeof(nom_fichier_absolu), "Dls/libdls%s.so", dls->tech_id );
    strncpy( dls->nom_fichier, nom_fichier_absolu, sizeof(dls->nom_fichier) );                 /* Init des variables communes */

    dls->handle = dlopen( nom_fichier_absolu, RTLD_LOCAL | RTLD_NOW );                      /* Ouverture du fichier librairie */
    if (!dls->handle)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                   "%s: Candidat '%s' dlopen failed (%s)", __func__, dls->tech_id, dlerror() );
       Dls_stop_plugin_reel ( dls->tech_id, dls );
       return(FALSE);
     }

    dls->go = dlsym( dls->handle, "Go" );                                                    /* Recherche de la fonction 'Go' */
    if (!dls->go)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                 "%s: Candidat '%s' failed sur absence GO", __func__, dls->tech_id );
       Dls_stop_plugin_reel ( dls->tech_id, dls );
       dlclose( dls->handle );
       dls->handle = NULL;
       return(FALSE);
     }
    dls->version = dlsym( dls->handle, "version" );                                            /* Recherche de la fonction */
    if (!dls->version)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_WARNING,
                "%s: Candidat '%s' does not provide version function", __func__, dls->tech_id );
       Dls_stop_plugin_reel ( dls->tech_id, dls );
       dlclose( dls->handle );
       dls->handle = NULL;
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
       liste_bit = g_slist_next(liste_bit);
       if (!strcasecmp(msg->tech_id, plugin->tech_id))
        { Dls_data_set_MSG ( &plugin->vars, msg->tech_id, msg->acronyme, (gpointer *)&msg, FALSE, FALSE ); }
     }

    liste_bit = Partage->Dls_data_BOOL;                                               /* Decharge tous les booleens du module */
    while(liste_bit)
     { struct DLS_BOOL *bool = liste_bit->data;
       liste_bit = g_slist_next(liste_bit);
       if (!strcasecmp(bool->tech_id, plugin->tech_id) && strcasecmp(bool->acronyme, "IO_COMM") )
        {      if (bool->classe == MNEMO_BISTABLE)
           { Dls_data_set_BI   ( &plugin->vars, bool->tech_id, bool->acronyme, (gpointer)&bool, FALSE ); }
          else if (bool->classe == MNEMO_MONOSTABLE)
           { Dls_data_set_MONO ( &plugin->vars, bool->tech_id, bool->acronyme, (gpointer)&bool, FALSE ); }
        }
     }

    liste_bit = Partage->Dls_data_WATCHDOG;                                          /* Decharge tous les watchdogs du module */
    while(liste_bit)
     { struct DLS_WATCHDOG *wtd = liste_bit->data;
       liste_bit = g_slist_next(liste_bit);
       if (!strcasecmp(wtd->tech_id, plugin->tech_id))
        { Dls_data_set_WATCHDOG ( &plugin->vars, wtd->tech_id, wtd->acronyme, (gpointer *)&wtd, FALSE ); }
     }

    liste_bit = Partage->Dls_data_VISUEL;                                              /* Decharge tous les visuels du module */
    while(liste_bit)
     { struct DLS_VISUEL *visu = liste_bit->data;
       liste_bit = g_slist_next(liste_bit);
       if (!strcasecmp(visu->tech_id, plugin->tech_id))
        { Dls_data_set_VISUEL ( &plugin->vars, visu->tech_id, visu->acronyme, (gpointer *)&visu, 0, "black", FALSE, "resetted" ); }
     }

    liste_bit = Partage->Dls_data_DO;                                            /* Decharge toutes les sorties TOR du module */
    while(liste_bit)
     { struct DLS_DO *dout = liste_bit->data;
       liste_bit = g_slist_next(liste_bit);
       if (!strcasecmp(dout->tech_id, plugin->tech_id))
        { Dls_data_set_DO ( &plugin->vars, dout->tech_id, dout->acronyme, (gpointer *)&dout, FALSE ); }
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
       if (plugin->handle)                                   /* Peut etre à 0 si changement de librairie et erreur de link */
        { if (dlclose( plugin->handle ))
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: dlclose error '%s' for '%s' (%s)", __func__,
                       dlerror(), plugin->tech_id, plugin->shortname );
           }
          plugin->handle = NULL;
          Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: plugin '%s' (%s) unloaded", __func__,
                    plugin->tech_id, plugin->shortname );
        }
       Dls_Charger_un_plugin ( plugin, TRUE );
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
                 plugin->tech_id, plugin->nom );
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
    gint id;
    if (!dls_syn_parent) id = 1;
    else id = Json_get_int ( element, "id" );

    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Starting for syn id '%d'", __func__, id );
    struct DLS_SYN *dls_syn = g_try_malloc0( sizeof(struct DLS_SYN) );
    if (!dls_syn)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory Error for id '%s'", __func__, id );
       return;
     }
    dls_syn->syn_id = id;

    Dls_foreach_plugins ( dls_syn, Dls_Add_plugin_to_dls_syn );

    JsonNode *syn_enfants = Json_node_create ();
    if (!syn_enfants)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory Error for id '%s'", __func__, id );
       g_free(dls_syn);
       return;
     }

    SQL_Select_to_json_node ( syn_enfants, "enfants",
                              "SELECT id FROM syns "
                              "WHERE parent_id='%d' AND id!=1",                        /* Pas de bouclage sur le synoptique 1 */
                              id );

    Json_node_foreach_array_element ( syn_enfants, "enfants", Dls_recalculer_arbre_syn_for_childs, dls_syn );
    if (dls_syn_parent)
     { dls_syn_parent->Dls_sub_syns = g_slist_append ( dls_syn_parent->Dls_sub_syns, dls_syn ); }
    else
     { Partage->com_dls.Dls_syns = dls_syn; }
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
    Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: Arbre syn '%d' erased", dls_syn->syn_id );
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
/* Dls_Charger_plugins: Ouverture de toutes les librairies possibles pour le DLS                                              */
/* Entrée: Rien                                                                                                               */
/* Sortie: Rien                                                                                                               */
/******************************************************************************************************************************/
 void Dls_Charger_plugins ( gboolean compil )
  { struct DB *db;
    pthread_mutex_lock( &Partage->com_dls.synchro );
    if ( Recuperer_plugins_dlsDB ( &db ) )
     { struct DLS_PLUGIN *dls;
       while ( (dls = Recuperer_plugins_dlsDB_suite( &db )) != NULL )
        { Dls_Charger_un_plugin( dls, compil );                                                       /* Chargement du plugin */
          Partage->com_dls.Dls_plugins = g_slist_append( Partage->com_dls.Dls_plugins, dls );
        }
     }
    pthread_mutex_unlock( &Partage->com_dls.synchro );
  }
/******************************************************************************************************************************/
/* Dls_Debug_plugin_reel: Active le debug d'un plugin                                                                         */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dls_Debug_plugin_reel ( gpointer user_data, struct DLS_PLUGIN *plugin )
  { gchar *tech_id = (gchar *)user_data;
    if ( ! strcasecmp ( plugin->tech_id, tech_id ) )
     { gchar chaine[128];
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: '%s' debug started ('%s')", __func__,
                 plugin->tech_id, plugin->nom );
       plugin->debug = plugin->vars.debug = TRUE;
       g_snprintf(chaine, sizeof(chaine), "UPDATE dls SET debug='1' WHERE tech_id = '%s'", plugin->tech_id );
       SQL_Write ( chaine );
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
     { gchar chaine[128];
       Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_DEBUG, "%s: '%s' debug stopped ('%s')", __func__,
                 plugin->tech_id, plugin->nom );
       plugin->debug = plugin->vars.debug = FALSE;
       g_snprintf(chaine, sizeof(chaine), "UPDATE dls SET debug='0' WHERE tech_id = '%s'", plugin->tech_id );
       SQL_Write ( chaine );
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
