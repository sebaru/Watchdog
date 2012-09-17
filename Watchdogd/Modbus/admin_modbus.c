/**********************************************************************************************************/
/* Watchdogd/Admin/admin_modbus.c        Gestion des connexions Admin MODBUS au serveur watchdog          */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 05 sept. 2010 12:01:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_modbus.c
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
 
 #include "watchdogd.h"
 #include "Modbus.h"

/**********************************************************************************************************/
/* Admin_modbus_reload: Demande le rechargement des conf MODBUS                                           */
/* Entrée: le client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_modbus_reload ( struct CLIENT_ADMIN *client )
  { Cfg_modbus.reload = TRUE;
    Write_admin ( client->connexion, " MODBUS Reloading in progress\n" );
    while (Cfg_modbus.reload) sched_yield();
    Write_admin ( client->connexion, " MODBUS Reloading done\n" );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_modbus_list ( struct CLIENT_ADMIN *client )
  { GSList *liste_modules;
    gchar chaine[512];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des modules MODBUS\n" );
    Write_admin ( client->connexion, chaine );

    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );
       
    pthread_mutex_lock( &Cfg_modbus.lib->synchro );
    liste_modules = Cfg_modbus.Modules_MODBUS;
    while ( liste_modules )
     { struct MODULE_MODBUS *module;
       module = (struct MODULE_MODBUS *)liste_modules->data;

       g_snprintf( chaine, sizeof(chaine),
                   " MODBUS[%02d] -> bit=%04d, enable=%d, started=%d, mode=%02d, watchdog=%03d, IP=%s,\n"
                   "               min_e_tor=%03d, min_e_ana=%03d, min_s_tor=%03d, min_s_ana=%03d\n"
                   "               trans.=%06d, deco.=%02d, last_reponse=%03ds ago, retente=in %03d, date_next_eana=in %03d\n",
                   module->modbus.id, module->modbus.bit, module->modbus.enable,
                   module->started, module->mode, module->modbus.watchdog, module->modbus.ip,
                   module->modbus.min_e_tor, module->modbus.min_e_ana,
                   module->modbus.min_s_tor, module->modbus.min_s_ana,
                   module->transaction_id, module->nbr_deconnect,
                  (Partage->top - module->date_last_reponse)/10,                   
                   (module->date_retente   ? (module->date_retente   - Partage->top)/10 : -1),
                   (module->date_next_eana > Partage->top ? (module->date_next_eana - Partage->top)/10 : -1)
                 );
       Write_admin ( client->connexion, chaine );
       liste_modules = liste_modules->next;                                  /* Passage au module suivant */
     }
    pthread_mutex_unlock( &Cfg_modbus.lib->synchro );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_modbus_start ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Demarrage d'un module MODBUS\n" );
    Write_admin ( client->connexion, chaine );

    Cfg_modbus.admin_start = id;

    g_snprintf( chaine, sizeof(chaine), " Module MODBUS %d started\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_modbus_stop ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Arret d'un module MODBUS\n" );
    Write_admin ( client->connexion, chaine );

    Cfg_modbus.admin_stop = id;

    g_snprintf( chaine, sizeof(chaine), " Module MODBUS %d stopped\n", id );
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_command ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_modbus_start ( client, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_modbus_list ( client );
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_modbus_stop ( client, num );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_modbus_reload(client);
     }
    else if ( ! strcmp ( commande, "add" ) )
     { struct MODBUSDB modbus;
       gint retour;

       sscanf ( ligne, "%s %d,%d,%s,%d,%d,%d,%d,%s", commande,       /* Découpage de la ligne de commande */
                &modbus.enable, &modbus.bit, modbus.ip,
                &modbus.min_e_tor, &modbus.min_e_ana, &modbus.min_s_tor, &modbus.min_s_ana,
                modbus.libelle
              );
       retour = Ajouter_modbusDB ( &modbus );
       if (retour == -1)
        { Write_admin ( client->connexion, "Error, MODBUS not added\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " MODBUS %s added. New ID=%d\n", modbus.ip, retour );
          Write_admin ( client->connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "change" ) )
     { struct MODBUSDB modbus;
       gint retour;
       sscanf ( ligne, "%s %d,%d,%d,%s,%d,%d,%d,%d,%s", commande,    /* Découpage de la ligne de commande */
                &modbus.id, &modbus.enable, &modbus.bit, modbus.ip,
                &modbus.min_e_tor, &modbus.min_e_ana, &modbus.min_s_tor, &modbus.min_s_ana,
                modbus.libelle
              );
       retour = Modifier_modbusDB ( &modbus );
       if (retour == FALSE)
        { Write_admin ( client->connexion, "Error, MODBUS not changed\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " MODBUS %s changed\n", modbus.ip );
          Write_admin ( client->connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "del" ) )
     { struct MODBUSDB modbus;
       gboolean retour;
       sscanf ( ligne, "%s %d", commande, &modbus.id );              /* Découpage de la ligne de commande */
       retour = Retirer_modbusDB ( &modbus );
       if (retour == FALSE)
        { Write_admin ( client->connexion, "Error, MODBUS not erased\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " MODBUS %d erased\n", modbus.id );
          Write_admin ( client->connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'MODBUS'\n" );
       Write_admin ( client->connexion,
                     "  add xxxxxxxxxxxxxxxxxxxxxx             - Ajoute un module modbus\n" );
       Write_admin ( client->connexion,
                     "  change id xxxxxxxxxxxxxxxxxxxxx        - Modifie le module id\n" );
       Write_admin ( client->connexion,
                     "  del id                                 - Supprime le module id\n" );
       Write_admin ( client->connexion,
                     "  start id                               - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  stop id                                - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  list                                   - Liste les modules MODBUS\n" );
       Write_admin ( client->connexion,
                     "  reload                                 - Recharge la configuration\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown MODBUS command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
