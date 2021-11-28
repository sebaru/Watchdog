/******************************************************************************************************************************/
/* Watchdogd/Onduleur/Onduleur.c  Gestion des upss UPS Watchdgo 2.0                                                        */
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
/* Entrée: le pointeur sur la PROCESS                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gboolean Ups_Lire_config ( void )
  { gchar *result;
    Creer_configDB ( Cfg_ups.lib->name, "debug", "false" );
    result = Recuperer_configDB_by_nom ( Cfg_ups.lib->name, "debug" );
    Cfg_ups.lib->Thread_debug = !g_ascii_strcasecmp(result, "true");
    g_free(result);
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Modbus_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                 */
/* Entrée: le pointeur sur la PROCESS                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Ups_Creer_DB ( void )
  { gint database_version;

    gchar *database_version_string = Recuperer_configDB_by_nom( Cfg_ups.lib->name, "database_version" );
    if (database_version_string)
     { database_version = atoi( database_version_string );
       g_free(database_version_string);
     } else database_version=0;

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'. Thread_Version '%s'.", __func__, database_version, WTD_VERSION );

    if (database_version==0)
     { SQL_Write ( "CREATE TABLE IF NOT EXISTS `ups` ("
                   "`id` int(11) NOT NULL AUTO_INCREMENT,"
                   "`tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL,"
                   "`enable` TINYINT(1) NOT NULL DEFAULT '0',"
                   "`host` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                   "`name` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                   "`admin_username` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                   "`admin_password` VARCHAR(32) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,"
                   "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                   "PRIMARY KEY (`id`)"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;" );
       goto end;
     }
end:
    database_version = 1;
    Modifier_configDB_int ( Cfg_ups.lib->name, "database_version", database_version );
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
                "SELECT tech_id,host,name,admin_username,admin_password,enable,date_create FROM ups ORDER BY host,name");

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
     { g_snprintf( ups->tech_id,  sizeof(ups->tech_id),  "%s", db->row[0] );
       g_snprintf( ups->host,     sizeof(ups->host),     "%s", db->row[1] );
       g_snprintf( ups->name,     sizeof(ups->name),     "%s", db->row[2] );
       g_snprintf( ups->admin_username, sizeof(ups->admin_username), "%s", db->row[3] );
       g_snprintf( ups->admin_password, sizeof(ups->admin_password), "%s", db->row[4] );
       g_snprintf( ups->date_create, sizeof(ups->date_create), "%s", db->row[6] );
       ups->enable = atoi(db->row[5]);
     }
    return(ups);
  }
/******************************************************************************************************************************/
/* Charger_tous_ups: Requete la DB pour charger les upss ups                                                               */
/* Entrée: rien                                                                                                               */
/* Sortie: le nombre de upss trouvé                                                                                        */
/******************************************************************************************************************************/
 static gboolean Charger_tous_ups ( void  )
  { struct MODULE_UPS *ups;
    struct DB *db;
    gint cpt;

/**************************************************** Chargement des upss **************************************************/
    if ( (db = Recuperer_ups()) == NULL )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "%s: Recuperer_ups Failed", __func__ );
       return(FALSE);
     }

    Cfg_ups.Modules_UPS = NULL;
    cpt = 0;
    while ( (ups = Recuperer_ups_suite( db )) != NULL )
     { cpt++;                                                                  /* Nous avons ajouté un ups dans la liste ! */
                                                                                            /* Ajout dans la liste de travail */
       pthread_mutex_lock( &Cfg_ups.lib->synchro );
       Cfg_ups.Modules_UPS = g_slist_prepend ( Cfg_ups.Modules_UPS, ups );
       pthread_mutex_unlock( &Cfg_ups.lib->synchro );
       Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_INFO,
                "%s: tech_id='%s', host=%s, name=%s", __func__,
                 ups->tech_id, ups->host, ups->name );
     }
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_INFO, "%s: %03d UPS found  !", __func__, cpt );

    Libere_DB_SQL( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Deconnecter: Deconnexion du ups                                                                                         */
/* Entrée: un id                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Deconnecter_UPS ( struct MODULE_UPS *ups )
  { if (!ups) return;

    if (ups->started == TRUE)
     { upscli_disconnect( &ups->upsconn );
       ups->started = FALSE;
     }

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
              "%s: %s disconnected (host='%s')", __func__, ups->tech_id, ups->host );
    Zmq_Send_DI_to_master ( Cfg_ups.lib, ups->tech_id, "IO_COMM", FALSE );
  }
/******************************************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                                               */
/* Entrée: une nom et un password                                                                                             */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                                              */
/******************************************************************************************************************************/
 static gboolean Connecter_ups ( struct MODULE_UPS *ups )
  { gchar buffer[80];
    int connexion;

    if ( (connexion = upscli_connect( &ups->upsconn, ups->host, UPS_PORT_TCP, UPSCLI_CONN_TRYSSL)) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: connexion refused by ups (host '%s' -> %s)", __func__, ups->tech_id,
                 ups->host, (char *)upscli_strerror(&ups->upsconn) );
       return(FALSE);
     }

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
              "%s: %s connected (host='%s')", __func__, ups->tech_id, ups->host );
