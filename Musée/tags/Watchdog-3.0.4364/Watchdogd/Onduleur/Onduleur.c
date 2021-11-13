/******************************************************************************************************************************/
/* Watchdogd/Onduleur/Onduleur.c  Gestion des modules UPS Watchdgo 2.0                                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mar. 10 nov. 2009 15:56:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Onduleur.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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

 #include <stdio.h>
 #include <sys/prctl.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <upsclient.h>
 #include <locale.h>

/*********************************************************** Headers **********************************************************/
 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Onduleur.h"

/******************************************************************************************************************************/
/* Ups_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                    */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Ups_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_ups.lib->Thread_debug = FALSE;                                                         /* Settings default parameters */
    Cfg_ups.enable            = FALSE;

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_INFO,                                             /* Print Config */
                "Ups_Lire_config: '%s' = %s", nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_ups.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_ups.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
                   "%s: Unknown Parameter '%s'(='%s') in Database", __func__, nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Ups_send_status_to_master: Envoie le bit de comm au master selon le status du GSM                                          */
/* Entrée: le status du GSM                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Ups_send_status_to_master ( struct MODULE_UPS *ups, gboolean status )
  { if (Config.instance_is_master==TRUE)                                                        /* si l'instance est Maitre */
     { Dls_data_set_bool ( ups->tech_id, "COMM", &ups->bit_comm, status ); }                              /* Communication OK */
/*    else
     { 
     }*/
  }
/******************************************************************************************************************************/
/* Recuperer_liste_id_MODULE_UPS: Recupération de la liste des ids des upss                                                   */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 static struct DB *Recuperer_ups ( void )
  { gchar requete[512];
    struct DB *db;

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s: DB connexion failed", __func__ );
       return(NULL);
     }

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT id,tech_id,host,name,username,password,enable "
                " FROM %s ORDER BY host,name", NOM_TABLE_UPS );

    if (Lancer_requete_SQL ( db, requete ) == FALSE)                                           /* Execution de la requete SQL */
     { Libere_DB_SQL (&db);
       return(NULL);
     }
    return(db);
  }
/******************************************************************************************************************************/
/* Recuperer_liste_id_MODULE_UPS: Recupération de la liste des ids des upss                                                   */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 static struct MODULE_UPS *Recuperer_ups_suite( struct DB *db )
  { struct MODULE_UPS *ups;

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( db );
       return(NULL);
     }

    ups = (struct MODULE_UPS *)g_try_malloc0( sizeof(struct MODULE_UPS) );
    if (!ups) Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s: Erreur allocation mémoire", __func__ );
    else
     { g_snprintf( ups->tech_id,  sizeof(ups->tech_id),  "%s", db->row[1] );
       g_snprintf( ups->host,     sizeof(ups->host),     "%s", db->row[2] );
       g_snprintf( ups->name,     sizeof(ups->name),     "%s", db->row[3] );
       g_snprintf( ups->username, sizeof(ups->username), "%s", db->row[4] );
       g_snprintf( ups->password, sizeof(ups->password), "%s", db->row[5] );
       ups->enable            = atoi(db->row[6]);
       ups->id                = atoi(db->row[0]);
     }
    return(ups);
  }
