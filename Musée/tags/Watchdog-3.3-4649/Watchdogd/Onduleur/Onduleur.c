/******************************************************************************************************************************/
/* Watchdogd/Onduleur/Onduleur.c  Gestion des modules UPS Watchdgo 2.0                                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mar. 10 nov. 2009 15:56:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Onduleur.c
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
 struct UPS_CONFIG Cfg_ups;
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
/* Deconnecter: Deconnexion du module                                                                                         */
/* Entrée: un id                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Deconnecter_UPS ( struct MODULE_UPS *module )
  { if (!module) return;

    if (module->started == TRUE)
     { upscli_disconnect( &module->upsconn );
       module->started = FALSE;
     }

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
              "%s: %s disconnected (host='%s')", __func__, module->tech_id, module->host );
    Send_zmq_DI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "COMM", FALSE );
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
       if (Dls_auto_create_plugin( module->tech_id, "Gestion de l'onduleur" ) == FALSE)
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", module->tech_id ); }

       Mnemo_auto_create_DI ( module->tech_id, "COMM", "Statut de la communication avec l'onduleur" );
       Mnemo_auto_create_DI ( module->tech_id, "OUTLET_1_STATUS", "Statut de la prise n°1" );
       Mnemo_auto_create_DI ( module->tech_id, "OUTLET_2_STATUS", "Statut de la prise n°2" );
       Mnemo_auto_create_DI ( module->tech_id, "UPS_ONLINE", "UPS Online" );
       Mnemo_auto_create_DI ( module->tech_id, "UPS_CHARGING", "UPS en charge" );
       Mnemo_auto_create_DI ( module->tech_id, "UPS_ON_BATT",  "UPS sur batterie" );
       Mnemo_auto_create_DI ( module->tech_id, "UPS_REPLACE_BATT",  "Batteries UPS a changer" );
       Mnemo_auto_create_DI ( module->tech_id, "UPS_ALARM",  "UPS en alarme !" );

       Mnemo_auto_create_AI ( module->tech_id, "LOAD", "Charge onduleur", "%" );
       Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "LOAD", 0.0, FALSE );

       Mnemo_auto_create_AI ( module->tech_id, "REALPOWER", "Charge onduleur", "W" );
       Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "REALPOWER", 0.0, FALSE );

       Mnemo_auto_create_AI ( module->tech_id, "BATTERY_CHARGE", "Charge batterie", "%" );
       Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "BATTERY_CHARGE", 0.0, FALSE );

       Mnemo_auto_create_AI ( module->tech_id, "INPUT_VOLTAGE", "Tension d'entrée", "V" );
       Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "INPUT_VOLTAGE", 0.0, FALSE );

       Mnemo_auto_create_AI ( module->tech_id, "BATTERY_RUNTIME", "Durée de batterie restante", "s" );
       Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "BATTERY_RUNTIME", 0.0, FALSE );

       Mnemo_auto_create_AI ( module->tech_id, "BATTERY_VOLTAGE", "Tension batterie", "V" );
       Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "BATTERY_VOLTAGE", 0.0, FALSE );

       Mnemo_auto_create_AI ( module->tech_id, "INPUT_HZ", "Fréquence d'entrée", "HZ" );
       Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "INPUT_HZ", 0.0, FALSE );

       Mnemo_auto_create_AI ( module->tech_id, "OUTPUT_CURRENT", "Courant de sortie", "A" );
       Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "OUTPUT_CURRENT", 0.0, FALSE );

       Mnemo_auto_create_AI ( module->tech_id, "OUTPUT_HZ", "Fréquence de sortie", "HZ" );
       Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "OUTPUT_HZ", 0.0, FALSE );

       Mnemo_auto_create_AI ( module->tech_id, "OUTPUT_VOLTAGE", "Tension de sortie", "V" );
       Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "OUTPUT_VOLTAGE", 0.0, FALSE );

       Mnemo_auto_create_DO ( module->tech_id, "LOAD_OFF", "Coupe la sortie ondulée" );
       Mnemo_auto_create_DO ( module->tech_id, "LOAD_ON", "Active la sortie ondulée" );
       Mnemo_auto_create_DO ( module->tech_id, "OUTLET_1_OFF", "Désactive la prise n°1" );
       Mnemo_auto_create_DO ( module->tech_id, "OUTLET_1_ON", "Active la prise n°1" );
       Mnemo_auto_create_DO ( module->tech_id, "OUTLET_2_OFF", "Désactive la prise n°2" );
       Mnemo_auto_create_DO ( module->tech_id, "OUTLET_2_ON", "Active la prise n°2" );
       Mnemo_auto_create_DO ( module->tech_id, "START_DEEP_BAT", "Active un test de decharge profond" );
       Mnemo_auto_create_DO ( module->tech_id, "START_QUICK_BAT", "Active un test de decharge léger" );
       Mnemo_auto_create_DO ( module->tech_id, "STOP_TEST_BAT", "Stop le test de décharge batterie" );
     }

    module->date_next_connexion = 0;
    module->started = TRUE;
    module->nbr_connexion++;
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
              "%s: %s up and running (host='%s')", __func__, module->tech_id, module->host );
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
    pthread_mutex_unlock( &Cfg_ups.lib->synchro );
    Deconnecter_UPS ( module );
    g_free(module);
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
/* Onduleur_set_instcmd: Envoi d'une instant commande à l'ups                                                                 */
/* Entrée : l'ups, le nom de la commande                                                                                      */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                                          */
/******************************************************************************************************************************/
 static void Onduleur_set_instcmd ( struct MODULE_UPS *module, gchar *nom_cmd )
  { gchar buffer[80];

    if (module->started != TRUE) return;

    g_snprintf( buffer, sizeof(buffer), "INSTCMD %s %s\n", module->name, nom_cmd );
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE, "%s: %s: Sending '%s'", __func__, module->tech_id, buffer );
    if ( upscli_sendline( &module->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                 "%s: %s: Sending INSTCMD failed with error '%s' for '%s'", __func__, module->tech_id,
                 (char *)upscli_strerror(&module->upsconn), buffer );
       Deconnecter_UPS ( module );
       return;
     }

    if ( upscli_readline( &module->upsconn, buffer, sizeof(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Reading INSTCMD result failed (%s) error %s", __func__, module->tech_id,
                 nom_cmd, (char *)upscli_strerror(&module->upsconn) );
       Deconnecter_UPS ( module );
       return;
     }
    else
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE, "%s: %s: Sending '%s' OK", __func__, module->tech_id, nom_cmd );
     }
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
       Deconnecter_UPS ( module );
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
    Deconnecter_UPS ( module );
    module->date_next_connexion = Partage->top + UPS_RETRY;
    return(NULL);
  }