/********************************************************* UPSDESC ************************************************************/
    g_snprintf( buffer, sizeof(buffer), "GET UPSDESC %s\n", ups->name );
    if ( upscli_sendline( &ups->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Sending GET UPSDESC failed (%s)", __func__, ups->tech_id,
                (char *)upscli_strerror(&ups->upsconn) );
     }
    else
     { if ( upscli_readline( &ups->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                   "%s: %s: Reading GET UPSDESC failed (%s)", __func__, ups->tech_id,
                   (char *)upscli_strerror(&ups->upsconn) );
        }
       else
        { g_snprintf( ups->description, sizeof(ups->description), "%s", buffer + strlen(ups->name) + 9 );
          Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                   "%s: %s: Reading GET UPSDESC %s", __func__, ups->tech_id,
                   ups->description );
        }
     }
/**************************************************** USERNAME ****************************************************************/
    g_snprintf( buffer, sizeof(buffer), "USERNAME %s\n", ups->admin_username );
    if ( upscli_sendline( &ups->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Sending USERNAME failed %s", __func__, ups->tech_id,
                (char *)upscli_strerror(&ups->upsconn) );
     }
    else
     { if ( upscli_readline( &ups->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                   "%s: %s: Reading USERNAME failed %s", __func__, ups->tech_id,
                   (char *)upscli_strerror(&ups->upsconn) );
        }
       else
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                   "%s: %s: Reading USERNAME %s", __func__, ups->tech_id,
                    buffer );
        }
     }

/******************************************************* PASSWORD *************************************************************/
    g_snprintf( buffer, sizeof(buffer), "PASSWORD %s\n", ups->admin_password );
    if ( upscli_sendline( &ups->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Sending PASSWORD failed %s", __func__, ups->tech_id,
                (char *)upscli_strerror(&ups->upsconn) );
     }
    else
     { if ( upscli_readline( &ups->upsconn, buffer, sizeof(buffer) ) == -1 )
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                   "%s: %s: Reading PASSWORD failed %s", __func__, ups->tech_id,
                   (char *)upscli_strerror(&ups->upsconn) );
        }
       else
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                   "%s: %s: Reading PASSWORD %s", __func__, ups->tech_id,
                   buffer );
        }
     }

