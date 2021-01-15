/******************************************************************************************************************************/
/* Watchdogd/Modbus/Modbus.c  Gestion des modules MODBUS Watchdgo 2.0                                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         jeu. 24 déc. 2009 12:59:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Modbus.c
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
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <errno.h>
 #include <sys/prctl.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <semaphore.h>
 #include <netinet/in.h>
 #include <netdb.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Modbus.h"
 struct MODBUS_CONFIG Cfg_modbus;
/******************************************************************************************************************************/
/* Modbus_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                 */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gboolean Modbus_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Creer_configDB ( NOM_THREAD, "debug", "false" );
    Cfg_modbus.lib->Thread_debug = FALSE;                                                      /* Settings default parameters */

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s' = %s", __func__, nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_modbus.lib->Thread_debug = TRUE;  }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Modbus_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                 */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Modbus_Creer_DB ( void )
  { gint database_version;

    gchar *database_version_string = Recuperer_configDB_by_nom( "modbus", "database_version" );
    if (database_version_string)
     { database_version = atoi( database_version_string );
       g_free(database_version_string);
     } else database_version=0;

    Info_new( Config.log, Config.log_db, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'. Thread_Version '%s'.", __func__, database_version, WTD_VERSION );

    if (database_version==0)
     { SQL_Write ( "CREATE TABLE IF NOT EXISTS `modbus_modules` ("
                   "`id` int(11) NOT NULL AUTO_INCREMENT,"
                   "`date_create` datetime NOT NULL DEFAULT NOW(),"
                   "`enable` tinyint(1) NOT NULL,"
                   "`hostname` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                   "`tech_id` varchar(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT hostname,"
                   "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                   "`watchdog` int(11) NOT NULL DEFAULT 50,"
                   "`max_request_par_sec` int(11) NOT NULL DEFAULT 50,"
                   "PRIMARY KEY (`id`)"
                   ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=1 ;" );
       goto end;
     }

    if (database_version < 1)
     { SQL_Write ( "ALTER TABLE `modbus_modules` DROP `map_EA`" );
       SQL_Write ( "ALTER TABLE `modbus_modules` DROP `map_E`" );
       SQL_Write ( "ALTER TABLE `modbus_modules` DROP `max_nbr_E`" );
       SQL_Write ( "ALTER TABLE `modbus_modules` DROP `map_A`" );
       SQL_Write ( "ALTER TABLE `modbus_modules` DROP `map_AA`" );
     }

    if (database_version < 2)
     { SQL_Write ( "ALTER TABLE `modbus_modules` ADD `max_request_par_sec` int(11) NOT NULL DEFAULT 50" );
     }
    database_version = 2;
end:
    Modifier_configDB_int ( "modbus", "database_version", database_version );
  }
/******************************************************************************************************************************/
/* Recuperer_liste_id_modbusDB: Recupération de la liste des ids des modbuss                                                  */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 gboolean Recuperer_modbusDB ( struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                                      /* Requete SQL */
                "SELECT id,date_create,enable,hostname,tech_id,watchdog,description,max_request_par_sec "
                " FROM %s ORDER BY description",
                NOM_TABLE_MODULE_MODBUS );

    return ( Lancer_requete_SQL ( db, requete ) );                                             /* Execution de la requete SQL */
  }
/******************************************************************************************************************************/
/* Recuperer_liste_id_modbusDB: Recupération de la liste des ids des modbuss                                                  */
/* Entrée: un log et une database                                                                                             */
/* Sortie: une GList                                                                                                          */
/******************************************************************************************************************************/
 struct MODBUSDB *Recuperer_modbusDB_suite( struct DB *db )
  { struct MODBUSDB *modbus;

    Recuperer_ligne_SQL(db);                                                               /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       return(NULL);
     }

    modbus = (struct MODBUSDB *)g_try_malloc0( sizeof(struct MODBUSDB) );
    if (!modbus) Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR,
                          "%s: Erreur allocation mémoire", __func__ );
    else
     { g_snprintf( modbus->description, sizeof(modbus->description), "%s", db->row[6] );
       g_snprintf( modbus->tech_id,     sizeof(modbus->tech_id),     "%s", db->row[4] );
       g_snprintf( modbus->hostname,    sizeof(modbus->hostname),    "%s", db->row[3] );
       g_snprintf( modbus->date_create, sizeof(modbus->date_create), "%s", db->row[1] );
       modbus->id                  = atoi(db->row[0]);
       modbus->enable              = atoi(db->row[2]);
       modbus->watchdog            = atoi(db->row[5]);
       modbus->max_request_par_sec = atoi(db->row[7]);
     }
    return(modbus);
  }