/******************************************************************************************************************************/
/* Charger_tous_ups: Requete la DB pour charger les modules ups                                                               */
/* Entrée: rien                                                                                                               */
/* Sortie: le nombre de modules trouvé                                                                                        */
/******************************************************************************************************************************/
 static gboolean Charger_tous_ups ( void  )
  { struct MODULE_UPS *module;
    struct DB *db;
    gint cpt;

/**************************************************** Chargement des modules **************************************************/
    if ( (db = Recuperer_ups()) == NULL )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "%s: Recuperer_ups Failed", __func__ );
       return(FALSE);
     }

    Cfg_ups.Modules_UPS = NULL;
    cpt = 0;
    while ( (module = Recuperer_ups_suite( db )) != NULL )
     { cpt++;                                                                  /* Nous avons ajouté un module dans la liste ! */
                                                                                            /* Ajout dans la liste de travail */
       pthread_mutex_lock( &Cfg_ups.lib->synchro );
       Cfg_ups.Modules_UPS = g_slist_prepend ( Cfg_ups.Modules_UPS, module );
       pthread_mutex_unlock( &Cfg_ups.lib->synchro );
       Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_INFO,
                "%s: tech_id='%s', host=%s, name=%s", __func__,
                 module->tech_id, module->host, module->name );
     }
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_INFO, "%s: %03d UPS found  !", __func__, cpt );

    Libere_DB_SQL( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Rechercher_MODULE_UPS: Recupération du ups dont le num est en parametre                                                    */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 static void Decharger_un_UPS ( struct MODULE_UPS *module )
  { if (!module) return;
    pthread_mutex_lock( &Cfg_ups.lib->synchro );
    Cfg_ups.Modules_UPS = g_slist_remove ( Cfg_ups.Modules_UPS, module );
    g_free(module);
    pthread_mutex_unlock( &Cfg_ups.lib->synchro );
  }
/******************************************************************************************************************************/
/* Decharger_tous_Decharge l'ensemble des modules UPS                                                                         */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Decharger_tous_UPS ( void  )
  { struct MODULE_UPS *module;
    while ( Cfg_ups.Modules_UPS )
     { module = (struct MODULE_UPS *)Cfg_ups.Modules_UPS->data;
       Decharger_un_UPS ( module );
     }
  }
/******************************************************************************************************************************/
/* Deconnecter: Deconnexion du module                                                                                         */
/* Entrée: un id                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Deconnecter_module ( struct MODULE_UPS *module )
  { if (!module) return;

    if (module->started == TRUE)
     { upscli_disconnect( &module->upsconn );
       module->started = FALSE;
     }

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
              "%s: %s disconnected (host='%s')", __func__, module->tech_id, module->host );
    Ups_send_status_to_master ( module, FALSE );
  }
/******************************************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                                               */
/* Entrée: une nom et un password                                                                                             */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                                              */
/******************************************************************************************************************************/
 static gboolean Connecter_ups ( struct MODULE_UPS *module )
  { gchar buffer[80];
    int connexion;

    if ( (connexion = upscli_connect( &module->upsconn, module->host, UPS_PORT_TCP, UPSCLI_CONN_TRYSSL)) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: connexion refused by module (host '%s' -> %s)", __func__, module->tech_id,
                 module->host, (char *)upscli_strerror(&module->upsconn) );
       return(FALSE);
     }

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
              "%s: %s connected (host='%s')", __func__, module->tech_id, module->host );
/********************************************************* UPSDESC ************************************************************/
    g_snprintf( buffer, sizeof(buffer), "GET UPSDESC %s\n", module->name );
    if ( upscli_sendline( &module->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Sending GET UPSDESC failed (%s)", __func__, module->tech_id,
                (char *)upscli_strerror(&module->upsconn) );
     }
    else
     { if ( upscli_readline( &module->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                   "%s: %s: Reading GET UPSDESC failed (%s)", __func__, module->tech_id,
                   (char *)upscli_strerror(&module->upsconn) );
        }
       else
        { g_snprintf( module->libelle, sizeof(module->libelle), "%s", buffer + strlen(module->name) + 9 );
          Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                   "%s: %s: Reading GET UPSDESC %s", __func__, module->tech_id,
                   module->libelle );
        }
     }
/**************************************************** USERNAME ****************************************************************/
    g_snprintf( buffer, sizeof(buffer), "USERNAME %s\n", module->username );
    if ( upscli_sendline( &module->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Sending USERNAME failed %s", __func__, module->tech_id,
                (char *)upscli_strerror(&module->upsconn) );
     }
    else
     { if ( upscli_readline( &module->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                   "%s: %s: Reading USERNAME failed %s", __func__, module->tech_id,
                   (char *)upscli_strerror(&module->upsconn) );
        }
       else
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                   "%s: %s: Reading USERNAME %s", __func__, module->tech_id,
                    buffer );
        }
     }

/******************************************************* PASSWORD *************************************************************/
    g_snprintf( buffer, sizeof(buffer), "PASSWORD %s\n", module->password );
    if ( upscli_sendline( &module->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Sending PASSWORD failed %s", __func__, module->tech_id,
                (char *)upscli_strerror(&module->upsconn) );
     }
    else
     { if ( upscli_readline( &module->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                   "%s: %s: Reading PASSWORD failed %s", __func__, module->tech_id,
                   (char *)upscli_strerror(&module->upsconn) );
        }
       else
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                   "%s: %s: Reading PASSWORD %s", __func__, module->tech_id,
                   buffer );
        }
     }

