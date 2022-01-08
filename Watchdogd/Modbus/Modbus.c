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

/******************************************************************************************************************************/
/* Modbus_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                 */
/* Entrée: le pointeur sur la PROCESS                                                                                         */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Modbus_Creer_DB ( struct PROCESS *lib )
  { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE,
             "%s: Database_Version detected = '%05d'.", __func__, lib->database_version );

    SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `%s` ("
                    "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                    "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                    "`uuid` VARCHAR(37) COLLATE utf8_unicode_ci NOT NULL,"
                    "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                    "`description` VARCHAR(128) COLLATE utf8_unicode_ci NOT NULL DEFAULT 'DEFAULT',"
                    "`enable` TINYINT(1) NOT NULL DEFAULT '0',"
                    "`hostname` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                    "`watchdog` INT(11) NOT NULL DEFAULT 50,"
                    "`max_request_par_sec` INT(11) NOT NULL DEFAULT 50,"
                    "FOREIGN KEY (`uuid`) REFERENCES `processes` (`uuid`) ON DELETE CASCADE ON UPDATE CASCADE"
                    ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;", lib->name );

    SQL_Write_new ( "CREATE TABLE IF NOT EXISTS `modbus_DI` ("
                    "`id` int(11) PRIMARY KEY AUTO_INCREMENT,"
                    "`date_create` DATETIME NOT NULL DEFAULT NOW(),"
                    "`thread_tech_id` VARCHAR(32) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                    "`thread_acronyme` VARCHAR(64) COLLATE utf8_unicode_ci UNIQUE NOT NULL DEFAULT '',"
                    "UNIQUE (thread_tech_id, thread_acronyme),"
                    "FOREIGN KEY (`thread_tech_id`) REFERENCES `modbus` (`thread_tech_id`) ON DELETE CASCADE ON UPDATE CASCADE"
                    ") ENGINE=INNODB  DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci AUTO_INCREMENT=10000 ;" );

    if (lib->database_version < 3)
     { SQL_Write_new ( "INSERT INTO mappings (thread_tech_id, thread_acronyme, tech_id, acronyme) "
                       "SELECT map_tech_id, "
                       "CONCAT ( 'DI', LPAD ( CONVERT (REPLACE ( mnemos_DI.map_tag, 'DI', '' ), INTEGER ), 3, '0' ) ), tech_id, acronyme "
                       "FROM mnemos_DI WHERE map_thread ='MODBUS' " );
     }

    Process_set_database_version ( lib, 3 );
  }
/******************************************************************************************************************************/
/* Deconnecter: Deconnexion du module                                                                                         */
/* Entrée: un id                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Deconnecter_module ( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    if (!module) return;
    if (vars->started == FALSE) return;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    close ( vars->connexion );
    vars->connexion = 0;
    vars->started = FALSE;
    vars->request = FALSE;
    vars->nbr_deconnect++;
    vars->date_retente = Partage->top + MODBUS_RETRY;
    if (vars->DI_root) json_node_unref(vars->DI_root);
    if (vars->DI) g_free(vars->DI);
    if (vars->AI) g_free(vars->AI);
    if (vars->DO) g_free(vars->DO);
    SubProcess_send_comm_to_master_new ( module, FALSE );
    Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': Module '%s' disconnected", __func__, thread_tech_id, hostname );
  }
/******************************************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                                               */
/* Entrée: une nom et un password                                                                                             */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                                              */
/******************************************************************************************************************************/
 static gboolean Connecter_module ( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct addrinfo *result, *rp;
    struct timeval sndtimeout;
    struct addrinfo hints;
    gint connexion = 0, s;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    sndtimeout.tv_sec  = 10;
    sndtimeout.tv_usec =  0;

    Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s' : Trying to connect module to '%s'", __func__,
              thread_tech_id, hostname );

    s = getaddrinfo( hostname, "502", &hints, &result);
    if (s != 0)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                "%s: '%s': getaddrinfo Failed for module %s (%s)", __func__, thread_tech_id, hostname, gai_strerror(s) );
       return(FALSE);
     }

   /* getaddrinfo() returns a list of address structures.
       Try each address until we successfully connect(2).
       If socket(2) (or connect(2)) fails, we (close the socket
       and) try the next address. */

    for (rp = result; rp != NULL; rp = rp->ai_next)
     { connexion = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
       if (connexion == -1)
        { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                   "%s: '%s': Socket creation failed for modbus '%s'", __func__, thread_tech_id, hostname );
          continue;
        }

       if ( setsockopt ( connexion, SOL_SOCKET, SO_SNDTIMEO, (char *)&sndtimeout, sizeof(sndtimeout)) < 0 )
        { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                   "%s: '%s': Socket Set Options failed for modbus '%s'", __func__, thread_tech_id, hostname );
          continue;
        }

       if (connect(connexion, rp->ai_addr, rp->ai_addrlen) != -1)
        { Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
                   "%s: '%s': Using family=%d for host '%s'", __func__, thread_tech_id, rp->ai_family, hostname );

          break;  /* Success */
        }
       else
        { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                   "%s: '%s': connexion refused by module '%s' family=%d error '%s'", __func__,
                   thread_tech_id, hostname, rp->ai_family, strerror(errno) );
        }
       close(connexion);                                                       /* Suppression de la socket qui n'a pu aboutir */
     }
    freeaddrinfo(result);
    if (rp == NULL) return(FALSE);                                                                     /* Erreur de connexion */

    fcntl( connexion, F_SETFL, SO_KEEPALIVE | SO_REUSEADDR );
    vars->connexion = connexion;                                                                          /* Sauvegarde du fd */
    vars->date_last_reponse = Partage->top;
    vars->date_retente   = 0;
    vars->transaction_id = 1;
    vars->started = TRUE;
    vars->mode = MODBUS_GET_DESCRIPTION;
    vars->DI = NULL;
    vars->DI_root = NULL;
    vars->AI = NULL;
    vars->DO = NULL;
    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: '%s': Module Connected", __func__, thread_tech_id );

    return(TRUE);
  }