/************************************************** PREPARE LES AI ************************************************************/
    if (!ups->nbr_connexion)
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_INFO,
                "%s: %s: Initialise le DLS et charge les AI ", __func__, ups->tech_id );
       if (Dls_auto_create_plugin( ups->tech_id, "Gestion de l'onduleur" ) == FALSE)
        { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", ups->tech_id ); }

       Mnemo_auto_create_DI ( FALSE, ups->tech_id, "IO_COMM", "Statut de la communication avec l'onduleur" );
       Mnemo_auto_create_DI ( FALSE, ups->tech_id, "OUTLET_1_STATUS", "Statut de la prise n°1" );
       Mnemo_auto_create_DI ( FALSE, ups->tech_id, "OUTLET_2_STATUS", "Statut de la prise n°2" );
       Mnemo_auto_create_DI ( FALSE, ups->tech_id, "UPS_ONLINE", "UPS Online" );
       Mnemo_auto_create_DI ( FALSE, ups->tech_id, "UPS_CHARGING", "UPS en charge" );
       Mnemo_auto_create_DI ( FALSE, ups->tech_id, "UPS_ON_BATT",  "UPS sur batterie" );
       Mnemo_auto_create_DI ( FALSE, ups->tech_id, "UPS_REPLACE_BATT",  "Batteries UPS a changer" );
       Mnemo_auto_create_DI ( FALSE, ups->tech_id, "UPS_ALARM",  "UPS en alarme !" );

       Mnemo_auto_create_AI ( FALSE, ups->tech_id, "LOAD", "Charge onduleur", "%" );
       Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "LOAD", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, ups->tech_id, "REALPOWER", "Charge onduleur", "W" );
       Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "REALPOWER", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, ups->tech_id, "BATTERY_CHARGE", "Charge batterie", "%" );
       Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "BATTERY_CHARGE", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, ups->tech_id, "INPUT_VOLTAGE", "Tension d'entrée", "V" );
       Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "INPUT_VOLTAGE", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, ups->tech_id, "BATTERY_RUNTIME", "Durée de batterie restante", "s" );
       Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "BATTERY_RUNTIME", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, ups->tech_id, "BATTERY_VOLTAGE", "Tension batterie", "V" );
       Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "BATTERY_VOLTAGE", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, ups->tech_id, "INPUT_HZ", "Fréquence d'entrée", "HZ" );
       Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "INPUT_HZ", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, ups->tech_id, "OUTPUT_CURRENT", "Courant de sortie", "A" );
       Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "OUTPUT_CURRENT", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, ups->tech_id, "OUTPUT_HZ", "Fréquence de sortie", "HZ" );
       Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "OUTPUT_HZ", 0.0, FALSE );

       Mnemo_auto_create_AI ( FALSE, ups->tech_id, "OUTPUT_VOLTAGE", "Tension de sortie", "V" );
       Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "OUTPUT_VOLTAGE", 0.0, FALSE );

       Mnemo_auto_create_DO ( FALSE, ups->tech_id, "LOAD_OFF", "Coupe la sortie ondulée" );
       Mnemo_auto_create_DO ( FALSE, ups->tech_id, "LOAD_ON", "Active la sortie ondulée" );
       Mnemo_auto_create_DO ( FALSE, ups->tech_id, "OUTLET_1_OFF", "Désactive la prise n°1" );
       Mnemo_auto_create_DO ( FALSE, ups->tech_id, "OUTLET_1_ON", "Active la prise n°1" );
       Mnemo_auto_create_DO ( FALSE, ups->tech_id, "OUTLET_2_OFF", "Désactive la prise n°2" );
       Mnemo_auto_create_DO ( FALSE, ups->tech_id, "OUTLET_2_ON", "Active la prise n°2" );
       Mnemo_auto_create_DO ( FALSE, ups->tech_id, "START_DEEP_BAT", "Active un test de décharge profond" );
       Mnemo_auto_create_DO ( FALSE, ups->tech_id, "START_QUICK_BAT", "Active un test de décharge léger" );
       Mnemo_auto_create_DO ( FALSE, ups->tech_id, "STOP_TEST_BAT", "Stop le test de décharge batterie" );
     }

    ups->date_next_connexion = 0;
    ups->started = TRUE;
    ups->nbr_connexion++;
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
              "%s: %s up and running (host='%s')", __func__, ups->tech_id, ups->host );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Rechercher_MODULE_UPS: Recupération du ups dont le num est en parametre                                                    */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 static void Decharger_un_UPS ( struct MODULE_UPS *ups )
  { if (!ups) return;
    pthread_mutex_lock( &Cfg_ups.lib->synchro );
    Cfg_ups.Modules_UPS = g_slist_remove ( Cfg_ups.Modules_UPS, ups );
    pthread_mutex_unlock( &Cfg_ups.lib->synchro );
    Deconnecter_UPS ( ups );
    g_free(ups);
  }
/******************************************************************************************************************************/
/* Decharger_tous_Decharge l'ensemble des upss UPS                                                                         */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Decharger_tous_UPS ( void  )
  { struct MODULE_UPS *ups;
    while ( Cfg_ups.Modules_UPS )
     { ups = (struct MODULE_UPS *)Cfg_ups.Modules_UPS->data;
       Decharger_un_UPS ( ups );
     }
  }
