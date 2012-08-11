/**********************************************************************************************************/
/* Watchdogd/Imsg/admin_imsg.c        Gestion des connexions Admin IMSG au serveur watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                  sam. 28 juil. 2012 16:35:09 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_imsg.c
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
/* Admin_imsg_reload: Demande le rechargement des conf IMSG                                               */
/* Entrée: le client                                                                                      */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_imsg_send ( struct CLIENT_ADMIN *client, gchar *to, gchar message )
  { 
  }
/**********************************************************************************************************/
/* Admin_imsg_list: Liste l'ensemble des capteurs imsg présent dans la conf                           */
/* Entrée: le client                                                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_imsg_list ( struct CLIENT_ADMIN *client )
  { GList *liste_modules;
    gchar chaine[512];
/*
    g_snprintf( chaine, sizeof(chaine), " -- Liste des modules/capteurs IMSG\n" );
    Write_admin ( client->connexion, chaine );

    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );
       
    liste_modules = Partage->com_imsg.Modules_IMSG;
    while ( liste_modules )
     { struct MODULE_IMSG *module;
       module = (struct MODULE_IMSG *)liste_modules->data;

       g_snprintf( chaine, sizeof(chaine),
                   " IMSG[%02d] -> type=%02d,canal=%02d,e_min=%03d,ea_min=%03d,a_min=%03d,libelle=%s\n"
                   "                 date_last_view=%d\n",
                   module->imsg.id, module->imsg.type, module->imsg.canal,
                   module->imsg.e_min, module->imsg.ea_min, module->imsg.a_min,
                   module->imsg.libelle, (gint)module->date_last_view
                 );
       Write_admin ( client->connexion, chaine );
       liste_modules = liste_modules->next;
     }*/
  }
/**********************************************************************************************************/
/* Admin_imsg_del: Retire le capteur/module imsg dont l'id est en parametre                           */
/* Entrée: le client et l'id                                                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_imsg_del ( struct CLIENT_ADMIN *client, gint id )
  { gchar chaine[128];
/*
    g_snprintf( chaine, sizeof(chaine), " -- Suppression du module imsg %d\n", id );
    Write_admin ( client->connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );

    if (Partage->com_imsg.Retirer_imsgDB)
     { if (Partage->com_imsg.Retirer_imsgDB( id ))
        { g_snprintf( chaine, sizeof(chaine), " Module erased.\n You should reload configuration...\n" ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT erased.\n" ); }
       Write_admin ( client->connexion, chaine );
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error, thread not loaded.\n" );
       Write_admin ( client->connexion, chaine );
     }*/
  }
/**********************************************************************************************************/
/* Admin_imsg_add: Ajoute un capteur/module IMSG                                                      */
/* Entrée: le client et la structure de reference du capteur                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_imsg_add ( struct CLIENT_ADMIN *client, struct IMSGDB *imsg )
  { gchar chaine[128];
/*
    g_snprintf( chaine, sizeof(chaine), " -- Ajout d'un module imsg\n" );
    Write_admin ( client->connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );

    if (Partage->com_imsg.Ajouter_imsgDB)
     { gint last_id;
       last_id = Partage->com_imsg.Ajouter_imsgDB( imsg );
       if ( last_id != -1 )
        { g_snprintf( chaine, sizeof(chaine), " Module added. New ID=%d.\n You should reload configuration...\n", last_id ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT added.\n" ); }
       Write_admin ( client->connexion, chaine );
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error, thread not loaded.\n" );
       Write_admin ( client->connexion, chaine );
     }*/
  }