/******************************************************************************************************************************/
/* Deconnecter: Deconnexion du module                                                                                         */
/* Entrée: un id                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Deconnecter_module ( struct MODULE_MODBUS *module )
  { if (!module) return;
    if (module->started == FALSE) return;

    close ( module->connexion );
    module->connexion = 0;
    module->started = FALSE;
    module->request = FALSE;
    module->nbr_deconnect++;
    module->date_retente = Partage->top + MODBUS_RETRY;
    if (module->DI) g_free(module->DI);
    if (module->AI) g_free(module->AI);
    if (module->DO) g_free(module->DO);
    Dls_data_set_WATCHDOG ( NULL, module->modbus.tech_id, "IO_COMM", &module->bit_comm, 0 );
    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s': Module disconnected", __func__, module->modbus.tech_id );
  }
/******************************************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                                               */
/* Entrée: une nom et un password                                                                                             */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                                              */
/******************************************************************************************************************************/
 static gboolean Connecter_module ( struct MODULE_MODBUS *module )
  { struct addrinfo *result, *rp;
    struct timeval sndtimeout;
    struct addrinfo hints;
    gint connexion = 0, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    sndtimeout.tv_sec  = 10;
    sndtimeout.tv_usec =  0;

    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG, "%s: '%s' : Trying to connect module %d to %s", __func__,
              module->modbus.tech_id, module->modbus.id, module->modbus.hostname );

    s = getaddrinfo( module->modbus.hostname, "502", &hints, &result);
    if (s != 0)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                "%s: '%s': getaddrinfo Failed for module %d (%s) (%s)", __func__, module->modbus.tech_id,
                 module->modbus.id, module->modbus.hostname, gai_strerror(s) );
       return(FALSE);
     }

   /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next)
     { connexion = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
       if (connexion == -1)
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                   "%s: '%s': Socket creation failed for modbus %d (%s)", __func__, module->modbus.tech_id,
                    module->modbus.id, module->modbus.hostname );
          continue;
        }

       if ( setsockopt ( connexion, SOL_SOCKET, SO_SNDTIMEO, (char *)&sndtimeout, sizeof(sndtimeout)) < 0 )
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                   "%s: '%s': Socket Set Options failed for modbus %d (%s)", __func__, module->modbus.tech_id,
                    module->modbus.id, module->modbus.hostname );
          continue;
        }

       if (connect(connexion, rp->ai_addr, rp->ai_addrlen) != -1)
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
                   "%s: '%s': %d (%s) family=%d", __func__, module->modbus.tech_id,
                    module->modbus.id, module->modbus.hostname, rp->ai_family );

          break;  /* Success */
        }
       else
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE,
                   "%s: '%s': connexion refused by module %d (%s) family=%d error '%s'", __func__, module->modbus.tech_id,
                    module->modbus.id, module->modbus.hostname, rp->ai_family, strerror(errno) );
        }
       close(connexion);                                                       /* Suppression de la socket qui n'a pu aboutir */
     }
    freeaddrinfo(result);
    if (rp == NULL) return(FALSE);                                                                     /* Erreur de connexion */

    fcntl( connexion, F_SETFL, SO_KEEPALIVE | SO_REUSEADDR );
    module->connexion = connexion;                                                                        /* Sauvegarde du fd */
    module->date_last_reponse = Partage->top;
    module->date_retente   = 0;
    module->transaction_id = 1;
    module->started = TRUE;
    module->mode = MODBUS_GET_DESCRIPTION;
    module->DI = NULL;
    module->AI = NULL;
    module->DO = NULL;
    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s': Module Connected", __func__, module->modbus.tech_id );

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Interroger_description : envoie une commande d'identification au module                                                    */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_description( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x2020 );
    requete.nbr            = htons( 16 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed for module %d (%s): error %d", __func__, module->modbus.tech_id,
               module->modbus.id, module->modbus.hostname, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG, "%s: '%s': OK for %d",
                 __func__, module->modbus.tech_id, module->modbus.id );
       module->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_description : envoie une commande d'identification au module                                                    */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_firmware( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x2023 );
    requete.nbr            = htons( 16 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed for module %d (%s): error %d", __func__, module->modbus.tech_id,
               module->modbus.id, module->modbus.hostname, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG, "%s: '%s': OK for %d",
                 __func__, module->modbus.tech_id, module->modbus.id );
       module->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Init_watchdog1( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x100A );                                                                   /* Stop Timer */
    requete.valeur         = htons( 0x0000 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "%s: '%s': 'stop watchdog failed' for %d (error %d)", __func__, module->modbus.tech_id, module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
               "%s: '%s': 'stop watchdog OK' for %d", __func__, module->modbus.tech_id, module->modbus.id );
       module->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Init_watchdog2( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x1009 );                                   /* Close MODBUS socket after watchdog timeout */
    requete.valeur         = htons( 0x0001 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "%s: '%s': 'close modbus tcp on watchdog' failed for %d (error %d)", __func__, module->modbus.tech_id,
               module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                "%s: '%s': 'close modbus tcp on watchdog' OK for %d", __func__, module->modbus.tech_id, module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: This register stores the watchdog timeout value as an unsigned 16 bit value. The Description default value is 0.   */
/* Setting this value will not trigger the watchdog. However, a non zero value must be stored in this register before the     */
/* watchdog can be triggered. The time value is stored in multiples of 100ms (e.g., 0x0009 is .9 seconds). It is not possible */
/* to modify this value while the watchdog is running                                                                         */
/******************************************************************************************************************************/
 static void Init_watchdog3( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x1000 );                                                       /* Watchdog Time register */
    requete.valeur         = htons( module->modbus.watchdog );                     /* coupure sortie, en 100ième de secondes  */

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "%s: '%s': 'init watchdog timer' failed for %d (error %d)", __func__, module->modbus.tech_id,
               module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                "%s: '%s': 'init watchdog timer' OK for %d", __func__, module->modbus.tech_id, module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Init_watchdog4( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x100A );
    requete.valeur         = htons( 0x0001 );                                                                  /* Start Timer */

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                "%s: '%s': 'watchdog start' failed for %d (error %d)", __func__, module->modbus.tech_id,
                 module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                "%s: '%s': Init_watchdog_modbus: 'watchdog start' OK for %d", __func__, module->modbus.tech_id, module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_nbr_entree_ANA : Demander au module d'envoyer son nombre d'entree ANALOGIQUE                                    */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_nbr_entree_ANA( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1023 );
    requete.nbr            = htons( 0x0001 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed for %d (error %d)", __func__, module->modbus.tech_id,
                module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                "%s: '%s': OK for %d", __func__, module->modbus.tech_id, module->modbus.id );
       module->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_nbr_entree_ANA : Demander au module d'envoyer son nombre de sortie ANALOGIQUE                                   */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_nbr_sortie_ANA( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1022 );
    requete.nbr            = htons( 0x0001 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed %d (error %d)", __func__, module->modbus.tech_id,
                module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
               "%s: '%s': OK", __func__, module->modbus.tech_id, module->modbus.id );
       module->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_nbr_entree_TOR : Demander au module d'envoyer son nombre d'entree TOR                                           */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_nbr_entree_TOR( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1025 );
    requete.nbr            = htons( 0x0001 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed %d (error %d - '%s')", __func__, module->modbus.tech_id, module->modbus.id, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                "%s: '%s': OK for %d", __func__, module->modbus.tech_id, module->modbus.id );
       module->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_nbr_sortie_TOR : Demander au module d'envoyer son nombre de sortie TOR                                          */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_nbr_sortie_TOR( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1024 );
    requete.nbr            = htons( 0x0001 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                 "%s: '%s': write error Module %d (error %d, '%s')", __func__, module->modbus.tech_id,
                 module->modbus.id, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                 "%s: '%s': OK for %d", __func__, module->modbus.tech_id, module->modbus.id );
       module->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Interroger_entree_tor( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_COIL;
    requete.adresse        = 0x00;
    requete.nbr            = htons( module->nbr_entree_tor );

    if ( write ( module->connexion, &requete, 12 ) != 12 )                                             /* Envoi de la requete */
     { Deconnecter_module( module ); }
    else module->request = TRUE;                                                                  /* Une requete a élé lancée */
  }
/******************************************************************************************************************************/
/* Interroger_entree_ana: Interrogation des entrees analogique d'un module wago                                               */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Interroger_entree_ana( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = 0x00;
    requete.nbr            = htons( module->nbr_entree_ana );

    if ( write ( module->connexion, &requete, 12 ) != 12 )                                             /* Envoi de la requete */
     { Deconnecter_module( module ); }
    else module->request = TRUE;                                                                  /* Une requete a élé lancée */
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Interroger_sortie_tor( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint cpt_poid, cpt_byte, cpt, taille, nbr_data;

    memset(&requete, 0, sizeof(requete) );                                               /* Mise a zero globale de la requete */
    nbr_data = ((module->nbr_sortie_tor-1)/8)+1;
    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    taille                 = 0x0007 + nbr_data;
    requete.taille         = htons( taille );                                                                       /* taille */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_MULTIPLE_COIL;
    requete.adresse        = 0x00;
    requete.nbr            = htons( module->nbr_sortie_tor );                                                    /* bit count */
    requete.data[2]        = nbr_data;                                                                          /* Byte count */

    if (module->DO)
     { for ( cpt_poid = 1, cpt_byte = 3, cpt = 0; cpt<module->nbr_sortie_tor; cpt++ )
        { if (cpt_poid == 256) { cpt_byte++; cpt_poid = 1; }
          if ( module->DO[cpt] )
           { if (Dls_data_get_DO( NULL, NULL, &module->DO[cpt] ) ) { requete.data[cpt_byte] |= cpt_poid; } }
          cpt_poid = cpt_poid << 1;
        }
     }

    if ( write ( module->connexion, &requete, taille+6 ) != taille+6 )               /* Envoi de la requete (taille + header )*/
     { Deconnecter_module( module ); }
    else module->request = TRUE;                                                                  /* Une requete a élé lancée */
  }
/******************************************************************************************************************************/
/* Interroger_sortie_ana: Envoie les informations liées aux sorties ANA du module                                             */
/* Entrée: le module à interroger                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Interroger_sortie_ana( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint cpt_byte, cpt, taille;

    memset(&requete, 0, sizeof(requete) );                                               /* Mise a zero globale de la requete */
    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    taille                 = 0x0006 + (module->nbr_sortie_ana*2 + 1);
    requete.taille         = htons( taille );                                                                       /* taille */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_MULTIPLE_REGISTER;
    requete.adresse        = 0x00;
    requete.nbr            = htons( module->nbr_sortie_ana );                                                    /* bit count */
    requete.data[2]        = (module->nbr_sortie_ana*2);                                                        /* Byte count */
    for ( cpt_byte = 3, cpt = 0; cpt<module->nbr_sortie_ana; cpt++)
      { /* Attention, parser selon le type de sortie ! (12 bits ? 10 bits ? conversion ??? */
        requete.data [cpt_byte  ] = 0x30; /*Partage->aa[cpt_a].val_int>>5;*/
        requete.data [cpt_byte+1] = 0x00; /*(Partage->aa[cpt_a].val_int & 0x1F)<<3;*/
        cpt_byte += 2;
      }

    if ( write ( module->connexion, &requete, taille+6 ) != taille+6 )               /* Envoi de la requete (taille + header )*/
     { Deconnecter_module( module ); }
    else module->request = TRUE;                                                                  /* Une requete a élé lancée */
  }
/******************************************************************************************************************************/
/* Modbus_do_mapping : mappe les entrees/sorties Wago avec la zone de mémoire interne dynamique                               */
/* Entrée : la structure referencant le module                                                                                */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Modbus_do_mapping ( struct MODULE_MODBUS *module )
  { gchar critere[80];
    struct DB *db;

    module->AI = g_try_malloc0( sizeof(gpointer) * module->nbr_entree_ana );
    if (!module->AI)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "%s: '%s': Memory Error for AI", __func__, module->modbus.tech_id );
       return;
     } else Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
                      "%s: '%s': Allocated %d AI", __func__, module->modbus.tech_id, module->nbr_entree_ana );

    module->DI = g_try_malloc0( sizeof(gpointer) * module->nbr_entree_tor );
    if (!module->DI)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "%s: '%s': Memory Error for DI", __func__ , module->modbus.tech_id);
       return;
     } else Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
                      "%s: '%s': Allocated %d DI", __func__, module->modbus.tech_id, module->nbr_entree_tor );

    module->DO = g_try_malloc0( sizeof(gpointer) * module->nbr_sortie_tor );
    if (!module->DO)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "%s: '%s': Memory Error for DO", __func__, module->modbus.tech_id );
       return;
     } else Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
                      "%s: '%s': Allocated %d DO", __func__, module->modbus.tech_id, module->nbr_sortie_tor );