/******************************************************************************************************************************/
/* Envoyer_sortie_ups: Envoi des sorties/InstantCommand à l'ups                                                               */
/* Entrée: identifiants des modules ups                                                                                       */
/* Sortie: TRUE si pas de probleme, FALSE sinon                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_sortie_aux_ups( void )
  { struct ZMQ_TARGET *event;
    gchar buffer[256];
    void *payload;
    gint byte;
                                                                                            /* Reception d'un paquet master ? */
    while( (byte=Recv_zmq_with_tag ( Cfg_ups.zmq_from_bus, NOM_THREAD, &buffer, sizeof(buffer)-1, &event, &payload )) > 0)
     { JsonObject *object;
       JsonNode *Query;
       buffer[byte] = 0;

       if ( !strcasecmp( event->tag, "SET_DO" ) )
        { gchar *tech_id, *acronyme;
          Query = json_from_string ( payload, NULL );

          if (!Query)
           { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s: requete non Json", __func__ ); continue; }
          object = json_node_get_object (Query);
          if (!object)
           { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s: Object non trouvé", __func__ );
             json_node_unref (Query);
             continue;
           }

          tech_id  = json_object_get_string_member ( object, "tech_id" );
          acronyme = json_object_get_string_member ( object, "acronyme" );
          if (!tech_id)
           { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque tech_id", __func__ );
             json_node_unref (Query);
             continue;
           }
          else if (!acronyme)
           { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque acronyme", __func__ );
             json_node_unref (Query);
             continue;
           }

          Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG, "%s: Recu SET_DO from bus: %s:%s", __func__, tech_id, acronyme );

          GSList *liste = Cfg_ups.Modules_UPS;
          while (liste)
           { struct MODULE_UPS *module = (struct MODULE_UPS *)liste->data;
             if (!strcasecmp(module->tech_id, tech_id))
              { if (!strcasecmp(acronyme, "LOAD_OFF"))        Onduleur_set_instcmd ( module, "load.off" );
                if (!strcasecmp(acronyme, "LOAD_ON"))         Onduleur_set_instcmd ( module, "load.on" );
                if (!strcasecmp(acronyme, "OUTLET_1_OFF"))    Onduleur_set_instcmd ( module, "outlet.1.load.off" );
                if (!strcasecmp(acronyme, "OUTLET_1_ON"))     Onduleur_set_instcmd ( module, "outlet.1.load.on" );
                if (!strcasecmp(acronyme, "OUTLET_2_OFF"))    Onduleur_set_instcmd ( module, "outlet.2.load.off" );
                if (!strcasecmp(acronyme, "OUTLET_2_ON"))     Onduleur_set_instcmd ( module, "outlet.2.load.on" );
                if (!strcasecmp(acronyme, "START_DEEP_BAT"))  Onduleur_set_instcmd ( module, "test.battery.start.deep" );
                if (!strcasecmp(acronyme, "START_QUICK_BAT")) Onduleur_set_instcmd ( module, "test.battery.start.quick" );
                if (!strcasecmp(acronyme, "STOP_TEST_BAT"))   Onduleur_set_instcmd ( module, "test.battery.stop" );
              }
             liste = g_slist_next(liste);
           }
          json_node_unref (Query);
        }
     }
  }
