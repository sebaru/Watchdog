/**********************************************************************************************************/
/* Watchdogd/Admin/admin_rfxcom.c        Gestion des connexions Admin RFXCOM au serveur watchdog          */
/* Projet WatchDog version 2.0       Gestion d'habitat                    mer. 13 juin 2012 23:02:08 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_rfxcom.c
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

/**********************************************************************************************************/
/* Admin_rfxcom_reload: Demande le rechargement des conf RFXCOM                                           */
/* Entrée: le client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_rfxcom_reload ( struct CLIENT_ADMIN *client )
  { Partage->com_rfxcom.Thread_reload = TRUE;
    Write_admin ( client->connexion, " RFXCOM Reloading in progress\n" );
    while (Partage->com_rfxcom.Thread_reload) sched_yield();
    Write_admin ( client->connexion, " RFXCOM Reloading done\n" );
  }
/**********************************************************************************************************/
/* Admin_rfxcom_list: Liste l'ensemble des capteurs rfxcom présent dans la conf                           */
/* Entrée: le client                                                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rfxcom_list ( struct CLIENT_ADMIN *client )
  { GList *liste_modules;
    gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des modules/capteurs RFXCOM\n" );
    Write_admin ( client->connexion, chaine );

    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );
       
    liste_modules = Partage->com_rfxcom.Modules_RFXCOM;
    while ( liste_modules )
     { struct MODULE_RFXCOM *module;
       module = (struct MODULE_RFXCOM *)liste_modules->data;

       g_snprintf( chaine, sizeof(chaine),
                   " RFXCOM[%02d] -> type=%02d,canal=%02d,e_min=%03d,ea_min=%03d,a_min=%03d,libelle=%s\n"
                   "                 date_last_view=%d\n",
                   module->rfxcom.id, module->rfxcom.type, module->rfxcom.canal,
                   module->rfxcom.e_min, module->rfxcom.ea_min, module->rfxcom.a_min,
                   module->rfxcom.libelle, (gint)module->date_last_view
                 );
       Write_admin ( client->connexion, chaine );
       liste_modules = liste_modules->next;
     }
  }
/**********************************************************************************************************/
/* Admin_rfxcom_del: Retire le capteur/module rfxcom dont l'id est en parametre                           */
/* Entrée: le client et l'id                                                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rfxcom_del ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Suppression du module rfxcom %d\n", id );
    Write_admin ( client->connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );

    if (Partage->com_rfxcom.Retirer_rfxcomDB)
     { if (Partage->com_rfxcom.Retirer_rfxcomDB( id ))
        { g_snprintf( chaine, sizeof(chaine), " Module erased.\n You should reload configuration...\n" ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT erased.\n" ); }
       Write_admin ( client->connexion, chaine );
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error, thread not loaded.\n" );
       Write_admin ( client->connexion, chaine );
     }
  }
/**********************************************************************************************************/
/* Admin_rfxcom_add: Ajoute un capteur/module RFXCOM                                                      */
/* Entrée: le client et la structure de reference du capteur                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rfxcom_add ( struct CLIENT_ADMIN *client, struct CMD_TYPE_RFXCOM *rfxcom )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Ajout d'un module rfxcom\n" );
    Write_admin ( client->connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );

    if (Partage->com_rfxcom.Ajouter_rfxcomDB)
     { gint last_id;
       last_id = Partage->com_rfxcom.Ajouter_rfxcomDB( rfxcom );
       if ( last_id != -1 )
        { g_snprintf( chaine, sizeof(chaine), " Module added. New ID=%d.\n You should reload configuration...\n", last_id ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT added.\n" ); }
       Write_admin ( client->connexion, chaine );
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error, thread not loaded.\n" );
       Write_admin ( client->connexion, chaine );
     }
  }
/**********************************************************************************************************/
/* Admin_rfxcom_change: Modifie la configuration d'un capteur RFXCOM                                      */
/* Entrée: le client et la structure de reference du capteur                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_rfxcom_change ( struct CLIENT_ADMIN *client, struct CMD_TYPE_RFXCOM *rfxcom )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Modification d'un module rfxcom\n" );
    Write_admin ( client->connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );

    if (Partage->com_rfxcom.Modifier_rfxcomDB)
     { if ( Partage->com_rfxcom.Modifier_rfxcomDB( rfxcom ) )
        { g_snprintf( chaine, sizeof(chaine), " Module %d changed.\n You should reload configuration...\n", rfxcom->id ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT changed.\n" ); }
       Write_admin ( client->connexion, chaine );
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error, thread not loaded.\n" );
       Write_admin ( client->connexion, chaine );
     }
  }
/**********************************************************************************************************/
/* Admin_rfxcom: Fonction gerant les différentes commandes possible pour l'administration rfxcom          */
/* Entrée: le client d'admin et la ligne de commande                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_rfxcom ( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "add" ) )
     { struct CMD_TYPE_RFXCOM rfxcom;
       memset( &rfxcom, 0, sizeof(struct CMD_TYPE_RFXCOM) );
       sscanf ( ligne, "%s %d %d %d %d %d %s", commande,             /* Découpage de la ligne de commande */
                (gint *)&rfxcom.type, (gint *)&rfxcom.canal, &rfxcom.e_min, &rfxcom.ea_min, &rfxcom.a_min, rfxcom.libelle );
       Admin_rfxcom_add ( client, &rfxcom );
     }
    else if ( ! strcmp ( commande, "change" ) )
     { struct CMD_TYPE_RFXCOM rfxcom;
       memset( &rfxcom, 0, sizeof(struct CMD_TYPE_RFXCOM) );
       sscanf ( ligne, "%s %d %d %d %d %d %d %s", commande,          /* Découpage de la ligne de commande */
                &rfxcom.id, (gint *)&rfxcom.type, (gint *)&rfxcom.canal,
                &rfxcom.e_min, &rfxcom.ea_min, &rfxcom.a_min, rfxcom.libelle );
       Admin_rfxcom_change ( client, &rfxcom );
     }
    else if ( ! strcmp ( commande, "del" ) )
     { gint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_rfxcom_del ( client, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_rfxcom_list ( client );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_rfxcom_reload(client);
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'RFXCOM'\n" );
       Write_admin ( client->connexion,
                     "  add type canal e_min ea_min a_min libelle\n"
                     "                                         - Ajoute un module\n" );
       Write_admin ( client->connexion,
                     "  change ID type canal e_min ea_min a_min libelle\n"
                     "                                         - Edite le module ID\n" );
       Write_admin ( client->connexion,
                     "  del ID                                 - Retire le module ID\n" );
       Write_admin ( client->connexion,
                     "  list                                   - Affiche les status des equipements RFXCOM\n" );
       Write_admin ( client->connexion,
                     "  reload                                 - Recharge la configuration\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown RFXCOM command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