/************************************************** PREPARE LES AI ************************************************************/
    if (!module->nbr_connexion)
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_INFO,
                "%s: %s: Initialise le DLS et charge les AI ", __func__, module->tech_id );
       if (Dls_auto_create_plugin( module->tech_id, "Gestion de l'onduleur" ) == -1)
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", module->tech_id ); }

       Mnemo_auto_create_DI ( module->tech_id, "COMM", "Statut de la communication avec l'onduleur" );
       Mnemo_auto_create_DI ( module->tech_id, "OUTLET_1_STATUS", "Statut de la prise n°1" );
       Mnemo_auto_create_DI ( module->tech_id, "OUTLET_2_STATUS", "Statut de la prise n°2" );
       Mnemo_auto_create_DI ( module->tech_id, "UPS_ONLINE", "UPS Online" );
       Mnemo_auto_create_DI ( module->tech_id, "UPS_CHARGING", "UPS en charge" );
       Mnemo_auto_create_DI ( module->tech_id, "UPS_ON_BATT",  "UPS sur batterie" );

       Mnemo_auto_create_AI ( module->tech_id, "LOAD", "Charge onduleur", "%" );
       Dls_data_set_AI ( module->tech_id, "LOAD", &module->ai_load, 0.0 );
       Charger_conf_AI ( module->ai_load );

       Mnemo_auto_create_AI ( module->tech_id, "REALPOWER", "Charge onduleur", "W" );
       Dls_data_set_AI ( module->tech_id, "REALPOWER", &module->ai_realpower, 0.0 );
       Charger_conf_AI ( module->ai_realpower );

       Mnemo_auto_create_AI ( module->tech_id, "BATTERY_CHARGE", "Charge batterie", "%" );
       Dls_data_set_AI ( module->tech_id, "BATTERY_CHARGE", &module->ai_battery_charge, 0.0 );
       Charger_conf_AI ( module->ai_battery_charge );

       Mnemo_auto_create_AI ( module->tech_id, "INPUT_VOLTAGE", "Tension d'entrée", "V" );
       Dls_data_set_AI ( module->tech_id, "INPUT_VOLTAGE", &module->ai_input_voltage, 0.0 );
       Charger_conf_AI ( module->ai_input_voltage );

       Mnemo_auto_create_AI ( module->tech_id, "BATTERY_RUNTIME", "Durée de batterie restante", "s" );
       Dls_data_set_AI ( module->tech_id, "BATTERY_RUNTIME", &module->ai_battery_runtime, 0.0 );
       Charger_conf_AI ( module->ai_battery_runtime );

       Mnemo_auto_create_AI ( module->tech_id, "BATTERY_VOLTAGE", "Tension batterie", "V" );
       Dls_data_set_AI ( module->tech_id, "BATTERY_VOLTAGE", &module->ai_battery_voltage, 0.0 );
       Charger_conf_AI ( module->ai_battery_voltage );

       Mnemo_auto_create_AI ( module->tech_id, "INPUT_HZ", "Fréquence d'entrée", "HZ" );
       Dls_data_set_AI ( module->tech_id, "INPUT_HZ", &module->ai_input_frequency, 0.0 );
       Charger_conf_AI ( module->ai_input_frequency );

       Mnemo_auto_create_AI ( module->tech_id, "OUTPUT_CURRENT", "Courant de sortie", "A" );
       Dls_data_set_AI ( module->tech_id, "OUTPUT_CURRENT", &module->ai_output_current, 0.0 );
       Charger_conf_AI ( module->ai_output_current );

       Mnemo_auto_create_AI ( module->tech_id, "OUTPUT_HZ", "Fréquence de sortie", "HZ" );
       Dls_data_set_AI ( module->tech_id, "OUTPUT_HZ", &module->ai_output_frequency, 0.0 );
       Charger_conf_AI ( module->ai_output_frequency );

       Mnemo_auto_create_AI ( module->tech_id, "OUTPUT_VOLTAGE", "Tension de sortie", "V" );
       Dls_data_set_AI ( module->tech_id, "OUTPUT_VOLTAGE", &module->ai_output_voltage, 0.0 );
       Charger_conf_AI ( module->ai_output_voltage );

       Mnemo_auto_create_DO ( module->tech_id, "LOAD_OFF", "Coupe la sortie ondulée" );
       Dls_data_set_DO ( module->tech_id, "LOAD_OFF", &module->do_load_off, 0.0 );

       Mnemo_auto_create_DO ( module->tech_id, "LOAD_ON", "Active la sortie ondulée" );
       Dls_data_set_DO ( module->tech_id, "LOAD_ON", &module->do_load_on, 0.0 );

       Mnemo_auto_create_DO ( module->tech_id, "OUTLET_1_OFF", "Désactive la prise n°1" );
       Dls_data_set_DO ( module->tech_id, "OUTLET_1_OFF", &module->do_outlet_1_off, 0.0 );

       Mnemo_auto_create_DO ( module->tech_id, "OUTLET_1_ON", "Active la prise n°1" );
       Dls_data_set_DO ( module->tech_id, "OUTLET_1_ON", &module->do_outlet_1_on, 0.0 );

       Mnemo_auto_create_DO ( module->tech_id, "OUTLET_2_OFF", "Désactive la prise n°2" );
       Dls_data_set_DO ( module->tech_id, "OUTLET_2_OFF", &module->do_outlet_2_off, 0.0 );

       Mnemo_auto_create_DO ( module->tech_id, "OUTLET_2_ON", "Active la prise n°2" );
       Dls_data_set_DO ( module->tech_id, "OUTLET_2_ON", &module->do_outlet_2_on, 0.0 );

       Mnemo_auto_create_DO ( module->tech_id, "START_DEEP_BAT", "Active un test de decharge profond" );
       Dls_data_set_DO ( module->tech_id, "START_DEEP_BAT", &module->do_start_deep_bat, 0.0 );

       Mnemo_auto_create_DO ( module->tech_id, "START_QUICK_BAT", "Active un test de decharge léger" );
       Dls_data_set_DO ( module->tech_id, "START_QUICK_BAT", &module->do_start_quick_bat, 0.0 );

       Mnemo_auto_create_DO ( module->tech_id, "STOP_TEST_BAT", "Stop le test de décharge batterie" );
       Dls_data_set_DO ( module->tech_id, "STOP_TEST_BAT", &module->do_start_quick_bat, 0.0 );
     }

    module->date_next_connexion = 0;
    module->started = TRUE;
    module->nbr_connexion++;
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
              "%s: %s up and running (host='%s')", __func__, module->tech_id, module->host );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Onduleur_set_instcmd: Envoi d'une instant commande à l'ups                                                                 */