/******************************************************************************************************************************/
/* Interroger_description : envoie une commande d'identification au module                                                    */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_description( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x2020 );
    requete.nbr            = htons( 16 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': OK", __func__, thread_tech_id );
       vars->request = TRUE;                                                                      /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_description : envoie une commande d'identification au module                                                    */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_firmware( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x2023 );
    requete.nbr            = htons( 16 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': OK", __func__, thread_tech_id );
       vars->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Init_watchdog1( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x100A );                                                                   /* Stop Timer */
    requete.valeur         = htons( 0x0000 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': OK", __func__, thread_tech_id );
       vars->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Init_watchdog2( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x1009 );                                   /* Close MODBUS socket after watchdog timeout */
    requete.valeur         = htons( 0x0001 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': OK", __func__, thread_tech_id );
       vars->request = TRUE;                                                /* Une requete a élé lancée */
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
 static void Init_watchdog3( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x1000 );                                                       /* Watchdog Time register */
    requete.valeur         = htons( Json_get_int ( module->config, "watchdog" ) ); /* coupure sortie, en 100ième de secondes  */

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': OK", __func__, thread_tech_id );
       vars->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Init_watchdog4( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x100A );
    requete.valeur         = htons( 0x0001 );                                                                  /* Start Timer */

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': OK", __func__, thread_tech_id );
       vars->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_nbr_entree_ANA : Demander au module d'envoyer son nombre d'entree ANALOGIQUE                                    */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_nbr_entree_ANA( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1023 );
    requete.nbr            = htons( 0x0001 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': OK", __func__, thread_tech_id );
       vars->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_nbr_entree_ANA : Demander au module d'envoyer son nombre de sortie ANALOGIQUE                                   */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_nbr_sortie_ANA( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1022 );
    requete.nbr            = htons( 0x0001 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': OK", __func__, thread_tech_id );
       vars->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_nbr_entree_TOR : Demander au module d'envoyer son nombre d'entree TOR                                           */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_nbr_entree_TOR( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1025 );
    requete.nbr            = htons( 0x0001 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
               "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': OK", __func__, thread_tech_id );
       vars->request = TRUE;                                                                    /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_nbr_sortie_TOR : Demander au module d'envoyer son nombre de sortie TOR                                          */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Interroger_nbr_sortie_TOR( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                                                /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1024 );
    requete.nbr            = htons( 0x0001 );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                 "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': OK", __func__, thread_tech_id );
       vars->request = TRUE;                                                                      /* Une requete a élé lancée */
     }
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Interroger_entree_tor( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_COIL;
    requete.adresse        = 0x00;
    requete.nbr            = htons( vars->nbr_entree_tor );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                 "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else { vars->request = TRUE; }                                                                /* Une requete a élé lancée */
  }
/******************************************************************************************************************************/
/* Interroger_entree_ana: Interrogation des entrees analogique d'un module wago                                               */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Interroger_entree_ana( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                                               /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = 0x00;
    requete.nbr            = htons( vars->nbr_entree_ana );

    gint retour = write ( vars->connexion, &requete, 12 );
    if ( retour != 12 )                                                                                /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                 "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else { vars->request = TRUE; }                                                                /* Une requete a élé lancée */
  }
/******************************************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                                      */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Interroger_sortie_tor( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint cpt_poid, cpt_byte, cpt, taille, nbr_data;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    memset(&requete, 0, sizeof(requete) );                                               /* Mise a zero globale de la requete */
    nbr_data = ((vars->nbr_sortie_tor-1)/8)+1;
    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    taille                 = 0x0007 + nbr_data;
    requete.taille         = htons( taille );                                                                       /* taille */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_MULTIPLE_COIL;
    requete.adresse        = 0x00;
    requete.nbr            = htons( vars->nbr_sortie_tor );                                                    /* bit count */
    requete.data[2]        = nbr_data;                                                                          /* Byte count */

    if (vars->DO)
     { for ( cpt_poid = 1, cpt_byte = 3, cpt = 0; cpt<vars->nbr_sortie_tor; cpt++ )
        { if (cpt_poid == 256) { cpt_byte++; cpt_poid = 1; }
          if ( vars->DO[cpt] )
           { if (Dls_data_get_DO( NULL, NULL, &vars->DO[cpt] ) ) { requete.data[cpt_byte] |= cpt_poid; } }
          cpt_poid = cpt_poid << 1;
        }
     }

    gint retour = write ( vars->connexion, &requete, taille+6 );
    if ( retour != taille+6 )                                                                          /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                 "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else { vars->request = TRUE; }                                                                /* Une requete a élé lancée */
  }
/******************************************************************************************************************************/
/* Interroger_sortie_ana: Envoie les informations liées aux sorties ANA du module                                             */
/* Entrée: le module à interroger                                                                                             */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Interroger_sortie_ana( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    struct TRAME_MODBUS_REQUETE requete;                                                     /* Definition d'une trame MODBUS */
    gint cpt_byte, cpt, taille;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );
    gchar *hostname       = Json_get_string ( module->config, "hostname" );

    memset(&requete, 0, sizeof(requete) );                                               /* Mise a zero globale de la requete */
    vars->transaction_id++;
    requete.transaction_id = htons(vars->transaction_id);
    requete.proto_id       = 0x00;                                                                            /* -> 0 = MOBUS */
    taille                 = 0x0006 + (vars->nbr_sortie_ana*2 + 1);
    requete.taille         = htons( taille );                                                                       /* taille */
    requete.unit_id        = 0x00;                                                                                    /* 0xFF */
    requete.fct            = MBUS_WRITE_MULTIPLE_REGISTER;
    requete.adresse        = 0x00;
    requete.nbr            = htons( vars->nbr_sortie_ana );                                                    /* bit count */
    requete.data[2]        = (vars->nbr_sortie_ana*2);                                                        /* Byte count */
    for ( cpt_byte = 3, cpt = 0; cpt<vars->nbr_sortie_ana; cpt++)
      { /* Attention, parser selon le type de sortie ! (12 bits ? 10 bits ? conversion ??? */
        requete.data [cpt_byte  ] = 0x30; /*Partage->aa[cpt_a].val_int>>5;*/
        requete.data [cpt_byte+1] = 0x00; /*(Partage->aa[cpt_a].val_int & 0x1F)<<3;*/
        cpt_byte += 2;
      }

    gint retour = write ( vars->connexion, &requete, taille+6 );
    if ( retour != taille+6 )                                                                          /* Envoi de la requete */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                 "%s: '%s': failed for module '%s': error %d/%s", __func__, thread_tech_id, hostname, retour, strerror(errno) );
       Deconnecter_module( module );
     }
    else { vars->request = TRUE; }                                                                /* Une requete a élé lancée */
  }
/******************************************************************************************************************************/
/* Modbus_do_mapping : mappe les entrees/sorties Wago avec la zone de mémoire interne dynamique                               */
/* Entrée : la structure referencant le module                                                                                */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Modbus_do_mapping ( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    gchar critere[80];
    struct DB *db;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    vars->AI = g_try_malloc0( sizeof(gpointer) * vars->nbr_entree_ana );
    if (!vars->AI)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': Memory Error for AI", __func__, thread_tech_id );
       return;
     } else Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
                      "%s: '%s': Allocated %d AI", __func__, thread_tech_id, vars->nbr_entree_ana );