/******************************************************************************************************************************/
/* Interroger_ups: Interrogation d'un ups                                                                                     */
/* Entrée: identifiants des modules ups                                                                                       */
/* Sortie: TRUE si pas de probleme, FALSE sinon                                                                               */
/******************************************************************************************************************************/
 static gboolean Interroger_ups( struct MODULE_UPS *module )
  { gchar *reponse;

    if ( (reponse = Onduleur_get_var ( module, "ups.load" )) != NULL )
     { Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "LOAD", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "ups.realpower" )) != NULL )
     { Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "REALPOWER", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.charge" )) != NULL )
     { Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "BATTERY_CHARGE", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "input.voltage" )) != NULL )
     { Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "INPUT_VOLTAGE", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.runtime" )) != NULL )
     { Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "BATTERY_RUNTIME", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "battery.voltage" )) != NULL )
     { Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "BATTERY_VOLTAGE", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "input.frequency" )) != NULL )
     { Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "INPUT_HZ", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "output.current" )) != NULL )
     { Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "OUTPUT_CURRENT", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "output.frequency" )) != NULL )
     { Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "OUTPUT_HZ", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( module, "output.voltage" )) != NULL )
     { Send_zmq_AI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "OUTPUT_VOLTAGE", atof(reponse+1), TRUE ); }

/*---------------------------------------------- Récupération des entrées TOR de l'UPS ---------------------------------------*/
    if ( (reponse = Onduleur_get_var ( module, "outlet.1.status" )) != NULL )
     { Send_zmq_DI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "OUTLET_1_STATUS", !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = Onduleur_get_var ( module, "outlet.2.status" )) != NULL )
     { Send_zmq_DI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "OUTLET_2_STATUS", !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = Onduleur_get_var ( module, "ups.status" )) != NULL )
     { Send_zmq_DI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "UPS_ONLINE",       (g_strrstr(reponse, "OL")?TRUE:FALSE) );
       Send_zmq_DI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "UPS_CHARGING",     (g_strrstr(reponse, "CHRG")?TRUE:FALSE) );
       Send_zmq_DI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "UPS_ON_BATT",      (g_strrstr(reponse, "OB")?TRUE:FALSE) );
       Send_zmq_DI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "UPS_REPLACE_BATT", (g_strrstr(reponse, "RB")?TRUE:FALSE) );
       Send_zmq_DI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "UPS_ALARM",        (g_strrstr(reponse, "ALARM")?TRUE:FALSE) );
     }
    Send_zmq_DI_to_master ( Cfg_ups.zmq_to_master, NOM_THREAD, module->tech_id, "COMM", TRUE );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du UPS                                                                                           */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct MODULE_UPS *module;
    GSList *liste;

    prctl(PR_SET_NAME, "W-UPS", 0, 0, 0 );
