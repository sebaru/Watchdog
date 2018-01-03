/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_modbus.c        Gestion des responses Admin MODBUS au serveur watchdog                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                                       dim. 05 sept. 2010 12:01:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

/******************************************************************************************************************************/
/* Modbus_mode_to_string: Convertit le mode modbus (int) en sa version chaine de caractere                                    */
/* Entr�e : le module_modbus                                                                                                  */
/* Sortie : char *mode_char                                                                                                   */
/******************************************************************************************************************************/
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
/******************************************************************************************************************************/
/* Admin_modbus_reload: Demande le rechargement des conf MODBUS                                                               */
/* Entr�e: la response                                                                                                       */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static gchar *Admin_modbus_reload ( gchar *response )
  { if (Cfg_modbus.lib->Thread_run == FALSE)
     { return(Admin_write ( response, " Thread MODBUS is not running" )); }
    
    Cfg_modbus.reload = TRUE;
    while (Cfg_modbus.reload) sched_yield();
    return(Admin_write ( response, " MODBUS Reload done" ));
  }
/******************************************************************************************************************************/
/* Admin_modbus_print: Affiche un modbus sur la response d'admin                                                             */
/* Entr�e: la response, le module a afficher                                                                                 */
/* Sortie: FALSE si erreur                                                                                                    */
/******************************************************************************************************************************/
 static gchar *Admin_modbus_print ( gchar *response, struct MODULE_MODBUS *module )
  { gchar chaine[512];
    g_snprintf( chaine, sizeof(chaine),
                " |---------------------------\n"
                " | MODBUS[%02d] ------> %s - %s\n"
                " | - enable = %d, started = %d (bit B%04d=%d), watchdog = %03d, IP = %s\n"
                " | - %03d Digital Input,  map_E = E%03d(->E%03d), %03d Analog  Input,  map_EA = EA%03d(->EA%03d)\n"
                " | - %03d Digital Output, map_A = A%03d(->A%03d), %03d Analog  Output, map_AA = AA%03d(->AA%03d)\n"
                " | - transaction_id = %06d, nbr_deconnect = %02d\n"
                " | - last_reponse = %03ds ago, date_next_eana = in %03ds\n"
                " | - retente = in %03ds\n",
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
               (module->date_next_eana > Partage->top ? (module->date_next_eana - Partage->top)/10 : -1),
               (module->date_retente > Partage->top   ? (module->date_retente   - Partage->top)/10 : -1)
              );
    return(Admin_write ( response, chaine ));
  }
/******************************************************************************************************************************/
/* Admin_modbus_liste : Print l'ensemble des informations op�rationnelles de tous les modules                                 */
/* Entr�e: La response                                                                                                       */
/* Sortie: N�ant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_modbus_list ( gchar *response )
  { GSList *liste_modules;
    gchar chaine[512];

    g_snprintf( chaine, sizeof(chaine), " -- Liste des modules MODBUS" );
    response = Admin_write ( response, chaine );

    pthread_mutex_lock( &Cfg_modbus.lib->synchro );
    liste_modules = Cfg_modbus.Modules_MODBUS;
    while ( liste_modules )
     { struct MODULE_MODBUS *module;
       module = (struct MODULE_MODBUS *)liste_modules->data;
       response = Admin_modbus_print ( response, module );
       liste_modules = liste_modules->next;                                                      /* Passage au module suivant */
     }
    pthread_mutex_unlock( &Cfg_modbus.lib->synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_modbus_show : Print l'ensemble des informations op�rationnelles li�es au module en parametre                         */
/* Entr�e: La response et le num�ro de module                                                                                */
/* Sortie: N�ant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_modbus_show ( gchar *response, gint num )
  { GSList *liste_modules;

    pthread_mutex_lock( &Cfg_modbus.lib->synchro );
    liste_modules = Cfg_modbus.Modules_MODBUS;
    while ( liste_modules )
     { struct MODULE_MODBUS *module;
       module = (struct MODULE_MODBUS *)liste_modules->data;
       if (module->modbus.id == num) { response = Admin_modbus_print ( response, module ); break; }
       liste_modules = liste_modules->next;                                                      /* Passage au module suivant */
     }
    pthread_mutex_unlock( &Cfg_modbus.lib->synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_modbus_set: Change un parametre dans la DB modbus                                                                    */
/* Entr�e: La response et la ligne de commande (champ valeur)                                                                */
/* Sortie: N�ant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_modbus_set ( gchar *response, gchar *ligne )
  { gchar id_char[16], param[32], valeur_char[64], chaine[128];
    struct MODULE_MODBUS *module = NULL;
    GSList *liste_modules;
    guint id, valeur;
    gint retour;

    if ( ! strcmp ( ligne, "list" ) )
     { response = Admin_write ( response, " | Parameter can be:" );
       response = Admin_write ( response, " | - enable, bit, watchdog, libelle," );
       response = Admin_write ( response, " | - map_E, map_EA, map_A, map_AA" );
       return(response);
     }

    sscanf ( ligne, "%s %s %[^]", id_char, param, valeur_char );
    id     = atoi ( id_char     );
    valeur = atoi ( valeur_char );

    pthread_mutex_lock( &Cfg_modbus.lib->synchro );                                         /* Recherche du module en m�moire */
    liste_modules = Cfg_modbus.Modules_MODBUS;
    while ( liste_modules )
     { module = (struct MODULE_MODBUS *)liste_modules->data;
       if (module->modbus.id == id) break;
       liste_modules = liste_modules->next;                                                      /* Passage au module suivant */
     }
    pthread_mutex_unlock( &Cfg_modbus.lib->synchro );

    if (!module)                                                                                             /* Si non trouv� */
     { response = Admin_write ( response, " | - Module not found" );
       return(response);
     }

    if ( ! strcmp( param, "enable" ) )
     { if (valeur)
        { module->modbus.enable = TRUE;
          module->nbr_deconnect  = 0;
        }
       else { module->modbus.enable = FALSE; }
     }
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
                 " Parameter %s not known for MODBUS id %s ('modbus set list' can help)", param, id_char );
       response = Admin_write ( response, chaine );
       return(response);
     }

    retour = Modifier_modbusDB ( &module->modbus );
    if (retour)
     { snprintf( chaine, sizeof(chaine), " | - ERROR : MODBUS module parameter '%s' NOT set", param ); }
    else
     { snprintf( chaine, sizeof(chaine), " | - MODBUS module parameter '%s' set", param ); }
    response = Admin_write ( response, chaine );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_command : Fonction principale de traitement des commandes du thread                                                  */
