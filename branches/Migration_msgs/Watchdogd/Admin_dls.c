/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_dls.c        Gestion des responses Admin DLS au serveur watchdog                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

/******************************************************************************************************************************/
/* Admin_dls_reload: Demande le rechargement des conf DLS                                                                     */
/* Entr�e: la response                                                                                                       */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gchar *Admin_dls_reload ( gchar *response_src )
  { gchar *response = response_src;
    Partage->com_dls.Thread_reload = TRUE;
    while (Partage->com_dls.Thread_reload) sched_yield();
    return(Admin_write ( response, " | - DLS Reload done" ));
  }
/******************************************************************************************************************************/
/* Admin_dls_list_dls_tree: Print la liste des plugins dls actif ou non sur un dls_tree donn�                                 */
/* Entr�e: La response                                                                                                        */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gchar *Admin_dls_list_dls_tree ( gchar *response_src, gint dls_id, gint syn_id, struct DLS_TREE *dls_tree )
  { gchar *response = response_src;
    gboolean found = FALSE;
    gchar chaine[256];
    GSList *liste;

    liste = dls_tree->Liste_plugin_dls;
    while(liste)                                                                            /* Liberation m�moire des modules */
     { struct PLUGIN_DLS *dls;
       struct tm *temps;
       gchar date[80];
       dls = (struct PLUGIN_DLS *)liste->data;

       if ( (dls_id == -1 && dls->plugindb.syn_id == syn_id)                          /* Affichage uniquement pour le bon dls */
          || dls_id == dls->plugindb.id)
        { temps = localtime( (time_t *)&dls->start_date );
          if (temps) { strftime( date, sizeof(date), "%F %T", temps ); }
          else       { g_snprintf( date, sizeof(date), "Erreur" ); }
          found = TRUE;
          g_snprintf( chaine, sizeof(chaine),
                      " | - SYN[%05d] - DLS[%06d] -> started=%d, start_date=%s, conso=%08.03f, nom=%s",
                      dls_tree->syn_vars.syn_id, dls->plugindb.id, dls->plugindb.on, date, dls->conso, dls->plugindb.shortname );
          response = Admin_write ( response, chaine );
          g_snprintf( chaine, sizeof(chaine),
                      " |                               debug = %d, comm_out = %d, defaut = %d/%d, alarme = %d/%d\n"
                      " |                               veille = %d, alerte = %d/%d\n"
                      " |                               derangement = %d/%d, danger = %d/%d",
                      dls->vars.debug, dls->vars.bit_comm_out, dls->vars.bit_defaut, dls->vars.bit_defaut_fixe,
                      dls->vars.bit_alarme, dls->vars.bit_alarme_fixe,
                      dls->vars.bit_veille,
                      dls->vars.bit_alerte, dls->vars.bit_alerte_fixe,
                      dls->vars.bit_derangement,dls->vars.bit_derangement_fixe,
                      dls->vars.bit_danger, dls->vars.bit_danger_fixe );
          response = Admin_write ( response, chaine );
        }
       liste = liste->next;
     }
    if (found)
     { g_snprintf( chaine, sizeof(chaine),
                   " | - SYN[%05d] - comm_out = %d, defaut = %d/%d, alarme = %d/%d\n"
                   " |                veille = %d/%d, alerte = %d/%d\n"
                   " |                derangement = %d/%d, danger = %d/%d",
                      dls_tree->syn_vars.syn_id, dls_tree->syn_vars.bit_comm_out,
                      dls_tree->syn_vars.bit_defaut, dls_tree->syn_vars.bit_defaut_fixe,
                      dls_tree->syn_vars.bit_alarme, dls_tree->syn_vars.bit_alarme_fixe,
                      dls_tree->syn_vars.bit_veille_partielle, dls_tree->syn_vars.bit_veille_totale,
                      dls_tree->syn_vars.bit_alerte, dls_tree->syn_vars.bit_alerte_fixe,
                      dls_tree->syn_vars.bit_derangement, dls_tree->syn_vars.bit_derangement_fixe,
                      dls_tree->syn_vars.bit_danger, dls_tree->syn_vars.bit_danger_fixe );
          response = Admin_write ( response, chaine );
     }

    liste = dls_tree->Liste_dls_tree;
    while (liste)
     { struct DLS_TREE *sub_dls_tree = liste->data;
       response = Admin_dls_list_dls_tree ( response, dls_id, syn_id, sub_dls_tree );
       liste = liste->next;
     }
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_dls_list: Print la liste des plugins dls actif ou non, mais charg�s                                                  */
/* Entr�e: La response                                                                                                       */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gchar *Admin_dls_list_syn ( gchar *response_src, gint syn_id )
  { gchar *response = response_src;
    GSList *liste_dls;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " | -- Liste des modules D.L.S du synoptique %d", syn_id );
    response = Admin_write ( response, chaine );
     
    pthread_mutex_lock( &Partage->com_dls.synchro );
    response = Admin_dls_list_dls_tree ( response, -1, syn_id, Partage->com_dls.Dls_tree );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_dls_list: Print la liste des plugins dls actif ou non, mais charg�s                                                  */
/* Entr�e: La response                                                                                                       */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gchar *Admin_dls_show ( gchar *response_src, gint dls_id )
  { gchar *response = response_src;
    GSList *liste_dls;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " | -- Affichage du plugin D.L.S %d", dls_id );
    response = Admin_write ( response, chaine );
     
    pthread_mutex_lock( &Partage->com_dls.synchro );
    response = Admin_dls_list_dls_tree ( response, dls_id, -1, Partage->com_dls.Dls_tree );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_dls_gcc_dls_tree: Print la liste des plugins dls actif ou non sur un dls_tree donn�                                  */
/* Entr�e: La response                                                                                                        */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gchar *Admin_dls_gcc_dls_tree ( gchar *response_src, gint id, struct DLS_TREE *dls_tree )
  { gchar chaine[256], buffer[1024];
    gchar *response = response_src;
    GSList *liste;

    response = response_src;
    if(id==-1)
     { liste = dls_tree->Liste_plugin_dls;
       while(liste)                                                                         /* Liberation m�moire des modules */
        { struct PLUGIN_DLS *dls;
          dls = (struct PLUGIN_DLS *)liste->data;

          Compiler_source_dls ( FALSE, dls->plugindb.id, buffer, sizeof(buffer) );
          g_snprintf( chaine, sizeof(chaine), " | - Compilation du DLS[%06d] done (no reset): %s", dls->plugindb.id, buffer );
          response = Admin_write ( response, chaine );
          liste = liste->next;
        }

       liste = dls_tree->Liste_dls_tree;
       while (liste)
        { struct DLS_TREE *sub_dls_tree = liste->data;
          response = Admin_dls_gcc_dls_tree ( response, id, sub_dls_tree );
          liste = liste->next;
        }
     }
    else
     { Compiler_source_dls ( FALSE, id, buffer, sizeof(buffer) );
       g_snprintf( chaine, sizeof(chaine), " | - Compilation du DLS[%06d] done (no reset): %s", id, buffer );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_dls_gcc: compile le plugin dont l'id est en parametre                                                                */
/* Entr�e: Le buffer a compl�ter, l'id du plugin                                                                              */
/* Sortie: Le buffer compl�t�                                                                                                 */
/******************************************************************************************************************************/
 static gchar *Admin_dls_gcc ( gchar *response_src, gint id )
  { gchar *response;
    gchar chaine[256];
    
    g_snprintf( chaine, sizeof(chaine), " | -- Compilation des plugins D.L.S" );
    response = Admin_write ( response_src, chaine );

    pthread_mutex_lock( &Partage->com_dls.synchro );                                                         /* Lock du mutex */
    response = Admin_dls_gcc_dls_tree( response, id, Partage->com_dls.Dls_tree );
    pthread_mutex_unlock( &Partage->com_dls.synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_dls_start: Demarre un plugin DLS                                                                                     */
/* Entr�e: Le buffer a compl�ter, l'id du plugin                                                                              */
/* Sortie: Le buffer compl�t�                                                                                                 */
/******************************************************************************************************************************/
 static gchar *Admin_dls_start ( gchar *response_src, gint id )
  { gchar chaine[128], requete[128];
    gchar *response = response_src;
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " | -- Demarrage d'un plugin D.L.S" );
    response = Admin_write ( response, chaine );

    while (Partage->com_dls.admin_start) sched_yield();
    Partage->com_dls.admin_start = id;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: impossible d'ouvrir la Base de donn�es %s", __func__,
                 Config.db_database );
       return(response);
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=1 WHERE id=%d", NOM_TABLE_DLS, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(response);
     }
    Libere_DB_SQL( &db );

    g_snprintf( chaine, sizeof(chaine), " | - Module DLS %d started", id );
    return(Admin_write ( response, chaine ));
  }
/******************************************************************************************************************************/
/* Admin_dls_debug: Active ou non le debug du plugin                                                                          */
/* Entr�e: La response, le num�ro du plugin, et le statut du debug                                                           */
/* Sortie: n�ant                                                                                                              */
/******************************************************************************************************************************/
 static void Admin_dls_debug_dls_tree ( gint id, gboolean debug, struct DLS_TREE *dls_tree )
  { GSList *liste;
    liste = dls_tree->Liste_plugin_dls;
    while(liste)                                                                /* On execute tous les modules un par un */
     { struct PLUGIN_DLS *plugin_actuel;
       plugin_actuel = (struct PLUGIN_DLS *)liste->data;

       if (plugin_actuel->plugindb.id == id)
        { plugin_actuel->vars.debug = debug;
          return;
        }
       liste = liste->next;
     }
    liste = dls_tree->Liste_dls_tree;
    while (liste)
     { Admin_dls_debug_dls_tree ( id, debug, liste->data );
       liste = liste->next;
     }
 }
/******************************************************************************************************************************/
/* Admin_dls_debug: Active ou non le debug du plugin                                                                          */
/* Entr�e: La response, le num�ro du plugin, et le statut du debug                                                           */
/* Sortie: n�ant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_dls_debug ( gchar *response_src, gint id, gboolean debug )
  { gchar *response = response_src;
    gchar chaine[128];
    GSList *liste_dls;
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " | -- Modification du statut de debug d'un plugin D.L.S" );
    response = Admin_write ( response, chaine );

    pthread_mutex_lock( &Partage->com_dls.synchro );                                                         /* Lock du mutex */
    Admin_dls_debug_dls_tree ( id, debug, Partage->com_dls.Dls_tree );
    pthread_mutex_unlock( &Partage->com_dls.synchro );

    g_snprintf( chaine, sizeof(chaine), " | - Module DLS: debug set to '%d'", debug );
    return(Admin_write ( response, chaine ));
  }
/******************************************************************************************************************************/
/* Admin_dls_stop: Arrete un plugin DLS                                                                                       */
/* Entr�e: Le buffer a compl�ter, l'id du plugin                                                                              */
/* Sortie: Le buffer compl�t�                                                                                                 */
/******************************************************************************************************************************/
 static gchar *Admin_dls_stop ( gchar *response_src, gint id )
  { gchar *response = response_src;
    gchar chaine[128], requete[128];
    struct DB *db;

    g_snprintf( chaine, sizeof(chaine), " | -- Arret d'un plugin D.L.S" );
    response = Admin_write ( response, chaine );

    while (Partage->com_dls.admin_stop) sched_yield();
    Partage->com_dls.admin_stop = id;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING, "%s: impossible d'ouvrir la Base de donn�es %s", __func__,
                 Config.db_database );
       return(response);
     }

    g_snprintf( requete, sizeof(requete), "UPDATE %s SET actif=0 WHERE id=%d", NOM_TABLE_DLS, id );

    if ( Lancer_requete_SQL ( db, requete ) == FALSE )
     { Libere_DB_SQL( &db );
       return(response);
     }
    Libere_DB_SQL( &db );

    g_snprintf( chaine, sizeof(chaine), " | - Module DLS %d stopped", id );
    return(Admin_write ( response, chaine ));
  }