/***************************************************** Mapping des DigitalInput ***********************************************/
    vars->DI_root = Json_node_create();
    if (!vars->DI_root)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': Memory Error for DI", __func__ , thread_tech_id); }
    else
     { Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': Allocated %d DI", __func__, thread_tech_id, vars->nbr_entree_tor );
       SQL_Select_to_json_node ( vars->DI_root, "modbus_DI",
                                 "SELECT di.*, mappings.tech_id, mappings.acronyme FROM modbus_DI AS di "
                                 "INNER JOIN mappings ON mappings.thread_tech_id  = di.thread_tech_id "
                                 "                   AND mappings.thread_acronyme = di.thread_acronyme "
                                 "WHERE modbus.thread_tech_id='%s'", thread_tech_id );

       vars->DI = g_try_malloc0( sizeof(JsonNode *) * vars->nbr_entree_tor );
       if (!vars->DI)
        { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': Memory Error for DI", __func__ , thread_tech_id);
          return;
        }

       JsonArray *array = Json_get_array ( vars->DI_root, "modbus_DI" );
       for ( gint cpt = 0; cpt < json_array_get_length ( Json_get_array ( vars->DI_root, "modbus_DI" ) ); cpt ++ )
        { JsonNode *element = json_array_get_element ( array, cpt );
          gint num;
          if (sscanf ( Json_get_string ( element, "thread_acronyme" ), "DI%d", &num ) == 1 && num < vars->nbr_entree_tor)
           { vars->DI[num] = element;
             Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: '%s': Mapping : DI%03d -> %s:%s", __func__, thread_tech_id,
                       num,
                       Json_get_string ( vars->DI[cpt], "tech_id" ),
                       Json_get_string ( vars->DI[cpt], "acronyme" ) );
             Json_node_add_int ( vars->DI[num], "etat", -1 );                 /* Pour forcer une premiere comm vers le master */
           } else Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING, "%s: '%s': map DI : num %d out of range '%d' OR sscanf error",
                            __func__, thread_tech_id, num, vars->nbr_entree_tor );
        }
     }

    vars->DO = g_try_malloc0( sizeof(gpointer) * vars->nbr_sortie_tor );
    if (!vars->DO)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': Memory Error for DO", __func__, thread_tech_id );
       return;
     } else Info_new( Config.log, module->lib->Thread_debug, LOG_INFO,
                      "%s: '%s': Allocated %d DO", __func__, thread_tech_id, vars->nbr_sortie_tor );

