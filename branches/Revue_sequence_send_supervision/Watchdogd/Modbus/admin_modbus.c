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
/* Modbus_mode_to_string: Convertit le mode modbus (int) en sa version chaine de caractere                */
/* Entrée : le module_modbus                                                                              */
/* Sortie : char *mode_char                                                                               */
/**********************************************************************************************************/
 static gchar *Modbus_mode_to_string ( struct MODULE_MODBUS *module )
  { static gchar chaine[32];
    if (!module)                     return("Wrong Module   ");
    if (module->date_retente > Partage->top)
     { g_snprintf( chaine, sizeof(chaine),  "Next Try : %03ds", (module->date_retente - Partage->top)/10 );
       return(chaine);
     }
    if (!module->started)            return("Disconnected   ");

    switch ( module->mode )
     {
       case MODBUS_GET_DESCRIPTION : return("Get_Description");
       case MODBUS_GET_FIRMWARE    : return("Get_firmware   ");
       case MODBUS_INIT_WATCHDOG1  : return("Init_Watchdog_1");
       case MODBUS_INIT_WATCHDOG2  : return("Init_Watchdog_2");
       case MODBUS_INIT_WATCHDOG3  : return("Init_Watchdog_3");
       case MODBUS_INIT_WATCHDOG4  : return("Init_Watchdog_4");
       case MODBUS_GET_NBR_AI      : return("Init_Get_Nbr_AI");
       case MODBUS_GET_NBR_AO      : return("Init_Get_Nbr_AO");
       case MODBUS_GET_NBR_DI      : return("Init_Get_Nbr_DI");
       case MODBUS_GET_NBR_DO      : return("Init_Get_Nbr_DO");
       case MODBUS_GET_DI          : return("Get DI State   ");
       case MODBUS_GET_AI          : return("Get AI State   ");
       case MODBUS_SET_DO          : return("Set DO State   ");
       case MODBUS_SET_AO          : return("Set AO State   ");
       default :                     return("Unknown mode   ");
     }
  }
/**********************************************************************************************************/
/* Admin_modbus_reload: Demande le rechargement des conf MODBUS                                           */
/* Entrée: la connexion                                                                                   */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Admin_modbus_reload ( struct CONNEXION *connexion )
  { if (Cfg_modbus.lib->Thread_run == FALSE)
     { Admin_write ( connexion, " Thread MODBUS is not running\n" );
       return;
     }
    
    Cfg_modbus.reload = TRUE;
    Admin_write ( connexion, " MODBUS Reloading in progress\n" );
    while (Cfg_modbus.reload) sched_yield();
    Admin_write ( connexion, " MODBUS Reloading done\n" );
  }
/**********************************************************************************************************/
/* Admin_modbus_print: Affiche un modbus sur la connexion d'admin                                         */
/* Entrée: la connexion, le module a afficher                                                             */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static void Admin_modbus_print ( struct CONNEXION *connexion, struct MODULE_MODBUS *module )
  { gchar chaine[512];
    g_snprintf( chaine, sizeof(chaine),
                " MODBUS[%02d] ------> %s - %s\n"
                "  | - enable = %d, started = %d (bit B%04d=%d), watchdog = %03d, IP = %s\n"
                "  | - %03d Digital Input,  map_E = E%03d(->E%03d), %03d Analog  Input,  map_EA = EA%03d(->EA%03d)\n"
                "  | - %03d Digital Output, map_A = A%03d(->A%03d), %03d Analog  Output, map_AA = AA%03d(->AA%03d)\n"
                "  | - transaction_id = %06d, nbr_deconnect = %02d, last_reponse = %03ds ago, date_next_eana = in %03ds\n"
                "  -\n",
                module->modbus.id, module->modbus.libelle,  Modbus_mode_to_string(module),
                module->modbus.enable, module->started, module->modbus.bit, B(module->modbus.bit),
                module->modbus.watchdog, module->modbus.ip,
                module->nbr_entree_tor, module->modbus.map_E,
               (module->nbr_entree_tor ? module->modbus.map_E + module->nbr_entree_tor - 1 : module->modbus.map_E),
                module->nbr_entree_ana, module->modbus.map_EA, 
               (module->nbr_entree_ana ? module->modbus.map_EA + module->nbr_entree_ana - 1 : module->modbus.map_EA),
                module->nbr_sortie_tor, module->modbus.map_A, 
               (module->nbr_sortie_tor ? module->modbus.map_A + module->nbr_sortie_tor - 1 : module->modbus.map_A),
                module->nbr_sortie_ana, module->modbus.map_AA, 
               (module->nbr_sortie_ana ? module->modbus.map_AA + module->nbr_sortie_ana - 1 : module->modbus.map_AA),
                module->transaction_id, module->nbr_deconnect,
               (Partage->top - module->date_last_reponse)/10,                   
               (module->date_next_eana > Partage->top ? (module->date_next_eana - Partage->top)/10 : -1)
              );
    Admin_write ( connexion, chaine );
  }