/******************************************************************************************************************************/
/* Admin_dls: Appell�e lorsque l'admin envoie une commande en mode dls dans la ligne de commande                              */
/* Entr�e: Le buffer a compl�ter, l'id du plugin                                                                              */
/* Sortie: Le buffer compl�t�                                                                                                 */
/******************************************************************************************************************************/
 gchar *Admin_dls ( gchar *response_src, gchar *ligne )
  { gchar *response = response_src;
    gchar commande[128];

    sscanf ( ligne, "%s", commande );                                                    /* D�coupage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* D�coupage de la ligne de commande */
       return(Admin_dls_start ( response, num ));
     }
    else if ( ! strcmp ( commande, "gcc" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* D�coupage de la ligne de commande */
       return(Admin_dls_gcc ( response, num ));
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* D�coupage de la ligne de commande */
       return(Admin_dls_stop ( response, num ));
     }
    else if ( ! strcmp ( commande, "debug" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* D�coupage de la ligne de commande */
       return(Admin_dls_debug ( response, num, TRUE ));
     }
    else if ( ! strcmp ( commande, "nodebug" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* D�coupage de la ligne de commande */
       return(Admin_dls_debug ( response, num, FALSE ));
     }
    else if ( ! strcmp ( commande, "reset" ) )
     { gchar chaine[64];
       int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* D�coupage de la ligne de commande */
       Reseter_un_plugin ( num );
       g_snprintf( chaine, sizeof(chaine), " | - Module DLS: Plugin %d resetted", num );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* D�coupage de la ligne de commande */
       return(Admin_dls_list_syn ( response, num ));
     }
    else if ( ! strcmp ( commande, "show" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* D�coupage de la ligne de commande */
       return(Admin_dls_show ( response, num ));
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { return(Admin_dls_reload( response ));
     }
    else if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'D.L.S'" );
       response = Admin_write ( response, "  debug $dls_id                          - Active le mode debug du plugin $id" );
       response = Admin_write ( response, "  nodebug $dls_id                        - Desactive le mode debug du plugin $id" );
       response = Admin_write ( response, "  start $dls_id                          - Demarre le module $id" );
       response = Admin_write ( response, "  stop $dls_id                           - Stop le module $id" );
       response = Admin_write ( response, "  reset $dls_id                          - Stop/Unload/Load/Start module $id" );
       response = Admin_write ( response, "  list $syn_id                           - Liste tous les DLS du synoptique $syn_id" );
       response = Admin_write ( response, "  show $dls_id                           - Affiche le status du D.L.S num�ro $dls_id" );
       response = Admin_write ( response, "  gcc $dls_id                            - Compile le plugin $id (-1 for all)" );
       response = Admin_write ( response, "  reload                                 - Recharge la configuration" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown DLS command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