/******************************* Recherche des event text EA a raccrocher aux bits internes ***********************************/
    if ( ! Recuperer_mnemos_AI_by_tag ( &db, thread_tech_id, "AI%%" ) )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': Error searching Database for '%s'",
                 __func__, thread_tech_id, critere );
     }
    else while ( Recuperer_mnemos_AI_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *map_tag = db->row[2], *libelle = db->row[3];
       gint num;
       Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': Map found '%s' '%s:%s' - %s",
                 __func__, thread_tech_id, map_tag, tech_id, acro, libelle );
       if ( sscanf ( map_tag, "AI%d", &num ) == 1 )                                          /* Découpage de la ligne ev_text */
        { if (num<vars->nbr_entree_ana)
           { Charger_confDB_AI ( tech_id, acro );
             Dls_data_get_AI ( tech_id, acro, &vars->AI[num] );        /* bit déjà existant deja dans la structure DLS DATA */
           }
          else Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING, "%s: '%s': map '%s': num %d out of range '%d'",
                         __func__, thread_tech_id, map_tag, num, vars->nbr_entree_ana );
        }
       else Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': event '%s': Sscanf Error",
                      __func__, thread_tech_id, map_tag );
     }
/*********************************** Recherche des events DO a raccrocher aux bits internes ***********************************/
    if ( ! Recuperer_mnemos_DO_by_tag ( &db, thread_tech_id, "DO%%" ) )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': Error searching Database for '%s'",
                 __func__, thread_tech_id, critere );
     }
    else while ( Recuperer_mnemos_DO_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *libelle = db->row[3], *map_tag = db->row[2];
       gint num;
       Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': Map found '%s' '%s:%s' - %s",
                 __func__, thread_tech_id, map_tag, tech_id, acro, libelle );
       if ( sscanf ( map_tag, "DO%d", &num ) == 1 )                                          /* Découpage de la ligne ev_text */
        { if (num<vars->nbr_sortie_tor)
           { Dls_data_get_DO ( tech_id, acro, &vars->DO[num] );        /* bit déjà existant deja dans la structure DLS DATA */
             if(vars->DO[num] == NULL) Dls_data_set_DO ( NULL, tech_id, acro, &vars->DO[num], FALSE );     /* Sinon, on le crée */
           }
          else Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING, "%s: '%s': map '%s': num %d out of range '%d'",
                         __func__, thread_tech_id, map_tag, num, vars->nbr_sortie_tor );
        }
       else Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': event '%s': Sscanf Error",
                      __func__, thread_tech_id, map_tag );
     }
