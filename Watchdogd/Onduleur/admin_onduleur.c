/**********************************************************************************************************/
/* Watchdogd/Admin/admin_onduleur.c        Gestion des connexions Admin ONDULEUR au serveur watchdog      */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 11 nov. 2009 11:28:29 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_onduleur.c
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
#ifdef bouh
/**********************************************************************************************************/
/* Admin_onduleur_reload: Demande le rechargement des conf ONDULEUR                                       */
/* Entrée: le client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_onduleur_reload ( struct CLIENT_ADMIN *client )
  { Partage->com_onduleur.Thread_reload = TRUE;
    Write_admin ( client->connexion, " ONDULEUR Reloading in progress\n" );
    while (Partage->com_onduleur.Thread_reload) sched_yield();
    Write_admin ( client->connexion, " ONDULEUR Reloading done\n" );
  }
/**********************************************************************************************************/
/* Admin_onduleur_list : L'utilisateur admin lance la commande "list" en mode onduleur                    */
/* Entrée: La connexion client ADMIN                                                                      */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 void Admin_onduleur_list ( struct CLIENT_ADMIN *client )
  { GList *liste_modules;
    gchar chaine[512];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des UPS\n" );
    Write_admin ( client->connexion, chaine );

    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );
       
    pthread_mutex_lock( &Partage->com_onduleur.synchro );
    liste_modules = Partage->com_onduleur.Modules_ONDULEUR;
    while ( liste_modules )
     { struct MODULE_ONDULEUR *module;
       module = (struct MODULE_ONDULEUR *)liste_modules->data;

       g_snprintf( chaine, sizeof(chaine),
                   " ONDULEUR[%02d] -> Host=%s, UPS=%s, actif=%d, started=%d nbr_deconnect=%d date_retente=%d\n"
                   "                   bit_comm=%d, ea_min=%d, e_min=%d, a_min=%d\n",
                   module->onduleur.id, module->onduleur.host, module->onduleur.ups,
                   module->onduleur.actif, module->started,
                   module->nbr_deconnect, (int)module->date_retente, module->onduleur.bit_comm,
                   module->onduleur.ea_min, module->onduleur.e_min, module->onduleur.a_min
                 );
       Write_admin ( client->connexion, chaine );
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock( &Partage->com_onduleur.synchro );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_onduleur_start ( struct CLIENT_ADMIN *client, struct DB *db, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Demarrage d'un UPS\n" );
    Write_admin ( client->connexion, chaine );

    while (Partage->com_onduleur.admin_start) sched_yield();
    Partage->com_onduleur.admin_start = id;

    if (Modifier_onduleurDB_set_start( Config.log, db, id, TRUE))
     { g_snprintf( chaine, sizeof(chaine), " Module ONDULEUR %d started\n", id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " -- error -- Module ONDULEUR NOT %d started\n", id ); }
    Write_admin ( client->connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_onduleur_stop ( struct CLIENT_ADMIN *client, struct DB *db, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Arret d'un UPS\n" );
    Write_admin ( client->connexion, chaine );

    while (Partage->com_onduleur.admin_stop) sched_yield();
    Partage->com_onduleur.admin_stop = id;

    if (Modifier_onduleurDB_set_start( Config.log, db, id, FALSE))
     { g_snprintf( chaine, sizeof(chaine), "Module ONDULEUR %d stopped\n", id ); }
    else
     { g_snprintf( chaine, sizeof(chaine), " -- error -- Module ONDULEUR NOT %d stopped\n", id ); }
    Write_admin ( client->connexion, chaine );
  }
#endif
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_onduleur ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       /*Admin_onduleur_start ( client, db, num );*/
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
/*       Admin_onduleur_stop ( client, db, num );*/
     }
    else if ( ! strcmp ( commande, "list" ) )
     {/* Admin_onduleur_list ( client );*/
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { /*Admin_onduleur_reload(client);*/
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'UPS'\n" );
       Write_admin ( client->connexion,
                     "  add x                                  - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  del x                                  - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  start id                               - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  stop id                                - Demarre le module id\n" );
       Write_admin ( client->connexion,
                     "  list                                   - Liste les modules ONDULEUR\n" );
       Write_admin ( client->connexion,
                     "  reload                                 - Recharge la configuration\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown NUT command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