/* Entrée : l'ups, le nom de la commande                                                                                      */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                                          */
/******************************************************************************************************************************/
 gboolean Onduleur_set_instcmd ( struct MODULE_UPS *module, gchar *nom_cmd )
  { gchar buffer[80];

    if (module->started != TRUE) return(FALSE);

    g_snprintf( buffer, sizeof(buffer), "INSTCMD %s %s\n", module->name, nom_cmd );
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG, "%s: %s: Sending '%s'", __func__, module->tech_id, buffer );
    if ( upscli_sendline( &module->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                 "%s: %s: Sending INSTCMD failed (%s) error %s", __func__, module->tech_id,
                 buffer, (char *)upscli_strerror(&module->upsconn) );
       Deconnecter_module ( module );
       return(FALSE);
     }

    if ( upscli_readline( &module->upsconn, buffer, sizeof(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Reading INSTCMD result failed (%s) error %s", __func__, module->tech_id,
                 nom_cmd, (char *)upscli_strerror(&module->upsconn) );
       Deconnecter_module ( module );
       return(FALSE);
     }
    else
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
                "%s: %s: Sending '%s' OK", __func__, module->tech_id, buffer );
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Onduleur_get_var: Recupere une valeur de la variable en parametre                                                          */
/* Entrée : l'ups, le nom de variable, la variable a renseigner                                                               */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                                          */
/******************************************************************************************************************************/
 static gchar *Onduleur_get_var ( struct MODULE_UPS *module, gchar *nom_var )
  { static gchar buffer[80];
    gint retour_read;

    if (module->started != TRUE) return(NULL);

    g_snprintf( buffer, sizeof(buffer), "GET VAR %s %s\n", module->name, nom_var );
    if ( upscli_sendline( &module->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Sending GET VAR failed (%s) error=%s", __func__, module->tech_id,
                buffer, (char *)upscli_strerror(&module->upsconn) );
       Deconnecter_module ( module );
       return(NULL);
     }

    retour_read = upscli_readline( &module->upsconn, buffer, sizeof(buffer) );
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
             "%s: %s: Reading GET VAR %s ReadLine result = %d, upscli_upserror = %d, buffer = %s", __func__, module->tech_id,
              nom_var, retour_read, upscli_upserror(&module->upsconn), buffer );
    if ( retour_read == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Reading GET VAR result failed (%s) error=%s", __func__, module->tech_id,
                 nom_var, (char *)upscli_strerror(&module->upsconn) );
       return(NULL);
     }

    if ( ! strncmp ( buffer, "VAR", 3 ) )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                "%s: %s: Reading GET VAR %s OK = %s", __func__, module->tech_id, nom_var, buffer );
       return(buffer + 6 + strlen(module->name) + strlen(nom_var));
     }

    if ( ! strcmp ( buffer, "ERR VAR-NOT-SUPPORTED" ) )
     { return(NULL);                                                         /* Variable not supported... is not an error ... */
     }

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
             "%s: %s: Reading GET VAR %s Failed : error %s (buffer %s)", __func__, module->tech_id,
              nom_var, (char *)upscli_strerror(&module->upsconn), buffer );
    return(NULL);
  }