/******************************* Recherche des event text EA a raccrocher aux bits internes ***********************************/
    if ( ! Recuperer_mnemos_AI_by_tag ( &db, module->modbus.tech_id, "AI%%" ) )
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "%s: '%s': Error searching Database for '%s'",
                 __func__, module->modbus.tech_id, critere );
     }
    else while ( Recuperer_mnemos_AI_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *map_tag = db->row[2], *libelle = db->row[3];
       gint num;
       Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s': Map found '%s' '%s:%s' - %s",
                 __func__, module->modbus.tech_id, map_tag, tech_id, acro, libelle );
       if ( sscanf ( map_tag, "AI%d", &num ) == 1 )                                          /* Découpage de la ligne ev_text */
        { if (num<module->nbr_entree_ana)
           { Charger_confDB_AI ( tech_id, acro );
             Dls_data_get_AI ( tech_id, acro, &module->AI[num] );        /* bit déjà existant deja dans la structure DLS DATA */
           }
          else Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "%s: '%s': map '%s': num %d out of range '%d'",
                         __func__, module->modbus.tech_id, map_tag, num, module->nbr_entree_ana );
        }
       else Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "%s: '%s': event '%s': Sscanf Error",
                      __func__, module->modbus.tech_id, map_tag );
     }
/******************************* Recherche des event text EA a raccrocher aux bits internes ***********************************/
    if ( ! Recuperer_mnemos_DI_by_tag ( &db, module->modbus.tech_id, "DI%%" ) )
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "%s: '%s': Error searching Database for '%s'",
                 __func__, module->modbus.tech_id, critere );
     }
    else while ( Recuperer_mnemos_DI_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *libelle = db->row[3], *map_tag = db->row[2];
       gint num;
       Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s': Map found '%s' '%s:%s' - %s",
                 __func__, module->modbus.tech_id, map_tag, tech_id, acro, libelle );
       if ( sscanf ( map_tag, "DI%d", &num ) == 1 )                                          /* Découpage de la ligne ev_text */
        { if (num<module->nbr_entree_tor)
           { Dls_data_get_DI ( tech_id, acro, &module->DI[num] );        /* bit déjà existant deja dans la structure DLS DATA */
             if(module->DI[num] == NULL) Dls_data_set_DI ( NULL, tech_id, acro, &module->DI[num], FALSE );/* Sinon, on le crée */
           }
          else Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "%s: '%s': map '%s': num %d out of range '%d'",
                         __func__, module->modbus.tech_id, map_tag, num, module->nbr_entree_tor );
        }
       else Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "%s: '%s': event '%s': Sscanf Error",
                      __func__, module->modbus.tech_id, map_tag );
     }