/**********************************************************************************************************/
/* Admin_modbus_liste : Print l'ensemble des informations opérationnelles de tous les modules             */
/* Entrée: La connexion                                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_modbus_list ( struct CONNEXION *connexion )
  { GSList *liste_modules;
    gchar chaine[512];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des modules MODBUS\n" );
    Admin_write ( connexion, chaine );

    pthread_mutex_lock( &Cfg_modbus.lib->synchro );
    liste_modules = Cfg_modbus.Modules_MODBUS;
    while ( liste_modules )
     { struct MODULE_MODBUS *module;
       module = (struct MODULE_MODBUS *)liste_modules->data;
       Admin_modbus_print ( connexion, module );
       liste_modules = liste_modules->next;                                  /* Passage au module suivant */
     }
    pthread_mutex_unlock( &Cfg_modbus.lib->synchro );
  }
/**********************************************************************************************************/
/* Admin_modbus_show : Print l'ensemble des informations opérationnelles liées au module en parametre     */
/* Entrée: La connexion et le numéro de module                                                            */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_modbus_show ( struct CONNEXION *connexion, gint num )
  { GSList *liste_modules;

    pthread_mutex_lock( &Cfg_modbus.lib->synchro );
    liste_modules = Cfg_modbus.Modules_MODBUS;
    while ( liste_modules )
     { struct MODULE_MODBUS *module;
       module = (struct MODULE_MODBUS *)liste_modules->data;
       if (module->modbus.id == num) { Admin_modbus_print ( connexion, module ); break; }
       liste_modules = liste_modules->next;                                  /* Passage au module suivant */
     }
    pthread_mutex_unlock( &Cfg_modbus.lib->synchro );
  }
/**********************************************************************************************************/
/* Admin_modbus_set: Change un parametre dans la DB modbus                                                */
/* Entrée: La connexion et la ligne de commande (champ valeur)                                            */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Admin_modbus_set ( struct CONNEXION *connexion, gchar *ligne )
  { gchar id_char[16], param[32], valeur_char[64], chaine[128];
    struct MODULE_MODBUS *module = NULL;
    GSList *liste_modules;
    guint id, valeur;
    gint retour;

    if ( ! strcmp ( ligne, "list" ) )
     { Admin_write ( connexion, " | Parameter can be:\n" );
       Admin_write ( connexion, " | - enable, bit, watchdog, libelle,\n" );
       Admin_write ( connexion, " | - map_E, map_EA, map_A, map_AA\n" );
       Admin_write ( connexion, " -\n" );
       return;
     }

    sscanf ( ligne, "%s %s %[^\n]", id_char, param, valeur_char );
    id     = atoi ( id_char     );
    valeur = atoi ( valeur_char );

    pthread_mutex_lock( &Cfg_modbus.lib->synchro );                     /* Recherche du module en mémoire */
    liste_modules = Cfg_modbus.Modules_MODBUS;
    while ( liste_modules )
     { module = (struct MODULE_MODBUS *)liste_modules->data;
       if (module->modbus.id == id) break;
       liste_modules = liste_modules->next;                                  /* Passage au module suivant */
     }
    pthread_mutex_unlock( &Cfg_modbus.lib->synchro );

    if (!module)                                                                         /* Si non trouvé */
     { Admin_write( connexion, " Module not found\n");
       return;
     }

    if ( ! strcmp( param, "enable" ) )
     { module->modbus.enable = (valeur ? TRUE : FALSE); }
    else if ( ! strcmp( param, "bit" ) )
     { module->modbus.bit = valeur; }
    else if ( ! strcmp( param, "watchdog" ) )
     { module->modbus.watchdog = valeur; }
    else if ( ! strcmp( param, "map_E" ) )
     { module->modbus.map_E = valeur; }
    else if ( ! strcmp( param, "map_EA" ) )
     { module->modbus.map_EA = valeur; }
    else if ( ! strcmp( param, "map_A" ) )
     { module->modbus.map_A = valeur; }
    else if ( ! strcmp( param, "map_AA" ) )
     { module->modbus.map_AA = valeur; }
    else if ( ! strcmp( param, "libelle" ) )
     { g_snprintf( module->modbus.libelle, sizeof(module->modbus.libelle), "%s", valeur_char ); }
    else if ( ! strcmp( param, "ip" ) )
     { g_snprintf( module->modbus.ip, sizeof(module->modbus.ip), "%s", valeur_char ); }
    else
     { g_snprintf( chaine, sizeof(chaine),
                 " Parameter %s not known for MODBUS id %s ('modbus set list' can help)\n", param, id_char );
       Admin_write ( connexion, chaine );
       return;
     }

    retour = Modifier_modbusDB ( &module->modbus );
    if (retour)
     { snprintf( chaine, sizeof(chaine), " ERROR : MODBUS module parameter '%s' NOT set\n", param ); }
    else
     { snprintf( chaine, sizeof(chaine), " MODBUS module parameter '%s' set\n", param ); }
    Admin_write ( connexion, chaine );
  }