/******************************************************************************************************************************/
/* Onduleur_set_instcmd: Envoi d'une instant commande à l'ups                                                                 */
/* Entrée : l'ups, le nom de la commande                                                                                      */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                                          */
/******************************************************************************************************************************/
 static void Onduleur_set_instcmd ( struct MODULE_UPS *ups, gchar *nom_cmd )
  { gchar buffer[80];

    if (ups->started != TRUE) return;

    g_snprintf( buffer, sizeof(buffer), "INSTCMD %s %s\n", ups->name, nom_cmd );
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE, "%s: %s: Sending '%s'", __func__, ups->tech_id, buffer );
    if ( upscli_sendline( &ups->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                 "%s: %s: Sending INSTCMD failed with error '%s' for '%s'", __func__, ups->tech_id,
                 (char *)upscli_strerror(&ups->upsconn), buffer );
       Deconnecter_UPS ( ups );
       return;
     }

    if ( upscli_readline( &ups->upsconn, buffer, sizeof(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Reading INSTCMD result failed (%s) error %s", __func__, ups->tech_id,
                 nom_cmd, (char *)upscli_strerror(&ups->upsconn) );
       Deconnecter_UPS ( ups );
       return;
     }
    else
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE, "%s: %s: Sending '%s' OK", __func__, ups->tech_id, nom_cmd );
     }
  }
/******************************************************************************************************************************/
/* Onduleur_get_var: Recupere une valeur de la variable en parametre                                                          */
/* Entrée : l'ups, le nom de variable, la variable a renseigner                                                               */
/* Sortie : TRUE si pas de probleme, FALSE si erreur                                                                          */
/******************************************************************************************************************************/
 static gchar *Onduleur_get_var ( struct MODULE_UPS *ups, gchar *nom_var )
  { static gchar buffer[80];
    gint retour_read;

    if (ups->started != TRUE) return(NULL);

    g_snprintf( buffer, sizeof(buffer), "GET VAR %s %s\n", ups->name, nom_var );
    if ( upscli_sendline( &ups->upsconn, buffer, strlen(buffer) ) == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Sending GET VAR failed (%s) error=%s", __func__, ups->tech_id,
                buffer, (char *)upscli_strerror(&ups->upsconn) );
       Deconnecter_UPS ( ups );
       return(NULL);
     }

    retour_read = upscli_readline( &ups->upsconn, buffer, sizeof(buffer) );
    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
             "%s: %s: Reading GET VAR %s ReadLine result = %d, upscli_upserror = %d, buffer = %s", __func__, ups->tech_id,
              nom_var, retour_read, upscli_upserror(&ups->upsconn), buffer );
    if ( retour_read == -1 )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
                "%s: %s: Reading GET VAR result failed (%s) error=%s", __func__, ups->tech_id,
                 nom_var, (char *)upscli_strerror(&ups->upsconn) );
       return(NULL);
     }

    if ( ! strncmp ( buffer, "VAR", 3 ) )
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG,
                "%s: %s: Reading GET VAR %s OK = %s", __func__, ups->tech_id, nom_var, buffer );
       return(buffer + 6 + strlen(ups->name) + strlen(nom_var));
     }

    if ( ! strcmp ( buffer, "ERR VAR-NOT-SUPPORTED" ) )
     { return(NULL);                                                         /* Variable not supported... is not an error ... */
     }

    Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING,
             "%s: %s: Reading GET VAR %s Failed : error %s (buffer %s)", __func__, ups->tech_id,
              nom_var, (char *)upscli_strerror(&ups->upsconn), buffer );
    Deconnecter_UPS ( ups );
    ups->date_next_connexion = Partage->top + UPS_RETRY;
    return(NULL);
  }
