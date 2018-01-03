/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_rs485.c        Gestion des responses Admin RS485 au serveur watchdog                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_rs485.c
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
 #include "Rs485.h"

/******************************************************************************************************************************/
/* Admin_rs485_reload: Demande le rechargement des conf RS485                                                                 */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_rs485_reload ( gchar *response )
  { if (Cfg_rs485.lib->Thread_run == FALSE)
     { response = response = Admin_write ( response, " Thread RS485 is not running" );
       return(response);
     }
    
    Cfg_rs485.reload = TRUE;
    while (Cfg_rs485.reload) sched_yield();
    response = Admin_write ( response, " RS485 Reload done" );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_rs485_print : Affiche en details les infos d'un module RS495 en parametre                                            */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_rs485_print ( gchar *response, struct MODULE_RS485 *module )
  { gchar chaine[1024];
    g_snprintf( chaine, sizeof(chaine),
                " RS485[%02d] ------> num=%02d, requete = %03ds ago\n"
                "  | - enable = %d, started = %d, bit_comm = B%04d(=%d)\n"
                "  | - EA = %03d-%03d, E = %03d-%03d, S = %03d-%03d, SA = %03d-%03d\n"
                "  | - next_get_ana=in %03ds, nbr_deconnect=%02d",
                module->rs485.id, module->rs485.num, (Partage->top - module->date_requete)/10,
                module->rs485.enable, module->started, module->rs485.bit_comm, B(module->rs485.bit_comm),
                module->rs485.ea_min, module->rs485.ea_max,
                module->rs485.e_min, module->rs485.e_max,
                module->rs485.s_min, module->rs485.s_max,
                module->rs485.sa_min, module->rs485.sa_max,
                (module->date_next_get_ana > Partage->top ? (module->date_next_get_ana - Partage->top)/10 : -1),
                module->nbr_deconnect
              );
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_rs485_list: Affichage de toutes les infos opérationnelles de tous les modules rs485                                  */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_rs485_list ( gchar *response )
  { GSList *liste_modules;

    pthread_mutex_lock ( &Cfg_rs485.lib->synchro );
    liste_modules = Cfg_rs485.Modules_RS485;
    while ( liste_modules )
     { struct MODULE_RS485 *module;
       module = (struct MODULE_RS485 *)liste_modules->data;
       response = Admin_rs485_print ( response, module );
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_rs485.lib->synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_rs485_show: Affichage des infos opérationnelles liées au module en parametre                                         */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_rs485_show ( gchar *response, gint num )
  { GSList *liste_modules;

    pthread_mutex_lock ( &Cfg_rs485.lib->synchro );
    liste_modules = Cfg_rs485.Modules_RS485;
    while ( liste_modules )
     { struct MODULE_RS485 *module;
       module = (struct MODULE_RS485 *)liste_modules->data;
       if (module->rs485.id == num) { response = Admin_rs485_print ( response, module ); break; }
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_rs485.lib->synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_rs485_del: Retire le capteur/module rs485 dont l'id est en parametre                                                 */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_rs485_del ( gchar *response, gint id )
  { struct MODULE_RS485 *module;
    gchar chaine[128];

    module = Chercher_module_rs485_by_id ( id );
    if (!module)
     { g_snprintf( chaine, sizeof(chaine), " | - Module not found." ); }
    else if (module->started)
     { g_snprintf( chaine, sizeof(chaine), " | - Module %02d is not stopped. Stop it before deleting.", id ); }
    else
     { if (Retirer_rs485DB( &module->rs485 ))
        { g_snprintf( chaine, sizeof(chaine), " | - Module %02d is erased.", id ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " | - Error. Module %02d is NOT erased.", id ); }
     }
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_rs485_add: Ajoute un capteur/module RS485                                                                            */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_rs485_add ( gchar *response, struct RS485DB *rs485 )
  { gchar chaine[128];
    gint last_id;

    last_id = Ajouter_rs485DB( rs485 );
    if ( last_id != -1 )
     { g_snprintf( chaine, sizeof(chaine), " | - Module added. New ID=%d.", last_id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " | - Error. Module NOT added." ); }
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_rs485_set: Modifie la configuration d'un capteur RS485                                                               */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_rs485_set ( gchar *response, struct RS485DB *rs485 )
  { gchar chaine[128];

    if ( Modifier_rs485DB( rs485 ) )
     { g_snprintf( chaine, sizeof(chaine), " | - Module %d set.", rs485->id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " | - Error. Module NOT set." ); }
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_rs485_start: Demande le demarrage du traitement du module en paremetre                                               */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_rs485_start ( gchar *response, gint id )
  { gchar chaine[128];

    Cfg_rs485.admin_start = id;

    g_snprintf( chaine, sizeof(chaine), " | - Module RS485 %d started", id );
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_rs485_stop: Demande l'arret du traitement du module en paremetre                                                     */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 static gchar *Admin_rs485_stop ( gchar *response, gint id )
  { gchar chaine[128];

    Cfg_rs485.admin_stop = id;

    g_snprintf( chaine, sizeof(chaine), " | - Module RS485 %d stopped", id );
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                                     */
/* Entrée: Le buffer d'entrée a compléter                                                                                     */
/* Sortie: Le buffer de sortie complété                                                                                       */
/******************************************************************************************************************************/
 gchar *Admin_command ( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "add" ) )
     { struct RS485DB rs485;
       memset( &rs485, 0, sizeof(struct RS485DB) );
       if (sscanf ( ligne, "%s %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%[^\n]", commande,/* Découpage de la ligne de commande */
                   &rs485.num, &rs485.bit_comm, (gint *)&rs485.enable,
                   &rs485.ea_min, &rs485.ea_max,
                   &rs485.e_min, &rs485.e_max,
                   &rs485.s_min, &rs485.s_max,
                   &rs485.sa_min, &rs485.sa_max,
                   rs485.libelle ) != 13) return(response);
       response = Admin_rs485_add ( response, &rs485 );
     }
    else if ( ! strcmp ( commande, "del" ) )
     { gint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       response = Admin_rs485_del ( response, num );
     }
    else if ( ! strcmp ( commande, "set" ) )
     { struct RS485DB rs485;
       memset( &rs485, 0, sizeof(struct RS485DB) );
       if (sscanf ( ligne, "%s %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%[^\n]", commande,/* Découpage de la ligne de commande */
                   &rs485.id, &rs485.num, &rs485.bit_comm, (gint *)&rs485.enable,
                   &rs485.ea_min, &rs485.ea_max,
                   &rs485.e_min, &rs485.e_max,
                   &rs485.s_min, &rs485.s_max,
                   &rs485.sa_min, &rs485.sa_max,
                   rs485.libelle ) != 14) return(response);
       response = Admin_rs485_set ( response, &rs485 );
     }
    else if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       response = Admin_rs485_start ( response, num );
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       response = Admin_rs485_stop ( response, num );
     }
    else if ( ! strcmp ( commande, "show" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       response = Admin_rs485_show ( response, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { response = Admin_rs485_list ( response );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { response = Admin_rs485_reload(response);
     }
    else if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { gboolean retour;
       response =  Admin_dbcfg_thread ( response, NOM_THREAD, ligne+6 );                        /* Si changement de parametre */
       retour = Rs485_Lire_config();
       g_snprintf( chaine, sizeof(chaine), " Reloading Thread Parameters from Database -> %s", (retour ? "Success" : "Failed") );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'RS485'" );
       response = Admin_write ( response, "  dbcfg ...                              - Get/Set Database Parameters" );
       response = Admin_write ( response, "  add num,bit_comm,enable,ea_min,ea_max,e_min,e_max,s_min,s_max,sa_min,sa_max,libelle" );
       response = Admin_write ( response, "                                         - Ajoute un module RS485" );
       response = Admin_write ( response, "  set id,num,bit_comm,enable,ea_min,ea_max,e_min,e_max,s_min,s_max,sa_min,sa_max,libelle" );
       response = Admin_write ( response, "                                         - Modifie le module id" );
       response = Admin_write ( response, "  del id                                 - Retire le module id" );
       response = Admin_write ( response, "  start id                               - Demarre le module id" );
       response = Admin_write ( response, "  stop id                                - Arrete le module id" );
       response = Admin_write ( response, "  show id                                - Affiche le module id" );
       response = Admin_write ( response, "  list                                   - Affiche les status des equipements RS485" );
       response = Admin_write ( response, "  reload                                 - Recharge les modules en memoire" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown RS485 command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
