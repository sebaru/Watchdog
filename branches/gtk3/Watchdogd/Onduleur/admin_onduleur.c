/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_ups.c        Gestion des responses Admin ONDULEUR au serveur watchdog                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                                         mer. 11 nov. 2009 11:28:29 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
/******************************************************************************************************************************/
/* Admin_ups_reload: Demande le rechargement des conf ONDULEUR                                                                */
/* Entrée: le response                                                                                                       */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gchar *Admin_ups_reload ( gchar *response )
  { if (Cfg_ups.lib->Thread_run == FALSE)
     { response = Admin_write ( response, " Thread ONDULEUR is not running" );
       return(response);
     }
    
    Cfg_ups.lib->Thread_sigusr1 = TRUE;
    while (Cfg_ups.lib->Thread_sigusr1) sched_yield();
    response = Admin_write ( response, " ONDULEUR Reload done" );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_ups_print : Affiche en details les infos d'un onduleur en parametre                                                  */
/* Entrée: La response response ADMIN et l'onduleur                                                                         */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                                           */
/******************************************************************************************************************************/
 static gchar *Admin_ups_print ( gchar *response, struct MODULE_UPS *module )
  { gchar chaine[1024];
    g_snprintf( chaine, sizeof(chaine),
                " UPS[%02d] ------> %s@%s %s\n"
                "  | - enable = %d, started = %d (bit B%04d=%d)\n"
                "  | - map_EA = EA%03d, map_E = E%03d, map_A = A%03d\n"
                "  | - username = %s, password = %s,\n"
                "  | - date_next_response = in %03ds",
                module->ups.id, module->ups.ups, module->ups.host, module->libelle,
                module->ups.enable, module->started, module->ups.bit_comm, B(module->ups.bit_comm),
                module->ups.map_EA, module->ups.map_E, module->ups.map_A,
                module->ups.username, module->ups.password,
          (int)(module->date_next_connexion > Partage->top ? (module->date_next_connexion - Partage->top)/10 : -1)
              );
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_ups_list : Affichage de toutes les infos opérationnelles de tous les onduleurs                                       */
/* Entrée: La response response ADMIN                                                                                       */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                                           */
/******************************************************************************************************************************/
 static gchar *Admin_ups_list ( gchar *response )
  { GSList *liste_modules;
       
    pthread_mutex_lock ( &Cfg_ups.lib->synchro );
    liste_modules = Cfg_ups.Modules_UPS;
    while ( liste_modules )
     { struct MODULE_UPS *module;
       module = (struct MODULE_UPS *)liste_modules->data;
       response = Admin_ups_print ( response, module );
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_ups.lib->synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_ups_show : Affichage des infos opérationnelles liées à l'onduleur en parametre                                       */
/* Entrée: La response response ADMIN et le numero de l'onduleur                                                            */
/* Sortie: Rien, tout est envoyé dans le pipe Admin                                                                           */
/******************************************************************************************************************************/
 static gchar *Admin_ups_show ( gchar *response, gint num )
  { GSList *liste_modules;
       
    pthread_mutex_lock ( &Cfg_ups.lib->synchro );
    liste_modules = Cfg_ups.Modules_UPS;
    while ( liste_modules )
     { struct MODULE_UPS *module;
       module = (struct MODULE_UPS *)liste_modules->data;
       if (module->ups.id == num) { response = Admin_ups_print ( response, module ); break; }
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_ups.lib->synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_ups_start: Demande le demarrage d'un onduleur en parametre                                                           */
/* Entrée: La response et le numéro d'onduleur                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_ups_start ( gchar *response, gint id )
  { gchar chaine[128];

    Cfg_ups.admin_start = id;

    g_snprintf( chaine, sizeof(chaine), " | - Module UPS %d started", id );
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_ups_stop: Demande l'arret d'un onduleur en parametre                                                                 */
/* Entrée: La response et le numéro d'onduleur                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_ups_stop ( gchar *response, gint id )
  { gchar chaine[128];

    Cfg_ups.admin_stop = id;

    g_snprintf( chaine, sizeof(chaine), " | - Module UPS %d stopped", id );
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_ups_set: Change un parametre dans la DB ups                                                                          */
/* Entrée: La response et la ligne de commande (champ valeur)                                                                */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_ups_set ( gchar *response, gchar *ligne )
  { gchar id_char[16], param[32], valeur_char[64], chaine[128];
    struct MODULE_UPS *module = NULL;
    GSList *liste_modules;
    guint id, valeur;
    gint retour;

    if ( ! strcmp ( ligne, "list" ) )
     { response = Admin_write ( response, " | Parameter can be:" );
       response = Admin_write ( response, " | - enable, host, ups, username, password, bit_comm," );
       response = Admin_write ( response, " | - map_E, map_EA, map_A" );
       return(response);
     }

    if (sscanf ( ligne, "%s %s %[^\n]", id_char, param, valeur_char ) != 3) return(response);
    id     = atoi ( id_char     );
    valeur = atoi ( valeur_char );

    pthread_mutex_lock( &Cfg_ups.lib->synchro );                                            /* Recherche du module en mémoire */
    liste_modules = Cfg_ups.Modules_UPS;
    while ( liste_modules )
     { module = (struct MODULE_UPS *)liste_modules->data;
       if (module->ups.id == id) break;
       liste_modules = liste_modules->next;                                                      /* Passage au module suivant */
     }
    pthread_mutex_unlock( &Cfg_ups.lib->synchro );

    if (!module)                                                                         /* Si non trouvé */
     { response = Admin_write( response, " Module not found\n");
       return(response);
     }

    if ( ! strcmp( param, "enable" ) )
     { module->ups.enable = (valeur ? TRUE : FALSE); }
    else if ( ! strcmp( param, "bit_comm" ) )
     { module->ups.bit_comm = valeur; }
    else if ( ! strcmp( param, "map_E" ) )
     { module->ups.map_E = valeur; }
    else if ( ! strcmp( param, "map_EA" ) )
     { module->ups.map_EA = valeur; }
    else if ( ! strcmp( param, "map_A" ) )
     { module->ups.map_A = valeur; }
    else if ( ! strcmp( param, "host" ) )
     { g_snprintf( module->ups.host, sizeof(module->ups.host), "%s", valeur_char ); }
    else if ( ! strcmp( param, "ups" ) )
     { g_snprintf( module->ups.ups, sizeof(module->ups.ups), "%s", valeur_char ); }
    else if ( ! strcmp( param, "username" ) )
     { g_snprintf( module->ups.username, sizeof(module->ups.username), "%s", valeur_char ); }
    else if ( ! strcmp( param, "password" ) )
     { g_snprintf( module->ups.password, sizeof(module->ups.password), "%s", valeur_char ); }
    else
     { g_snprintf( chaine, sizeof(chaine),
                 " | - Parameter %s not known for UPS id %s ('ups set list' can help)", param, id_char );
       response = Admin_write ( response, chaine );
       return(response);
     }

    retour = Modifier_upsDB ( &module->ups );
    if (retour)
     { response = Admin_write ( response, " | - ERROR : UPS module NOT set" ); }
    else
     { response = Admin_write ( response, " | - UPS module parameter set" ); }
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_command : Fonction principale de traitement des commandes du thread                                                  */
/* Entrée: La response et la ligne de commande a parser                                                                      */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_command ( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       response = Admin_ups_start ( response, num );
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       response = Admin_ups_stop ( response, num );
     }
    else if ( ! strcmp ( commande, "show" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                    /* Découpage de la ligne de commande */
       response = Admin_ups_show ( response, num );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { response = Admin_ups_list ( response );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { response = Admin_ups_reload(response);
     }
    else if ( ! strcmp ( commande, "add" ) )
     { struct UPSDB ups;
       gint retour;
       sscanf ( ligne, "%s %[^\n]", commande, ups.ups );
       ups.enable = TRUE;
       retour = Ajouter_upsDB ( &ups );
       if (retour == -1)
        { response = Admin_write ( response, "Error, UPS not added\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " UPS %s added. New ID=%d\n", ups.ups, retour );
          response = Admin_write ( response, chaine );
          Cfg_ups.lib->Thread_sigusr1 = TRUE;                           /* Rechargement des modules UPS en mémoire de travail */
        }
     }
    else if ( ! strcmp ( commande, "set" ) )
     { response = Admin_ups_set ( response, ligne+4 );
       Cfg_ups.lib->Thread_sigusr1 = TRUE;                              /* Rechargement des modules UPS en mémoire de travail */
     }
    else if ( ! strcmp ( commande, "del" ) )
     { struct UPSDB ups;
       gboolean retour;
       sscanf ( ligne, "%s %d", commande, &ups.id );                 /* Découpage de la ligne de commande */
       retour = Retirer_upsDB ( &ups );
       if (retour == FALSE)
        { response = Admin_write ( response, "Error, UPS not erased\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " UPS %d erased\n", ups.id );
          response = Admin_write ( response, chaine );
          Cfg_ups.lib->Thread_sigusr1 = TRUE;                           /* Rechargement des modules UPS en mémoire de travail */
        }
     }
    else if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, " | -- Watchdog ADMIN -- Help du mode 'UPS'" );
       response = Admin_write ( response, " | -  add $ups              - Ajoute un UPS" );
       response = Admin_write ( response, " | - set $id $champ $val   - Set $val to $champ for module $id" );
       response = Admin_write ( response, " | - set list              - List parameter that can be set" );
       response = Admin_write ( response, " | - del $id               - Delete UPS id" );
       response = Admin_write ( response, " | - start $id             - Start UPS id" );
       response = Admin_write ( response, " | -  stop $id              - Stop UPS id" );
       response = Admin_write ( response, " | - show $id              - Show UPS id" );
       response = Admin_write ( response, " | - list                  - Liste les modules ONDULEUR" );
       response = Admin_write ( response, " | - reload                - Recharge la configuration" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " | - Unknown NUT command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