/******************************************************************************************************************************/
/* Envoyer_sortie_ups: Envoi des sorties/InstantCommand à l'ups                                                               */
/* Entrée: identifiants des upss ups                                                                                       */
/* Sortie: TRUE si pas de probleme, FALSE sinon                                                                               */
/******************************************************************************************************************************/
 static void Envoyer_sortie_aux_ups( void )
  { JsonNode *request;
    while ( (request = Thread_Listen_to_master ( Cfg_ups.lib ) ) != NULL)
     { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
       if ( !strcasecmp( zmq_tag, "SET_DO" ) )
        { gchar *tech_id, *acronyme;
          tech_id  = Json_get_string ( request, "tech_id" );
          acronyme = Json_get_string ( request, "acronyme" );
          if (!tech_id)
           { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque tech_id", __func__ ); }
          else if (!acronyme)
           { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque acronyme", __func__ ); }
          else
           { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG, "%s: Recu SET_DO from bus: %s:%s", __func__, tech_id, acronyme );

             GSList *liste = Cfg_ups.Modules_UPS;
             while (liste)
              { struct MODULE_UPS *ups = (struct MODULE_UPS *)liste->data;
                if (!strcasecmp(ups->tech_id, tech_id))
                 { if (!strcasecmp(acronyme, "LOAD_OFF"))        Onduleur_set_instcmd ( ups, "load.off" );
                   if (!strcasecmp(acronyme, "LOAD_ON"))         Onduleur_set_instcmd ( ups, "load.on" );
                   if (!strcasecmp(acronyme, "OUTLET_1_OFF"))    Onduleur_set_instcmd ( ups, "outlet.1.load.off" );
                   if (!strcasecmp(acronyme, "OUTLET_1_ON"))     Onduleur_set_instcmd ( ups, "outlet.1.load.on" );
                   if (!strcasecmp(acronyme, "OUTLET_2_OFF"))    Onduleur_set_instcmd ( ups, "outlet.2.load.off" );
                   if (!strcasecmp(acronyme, "OUTLET_2_ON"))     Onduleur_set_instcmd ( ups, "outlet.2.load.on" );
                   if (!strcasecmp(acronyme, "START_DEEP_BAT"))  Onduleur_set_instcmd ( ups, "test.battery.start.deep" );
                   if (!strcasecmp(acronyme, "START_QUICK_BAT")) Onduleur_set_instcmd ( ups, "test.battery.start.quick" );
                   if (!strcasecmp(acronyme, "STOP_TEST_BAT"))   Onduleur_set_instcmd ( ups, "test.battery.stop" );
                 }
                liste = g_slist_next(liste);
              }
           }
        }
       json_node_unref (request);
     }
  }
/******************************************************************************************************************************/
/* Interroger_ups: Interrogation d'un ups                                                                                     */
/* Entrée: identifiants des upss ups                                                                                       */
/* Sortie: TRUE si pas de probleme, FALSE sinon                                                                               */
/******************************************************************************************************************************/
 static gboolean Interroger_ups( struct MODULE_UPS *ups )
  { gchar *reponse;

    if ( (reponse = Onduleur_get_var ( ups, "ups.load" )) != NULL )
     { Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "LOAD", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( ups, "ups.realpower" )) != NULL )
     { Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "REALPOWER", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( ups, "battery.charge" )) != NULL )
     { Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "BATTERY_CHARGE", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( ups, "input.voltage" )) != NULL )
     { Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "INPUT_VOLTAGE", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( ups, "battery.runtime" )) != NULL )
     { Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "BATTERY_RUNTIME", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( ups, "battery.voltage" )) != NULL )
     { Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "BATTERY_VOLTAGE", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( ups, "input.frequency" )) != NULL )
     { Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "INPUT_HZ", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( ups, "output.current" )) != NULL )
     { Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "OUTPUT_CURRENT", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( ups, "output.frequency" )) != NULL )
     { Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "OUTPUT_HZ", atof(reponse+1), TRUE ); }

    if ( (reponse = Onduleur_get_var ( ups, "output.voltage" )) != NULL )
     { Zmq_Send_AI_to_master ( Cfg_ups.lib, ups->tech_id, "OUTPUT_VOLTAGE", atof(reponse+1), TRUE ); }