/******************************* Recherche des event text EA a raccrocher aux bits internes ***********************************/
    Info_new( Config.log, module->lib->Thread_debug, LOG_NOTICE, "%s: '%s': Module '%s' : mapping done",
              __func__, thread_tech_id, Json_get_string ( module->config, "description" ) );
  }
/******************************************************************************************************************************/
/* Recuperer_borne: Recupere les informations d'une borne MODBUS                                                              */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Modbus_Processer_trame( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    vars->nbr_oct_lu = 0;
    vars->request = FALSE;                                                                       /* Une requete a été traitée */

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if ( (guint16) vars->response.proto_id )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING, "%s: '%s': wrong proto_id", __func__, thread_tech_id );
       Deconnecter_module( module );
     }

    gint cpt_byte, cpt_poid, cpt;
    vars->date_last_reponse = Partage->top;                                                        /* Estampillage de la date */
    SubProcess_send_comm_to_master_new ( module, TRUE );
    if (ntohs(vars->response.transaction_id) != vars->transaction_id)                                     /* Mauvaise reponse */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                "%s: '%s': wrong transaction_id: attendu %d, recu %d", __func__, thread_tech_id,
                 vars->transaction_id, ntohs(vars->response.transaction_id) );
     }
    if ( vars->response.fct >=0x80 )
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                "%s: '%s': Erreur Reponse, Error %d, Exception code %d", __func__, thread_tech_id,
                 vars->response.fct, (int)vars->response.data[0] );
       Deconnecter_module( module );
       return;
     }
    switch (vars->mode)
     { case MODBUS_GET_DI:
            for ( cpt_poid = 1, cpt_byte = 1, cpt = 0; cpt<vars->nbr_entree_tor; cpt++)
             { gint new_etat = (vars->response.data[ cpt_byte ] & cpt_poid);
               gint old_etat = Json_get_int ( vars->DI[cpt], "etat" );
               if (old_etat != new_etat)
                { Zmq_Send_DI_to_master_new ( module, Json_get_string ( vars->DI[cpt], "tech_id" ),
                                                      Json_get_string ( vars->DI[cpt], "acronyme" ), new_etat );
                  Json_node_add_int ( vars->DI[cpt], "etat", new_etat );
                }
               cpt_poid = cpt_poid << 1;
               if (cpt_poid == 256) { cpt_byte++; cpt_poid = 1; }
             }
            vars->mode = MODBUS_GET_AI;
            break;
       case MODBUS_GET_AI:
            for ( cpt = 0; cpt<vars->nbr_entree_ana; cpt++)
             { struct DLS_AI *ai = vars->AI[cpt];
               if (!ai) continue;                                                    /* Si pas mappé, bah on ne la stocke pas */
               switch( ai->type )
                { case WAGO_750455:
                       if ( ! (vars->response.data[ 2*cpt + 2 ] & 0x03) )
                        { int reponse;
                          reponse  = vars->response.data[ 2*cpt + 1 ] << 5;
                          reponse |= vars->response.data[ 2*cpt + 2 ] >> 3;
                          Dls_data_set_AI ( NULL, NULL, &vars->AI[cpt],
                                           (gdouble)(reponse*(ai->max - ai->min))/4095.0 + ai->min, TRUE );
                        }
                       else { Dls_data_set_AI ( NULL, NULL, &vars->AI[cpt], 0.0, FALSE ); }
                       break;
                  case WAGO_750461:                                                                            /* Borne PT100 */
                        { gint16 reponse;
                          reponse  = vars->response.data[ 2*cpt + 1 ] << 8;
                          reponse |= vars->response.data[ 2*cpt + 2 ];
                          if (reponse > -2000 && reponse < 8500)
                           { Dls_data_set_AI ( NULL, NULL, &vars->AI[cpt], (gdouble)(reponse/10.0), TRUE ); }
                          else { Dls_data_set_AI ( NULL, NULL, &vars->AI[cpt], 0.0, FALSE ); }
                        }
                       break;
                }
             }
            vars->mode = MODBUS_SET_DO;
            break;
       case MODBUS_SET_DO:
            vars->mode = MODBUS_SET_AO;
            break;
       case MODBUS_SET_AO:
            vars->mode = MODBUS_GET_DI;
            break;
       case MODBUS_GET_DESCRIPTION:
          { gchar chaine[32];
            gint taille;
            memset ( chaine, 0, sizeof(chaine) );
            taille = vars->response.data[0];
            if (taille>=sizeof(chaine)) taille=sizeof(chaine)-1;
            chaine[0] = ntohs( (gint16)vars->response.data[1] );
            chaine[2] = ntohs( (gint16)vars->response.data[3] );
            chaine[taille] = 0;
            Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': Description (size %d) = '%s'",
                      __func__, thread_tech_id, taille, chaine );
            vars->mode = MODBUS_GET_FIRMWARE;
            break;
         }
       case MODBUS_GET_FIRMWARE:
          { gchar chaine[64];
            gint taille;
            memset ( chaine, 0, sizeof(chaine) );
            taille = vars->response.data[0];
            if (taille>=sizeof(chaine)) taille=sizeof(chaine)-1;
            chaine[0] = ntohs( (gint16)vars->response.data[1] );
            chaine[2] = ntohs( (gint16)vars->response.data[3] );
            chaine[taille] = 0;
            Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': Firmware (size %d) = '%s'",
                      __func__, thread_tech_id, taille, chaine );
            vars->mode = MODBUS_INIT_WATCHDOG1;
            break;
         }
       case MODBUS_INIT_WATCHDOG1:
            Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': Watchdog1 = %d %d",
                      __func__, thread_tech_id,
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 0) ),
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 2) )
                    );
            vars->mode = MODBUS_INIT_WATCHDOG2;
            break;
       case MODBUS_INIT_WATCHDOG2:
            Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': Watchdog2 = %d %d",
                      __func__, thread_tech_id,
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 0) ),
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 2) )
                    );
            vars->mode = MODBUS_INIT_WATCHDOG3;
            break;
       case MODBUS_INIT_WATCHDOG3:
            Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': Watchdog3 = %d %d",
                      __func__, thread_tech_id,
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 0) ),
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 2) )
                    );
            vars->mode = MODBUS_INIT_WATCHDOG4;
            break;
       case MODBUS_INIT_WATCHDOG4:
            Info_new( Config.log, module->lib->Thread_debug, LOG_DEBUG, "%s: '%s': Watchdog4 = %d %d",
                      __func__, thread_tech_id,
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 0) ),
                      ntohs( *(gint16 *)((gchar *)&vars->response.data + 2) )
                    );
            vars->mode = MODBUS_GET_NBR_AI;
            break;
       case MODBUS_GET_NBR_AI:
             { vars->nbr_entree_ana = ntohs( *(gint16 *)((gchar *)&vars->response.data + 1) ) / 16;
               Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': Get %03d Entree ANA",
                         __func__, thread_tech_id, vars->nbr_entree_ana
                       );
               for (gint cpt=0; cpt<vars->nbr_entree_ana; cpt++)
                { SQL_Write_new ( "INSERT IGNORE INTO %s_AI SET modbus_id=%d, num=%d",
                                  module->lib->name, Json_get_int ( module->config, "id" ), cpt );
                }
               vars->mode = MODBUS_GET_NBR_AO;
             }
            break;
       case MODBUS_GET_NBR_AO:
             { vars->nbr_sortie_ana = ntohs( *(gint16 *)((gchar *)&vars->response.data + 1) ) / 16;
               Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': Get %03d Sortie ANA",
                         __func__, thread_tech_id, vars->nbr_sortie_ana
                       );
               for (gint cpt=0; cpt<vars->nbr_sortie_tor; cpt++)
                { SQL_Write_new ( "INSERT IGNORE INTO %s_AO SET modbus_id=%d, num=%d",
                                  module->lib->name, Json_get_int ( module->config, "id" ), cpt );
                }
               vars->mode = MODBUS_GET_NBR_DI;
             }
            break;
       case MODBUS_GET_NBR_DI:
             { gint nbr;
               nbr = ntohs( *(gint16 *)((gchar *)&vars->response.data + 1) );
               vars->nbr_entree_tor = nbr;
               Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': Get %03d Entree TOR",
                         __func__, thread_tech_id, vars->nbr_entree_tor );
               for (gint cpt=0; cpt<vars->nbr_entree_tor; cpt++)
                { SQL_Write_new ( "INSERT IGNORE INTO modbus_DI SET thread_tech_id='%s', thread_acronyme='DI%03d'",
                                  thread_tech_id, cpt );
                  SQL_Write_new ( "INSERT IGNORE INTO mappings SET thread_tech_id='%s', thread_acronyme='DI%03d'",
                                  thread_tech_id, cpt );
                }
               vars->mode = MODBUS_GET_NBR_DO;
             }
            break;
       case MODBUS_GET_NBR_DO:
             { vars->nbr_sortie_tor = ntohs( *(gint16 *)((gchar *)&vars->response.data + 1) );
               Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': Get %03d Sortie TOR",
                         __func__, thread_tech_id, vars->nbr_sortie_tor );
               for (gint cpt=0; cpt<vars->nbr_sortie_tor; cpt++)
                { SQL_Write_new ( "INSERT IGNORE INTO %s_DO SET modbus_id=%d, num=%d",
                                  module->lib->name, Json_get_int ( module->config, "id" ), cpt );
                }
               Modbus_do_mapping( module );                                        /* Initialise le mapping des I/O du module */
               vars->mode = MODBUS_GET_DI;
             }
            break;
     }
  }