/*********************************** Recherche des events DO a raccrocher aux bits internes ***********************************/
    if ( ! Recuperer_mnemos_DO_by_tag ( &db, module->modbus.tech_id, "DO%%" ) )
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "%s: '%s': Error searching Database for '%s'",
                 __func__, module->modbus.tech_id, critere );
     }
    else while ( Recuperer_mnemos_DO_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *libelle = db->row[3], *map_tag = db->row[2];
       gint num;
       Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s': Map found '%s' '%s:%s' - %s",
                 __func__, module->modbus.tech_id, map_tag, tech_id, acro, libelle );
       if ( sscanf ( map_tag, "DO%d", &num ) == 1 )                                          /* Découpage de la ligne ev_text */
        { if (num<module->nbr_sortie_tor)
           { Dls_data_get_DO ( tech_id, acro, &module->DO[num] );        /* bit déjà existant deja dans la structure DLS DATA */
             if(module->DO[num] == NULL) Dls_data_set_DO ( NULL, tech_id, acro, &module->DO[num], FALSE );     /* Sinon, on le crée */
           }
          else Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "%s: '%s': map '%s': num %d out of range '%d'",
                         __func__, module->modbus.tech_id, map_tag, num, module->nbr_sortie_tor );
        }
       else Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "%s: '%s': event '%s': Sscanf Error",
                      __func__, module->modbus.tech_id, map_tag );
     }
