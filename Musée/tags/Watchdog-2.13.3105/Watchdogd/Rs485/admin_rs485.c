/**********************************************************************************************************/
/* Watchdogd/Admin/admin_rs485.c        Gestion des connexions Admin RS485 au serveur watchdog           */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 jan 2009 14:43:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

/**********************************************************************************************************/
/* Admin_rs485_reload: Demande le rechargement des conf RS485                                             */
/* Entrée: le connexion                                                                                   */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_rs485_reload ( struct CONNEXION *connexion )
  { if (Cfg_rs485.lib->Thread_run == FALSE)
     { Admin_write ( connexion, " Thread RS485 is not running\n" );
       return;
     }
    
    Cfg_rs485.reload = TRUE;
    Admin_write ( connexion, " RS485 Reloading in progress\n" );
    while (Cfg_rs485.reload) sched_yield();
    Admin_write ( connexion, " RS485 Reloading done\n" );
  }
/**********************************************************************************************************/
/* Admin_rs485_print : Affiche en details les infos d'un module RS495 en parametre                        */
/* Entrée: La connexion connexion ADMIN et l'onduleur                                                     */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_rs485_print ( struct CONNEXION *connexion, struct MODULE_RS485 *module )
  { gchar chaine[1024];
    g_snprintf( chaine, sizeof(chaine),
                " RS485[%02d] ------> num=%02d, requete = %03ds ago\n"
                "  | - enable = %d, started = %d, bit_comm = B%04d(=%d)\n"
                "  | - EA = %03d-%03d, E = %03d-%03d, S = %03d-%03d, SA = %03d-%03d\n"
                "  | - next_get_ana=in %03ds, nbr_deconnect=%02d\n"
                "  -\n",
                module->rs485.id, module->rs485.num, (Partage->top - module->date_requete)/10,
                module->rs485.enable, module->started, module->rs485.bit_comm, B(module->rs485.bit_comm),
                module->rs485.ea_min, module->rs485.ea_max,
                module->rs485.e_min, module->rs485.e_max,
                module->rs485.s_min, module->rs485.s_max,
                module->rs485.sa_min, module->rs485.sa_max,
                (module->date_next_get_ana > Partage->top ? (module->date_next_get_ana - Partage->top)/10 : -1),
                module->nbr_deconnect
              );
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_rs485_list: Affichage de toutes les infos opérationnelles de tous les modules rs485              */
/* Entrée: La connexion connexion ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_rs485_list ( struct CONNEXION *connexion )
  { GSList *liste_modules;

    pthread_mutex_lock ( &Cfg_rs485.lib->synchro );
    liste_modules = Cfg_rs485.Modules_RS485;
    while ( liste_modules )
     { struct MODULE_RS485 *module;
       module = (struct MODULE_RS485 *)liste_modules->data;
       Admin_rs485_print ( connexion, module );
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_rs485.lib->synchro );
  }
/**********************************************************************************************************/
/* Admin_rs485_show: Affichage des infos opérationnelles liées au module en parametre                     */
/* Entrée: La connexion connexion ADMIN et le numero du module                                            */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_rs485_show ( struct CONNEXION *connexion, gint num )
  { GSList *liste_modules;

    pthread_mutex_lock ( &Cfg_rs485.lib->synchro );
    liste_modules = Cfg_rs485.Modules_RS485;
    while ( liste_modules )
     { struct MODULE_RS485 *module;
       module = (struct MODULE_RS485 *)liste_modules->data;
       if (module->rs485.id == num) { Admin_rs485_print ( connexion, module ); break; }
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_rs485.lib->synchro );
  }
/**********************************************************************************************************/
/* Admin_rs485_del: Retire le capteur/module rs485 dont l'id est en parametre                             */
/* Entrée: le connexion et l'id                                                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rs485_del ( struct CONNEXION *connexion, gint id )
  { struct MODULE_RS485 *module;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Suppression du module rs485 %02d\n", id );
    Admin_write ( connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Admin_write ( connexion, chaine );

    module = Chercher_module_rs485_by_id ( id );
    if (!module)
     { g_snprintf( chaine, sizeof(chaine), " Module not found.\n" ); }
    else if (module->started)
     { g_snprintf( chaine, sizeof(chaine), " Module %02d is not stopped. Stop it before deleting.\n", id ); }
    else
     { if (Retirer_rs485DB( &module->rs485 ))
        { g_snprintf( chaine, sizeof(chaine), " Module %02d is erased.\n", id ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error. Module %02d is NOT erased.\n", id ); }
     }
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_rs485_add: Ajoute un capteur/module RS485                                                        */
/* Entrée: le connexion et la structure de reference du capteur                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rs485_add ( struct CONNEXION *connexion, struct RS485DB *rs485 )
  { gchar chaine[128];
    gint last_id;

    g_snprintf( chaine, sizeof(chaine), " -- Ajout d'un module rs485\n" );
    Admin_write ( connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Admin_write ( connexion, chaine );

    last_id = Ajouter_rs485DB( rs485 );
    if ( last_id != -1 )
     { g_snprintf( chaine, sizeof(chaine), " Module added. New ID=%d.\n", last_id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT added.\n" ); }
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_rs485_set: Modifie la configuration d'un capteur RS485                                        */
/* Entrée: le connexion et la structure de reference du capteur                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rs485_set ( struct CONNEXION *connexion, struct RS485DB *rs485 )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Modification d'un module rs485\n" );
    Admin_write ( connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Admin_write ( connexion, chaine );

    if ( Modifier_rs485DB( rs485 ) )
     { g_snprintf( chaine, sizeof(chaine), " Module %d setd.\n", rs485->id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT setd.\n" ); }
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_rs485_start: Demande le demarrage du traitement du module en paremetre                           */
/* Entrée: Le connexion demandeur, l'id du module                                                            */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rs485_start ( struct CONNEXION *connexion, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Demarrage d'un module RS485\n" );
    Admin_write ( connexion, chaine );

    Cfg_rs485.admin_start = id;

    g_snprintf( chaine, sizeof(chaine), " Module RS485 %d started\n", id );
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_rs485_stop: Demande l'arret du traitement du module en paremetre                                 */
/* Entrée: Le connexion demandeur, l'id du module                                                         */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rs485_stop ( struct CONNEXION *connexion, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Arret d'un module RS485\n" );
    Admin_write ( connexion, chaine );

    Cfg_rs485.admin_stop = id;

    g_snprintf( chaine, sizeof(chaine), " Module RS485 %d stopped\n", id );
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                 */
/* Entrée: Le connexion d'admin, la ligne a traiter                                                       */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command ( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "add" ) )
     { struct RS485DB rs485;
       memset( &rs485, 0, sizeof(struct RS485DB) );
       sscanf ( ligne, "%s %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%[^\n]", commande,/* Découpage de la ligne de commande */
                &rs485.num, &rs485.bit_comm, (gint *)&rs485.enable,
                &rs485.ea_min, &rs485.ea_max,
                &rs485.e_min, &rs485.e_max,
                &rs485.s_min, &rs485.s_max,
                &rs485.sa_min, &rs485.sa_max,
                rs485.libelle );
       Admin_rs485_add ( connexion, &rs485 );
     }
    else if ( ! strcmp ( commande, "del" ) )
     { gint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_rs485_del ( connexion, num );
     }
    else if ( ! strcmp ( commande, "set" ) )
     { struct RS485DB rs485;
       memset( &rs485, 0, sizeof(struct RS485DB) );
       sscanf ( ligne, "%s %d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%[^\n]", commande,/* Découpage de la ligne de commande */
                &rs485.id, &rs485.num, &rs485.bit_comm, (gint *)&rs485.enable,
                &rs485.ea_min, &rs485.ea_max,
                &rs485.e_min, &rs485.e_max,
                &rs485.s_min, &rs485.s_max,
                &rs485.sa_min, &rs485.sa_max,
                rs485.libelle );
       Admin_rs485_set ( connexion, &rs485 );
     }
    else if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_rs485_start ( connexion, num );
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_rs485_stop ( connexion, num );
     }
    else if ( ! strcmp ( commande, "show" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_rs485_show ( connexion, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_rs485_list ( connexion );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_rs485_reload(connexion);
     }
    else if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { if (Admin_dbcfg_thread ( connexion, NOM_THREAD, ligne+6 ) == TRUE)   /* Si changement de parametre */
        { gboolean retour;
          retour = Rs485_Lire_config();
          g_snprintf( chaine, sizeof(chaine), " Reloading Thread Parameters from Database -> %s\n",
                      (retour ? "Success" : "Failed") );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'RS485'\n" );
       Admin_write ( connexion, "  dbcfg ...                              - Get/Set Database Parameters\n" );
       Admin_write ( connexion, "  add num,bit_comm,enable,ea_min,ea_max,e_min,e_max,s_min,s_max,sa_min,sa_max,libelle\n" );
       Admin_write ( connexion, "                                         - Ajoute un module RS485\n" );
       Admin_write ( connexion, "  set id,num,bit_comm,enable,ea_min,ea_max,e_min,e_max,s_min,s_max,sa_min,sa_max,libelle\n" );
       Admin_write ( connexion, "                                         - Modifie le module id\n" );
       Admin_write ( connexion, "  del id                                 - Retire le module id\n" );
       Admin_write ( connexion, "  start id                               - Demarre le module id\n" );
       Admin_write ( connexion, "  stop id                                - Arrete le module id\n" );
       Admin_write ( connexion, "  show id                                - Affiche le module id\n" );
       Admin_write ( connexion, "  list                                   - Affiche les status des equipements RS485\n" );
       Admin_write ( connexion, "  reload                                 - Recharge les modules en memoire\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown RS485 command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