/* Entr�e: La response et la ligne de commande a parser                                                                      */
/* Sortie: N�ant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_command ( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* D�coupage de la ligne de commande */

    if ( ! strcmp ( commande, "start" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* D�coupage de la ligne de commande */
       snprintf( chaine, sizeof(chaine), "%d enable 1", num );
       response = Admin_modbus_set ( response, chaine );
     }
    else if ( ! strcmp ( commande, "stop" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* D�coupage de la ligne de commande */
       snprintf( chaine, sizeof(chaine), "%d enable 0", num );
       response = Admin_modbus_set ( response, chaine );
     }
    else if ( ! strcmp ( commande, "list" ) )
     { response = Admin_modbus_list ( response );
     }
    else if ( ! strcmp ( commande, "show" ) )
     { int num;
       sscanf ( ligne, "%s %d", commande, &num );                                        /* D�coupage de la ligne de commande */
       response = Admin_modbus_show ( response, num );
     }
    else if ( ! strcmp ( commande, "reload" ) )
     { response = Admin_modbus_reload(response);
     }
    else if ( ! strcmp ( commande, "add" ) )
     { struct MODBUSDB modbus;
       gint retour;

       memset( &modbus, 0, sizeof(struct MODBUSDB) );
       if (sscanf ( ligne, "%s %s %s", commande,                                         /* D�coupage de la ligne de commande */
                    modbus.ip, modbus.libelle
                  ) != 3) return(response);
       modbus.watchdog = 40;
       retour = Ajouter_modbusDB ( &modbus );
       if (retour == -1)
        { response = Admin_write ( response, " | - Error, MODBUS not added" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " | - MODBUS %s added. New ID=%d", modbus.ip, retour );
          response = Admin_write ( response, chaine );
        }
     }
    else if ( ! strcmp ( commande, "set" ) )
     { response = Admin_modbus_set ( response, ligne+4 );
     }
    else if ( ! strcmp ( commande, "del" ) )
     { struct MODBUSDB modbus;
       gboolean retour;
       if (sscanf ( ligne, "%s %d", commande, &modbus.id ) != 2) return(response);       /* D�coupage de la ligne de commande */
       retour = Retirer_modbusDB ( &modbus );
       if (retour == FALSE)
        { response = Admin_write ( response, " | - Error, MODBUS not erased" ); }
       else
        { gchar chaine[80];
          g_snprintf( chaine, sizeof(chaine), " | -MODBUS %d erased", modbus.id );
          response = Admin_write ( response, chaine );
        }
     }
    else if ( ! strcmp ( commande, "dbcfg" ) )                /* Appelle de la fonction d�di�e � la gestion des parametres DB */
     { gboolean retour;
       response =  Admin_dbcfg_thread ( response, NOM_THREAD, ligne+6 );                        /* Si changement de parametre */
       retour = Modbus_Lire_config();
       g_snprintf( chaine, sizeof(chaine), " | - Reloading Thread Parameters from Database -> %s", (retour ? "Success" : "Failed") );
       response = Admin_write ( response, chaine );
     }
    else if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'MODBUS'" );
       response = Admin_write ( response, "  dbcfg ...            - Get/Set Database Parameters" );
       response = Admin_write ( response, "  add $ip $libelle     - Ajoute un module modbus" );
       response = Admin_write ( response, "  set $id $champ $val  - Set $val to $champ for module $id" );
       response = Admin_write ( response, "  set list             - List parameter that can be set" );
       response = Admin_write ( response, "  del $id              - Erase module $id" );
       response = Admin_write ( response, "  start $id            - Demarre le module $id" );
       response = Admin_write ( response, "  stop $id             - Arrete le module $id" );
       response = Admin_write ( response, "  show $id             - Affiche les informations du modbus $id" );
       response = Admin_write ( response, "  list                 - Liste les modules MODBUS" );
       response = Admin_write ( response, "  reload               - Recharge la configuration" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " | - Unknown MODBUS command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