/******************************************************************************************************************************/
/* Recuperer_borne: Recupere les informations d'une borne MODBUS                                                              */
/* Entrée: identifiants des modules et borne                                                                                  */
/* Sortie: ?                                                                                                                  */
/******************************************************************************************************************************/
 static void Recuperer_reponse_module( struct SUBPROCESS *module )
  { struct MODBUS_VARS *vars = module->vars;
    fd_set fdselect;
    struct timeval tv;
    gint retval, cpt;

    gchar *thread_tech_id = Json_get_string ( module->config, "thread_tech_id" );

    if (vars->date_last_reponse + 600 < Partage->top)                                      /* Detection attente trop longue */
     { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                "%s: '%s': Timeout module started=%d, mode=%02d, "
                "transactionID=%06d, nbr_deconnect=%02d, last_reponse=%03ds ago, retente=in %03ds, date_next_eana=in %03ds",
                 __func__, thread_tech_id, vars->started, vars->mode, vars->transaction_id, vars->nbr_deconnect,
                (Partage->top - vars->date_last_reponse)/10,
                (vars->date_retente > Partage->top   ? (vars->date_retente   - Partage->top)/10 : -1),
                (vars->date_next_eana > Partage->top ? (vars->date_next_eana - Partage->top)/10 : -1)
               );
       Deconnecter_module( module );
       return;
     }

    FD_ZERO(&fdselect);
    FD_SET(vars->connexion, &fdselect );
    tv.tv_sec = 0;
    tv.tv_usec= 1000;                                                                               /* Attente d'un caractere */
    retval = select(vars->connexion+1, &fdselect, NULL, NULL, &tv );

    if ( retval>0 && FD_ISSET(vars->connexion, &fdselect) )
     { int bute;
       if (vars->nbr_oct_lu<TAILLE_ENTETE_MODBUS)
            { bute = TAILLE_ENTETE_MODBUS; }
       else { bute = TAILLE_ENTETE_MODBUS + ntohs(vars->response.taille); }

       if (bute>=sizeof(struct TRAME_MODBUS_REPONSE))
        { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR,
                   "%s: '%s': bute = %d >= %d (sizeof(module->reponse)=%d, taille recue = %d)", __func__, thread_tech_id,
                    bute, sizeof(struct TRAME_MODBUS_REPONSE), sizeof(vars->response), ntohs(vars->response.taille) );
          Deconnecter_module( module );
          return;
        }

       cpt = read( vars->connexion, (unsigned char *)&vars->response + vars->nbr_oct_lu, bute-vars->nbr_oct_lu );
       if (cpt>=0)
        { vars->nbr_oct_lu += cpt;
          if (vars->nbr_oct_lu >= TAILLE_ENTETE_MODBUS + ntohs(vars->response.taille))
           { Modbus_Processer_trame( module ); }                                    /* Si l'on a trouvé une trame complète !! */
        }
       else
        { Info_new( Config.log, module->lib->Thread_debug, LOG_WARNING,
                    "%s: '%s': Read Error. Get %d, error %s", __func__, thread_tech_id, cpt, strerror(errno) );
          Deconnecter_module ( module );
        }
      }
  }
