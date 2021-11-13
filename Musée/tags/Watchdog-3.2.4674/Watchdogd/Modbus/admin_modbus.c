/******************************************************************************************************************************/
/* Watchdogd/Admin/admin_modbus.c        Gestion des responses Admin MODBUS au serveur watchdog                              */
/* Projet WatchDog version 3.0       Gestion d'habitat                                       dim. 05 sept. 2010 12:01:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_modbus.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

 extern struct MODBUS_CONFIG Cfg_modbus;

/******************************************************************************************************************************/
/* Modbus_mode_to_string: Convertit le mode modbus (int) en sa version chaine de caractere                                    */
/* Entrée : le module_modbus                                                                                                  */
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
/* Admin_json_list : fonction appelée pour lister les modules modbus                                                          */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 static void Admin_json_list ( JsonBuilder *builder )
  { GSList *liste_modules;
    json_builder_begin_array (builder);                                                                  /* Contenu du Status */

    pthread_mutex_lock( &Cfg_modbus.lib->synchro );
    liste_modules = Cfg_modbus.Modules_MODBUS;
    while ( liste_modules )
     { struct MODULE_MODBUS *module = liste_modules->data;

       json_builder_begin_object (builder);                                                    /* Contenu du Noeud Passerelle */
       Json_add_string ( builder, "tech_id", module->modbus.tech_id );
       Json_add_string ( builder, "mode", Modbus_mode_to_string(module) );
       Json_add_bool   ( builder, "started", module->started );
       Json_add_int    ( builder, "nbr_entree_tor", module->nbr_entree_tor );
       Json_add_int    ( builder, "nbr_sortie_tor", module->nbr_sortie_tor );
       Json_add_int    ( builder, "nbr_entree_ana", module->nbr_entree_ana );
       Json_add_int    ( builder, "nbr_sortie_ana", module->nbr_sortie_ana );
       Json_add_bool   ( builder, "comm", Dls_data_get_bool( NULL, NULL, &module->bit_comm) );
       Json_add_int    ( builder, "transaction_id", module->transaction_id );
       Json_add_int    ( builder, "nbr_request_par_sec", module->nbr_request_par_sec );
       Json_add_int    ( builder, "delai", module->delai );
       Json_add_int    ( builder, "nbr_deconnect", module->nbr_deconnect );
       Json_add_int    ( builder, "last_reponse", (Partage->top - module->date_last_reponse)/10 );
       Json_add_int    ( builder, "date_next_eana", (module->date_next_eana > Partage->top ? (module->date_next_eana - Partage->top)/10 : -1) );
       Json_add_int    ( builder, "date_retente", (module->date_retente > Partage->top   ? (module->date_retente   - Partage->top)/10 : -1) );

       json_builder_end_object (builder);                                                                 /* End Module Array */

       liste_modules = liste_modules->next;                                                      /* Passage au module suivant */
     }
    pthread_mutex_unlock( &Cfg_modbus.lib->synchro );

    json_builder_end_array (builder);                                                                         /* End Document */
  }
/******************************************************************************************************************************/
/* Admin_json : fonction appelé par le thread http lors d'une requete /run/                                                   */
/* Entrée : les adresses d'un buffer json et un entier pour sortir sa taille                                                  */
/* Sortie : les parametres d'entrée sont mis à jour                                                                           */
/******************************************************************************************************************************/
 void Admin_json ( gchar *commande, gchar **buffer_p, gsize *taille_p )
  { JsonBuilder *builder;

    *buffer_p = NULL;
    *taille_p = 0;

    builder = Json_create ();
    if (builder == NULL)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "%s : JSon builder creation failed", __func__ );
       return;
     }
/************************************************ Préparation du buffer JSON **************************************************/
                                                                      /* Lancement de la requete de recuperation des messages */
    if (!strcmp(commande, "/list")) { Admin_json_list ( builder ); }

/************************************************ Génération du JSON **********************************************************/
    *buffer_p = Json_get_buf ( builder, taille_p );
    return;
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
