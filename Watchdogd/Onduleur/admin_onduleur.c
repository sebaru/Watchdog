/**********************************************************************************************************/
/* Watchdogd/Admin/admin_ups.c        Gestion des connexions Admin ONDULEUR au serveur watchdog      */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 11 nov. 2009 11:28:29 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * admin_ups.c
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
 #include "Onduleur.h"
/**********************************************************************************************************/
/* Admin_ups_reload: Demande le rechargement des conf ONDULEUR                                            */
/* Entrée: le connexion                                                                                   */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_ups_reload ( struct CONNEXION *connexion )
  { if (Cfg_ups.lib->Thread_run == FALSE)
     { Admin_write ( connexion, " Thread ONDULEUR is not running\n" );
       return;
     }
    
    Cfg_ups.reload = TRUE;
    Admin_write ( connexion, " ONDULEUR Reloading in progress\n" );
    while (Cfg_ups.reload) sched_yield();
    Admin_write ( connexion, " ONDULEUR Reloading done\n" );
  }
/**********************************************************************************************************/
/* Admin_ups_list : L'utilisateur admin lance la commande "list" en mode ups                              */
/* Entrée: La connexion connexion ADMIN                                                                   */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                       */
/**********************************************************************************************************/
 static void Admin_ups_list ( struct CONNEXION *connexion )
  { GSList *liste_modules;
    gchar chaine[256];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des UPS\n" );
    Admin_write ( connexion, chaine );

    g_snprintf( chaine, sizeof(chaine), "Partage->top = %d\n", Partage->top );
    Admin_write ( connexion, chaine );
       
    pthread_mutex_lock ( &Cfg_ups.lib->synchro );
    liste_modules = Cfg_ups.Modules_UPS;
    while ( liste_modules )
     { struct MODULE_UPS *module;
       module = (struct MODULE_UPS *)liste_modules->data;

       g_snprintf( chaine, sizeof(chaine),
                   " UPS[%03d] -> Host=%s, UPS=%s, enable=%d, started=%d, nbr_deconnect=%d date_retente=%d\n"
                   "             username=%s, password=%s\n"
                   "             bit_comm=%d, ea_min=%d, e_min=%d, a_min=%d\n\n",
                   module->ups.id, module->ups.host, module->ups.ups,
                   module->ups.enable, module->started,
                   module->nbr_deconnect, (int)module->date_retente,
                   module->ups.username, module->ups.password,
                   module->ups.bit_comm,
                   module->ups.ea_min, module->ups.e_min, module->ups.a_min
                 );
       Admin_write ( connexion, chaine );
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_ups.lib->synchro );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_ups_start ( struct CONNEXION *connexion, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Demarrage d'un module UPS\n" );
    Admin_write ( connexion, chaine );

    Cfg_ups.admin_start = id;

    g_snprintf( chaine, sizeof(chaine), " Module UPS %d started\n", id );
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_ups_stop ( struct CONNEXION *connexion, gint id )
  { gchar chaine[128];

    g_snprintf( chaine, sizeof(chaine), " -- Arret d'un module UPS\n" );
    Admin_write ( connexion, chaine );

    Cfg_ups.admin_stop = id;

    g_snprintf( chaine, sizeof(chaine), " Module UPS %d stopped\n", id );
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Activer_ecoute: Permettre les connexions distantes au serveur watchdog                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 void Admin_command ( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_ups_start ( connexion, num );
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       Admin_ups_stop ( connexion, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_ups_list ( connexion );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_ups_reload(connexion);
     }
    else if ( ! strcmp ( commande, "add" ) )
     { struct UPSDB ups;
       gint retour;
       sscanf ( ligne, "%s %[^,],%[^,],%[^,],%[^,],%d,%d,%d,%d,%[^\n]", commande,    /* Découpage de la ligne de commande */
                ups.ups, ups.host, ups.username, ups.password,
                &ups.bit_comm, &ups.ea_min, &ups.e_min, &ups.a_min, ups.libelle
              );
       ups.enable = TRUE;
       retour = Ajouter_upsDB ( &ups );
       if (retour == -1)
        { Admin_write ( connexion, "Error, UPS not added\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " UPS %s added. New ID=%d\n", ups.ups, retour );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "change" ) )
     { struct UPSDB ups;
       gint retour;
       sscanf ( ligne, "%s %d,%s,%s,%s,%s,%d,%d,%d,%d,%s", commande, /* Découpage de la ligne de commande */
                &ups.id, ups.ups, ups.host, ups.username, ups.password,
                &ups.bit_comm, &ups.ea_min, &ups.e_min, &ups.a_min, ups.libelle
              );
       retour = Modifier_upsDB ( &ups );
       if (retour == FALSE)
        { Admin_write ( connexion, "Error, UPS not changed\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " UPS %s changed\n", ups.ups );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "del" ) )
     { struct UPSDB ups;
       gboolean retour;
       sscanf ( ligne, "%s %d", commande, &ups.id );                 /* Découpage de la ligne de commande */
       retour = Retirer_upsDB ( &ups );
       if (retour == FALSE)
        { Admin_write ( connexion, "Error, UPS not erased\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " UPS %d erased\n", ups.id );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'UPS'\n" );
       Admin_write ( connexion, "  add name,host,username,password,bit_comm,ea_min,e_min,a_min,libelle\n");
       Admin_write ( connexion, "                                         - Ajoute un UPS\n" );
       Admin_write ( connexion, "  change id,name,host,username,password,bit_comm,ea_min,e_min,a_min,libelle\n");
       Admin_write ( connexion, "                                         - Change UPS id\n" );
       Admin_write ( connexion, "  del id                                 - Delete UPS id\n" );
       Admin_write ( connexion, "  start id                               - Start UPS id\n" );
       Admin_write ( connexion, "  stop id                                - Stop UPS id\n" );
       Admin_write ( connexion, "  list                                   - Liste les modules ONDULEUR\n" );
       Admin_write ( connexion, "  reload                                 - Recharge la configuration\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown NUT command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