/******************************************************************************************************************************/
/* Run_subprocess: Prend en charge un des sous process du thread                                                              */
/* Entrée: la structure SUBPROCESS associée                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_subprocess ( struct SUBPROCESS *module )
  { SubProcess_init ( module, sizeof(struct MODBUS_VARS) );
    struct MODBUS_VARS *vars = module->vars;

    gchar *thread_tech_id      = Json_get_string ( module->config, "thread_tech_id" );
    gint   max_request_par_sec = Json_get_int    ( module->config, "max_request_par_sec" );

    if (Json_get_bool ( module->config, "enable" ) == FALSE)
     { Info_new( Config.log, module->lib->Thread_debug, LOG_ERR, "%s: '%s': Not Enabled. Stopping SubProcess", __func__, thread_tech_id );
       SubProcess_end ( module );
     }

    while(module->lib->Thread_run == TRUE && module->lib->Thread_reload == FALSE)            /* On tourne tant que necessaire */
     { usleep(vars->delai);
       sched_yield();

       SubProcess_send_comm_to_master_new ( module, module->comm_status );         /* Périodiquement envoie la comm au master */
/********************************************************* Ecoute du master ***************************************************/
       JsonNode *request;
       while ( (request = SubProcess_Listen_to_master_new ( module ) ) != NULL)
        { gchar *zmq_tag = Json_get_string ( request, "zmq_tag" );
          if ( !strcasecmp( zmq_tag, "SUBPROCESS_REMAP" ) )
           { if (  Json_has_member ( request, "thread_tech_id" ) &&
                  !strcasecmp( Json_get_string ( request, "thread_tech_id" ), thread_tech_id ) )
              { Deconnecter_module ( module ); }
           }
          json_node_unref(request);
        }

/********************************************************* Toutes les secondes ************************************************/
       if (Partage->top>=vars->last_top+10)                                                          /* Toutes les 1 secondes */
        { vars->nbr_request_par_sec = vars->nbr_request;
          vars->nbr_request = 0;
          if(vars->nbr_request_par_sec > max_request_par_sec) vars->delai += 50;
          else if(vars->delai>0) vars->delai -= 50;
          vars->last_top = Partage->top;
        }

/********************************************* Début de l'interrogation du module *********************************************/
       if ( vars->started == FALSE )                                               /* Si attente retente, on change de module */
        { if ( vars->date_retente <= Partage->top && Connecter_module(module)==FALSE )
           { Info_new( Config.log, module->lib->Thread_debug, LOG_INFO, "%s: '%s': Module DOWN. retrying in %ds",
                       __func__, thread_tech_id, MODBUS_RETRY/10 );
             vars->date_retente = Partage->top + MODBUS_RETRY;
           }
        }
       else
        { if ( vars->request )                                                           /* Requete en cours pour ce module ? */
           { Recuperer_reponse_module ( module ); }
          else
           { if (vars->date_next_eana<Partage->top)                                    /* Gestion décalée des I/O Analogiques */
              { vars->date_next_eana = Partage->top + MBUS_TEMPS_UPDATE_IO_ANA;                         /* Tous les 2 dixieme */
                vars->do_check_eana = TRUE;
              }
             switch (vars->mode)
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
                case MODBUS_GET_DI         : if (vars->nbr_entree_tor) Interroger_entree_tor( module );
                                             else vars->mode = MODBUS_GET_AI;
                                             break;
                case MODBUS_GET_AI         : if (vars->nbr_entree_ana && vars->do_check_eana)
                                                  Interroger_entree_ana( module );
                                             else vars->mode = MODBUS_SET_DO;
                                             break;
                case MODBUS_SET_DO         : if (vars->nbr_sortie_tor) Interroger_sortie_tor( module );
                                             else vars->mode = MODBUS_SET_AO;
                                             break;
                case MODBUS_SET_AO         : if (vars->nbr_sortie_ana && vars->do_check_eana)
                                              { Interroger_sortie_ana( module );
                                              }
                                             else vars->mode = MODBUS_GET_DI;
                                             vars->do_check_eana = FALSE;                                /* Le check est fait */
                                             vars->nbr_request++;
                                             break;
              }
           }
       }
     }
    SubProcess_end(module);
  }
/******************************************************************************************************************************/
/* Run_process: Run du Process                                                                                                */
/* Entrée: la structure PROCESS associée                                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Run_process ( struct PROCESS *lib )
  {
reload:
    Modbus_Creer_DB ( lib );                                                                   /* Création de la DB du thread */
    Thread_init ( "modbus", "I/O", lib, WTD_VERSION, "Manage WAGO System" );

    lib->config = Json_node_create();
    if(lib->config) SQL_Select_to_json_node ( lib->config, "subprocess", "SELECT * FROM %s WHERE uuid='%s'", lib->name, lib->uuid );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: %d subprocess to load", __func__, Json_get_int ( lib->config, "nbr_subprocess" ) );

    Json_node_foreach_array_element ( lib->config, "subprocess", Process_Load_one_subprocess, lib );   /* Chargement des modules */
    while( lib->Thread_run == TRUE && lib->Thread_reload == FALSE) sleep(1);                 /* On tourne tant que necessaire */
    Process_Unload_all_subprocess ( lib );

    if (lib->Thread_run == TRUE && lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }

    Thread_end ( lib );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