/**********************************************************************************************************/
/* Admin_imsg_change: Modifie la configuration d'un capteur IMSG                                      */
/* Entrée: le client et la structure de reference du capteur                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_imsg_change ( struct CLIENT_ADMIN *client, struct IMSGDB *imsg )
  { gchar chaine[128];
/*
    g_snprintf( chaine, sizeof(chaine), " -- Modification d'un module imsg\n" );
    Write_admin ( client->connexion, chaine );
    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Write_admin ( client->connexion, chaine );

    if (Partage->com_imsg.Modifier_imsgDB)
     { if ( Partage->com_imsg.Modifier_imsgDB( imsg ) )
        { g_snprintf( chaine, sizeof(chaine), " Module %d changed.\n You should reload configuration...\n", imsg->id ); }
       else
        { g_snprintf( chaine, sizeof(chaine), " Error. Module NOT changed.\n" ); }
       Write_admin ( client->connexion, chaine );
     }
    else
     { g_snprintf( chaine, sizeof(chaine), " Error, thread not loaded.\n" );
       Write_admin ( client->connexion, chaine );
     }*/
  }
/**********************************************************************************************************/
/* Admin_imsg: Fonction gerant les différentes commandes possible pour l'administration imsg          */
/* Entrée: le client d'admin et la ligne de commande                                                      */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 void Admin_command( struct CLIENT_ADMIN *client, gchar *ligne )
  { gchar commande[128];
#ifdef bouh
    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "add" ) )
     { struct IMSGDB imsg;
       memset( &imsg, 0, sizeof(struct IMSGDB) );
       sscanf ( ligne, "%s %d %d %d %d %d %s", commande,             /* Découpage de la ligne de commande */
                (gint *)&imsg.type, (gint *)&imsg.canal, &imsg.e_min, &imsg.ea_min, &imsg.a_min, imsg.libelle );
       Admin_imsg_add ( client, &imsg );
     }
    else if ( ! strcmp ( commande, "change" ) )
     { struct IMSGDB imsg;
       memset( &imsg, 0, sizeof(struct IMSGDB) );
       sscanf ( ligne, "%s %d %d %d %d %d %d %s", commande,          /* Découpage de la ligne de commande */
                &imsg.id, (gint *)&imsg.type, (gint *)&imsg.canal,
                &imsg.e_min, &imsg.ea_min, &imsg.a_min, imsg.libelle );
       Admin_imsg_change ( client, &imsg );
     }
    else if ( ! strcmp ( commande, "cmd" ) )
     { sscanf ( ligne, "%s %d %d %d %d %d %d %d", commande,          /* Découpage de la ligne de commande */
                &Partage->com_imsg.learn.id1, &Partage->com_imsg.learn.id2,
                &Partage->com_imsg.learn.id3, &Partage->com_imsg.learn.id4,
                &Partage->com_imsg.learn.unitcode,
                &Partage->com_imsg.learn.cmd,
                &Partage->com_imsg.learn.level
               );
       Partage->com_imsg.Thread_commande = TRUE;
       Write_admin ( client->connexion, " IMSG Sending CMD....\n" );
       while (Partage->com_imsg.Thread_commande) sched_yield();
       Write_admin ( client->connexion, " IMSG Done.\n" );
     }
    else if ( ! strcmp ( commande, "del" ) )
     { gint num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_imsg_del ( client, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_imsg_list ( client );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_imsg_reload(client);
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Write_admin ( client->connexion,
                     "  -- Watchdog ADMIN -- Help du mode 'IMSG'\n" );
       Write_admin ( client->connexion,
                     "  add type canal e_min ea_min a_min libelle\n"
                     "                                         - Ajoute un module\n" );
       Write_admin ( client->connexion,
                     "  change ID type canal e_min ea_min a_min libelle\n"
                     "                                         - Edite le module ID\n" );
       Write_admin ( client->connexion,
                     "  del ID                                 - Retire le module ID\n" );
       Write_admin ( client->connexion,
                     "  cmd id1 id2 id3 id4 unitcode cmdnumber - Envoie une commande IMSG\n" );
       Write_admin ( client->connexion,
                     "  list                                   - Affiche les status des equipements IMSG\n" );
       Write_admin ( client->connexion,
                     "  reload                                 - Recharge la configuration\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown IMSG command : %s\n", ligne );
       Write_admin ( client->connexion, chaine );
     }
#endif
  }
/*--------------------------------------------------------------------------------------------------------*/