/******************************* Recherche des event text EA a raccrocher aux bits internes ***********************************/
    Dls_data_set_WATCHDOG ( NULL, module->modbus.tech_id, "IO_COMM", &module->bit_comm, 0 );

    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE, "%s: '%s': Module '%s' : mapping done",
              __func__, module->modbus.tech_id, module->modbus.description );
  }
/******************************************************************************************************************************/
/* Recuperer_borne: Recupere les informations d'une borne MODBUS                                                              */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Modbus_Processer_trame( struct MODULE_MODBUS *module )
  { module->nbr_oct_lu = 0;
    module->request = FALSE;                                                                     /* Une requete a été traitée */

    if ( (guint16) module->response.proto_id )
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "%s: '%s': wrong proto_id", __func__, module->modbus.tech_id );
       Deconnecter_module( module );
     }
    else
     { int cpt_byte, cpt_poid, cpt;
       module->date_last_reponse = Partage->top;                                                   /* Estampillage de la date */
       Dls_data_set_WATCHDOG ( NULL, module->modbus.tech_id, "IO_COMM", &module->bit_comm, 600 );
       if (ntohs(module->response.transaction_id) != module->transaction_id)                              /* Mauvaise reponse */
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                   "%s: '%s': wrong transaction_id for module %d  attendu %d, recu %d", __func__, module->modbus.tech_id,
                    module->modbus.id, module->transaction_id, ntohs(module->response.transaction_id) );
        }
       if ( module->response.fct >=0x80 )
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                   "%s: '%s': Erreur Reponse Module %d, Error %d, Exception code %d", __func__, module->modbus.tech_id,
                    module->modbus.id, module->response.fct, (int)module->response.data[0] );
          Deconnecter_module( module );
        }
       else switch (module->mode)
        { case MODBUS_GET_DI:
               for ( cpt_poid = 1, cpt_byte = 1, cpt = 0; cpt<module->nbr_entree_tor; cpt++)
                { Dls_data_set_DI ( NULL, NULL, NULL, (gpointer)&module->DI[cpt], (module->response.data[ cpt_byte ] & cpt_poid) );
                  cpt_poid = cpt_poid << 1;
                  if (cpt_poid == 256) { cpt_byte++; cpt_poid = 1; }
                }
               module->mode = MODBUS_GET_AI;
               break;
          case MODBUS_GET_AI:
               for ( cpt = 0; cpt<module->nbr_entree_ana; cpt++)
                { struct DLS_AI *ai = module->AI[cpt];
                  if (!ai) continue;                                                 /* Si pas mappé, bah on ne la stocke pas */
                  switch( ai->type )
                   { case ENTREEANA_WAGO_750455:
                          if ( ! (module->response.data[ 2*cpt + 2 ] & 0x03) )
                           { int reponse;
                             reponse  = module->response.data[ 2*cpt + 1 ] << 5;
                             reponse |= module->response.data[ 2*cpt + 2 ] >> 3;
                             Dls_data_set_AI ( NULL, NULL, &module->AI[cpt], reponse, TRUE );
                           }
                          else { Dls_data_set_AI ( NULL, NULL, &module->AI[cpt], 0.0, FALSE );
                               }
                          break;
                     case ENTREEANA_WAGO_750461:                                                               /* Borne PT100 */
                           { gint16 reponse;
                             reponse  = module->response.data[ 2*cpt + 1 ] << 8;
                             reponse |= module->response.data[ 2*cpt + 2 ];
                             Dls_data_set_AI ( NULL, NULL, &module->AI[cpt], reponse, TRUE );
                           }
                          break;
                     default : break;
                   }
                }
               module->mode = MODBUS_SET_DO;
               break;
          case MODBUS_SET_DO:
               module->mode = MODBUS_SET_AO;
               break;
          case MODBUS_SET_AO:
               module->mode = MODBUS_GET_DI;
               break;
          case MODBUS_GET_DESCRIPTION:
             { gchar chaine[32];
               gint taille;
               memset ( chaine, 0, sizeof(chaine) );
               taille = module->response.data[0];
               if (taille>=sizeof(chaine)) taille=sizeof(chaine)-1;
               chaine[0] = ntohs( (gint16)module->response.data[1] );
               chaine[2] = ntohs( (gint16)module->response.data[3] );
               chaine[taille] = 0;
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s': Module %d Get Description (size %d) '%s'",
                         __func__, module->modbus.tech_id, module->modbus.id, taille, chaine );
               module->mode = MODBUS_GET_FIRMWARE;
               break;
            }
          case MODBUS_GET_FIRMWARE:
             { gchar chaine[64];
               gint taille;
               memset ( chaine, 0, sizeof(chaine) );
               taille = module->response.data[0];
               if (taille>=sizeof(chaine)) taille=sizeof(chaine)-1;
               chaine[0] = ntohs( (gint16)module->response.data[1] );
               chaine[2] = ntohs( (gint16)module->response.data[3] );
               chaine[taille] = 0;
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s': Module %d Get Firmware (size %d) %s",
                         __func__, module->modbus.tech_id, module->modbus.id, taille, chaine );
               module->mode = MODBUS_INIT_WATCHDOG1;
               break;
            }
          case MODBUS_INIT_WATCHDOG1:
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG, "%s: '%s': Module %d Watchdog1 = %d %d",
                         __func__, module->modbus.tech_id, module->modbus.id,
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 0) ),
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 2) )
                       );
               module->mode = MODBUS_INIT_WATCHDOG2;
               break;
          case MODBUS_INIT_WATCHDOG2:
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG, "%s: '%s': Module %d Watchdog2 = %d %d",
                         __func__, module->modbus.tech_id, module->modbus.id,
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 0) ),
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 2) )
                       );
               module->mode = MODBUS_INIT_WATCHDOG3;
               break;
          case MODBUS_INIT_WATCHDOG3:
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG, "%s: '%s': Module %d Watchdog3 = %d %d",
                         __func__, module->modbus.tech_id, module->modbus.id,
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 0) ),
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 2) )
                       );
               module->mode = MODBUS_INIT_WATCHDOG4;
               break;
          case MODBUS_INIT_WATCHDOG4:
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG, "%s: '%s': Module %d Watchdog4 = %d %d",
                         __func__, module->modbus.tech_id, module->modbus.id,
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 0) ),
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 2) )
                       );
               module->mode = MODBUS_GET_NBR_AI;
               break;
          case MODBUS_GET_NBR_AI:
               module->nbr_entree_ana = ntohs( *(gint16 *)((gchar *)&module->response.data + 1) ) / 16;
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s': Module %d Get number Entree ANA = %d",
                         __func__, module->modbus.tech_id, module->modbus.id, module->nbr_entree_ana
                       );
               module->mode = MODBUS_GET_NBR_AO;
               break;
          case MODBUS_GET_NBR_AO:
               module->nbr_sortie_ana = ntohs( *(gint16 *)((gchar *)&module->response.data + 1) ) / 16;
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s': Module %d Get number Sortie ANA = %d",
                         __func__, module->modbus.tech_id, module->modbus.id, module->nbr_sortie_ana
                       );
               module->mode = MODBUS_GET_NBR_DI;
               break;
          case MODBUS_GET_NBR_DI:
                { gint nbr;
                  nbr = ntohs( *(gint16 *)((gchar *)&module->response.data + 1) );
                  module->nbr_entree_tor = nbr;
                  Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s': Module %d Get number Entree TOR = %d",
                            __func__, module->modbus.tech_id, module->modbus.id, module->nbr_entree_tor );
                  module->mode = MODBUS_GET_NBR_DO;
                }
               break;
          case MODBUS_GET_NBR_DO:
               module->nbr_sortie_tor = ntohs( *(gint16 *)((gchar *)&module->response.data + 1) );
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s': Module %d Get number Sortie TOR = %d",
                         __func__, module->modbus.tech_id, module->modbus.id, module->nbr_sortie_tor );
               module->mode = MODBUS_GET_DI;
               Modbus_do_mapping( module );                                        /* Initialise le mapping des I/O du module */
               break;
        }
     }
    memset (&module->response, 0, sizeof(struct TRAME_MODBUS_REPONSE) );
  }