reload:
    memset( &Cfg_ups, 0, sizeof(Cfg_ups) );                                         /* Mise a zero de la structure de travail */
    Cfg_ups.lib = lib;                                             /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "W-UPS", lib, NOM_THREAD, "Manage UPS Module" );
    Cfg_ups.lib->TID = pthread_self();                                                      /* Sauvegarde du TID pour le pere */
    Ups_Lire_config ();                                                     /* Lecture de la configuration logiciel du thread */

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

    Cfg_ups.zmq_from_bus  = Connect_zmq ( ZMQ_SUB, "listen-to-bus",  "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    Cfg_ups.zmq_to_master = Connect_zmq ( ZMQ_PUB, "pub-to-master",  "inproc", ZMQUEUE_LOCAL_MASTER, 0 );
    Cfg_ups.Modules_UPS = NULL;                                                               /* Init des variables du thread */

    if ( Charger_tous_ups() == FALSE )                                                          /* Chargement des modules ups */
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "%s: No module UPS found -> stop", __func__ );
       goto end;
     }

    setlocale( LC_ALL, "C" );                                            /* Pour le formattage correct des , . dans les float */
    while(lib->Thread_run == TRUE)                                                        /* On tourne tant que l'on a besoin */
     { usleep(10000);
       sched_yield();

       if (lib->Thread_reload == TRUE)
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE, "%s: SIGUSR1", __func__ );
          break;
        }

       Envoyer_sortie_aux_ups();

       if (Cfg_ups.Modules_UPS == NULL)                                             /* Si pas de module référencés, on attend */
        { sleep(2); continue; }

       pthread_mutex_lock ( &Cfg_ups.lib->synchro );                                   /* Car utilisation de la liste chainée */
       liste = Cfg_ups.Modules_UPS;
       while (liste && lib->Thread_run == TRUE && lib->Thread_reload == FALSE)
        { module = (struct MODULE_UPS *)liste->data;
          if ( module->enable != TRUE ||                            /* si le module n'est pas enable, on ne le traite pas */
               Partage->top < module->date_next_connexion )                        /* Si attente retente, on change de module */
           { liste = g_slist_next(liste);                                  /* On prépare le prochain accès au prochain module */
             continue;
           }
/******************************************** Début de l'interrogation du module **********************************************/
          if ( ! module->started )                                                               /* Communication OK ou non ? */
           { if ( ! Connecter_ups( module ) )                                                 /* Demande de connexion a l'ups */
              { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "%s: %s: Module DOWN", __func__, module->tech_id );
                Deconnecter_UPS ( module );                                         /* Sur erreur, on deconnecte le module */
                module->date_next_connexion = Partage->top + UPS_RETRY;
              }
           }
          else
           { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG, "%s: %s: Interrogation ups", __func__, module->tech_id );
             if ( Interroger_ups ( module ) == FALSE )
              { Deconnecter_UPS ( module );
                module->date_next_connexion = Partage->top + UPS_RETRY;                       /* On retente dans longtemps */
              }
             else module->date_next_connexion = Partage->top + UPS_POLLING;               /* Update toutes les xx secondes */
           }
          liste = liste->next;                                            /* On prépare le prochain accès au prochain module */
        }
       pthread_mutex_unlock ( &Cfg_ups.lib->synchro );                                 /* Car utilisation de la liste chainée */
     }

    Decharger_tous_UPS();
    Close_zmq ( Cfg_ups.zmq_to_master );
    Close_zmq ( Cfg_ups.zmq_from_bus );

end:
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );

    if (lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