/******************************************************************************************************************************/
/* Envoyer_sortie_ups: Envoi des sorties/InstantCommand Ã  l'ups                                                               */
/* Entrée: identifiants des modules ups                                                                                       */
/* Sortie: TRUE si pas de probleme, FALSE sinon                                                                               */
/******************************************************************************************************************************/
 static gboolean Envoyer_sortie_ups( struct MODULE_UPS *module )
  { if (Dls_data_get_bool_up ( module->tech_id, "LOAD_OFF", &module->do_load_off))
     { if (Onduleur_set_instcmd ( module, "load.off" ) == FALSE) return(FALSE); }

    if (Dls_data_get_bool_up ( module->tech_id, "LOAD_ON", &module->do_load_on))
     { if (Onduleur_set_instcmd ( module, "load.on" ) == FALSE) return(FALSE); }

    if (Dls_data_get_bool_up ( module->tech_id, "OUTLET_1_OFF", &module->do_outlet_1_off))
     { if (Onduleur_set_instcmd ( module, "outlet.1.load.off" ) == FALSE) return(FALSE); }

    if (Dls_data_get_bool_up ( module->tech_id, "OUTLET_1_ON", &module->do_outlet_1_on))
     { if (Onduleur_set_instcmd ( module, "outlet.1.load.on" ) == FALSE) return(FALSE); }

    if (Dls_data_get_bool_up ( module->tech_id, "OUTLET_2_OFF", &module->do_outlet_2_off))
     { if (Onduleur_set_instcmd ( module, "outlet.2.load.off" ) == FALSE) return(FALSE); }

    if (Dls_data_get_bool_up ( module->tech_id, "OUTLET_2_ON", &module->do_outlet_2_on))
     { if (Onduleur_set_instcmd ( module, "outlet.2.load.on" ) == FALSE) return(FALSE); }

    if (Dls_data_get_bool_up ( module->tech_id, "START_DEEP_BAT", &module->do_start_deep_bat))
     { if (Onduleur_set_instcmd ( module, "test.battery.start.deep" ) == FALSE) return(FALSE); }

    if (Dls_data_get_bool_up ( module->tech_id, "START_QUICK_BAT", &module->do_start_quick_bat))
     { if (Onduleur_set_instcmd ( module, "test.battery.start.quick" ) == FALSE) return(FALSE); }

    if (Dls_data_get_bool_up ( module->tech_id, "STOP_TEST_BAT", &module->do_stop_test_bat))
     { if (Onduleur_set_instcmd ( module, "test.battery.stop" ) == FALSE) return(FALSE); }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Interroger_ups: Interrogation d'un ups                                                                                     */
/* Entrée: identifiants des modules ups                                                                                       */
/* Sortie: TRUE si pas de probleme, FALSE sinon                                                                               */
/******************************************************************************************************************************/
 static gboolean Interroger_ups( struct MODULE_UPS *module )
  { gchar *reponse;

    if ( (reponse = Onduleur_get_var ( module, "ups.load" )) != NULL )
     { Dls_data_set_AI ( module->tech_id, "LOAD", &module->ai_load, atof(reponse+1) ); }

    if ( (reponse = Onduleur_get_var ( module, "ups.realpower" )) != NULL )
     { Dls_data_set_AI ( module->tech_id, "REALPOWER", &module->ai_realpower, atof(reponse+1) ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.charge" )) != NULL )
     { Dls_data_set_AI ( module->tech_id, "BATTERY_CHARGE", &module->ai_battery_charge, atof(reponse+1) ); }

    if ( (reponse = Onduleur_get_var ( module, "input.voltage" )) != NULL )
     { Dls_data_set_AI ( module->tech_id, "INPUT_VOLTAGE", &module->ai_input_voltage, atof(reponse+1) ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.runtime" )) != NULL )
     { Dls_data_set_AI ( module->tech_id, "BATTERY_RUNTIME", &module->ai_battery_runtime, atof(reponse+1) ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.voltage" )) != NULL )
     { Dls_data_set_AI ( module->tech_id, "BATTERY_VOLTAGE", &module->ai_battery_voltage, atof(reponse+1) ); }

    if ( (reponse = Onduleur_get_var ( module, "input.frequency" )) != NULL )
     { Dls_data_set_AI ( module->tech_id, "INPUT_HZ", &module->ai_input_frequency, atof(reponse+1) ); }

    if ( (reponse = Onduleur_get_var ( module, "output.current" )) != NULL )
     { Dls_data_set_AI ( module->tech_id, "OUTPUT_CURRENT", &module->ai_output_current, atof(reponse+1) ); }

    if ( (reponse = Onduleur_get_var ( module, "output.frequency" )) != NULL )
     { Dls_data_set_AI ( module->tech_id, "OUTPUT_HZ", &module->ai_output_frequency, atof(reponse+1) ); }

    if ( (reponse = Onduleur_get_var ( module, "output.voltage" )) != NULL )
     { Dls_data_set_AI ( module->tech_id, "OUTPUT_VOLTAGE", &module->ai_output_voltage, atof(reponse+1) ); }

/*---------------------------------------------- Récupération des entrées TOR de l'UPS ---------------------------------------*/
    if ( (reponse = Onduleur_get_var ( module, "outlet.1.status" )) != NULL )
     { Dls_data_set_bool ( module->tech_id, "OUTLET_1_STATUS", &module->di_outlet_1_status, !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = Onduleur_get_var ( module, "outlet.2.status" )) != NULL )
     { Dls_data_set_bool ( module->tech_id, "OUTLET_2_STATUS", &module->di_outlet_2_status, !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = Onduleur_get_var ( module, "ups.status" )) != NULL )
     { Dls_data_set_bool ( module->tech_id, "UPS_ONLINE",   &module->di_ups_online,   !strncmp(reponse, "\"OL", 3) );
       Dls_data_set_bool ( module->tech_id, "UPS_CHARGING", &module->di_ups_charging, !strcmp(reponse, "\"OL CHRG\"") );
       Dls_data_set_bool ( module->tech_id, "UPS_ON_BATT",  &module->di_ups_on_batt,  !strcmp(reponse, "\"OB\"") );
     }
    Ups_send_status_to_master ( module, TRUE );

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du UPS                                                                                           */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct MODULE_UPS *module;
    GSList *liste;

    prctl(PR_SET_NAME, "W-UPS", 0, 0, 0 );
    memset( &Cfg_ups, 0, sizeof(Cfg_ups) );                                         /* Mise a zero de la structure de travail */
    Cfg_ups.lib = lib;                                             /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_ups.lib->TID = pthread_self();                                                      /* Sauvegarde du TID pour le pere */
    Ups_Lire_config ();                                                     /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
              "%s: Demarrage . . . TID = %p", __func__, pthread_self() );
    Cfg_ups.lib->Thread_run = TRUE;                                                                     /* Le thread tourne ! */

    g_snprintf( Cfg_ups.lib->admin_prompt, sizeof(Cfg_ups.lib->admin_prompt), "ups" );
    g_snprintf( Cfg_ups.lib->admin_help,   sizeof(Cfg_ups.lib->admin_help),   "Manage UPS Modules" );

    if (!Cfg_ups.enable)
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
                "%s: Thread is not enabled in config. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }

    if (Config.instance_is_master==FALSE)
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
                "%s: Instance is not Master. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }

    Cfg_ups.Modules_UPS = NULL;                                                               /* Init des variables du thread */

    if ( Charger_tous_ups() == FALSE )                                                          /* Chargement des modules ups */
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "%s: No module UPS found -> stop", __func__ );
       goto end;
     }

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    while(lib->Thread_run == TRUE)                                                        /* On tourne tant que l'on a besoin */
     { sleep(1);
       sched_yield();

       if (lib->Thread_reload == TRUE)
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE, "%s: SIGUSR1", __func__ );
          Ups_Lire_config();
          Decharger_tous_UPS();
          Charger_tous_ups();
          lib->Thread_reload = FALSE;
        }

/*       if (Cfg_ups.admin_start)
        { module = Chercher_module_ups_by_id ( Cfg_ups.admin_start );
          if (module) { module->enable = TRUE;
                        module->date_next_connexion = 0;
                        Modifier_MODULE_UPS( &module->name );
                      }
          Cfg_ups.admin_start = 0;
        }

       if (Cfg_ups.admin_stop)
        { module = Chercher_module_ups_by_id ( Cfg_ups.admin_stop );
          if (module) { module->enable = FALSE;
                        Deconnecter_module  ( module );
                        module->date_next_connexion = 0;                                         /* RAZ de la date de retente */
  /*                      Modifier_MODULE_UPS( &module->name );
                      }
          Cfg_ups.admin_stop = 0;
        }*/

       if (Cfg_ups.Modules_UPS == NULL)                                             /* Si pas de module référencés, on attend */
        { sleep(2); continue; }

       pthread_mutex_lock ( &Cfg_ups.lib->synchro );                                   /* Car utilisation de la liste chainée */
       liste = Cfg_ups.Modules_UPS;
       while (liste && (lib->Thread_run == TRUE))
        { module = (struct MODULE_UPS *)liste->data;
          if ( module->enable != TRUE ||                            /* si le module n'est pas enable, on ne le traite pas */
               Partage->top < module->date_next_connexion )                        /* Si attente retente, on change de module */
           { liste = g_slist_next(liste);                                  /* On prépare le prochain accès au prochain module */
             continue;
           }
/******************************************** Début de l'interrogation du module **********************************************/
          if ( ! module->started )                                                               /* Communication OK ou non ? */
           { if ( ! Connecter_ups( module ) )                                                 /* Demande de connexion a l'ups */
              { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                         "%s: %s: Module DOWN", __func__, module->tech_id );
                Deconnecter_module ( module );                                         /* Sur erreur, on deconnecte le module */
                module->date_next_connexion = Partage->top + UPS_RETRY;
              }
           }
          else
           { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                      "%s: %s: Envoi des sorties ups", __func__, module->tech_id );
             if ( Envoyer_sortie_ups ( module ) == FALSE )
              { Deconnecter_module ( module );                                         /* Sur erreur, on deconnecte le module */
                module->date_next_connexion = Partage->top + UPS_RETRY;                          /* On retente dans longtemps */
              }
             else
              { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                         "%s: %s: Interrogation ups", __func__, module->tech_id );
                if ( Interroger_ups ( module ) == FALSE )
                 { Deconnecter_module ( module );
                   module->date_next_connexion = Partage->top + UPS_RETRY;                       /* On retente dans longtemps */
                 }
                else module->date_next_connexion = Partage->top + UPS_POLLING;               /* Update toutes les xx secondes */
              }
           }
          liste = liste->next;                                            /* On prépare le prochain accès au prochain module */
        }
       pthread_mutex_unlock ( &Cfg_ups.lib->synchro );                                 /* Car utilisation de la liste chainée */
     }

    Decharger_tous_UPS();
end:
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    Cfg_ups.lib->Thread_run = FALSE;                                                            /* Le thread ne tourne plus ! */
    Cfg_ups.lib->TID = 0;                                                     /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