/******************************************************************************************************************************/
/* Recuperer_borne: Recupere les informations d'une borne MODBUS                                                              */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Recuperer_reponse_module( struct MODULE_MODBUS *module )
  { fd_set fdselect;
    struct timeval tv;
    gint retval, cpt;

    if (module->date_last_reponse + 600 < Partage->top)                                      /* Detection attente trop longue */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                "%s: '%s': Timeout module %d, enable=%d, started=%d, mode=%02d, "
                "transactionID=%06d, nbr_deconnect=%02d, last_reponse=%03ds ago, retente=in %03ds, date_next_eana=in %03ds",
                 __func__, module->modbus.tech_id, module->modbus.id, module->modbus.enable, module->started, module->mode,
                 module->transaction_id, module->nbr_deconnect,
                (Partage->top - module->date_last_reponse)/10,
                (module->date_retente > Partage->top   ? (module->date_retente   - Partage->top)/10 : -1),
                (module->date_next_eana > Partage->top ? (module->date_next_eana - Partage->top)/10 : -1)
               );
       Deconnecter_module( module );
       return;
     }

    FD_ZERO(&fdselect);
    FD_SET(module->connexion, &fdselect );
    tv.tv_sec = 0;
    tv.tv_usec= 1000;                                                                               /* Attente d'un caractere */
    retval = select(module->connexion+1, &fdselect, NULL, NULL, &tv );

    if ( retval>0 && FD_ISSET(module->connexion, &fdselect) )
     { int bute;
       if (module->nbr_oct_lu<TAILLE_ENTETE_MODBUS)
            { bute = TAILLE_ENTETE_MODBUS; }
       else { bute = TAILLE_ENTETE_MODBUS + ntohs(module->response.taille); }

       if (bute>=sizeof(struct TRAME_MODBUS_REPONSE))
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_CRIT,
                   "%s: '%s': bute = %d >= %d (sizeof(module->reponse)=%d, taille recue = %d)", __func__, module->modbus.tech_id,
                    bute, sizeof(struct TRAME_MODBUS_REPONSE), sizeof(module->response), ntohs(module->response.taille) );
          Deconnecter_module( module );
          return;
        }

       cpt = read( module->connexion, (unsigned char *)&module->response + module->nbr_oct_lu, bute-module->nbr_oct_lu );
       if (cpt>=0)
        { module->nbr_oct_lu += cpt;
          if (module->nbr_oct_lu >= TAILLE_ENTETE_MODBUS + ntohs(module->response.taille))
           { Modbus_Processer_trame( module );                                      /* Si l'on a trouvé une trame complète !! */
             module->nbr_oct_lu = 0;
           }
        }
       else
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                    "%s: '%s': Read Error for %d. Get %d, error %s", __func__, module->modbus.tech_id,
                    module->modbus.id, cpt, strerror(errno) );
          Deconnecter_module ( module );
        }
      }
  }