/******************************************************************************************************************************/
/* Admin_command : Fonction principale de traitement des commandes du thread                                                  */
/* Entrée: La connexion et la ligne de commande a parser                                                                      */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Admin_command ( struct CONNEXION *connexion, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       snprintf( chaine, sizeof(chaine), "%d enable 1", num );
       Admin_modbus_set ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       snprintf( chaine, sizeof(chaine), "%d enable 0", num );
       Admin_modbus_set ( connexion, chaine );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { Admin_modbus_list ( connexion );
     }
    else if ( ! strcmp ( commande, "show" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* Découpage de la ligne de commande */
       Admin_modbus_show ( connexion, num );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { Admin_modbus_reload(connexion);
     }
    else if ( ! strcmp ( commande, "add" ) )
     { struct MODBUSDB modbus;
       gint retour;

       memset( &modbus, 0, sizeof(struct MODBUSDB) );
       sscanf ( ligne, "%s %s %s", commande,                         /* Découpage de la ligne de commande */
                modbus.ip, modbus.libelle
              );
       modbus.watchdog = 40;
       retour = Ajouter_modbusDB ( &modbus );
       if (retour == -1)
        { Admin_write ( connexion, "Error, MODBUS not added\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " MODBUS %s added. New ID=%d\n", modbus.ip, retour );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "set" ) )
     { Admin_modbus_set ( connexion, ligne+4 );
     }
    else if ( ! strcmp ( commande, "del" ) )
     { struct MODBUSDB modbus;
       gboolean retour;
       sscanf ( ligne, "%s %d", commande, &modbus.id );              /* Découpage de la ligne de commande */
       retour = Retirer_modbusDB ( &modbus );
       if (retour == FALSE)
        { Admin_write ( connexion, "Error, MODBUS not erased\n" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " MODBUS %d erased\n", modbus.id );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "dbcfg" ) ) /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { if (Admin_dbcfg_thread ( connexion, NOM_THREAD, ligne+6 ) == TRUE)   /* Si changement de parametre */
        { gboolean retour;
          retour = Modbus_Lire_config();
          g_snprintf( chaine, sizeof(chaine), " Reloading Thread Parameters from Database -> %s\n",
                      (retour ? "Success" : "Failed") );
          Admin_write ( connexion, chaine );
        }
     }
    else if ( ! strcmp ( commande, "help" ) )
     { Admin_write ( connexion, "  -- Watchdog ADMIN -- Help du mode 'MODBUS'\n" );
       Admin_write ( connexion, "  dbcfg ...            - Get/Set Database Parameters\n" );
       Admin_write ( connexion, "  add $ip $libelle     - Ajoute un module modbus\n" );
       Admin_write ( connexion, "  set $id $champ $val  - Set $val to $champ for module $id\n" );
       Admin_write ( connexion, "  set list             - List parameter that can be set\n" );
       Admin_write ( connexion, "  del $id              - Erase module $id\n" );
       Admin_write ( connexion, "  start $id            - Demarre le module $id\n" );
       Admin_write ( connexion, "  stop $id             - Arrete le module $id\n" );
       Admin_write ( connexion, "  show $id             - Affiche les informations du modbus $id\n" );
       Admin_write ( connexion, "  list                 - Liste les modules MODBUS\n" );
       Admin_write ( connexion, "  reload               - Recharge la configuration\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown MODBUS command : %s\n", ligne );
       Admin_write ( connexion, chaine );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