/*---------------------------------------------- Récupération des entrées TOR de l'UPS ---------------------------------------*/
    if ( (reponse = Onduleur_get_var ( ups, "outlet.1.status" )) != NULL )
     { Zmq_Send_DI_to_master ( Cfg_ups.lib, ups->tech_id, "OUTLET_1_STATUS", !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = Onduleur_get_var ( ups, "outlet.2.status" )) != NULL )
     { Zmq_Send_DI_to_master ( Cfg_ups.lib, ups->tech_id, "OUTLET_2_STATUS", !strcmp(reponse, "\"on\"") ); }

    if ( (reponse = Onduleur_get_var ( ups, "ups.status" )) != NULL )
     { Zmq_Send_DI_to_master ( Cfg_ups.lib, ups->tech_id, "UPS_ONLINE",       (g_strrstr(reponse, "OL")?TRUE:FALSE) );
       Zmq_Send_DI_to_master ( Cfg_ups.lib, ups->tech_id, "UPS_CHARGING",     (g_strrstr(reponse, "DISCHRG")?FALSE:TRUE) );
       Zmq_Send_DI_to_master ( Cfg_ups.lib, ups->tech_id, "UPS_ON_BATT",      (g_strrstr(reponse, "OB")?TRUE:FALSE) );
       Zmq_Send_DI_to_master ( Cfg_ups.lib, ups->tech_id, "UPS_REPLACE_BATT", (g_strrstr(reponse, "RB")?TRUE:FALSE) );
       Zmq_Send_DI_to_master ( Cfg_ups.lib, ups->tech_id, "UPS_ALARM",        (g_strrstr(reponse, "ALARM")?TRUE:FALSE) );
     }
    Zmq_Send_DI_to_master ( Cfg_ups.lib, ups->tech_id, "IO_COMM", TRUE );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du UPS                                                                                           */
/******************************************************************************************************************************/
 void Run_process ( struct PROCESS *lib )
  { struct MODULE_UPS *ups;
    GSList *liste;

reload:
    memset( &Cfg_ups, 0, sizeof(Cfg_ups) );                                         /* Mise a zero de la structure de travail */
    Cfg_ups.lib = lib;                                             /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "ups", "I/O", lib, WTD_VERSION, "Manage UPS Module" );
    Ups_Lire_config ();                                                     /* Lecture de la configuration logiciel du thread */
    if (Config.instance_is_master==FALSE)
     { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_NOTICE,
                "%s: Instance is not Master. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }
    Ups_Creer_DB();

    Cfg_ups.Modules_UPS = NULL;                                                               /* Init des variables du thread */

    Charger_tous_ups();                                                                         /* Chargement des upss ups */

    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       Envoyer_sortie_aux_ups();

       if (Cfg_ups.Modules_UPS == NULL)                                                /* Si pas de ups référencés, on attend */
        { sleep(1); continue; }

       pthread_mutex_lock ( &Cfg_ups.lib->synchro );                                   /* Car utilisation de la liste chainée */
       liste = Cfg_ups.Modules_UPS;
       while (liste && lib->Thread_run == TRUE && lib->Thread_reload == FALSE)
        { ups = (struct MODULE_UPS *)liste->data;
          if ( ups->enable != TRUE ||                            /* si le ups n'est pas enable, on ne le traite pas */
               Partage->top < ups->date_next_connexion )                        /* Si attente retente, on change de ups */
           { liste = g_slist_next(liste);                                  /* On prépare le prochain accès au prochain ups */
             continue;
           }
/******************************************** Début de l'interrogation du ups **********************************************/
          if ( ! ups->started )                                                               /* Communication OK ou non ? */
           { if ( ! Connecter_ups( ups ) )                                                 /* Demande de connexion a l'ups */
              { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_WARNING, "%s: %s: Module DOWN", __func__, ups->tech_id );
                Deconnecter_UPS ( ups );                                         /* Sur erreur, on deconnecte le ups */
                ups->date_next_connexion = Partage->top + UPS_RETRY;
              }
           }
          else
           { Info_new( Config.log, Cfg_ups.lib->Thread_debug, LOG_DEBUG, "%s: %s: Interrogation ups", __func__, ups->tech_id );
             if ( Interroger_ups ( ups ) == FALSE )
              { Deconnecter_UPS ( ups );
                ups->date_next_connexion = Partage->top + UPS_RETRY;                       /* On retente dans longtemps */
              }
             else ups->date_next_connexion = Partage->top + UPS_POLLING;               /* Update toutes les xx secondes */
           }
          liste = liste->next;                                            /* On prépare le prochain accès au prochain ups */
        }
       pthread_mutex_unlock ( &Cfg_ups.lib->synchro );                                 /* Car utilisation de la liste chainée */
     }

    Decharger_tous_UPS();

end:
    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