/******************************************************************************************************************************/
/* Run_modbus_thread: Fait tourner un module modbus particulier                                                               */
/******************************************************************************************************************************/
 static void Run_modbus_thread ( struct MODULE_MODBUS *module )
  { gchar thread_name[30];
    g_snprintf( thread_name, sizeof(thread_name), "W-MODBUS%02d", module->modbus.id );
    prctl(PR_SET_NAME, thread_name, 0, 0, 0 );
    module->TID = pthread_self();                                                           /* Sauvegarde du TID pour le pere */
    module->last_top = 0;
    module->nbr_request = 0;
    module->nbr_request_par_sec = 0;
    module->delai = 0;

    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s' Started", __func__, module->modbus.tech_id );

    if (Dls_auto_create_plugin( module->modbus.tech_id, "Gestion du Wago" ) == FALSE)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", module->modbus.tech_id ); }
    Mnemo_auto_create_WATCHDOG ( FALSE, module->modbus.tech_id, "IO_COMM", "Statut de la communication avec le Wago" );

    while(Cfg_modbus.lib->Thread_run == TRUE && Cfg_modbus.lib->Thread_reload == FALSE)      /* On tourne tant que necessaire */
     { sched_yield();
       usleep(module->delai);

       if (Partage->top>=module->last_top+10)                                                        /* Toutes les 1 secondes */
        { module->nbr_request_par_sec = module->nbr_request;
          module->nbr_request = 0;
          if(module->nbr_request_par_sec > module->modbus.max_request_par_sec) module->delai += 50;
          else if(module->delai>0) module->delai -= 50;
          module->last_top = Partage->top;
        }
       if ( module->modbus.enable == FALSE && module->started )                                     /* Module a deconnecter ! */
        { Deconnecter_module ( module );
          continue;
        }

       if ( module->modbus.enable == FALSE ||                          /* Si module DOWN ou si UP mais dans le delai de retry */
            Partage->top < module->date_retente )                                  /* Si attente retente, on change de module */
        { sleep(1); continue; }

/********************************************* Début de l'interrogation du module *********************************************/
       if ( ! module->started )                                                                  /* Communication OK ou non ? */
        { if ( ! Connecter_module( module ) )
           { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s': Module %03d DOWN. retrying in %ds",
                       __func__, module->modbus.tech_id, module->modbus.id, MODBUS_RETRY/10 );
             module->date_retente = Partage->top + MODBUS_RETRY;
           }
        }
       else
        { if ( module->request )                                                         /* Requete en cours pour ce module ? */
           { Recuperer_reponse_module ( module ); }
          else
           { if (module->date_next_eana<Partage->top)                                  /* Gestion décalée des I/O Analogiques */
              { module->date_next_eana = Partage->top + MBUS_TEMPS_UPDATE_IO_ANA;                       /* Tous les 2 dixieme */
                module->do_check_eana = TRUE;
              }
             switch (module->mode)
              { case MODBUS_GET_DESCRIPTION: Interroger_description( module ); break;
                case MODBUS_GET_FIRMWARE   : Interroger_firmware( module ); break;
                case MODBUS_INIT_WATCHDOG1 : Init_watchdog1( module ); break;
                case MODBUS_INIT_WATCHDOG2 : Init_watchdog2( module ); break;
                case MODBUS_INIT_WATCHDOG3 : Init_watchdog3( module ); break;
                case MODBUS_INIT_WATCHDOG4 : Init_watchdog4( module ); break;
                case MODBUS_GET_NBR_AI     : Interroger_nbr_entree_ANA( module ); break;
                case MODBUS_GET_NBR_AO     : Interroger_nbr_sortie_ANA( module ); break;
                case MODBUS_GET_NBR_DI     : Interroger_nbr_entree_TOR( module ); break;
                case MODBUS_GET_NBR_DO     : Interroger_nbr_sortie_TOR( module ); break;
                case MODBUS_GET_DI         : if (module->nbr_entree_tor) Interroger_entree_tor( module );
                                             else module->mode = MODBUS_GET_AI;
                                             break;
                case MODBUS_GET_AI         : if (module->nbr_entree_ana && module->do_check_eana)
                                                  Interroger_entree_ana( module );
                                             else module->mode = MODBUS_SET_DO;
                                             break;
                case MODBUS_SET_DO         : if (module->nbr_sortie_tor) Interroger_sortie_tor( module );
                                             else module->mode = MODBUS_SET_AO;
                                             break;
                case MODBUS_SET_AO         : if (module->nbr_sortie_ana && module->do_check_eana)
                                              { Interroger_sortie_ana( module );
                                              }
                                             else module->mode = MODBUS_GET_DI;
                                             module->do_check_eana = FALSE;                              /* Le check est fait */
                                             module->nbr_request++;
                                             break;
              }
           }
       }
     }
    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: '%s' Exited", __func__, module->modbus.tech_id );
    pthread_exit(GINT_TO_POINTER(0));
  }
/******************************************************************************************************************************/
/* Charger_tous_Modbus: Requete la DB pour charger les modules et les bornes modbus                                           */
/* Entrée: rien                                                                                                               */
/* Sortie: le nombre de modules trouvé                                                                                        */
/******************************************************************************************************************************/
 static gboolean Charger_tous_MODBUS ( void  )
  { struct MODBUSDB *modbus;
    struct DB *db;
    gint cpt;

    db = Init_DB_SQL();
    if (!db) return(FALSE);

/*************************************************** Chargement des modules ***************************************************/
    if ( ! Recuperer_modbusDB( db ) )
     { Libere_DB_SQL( &db );
       return(FALSE);
     }

    Cfg_modbus.Modules_MODBUS = NULL;
    cpt = 0;
    while ( (modbus = Recuperer_modbusDB_suite(db)) != NULL )
     { struct MODULE_MODBUS *module;
       pthread_t tid;

       module = (struct MODULE_MODBUS *)g_try_malloc0( sizeof(struct MODULE_MODBUS) );
       if (!module)                                                                       /* Si probleme d'allocation mémoire */
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR,
                   "%s: Erreur allocation mémoire struct MODULE_MODBUS", __func__ );
          g_free(modbus);
          Libere_DB_SQL( &db );
          return(FALSE);
        }
       memcpy( &module->modbus, modbus, sizeof(struct MODBUSDB) );
       g_free(modbus);
       cpt++;                                                                  /* Nous avons ajouté un module dans la liste ! */
                                                                                            /* Ajout dans la liste de travail */
       Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
                "%s: tech_id='%s', enable='%d'", __func__, module->modbus.tech_id, module->modbus.enable );
       pthread_create( &tid, NULL, (void *)Run_modbus_thread, module );
       Cfg_modbus.Modules_MODBUS = g_slist_prepend ( Cfg_modbus.Modules_MODBUS, module );
     }
    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "%s: %d modules MODBUS found  !", __func__, cpt );
    Libere_DB_SQL( &db );
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Decharger_tous_modbus: Decharge l'ensemble des modules MODBUS                                                              */
/* Entrée: rien                                                                                                               */
/* Sortie: rien                                                                                                               */
/******************************************************************************************************************************/
 static void Decharger_tous_MODBUS ( void  )
  { struct MODULE_MODBUS *module;
    while ( Cfg_modbus.Modules_MODBUS )
     { module = (struct MODULE_MODBUS *)Cfg_modbus.Modules_MODBUS->data;
       Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                 "%s: Wait for sub-process end : W-MODBUS%02d '%s'", __func__, module->modbus.id, module->modbus.hostname );
       pthread_join( module->TID, NULL );                                                              /* Attente fin du fils */
       Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE,
                 "%s: Sub-process ended : W-MODBUS%02d '%s'", __func__, module->modbus.id, module->modbus.hostname );
       Cfg_modbus.Modules_MODBUS = g_slist_remove ( Cfg_modbus.Modules_MODBUS, module );
       g_free(module);
     }
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du MODBUS                                                                                        */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  {

reload:
    memset( &Cfg_modbus, 0, sizeof(Cfg_modbus) );                                   /* Mise a zero de la structure de travail */
    Cfg_modbus.lib = lib;                                          /* Sauvegarde de la structure pointant sur cette librairie */
    Thread_init ( "W-MODBUS", "I/O", lib, WTD_VERSION, "Manage Modbus System" );
    Modbus_Lire_config ();                                                  /* Lecture de la configuration logiciel du thread */
    if (Config.instance_is_master==FALSE)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE,
                "%s: Instance is not Master. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }
    Modbus_Creer_DB();

    Cfg_modbus.Modules_MODBUS = NULL;                                                         /* Init des variables du thread */

    if ( Charger_tous_MODBUS() == FALSE )                                                    /* Chargement des modules modbus */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "%s: No module MODBUS found -> stop", __func__ );
       Cfg_modbus.lib->Thread_run = FALSE;                                                      /* Le thread ne tourne plus ! */
     }

    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { usleep(100000);
     }
    Decharger_tous_MODBUS();

end:
    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
