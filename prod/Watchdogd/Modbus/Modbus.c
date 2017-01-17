/**********************************************************************************************************/
/* Watchdogd/Modbus/Modbus.c  Gestion des modules MODBUS Watchdgo 2.0                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                     jeu. 24 déc. 2009 12:59:27 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Modbus.c
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

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
 #include "Modbus.h"

/**********************************************************************************************************/
/* Modbus_Lire_config : Lit la config Watchdog et rempli la structure mémoire                             */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 gboolean Modbus_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_modbus.lib->Thread_debug = FALSE;                                     /* Settings default parameters */
    Cfg_modbus.enable            = FALSE; 


    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                     /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                "Modbus_Lire_config: Database connexion failed. Using Default Parameters" );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )       /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,                         /* Print Config */
                "Modbus_Lire_config: '%s' = %s", nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_modbus.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_modbus.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE,
                   "Modbus_Lire_config: Unknown Parameter '%s'(='%s') in Database", nom, valeur );
        }
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Retirer_modbusDB: Elimination d'un module modbus                                                       */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_modbusDB ( struct MODBUSDB *modbus )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Retirer_modbusDB: Database Connection Failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MODULE_MODBUS, modbus->id );

    retour = Lancer_requete_SQL ( db, requete );               /* Execution de la requete SQL */
    Libere_DB_SQL( &db );
    Cfg_modbus.reload = TRUE;                    /* Rechargement des modules MODBUS en mémoire de travail */
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_modbusDB: Ajout ou edition d'un modbus                                                         */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure modbus                        */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 static gint Ajouter_modifier_modbusDB ( struct MODBUSDB *modbus, gboolean ajout )
  { gchar requete[2048];
    gchar *libelle, *ip;
    gboolean retour_sql;
    struct DB *db;
    gint retour;

    libelle = Normaliser_chaine ( modbus->libelle );         /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Ajouter_modifier_modbusDB: Normalisation libelle impossible" );
       return(-1);
     }

    ip = Normaliser_chaine ( modbus->ip );                   /* Formatage correct des chaines */
    if (!ip)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Ajouter_modifier_modbusDB: Normalisation ip impossible" );
       Libere_DB_SQL( &db );
       w_free(libelle, "free libelle");
       return(-1);
     }

    if ( ajout == TRUE )
     { g_snprintf( requete, sizeof(requete),
                  "INSERT INTO %s(instance_id,enable,ip,bit,watchdog,libelle,map_E,map_EA,map_A,map_AA) "
                  "VALUES ('%s','%d','%s',%d,%d,'%s','%d','%d','%d','%d')",
                   NOM_TABLE_MODULE_MODBUS, Config.instance_id, modbus->enable, ip, modbus->bit, modbus->watchdog, libelle,
                   modbus->map_E, modbus->map_EA, modbus->map_A, modbus->map_AA
                 );
     }
    else
     { g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                  "UPDATE %s SET "             
                  "enable='%d',ip='%s',bit='%d',watchdog='%d',libelle='%s',"
                  "map_E='%d',map_EA='%d',map_A='%d',map_AA='%d'"
                  " WHERE id=%d",
                   NOM_TABLE_MODULE_MODBUS,
                   modbus->enable, ip, modbus->bit, modbus->watchdog, libelle,
                   modbus->map_E, modbus->map_EA, modbus->map_A, modbus->map_AA,
                   modbus->id );
      }
    w_free(ip, "free ip");
    w_free(libelle, "free libelle2");

    db = Init_DB_SQL();       
    if (!db)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Ajouter_modifier_modbusDB: Database Connection Failed" );
       return(-1);
     }

    retour_sql = Lancer_requete_SQL ( db, requete );                           /* Lancement de la requete */
    if ( retour_sql == TRUE )                                                          /* Si pas d'erreur */
     { if (ajout==TRUE) retour = Recuperer_last_ID_SQL ( db );            /* Retourne le nouvel ID modbus */
       else retour = 0;
     }
    else retour = -1;
    Libere_DB_SQL( &db );
    Cfg_modbus.reload = TRUE;                    /* Rechargement des modules MODBUS en mémoire de travail */
    return ( retour );                                            /* Pas d'erreur lors de la modification */
  }
/**********************************************************************************************************/
/* Ajouter_modbusDB: Ajout ou edition d'un modbus                                                         */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure modbus                        */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_modbusDB ( struct MODBUSDB *modbus )
  { return ( Ajouter_modifier_modbusDB ( modbus, TRUE ) ); }
/**********************************************************************************************************/
/* Modifier_modbusDB: Modification d'un modbus Watchdog                                                   */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gint Modifier_modbusDB( struct MODBUSDB *modbus )
  { return ( Ajouter_modifier_modbusDB ( modbus, FALSE ) ); }
/**********************************************************************************************************/
/* Recuperer_liste_id_modbusDB: Recupération de la liste des ids des modbuss                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_modbusDB ( struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,enable,ip,bit,watchdog,libelle,map_E,map_EA,map_A,map_AA "
                " FROM %s WHERE instance_id='%s' ORDER BY libelle",
                NOM_TABLE_MODULE_MODBUS, Config.instance_id );

    return ( Lancer_requete_SQL ( db, requete ) );             /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_modbusDB: Recupération de la liste des ids des modbuss                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MODBUSDB *Recuperer_modbusDB_suite( struct DB *db )
  { struct MODBUSDB *modbus;

    Recuperer_ligne_SQL(db);                              /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       return(NULL);
     }

    modbus = (struct MODBUSDB *)w_malloc0( sizeof(struct MODBUSDB), "new modbus" );
    if (!modbus) Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR,
                          "Recuperer_modbusDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &modbus->libelle, db->row[5], sizeof(modbus->libelle) );
       memcpy( &modbus->ip,      db->row[2], sizeof(modbus->ip) );
       modbus->id       = atoi(db->row[0]);
       modbus->enable   = atoi(db->row[1]);
       modbus->bit      = atoi(db->row[3]);
       modbus->watchdog = atoi(db->row[4]);
       modbus->map_E    = atoi(db->row[6]);
       modbus->map_EA   = atoi(db->row[7]);
       modbus->map_A    = atoi(db->row[8]);
       modbus->map_AA   = atoi(db->row[9]);
     }
    return(modbus);
  }
/**********************************************************************************************************/
/* Charger_tous_Modbus: Requete la DB pour charger les modules et les bornes modbus                       */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_tous_MODBUS ( void  )
  { struct DB *db;
    gint cpt;

    db = Init_DB_SQL();       
    if (!db) return(FALSE);

/********************************************** Chargement des modules ************************************/
    if ( ! Recuperer_modbusDB( db ) )
     { Libere_DB_SQL( &db );
       return(FALSE);
     }

    Cfg_modbus.Modules_MODBUS = NULL;
    cpt = 0;
    for ( ; ; )
     { struct MODULE_MODBUS *module;
       struct MODBUSDB *modbus;

       modbus = Recuperer_modbusDB_suite( db );
       if (!modbus) break;

       module = (struct MODULE_MODBUS *)w_malloc0( sizeof(struct MODULE_MODBUS), "new module" );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR,
                   "Charger_tous_MODBUS: Erreur allocation mémoire struct MODULE_MODBUS" );
          w_free(modbus, "free modbus");
          Libere_DB_SQL( &db );
          return(FALSE);
        }
       memcpy( &module->modbus, modbus, sizeof(struct MODBUSDB) );
       w_free(modbus, "free modbus");
       cpt++;                                              /* Nous avons ajouté un module dans la liste ! */
                                                                        /* Ajout dans la liste de travail */
       Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, 
                "Charger_tous_MODBUS: id=%d, enable=%d", module->modbus.id, module->modbus.enable );

       Cfg_modbus.Modules_MODBUS = g_slist_prepend ( Cfg_modbus.Modules_MODBUS, module );
     }
    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
             "Charger_tous_MODBUS: %d modules MODBUS found  !", cpt );
    Libere_DB_SQL( &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Deconnecter: Deconnexion du module                                                                     */
/* Entrée: un id                                                                                          */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Deconnecter_module ( struct MODULE_MODBUS *module )
  { gint cpt;
    if (!module) return;
    if (module->started == FALSE) return;

    close ( module->connexion );
    module->connexion = 0;
    module->started = FALSE;
    module->request = FALSE;
    module->nbr_deconnect++;
    module->date_retente = Partage->top + MODBUS_RETRY;
    for ( cpt = module->modbus.map_EA; cpt<module->nbr_entree_ana; cpt++)
     { SEA_range( cpt, 0 ); }
    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
             "Deconnecter_module : Module %d disconnected", module->modbus.id );
    SB( module->modbus.bit, 0 );                              /* Mise a zero du bit interne lié au module */
  }
/**********************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                           */
/* Entrée: une nom et un password                                                                         */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                          */
/**********************************************************************************************************/
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

    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
             "Connecter_module: Trying to connect module %d to %s", module->modbus.id, module->modbus.ip );

    s = getaddrinfo( module->modbus.ip, "502", &hints, &result);
    if (s != 0)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                "Connecter_module: getaddrinfo Failed for module %d (%s) (%s)",
                 module->modbus.id, module->modbus.ip, gai_strerror(s) );
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
                   "Connecter_module: Socket creation failed for modbus %d (%s)",
                    module->modbus.id, module->modbus.ip );
          continue;
        }

       if ( setsockopt ( connexion, SOL_SOCKET, SO_SNDTIMEO, (char *)&sndtimeout, sizeof(sndtimeout)) < 0 )
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                   "Connecter_module: Socket Set Options failed for modbus %d (%s)",
                    module->modbus.id, module->modbus.ip );
          continue;
        }
        
       if (connect(connexion, rp->ai_addr, rp->ai_addrlen) != -1)
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
                   "Connecter_module %d (%s) family=%d",
                    module->modbus.id, module->modbus.ip, rp->ai_family );

          break;  /* Success */
        }
       else
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE,
                   "Connecter_module: connexion refused by module %d (%s) family=%d error '%s'",
                    module->modbus.id, module->modbus.ip, rp->ai_family, strerror(errno) );
        }
       close(connexion);                                   /* Suppression de la socket qui n'a pu aboutir */
     }
    freeaddrinfo(result);
    if (rp == NULL) return(FALSE);                                                 /* Erreur de connexion */

    fcntl( connexion, F_SETFL, SO_KEEPALIVE | SO_REUSEADDR );
    module->connexion = connexion;                                                    /* Sauvegarde du fd */
    module->date_last_reponse = Partage->top;
    module->date_retente   = 0;
    module->transaction_id = 1;
    module->started = TRUE;
    module->mode = MODBUS_GET_DESCRIPTION;

    return(TRUE);
  }
/**********************************************************************************************************/
/* Decharger_un_modbus: Decharge (libere la mémoire) un module modbus                                     */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_un_MODBUS ( struct MODULE_MODBUS *module )
  { if (!module) return;
    Deconnecter_module  ( module );                                  /* Deconnexion du module en question */
    Cfg_modbus.Modules_MODBUS = g_slist_remove ( Cfg_modbus.Modules_MODBUS, module );
    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
             "Decharger_un_MODBUS: Dechargement module %d (%s)", module->modbus.id, module->modbus.ip );
    w_free(module, "free module");
  }
/**********************************************************************************************************/
/* Decharger_tous_modbus: Decharge l'ensemble des modules MODBUS                                          */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Decharger_tous_MODBUS ( void  )
  { struct MODULE_MODBUS *module;
    while ( Cfg_modbus.Modules_MODBUS )
     { module = (struct MODULE_MODBUS *)Cfg_modbus.Modules_MODBUS->data;
       Decharger_un_MODBUS ( module );
     }
  }
/**********************************************************************************************************/
/* Modbus_is_actif: Renvoi TRUE si au moins un des modules modbus est actif                               */
/* Entrée: rien                                                                                           */
/* Sortie: TRUE/FALSE                                                                                     */
/**********************************************************************************************************/
 static gboolean Modbus_is_actif ( void )
  { GSList *liste;
    liste = Cfg_modbus.Modules_MODBUS;
    while ( liste )
     { struct MODULE_MODBUS *module;
       module = ((struct MODULE_MODBUS *)liste->data);

       if (module->modbus.enable) return(TRUE);
       liste = liste->next;
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Interroger_description : envoie une commande d'identification au module                                */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Interroger_description( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                            /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x2020 );
    requete.nbr            = htons( 16 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                            /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "Interroger_description: failed for module %d (%s): error %d",
               module->modbus.id, module->modbus.ip, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG, "Interroger_description: OK for %d", module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/**********************************************************************************************************/
/* Interroger_description : envoie une commande d'identification au module                                */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Interroger_firmware( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                            /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x2023 );
    requete.nbr            = htons( 16 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                            /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "Interroger_firmware: failed for module %d (%s): error %d",
               module->modbus.id, module->modbus.ip, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG, "Interroger_firmware: OK for %d", module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Init_watchdog1( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x100A );
    requete.valeur         = htons( 0x0000 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                            /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "Init_watchdog_modbus: 'stop watchdog failed' for %d (error %d)", module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
               "Init_watchdog_modbus: 'stop watchdog OK' for %d", module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Init_watchdog2( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x1009 );
    requete.valeur         = htons( 0x0001 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                            /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "Init_watchdog_modbus: 'close modbus tcp on watchdog' failed for %d (error %d)",
               module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
               "Init_watchdog_modbus: 'close modbus tcp on watchdog' OK for %d", module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Init_watchdog3( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x1000 );
    requete.valeur         = htons( module->modbus.watchdog );                          /* coupure sortie */

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                            /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "Init_watchdog_modbus: 'init watchdog timer' failed for %d (error %d)",
               module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
               "Init_watchdog_modbus: 'init watchdog timer' OK for %d", module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Init_watchdog4( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_WRITE_REGISTER;
    requete.adresse        = htons( 0x100A );
    requete.valeur         = htons( 0x0001 );                                              /* Start Timer */

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                            /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                "Init_watchdog_modbus: 'watchdog start' failed for %d (error %d)",
                 module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                "Init_watchdog_modbus: 'watchdog start' OK for %d", module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/**********************************************************************************************************/
/* Interroger_nbr_entree_ANA : Demander au module d'envoyer son nombre d'entree ANALOGIQUE                */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Interroger_nbr_entree_ANA( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                            /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1023 );
    requete.nbr            = htons( 0x0001 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                            /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "Interroger_nbr_entree_ANA: failed for %d (error %d)",
                module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
               "Interroger_nbr_entree_ANA: OK for %d", module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/**********************************************************************************************************/
/* Interroger_nbr_entree_ANA : Demander au module d'envoyer son nombre de sortie ANALOGIQUE               */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Interroger_nbr_sortie_ANA( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                            /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1022 );
    requete.nbr            = htons( 0x0001 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                            /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "Interroger_nbr_sortie_ANA: failed %d (error %d)",
                module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
               "Interroger_nbr_sortie_ANA: OK", module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/**********************************************************************************************************/
/* Interroger_nbr_entree_TOR : Demander au module d'envoyer son nombre d'entree TOR                       */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Interroger_nbr_entree_TOR( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                            /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1025 );
    requete.nbr            = htons( 0x0001 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                            /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "Interroger_nbr_entree_TOR: failed %d (error %d)",
               module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
               "Interroger_nbr_entree_TOR: OK for %d", module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/**********************************************************************************************************/
/* Interroger_nbr_sortie_TOR : Demander au module d'envoyer son nombre de sortie TOR                      */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Interroger_nbr_sortie_TOR( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */
    gint retour;

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.taille         = htons( 0x006 );                            /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = htons( 0x1024 );
    requete.nbr            = htons( 0x0001 );

    retour = write ( module->connexion, &requete, 12 );
    if ( retour != 12 )                                                            /* Envoi de la requete */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
               "Interroger_nbr_sortie_TOR: failed %d (error %d)",
                module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
               "Interroger_nbr_sortie_TOR: OK for %d", module->modbus.id );
       module->request = TRUE;                                                /* Une requete a élé lancée */
     }
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_entree_tor( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_READ_COIL;
    requete.adresse        = 0x00;
    requete.nbr            = htons( module->nbr_entree_tor );

    if ( write ( module->connexion, &requete, 12 ) != 12 )                         /* Envoi de la requete */
     { Deconnecter_module( module ); }
    else module->request = TRUE;                                              /* Une requete a élé lancée */
  }
/**********************************************************************************************************/
/* Interroger_entree_ana: Interrogation des entrees analogique d'un module wago                           */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_entree_ana( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_READ_REGISTER;
    requete.adresse        = 0x00;
    requete.nbr            = htons( module->nbr_entree_ana );

    if ( write ( module->connexion, &requete, 12 ) != 12 )                         /* Envoi de la requete */
     { Deconnecter_module( module ); }
    else module->request = TRUE;                                              /* Une requete a élé lancée */
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_sortie_tor( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */
    gint cpt_a, cpt_poid, cpt_byte, cpt, taille, nbr_data;

    memset(&requete, 0, sizeof(requete) );                           /* Mise a zero globale de la requete */
    nbr_data = ((module->nbr_sortie_tor-1)/8)+1;
    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    taille                 = 0x0007 + nbr_data;
    requete.taille         = htons( taille );                                                   /* taille */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_WRITE_MULTIPLE_COIL;
    requete.adresse        = 0x00;
    requete.nbr            = htons( module->nbr_sortie_tor );                                /* bit count */
    requete.data[2]        = nbr_data;                                                      /* Byte count */
    cpt_a = module->modbus.map_A;
    for ( cpt_poid = 1, cpt_byte = 3, cpt = 0; cpt<module->nbr_sortie_tor; cpt++)
      { if (cpt_poid == 256) { cpt_byte++; cpt_poid = 1; }
        if ( A(cpt_a) ) requete.data[cpt_byte] |= cpt_poid;
        cpt_a++;
        cpt_poid = cpt_poid << 1;
      }
               
    if ( write ( module->connexion, &requete, taille+6 ) != taille+6 )/* Envoi de la requete (taille + header )*/
     { Deconnecter_module( module ); }
    else module->request = TRUE;                                              /* Une requete a élé lancée */
  }
/**********************************************************************************************************/
/* Interroger_sortie_ana: Envoie les informations liées aux sorties ANA du module                         */
/* Entrée: le module à interroger                                                                         */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Interroger_sortie_ana( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE requete;                                 /* Definition d'une trame MODBUS */
    gint cpt_a, cpt_byte, cpt, taille;

    memset(&requete, 0, sizeof(requete) );                           /* Mise a zero globale de la requete */
    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    taille                 = 0x0006 + (module->nbr_sortie_ana*2 + 1);
    requete.taille         = htons( taille );                                                   /* taille */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_WRITE_MULTIPLE_REGISTER;
    requete.adresse        = 0x00;
    requete.nbr            = htons( module->nbr_sortie_ana );                                /* bit count */
    requete.data[2]        = (module->nbr_sortie_ana*2);                                    /* Byte count */
    cpt_a = module->modbus.map_AA;
    for ( cpt_byte = 3, cpt = 0; cpt<module->nbr_sortie_ana; cpt++)
      { /* Attention, parser selon le type de sortie ! (12 bits ? 10 bits ? conversion ??? */
        requete.data [cpt_byte  ] = 0x30; /*Partage->aa[cpt_a].val_int>>5;*/
        requete.data [cpt_byte+1] = 0x00; /*(Partage->aa[cpt_a].val_int & 0x1F)<<3;*/
        cpt_a++; cpt_byte += 2;
      }
               
    if ( write ( module->connexion, &requete, taille+6 ) != taille+6 )/* Envoi de la requete (taille + header )*/
     { Deconnecter_module( module ); }
    else module->request = TRUE;                                              /* Une requete a élé lancée */
  }
/**********************************************************************************************************/
/* Recuperer_borne: Recupere les informations d'une borne MODBUS                                          */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Processer_trame( struct MODULE_MODBUS *module )
  { module->nbr_oct_lu = 0;
    module->request = FALSE;                                                 /* Une requete a été traitée */

    if ( (guint16) module->response.proto_id )
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Processer_trame: wrong proto_id" );
       Deconnecter_module( module );
     }
    else
     { int cpt_e, cpt_byte, cpt_poid, cpt;
       module->date_last_reponse = Partage->top;                               /* Estampillage de la date */
       SB( module->modbus.bit, 1 );                              /* Mise a 1 du bit interne lié au module */
       if (ntohs(module->response.transaction_id) != module->transaction_id)             /* Mauvaise reponse */
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                   "Processer_trame: wrong transaction_id for module %d  attendu %d, recu %d",
                    module->modbus.id, module->transaction_id, ntohs(module->response.transaction_id) );
        }
       if ( module->response.fct >=0x80 )
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                   "Processer_trame: Erreur Reponse Module %d, Error %d, Exception code %d",
                    module->modbus.id, module->response.fct, (int)module->response.data[0] );
          Deconnecter_module( module );
        }
       else switch (module->mode)
        { case MODBUS_GET_DI:
               cpt_e = module->modbus.map_E;
               for ( cpt_poid = 1, cpt_byte = 1, cpt = 0; cpt<module->nbr_entree_tor; cpt++)
                { SE( cpt_e, ( module->response.data[ cpt_byte ] & cpt_poid ) );
                  cpt_e++;
                  cpt_poid = cpt_poid << 1;
                  if (cpt_poid == 256) { cpt_byte++; cpt_poid = 1; }
                }
               module->mode = MODBUS_GET_AI;
               break;
          case MODBUS_GET_AI:
               cpt_e = module->modbus.map_EA;
               for ( cpt = 0; cpt<module->nbr_entree_ana; cpt++)
                { switch(Partage->ea[cpt_e].confDB.type)
                   { case ENTREEANA_WAGO_750455:
                          if ( ! (module->response.data[ 2*cpt + 2 ] & 0x03) )
                           { int reponse;
                             reponse  = module->response.data[ 2*cpt + 1 ] << 5;
                             reponse |= module->response.data[ 2*cpt + 2 ] >> 3;
                             SEA( cpt_e, reponse );
                           }
                          else SEA_range( cpt_e, 0 );
                          break;
                     case ENTREEANA_WAGO_750461:                                                               /* Borne PT100 */
                           { gint16 reponse;
                             reponse  = module->response.data[ 2*cpt + 1 ] << 8;
                             reponse |= module->response.data[ 2*cpt + 2 ];
                             SEA ( cpt_e, 1.0*reponse );
                           }
                          break;
                     default : SEA_range( cpt_e, 0 );
                   }
                  cpt_e++;
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
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
                         "Processer_trame: Get Description (size %d) %s", taille, (gchar *) chaine );
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
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
                         "Processer_trame: Get Firmware (size %d) %s", taille, (gchar *) chaine );
               module->mode = MODBUS_INIT_WATCHDOG1;
               break;
            }
          case MODBUS_INIT_WATCHDOG1:
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                        "Processer_trame: Watchdog1 = %d %d",
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 0) ),
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 2) )
                       );
               module->mode = MODBUS_INIT_WATCHDOG2;
               break;
          case MODBUS_INIT_WATCHDOG2:
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                        "Processer_trame: Watchdog2 = %d %d",
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 0) ),
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 2) )
                       );
               module->mode = MODBUS_INIT_WATCHDOG3;
               break;
          case MODBUS_INIT_WATCHDOG3:
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                        "Processer_trame: Watchdog3 = %d %d",
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 0) ),
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 2) )
                       );
               module->mode = MODBUS_INIT_WATCHDOG4;
               break;
          case MODBUS_INIT_WATCHDOG4:
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                        "Processer_trame: Watchdog4 = %d %d",
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 0) ),
                         ntohs( *(gint16 *)((gchar *)&module->response.data + 2) )
                       );
               module->mode = MODBUS_GET_NBR_AI;
               break;
          case MODBUS_GET_NBR_AI:
               module->nbr_entree_ana = ntohs( *(gint16 *)((gchar *)&module->response.data + 1) ) / 16;
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "Processer_trame: Get number Entree ANA = %d",
                         module->nbr_entree_ana
                       );
               module->mode = MODBUS_GET_NBR_AO;
               break;
          case MODBUS_GET_NBR_AO:
               module->nbr_sortie_ana = ntohs( *(gint16 *)((gchar *)&module->response.data + 1) ) / 16;
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "Processer_trame: Get number Sortie ANA = %d",
                         module->nbr_sortie_ana
                       );
               module->mode = MODBUS_GET_NBR_DI;
               break;
          case MODBUS_GET_NBR_DI:
               module->nbr_entree_tor = ntohs( *(gint16 *)((gchar *)&module->response.data + 1) );
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "Processer_trame: Get number Entree TOR = %d",
                         module->nbr_entree_tor
                       );
               module->mode = MODBUS_GET_NBR_DO;
               break;
          case MODBUS_GET_NBR_DO:
               module->nbr_sortie_tor = ntohs( *(gint16 *)((gchar *)&module->response.data + 1) );
               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "Processer_trame: Get number Sortie TOR = %d",
                         module->nbr_sortie_tor
                       );
               module->mode = MODBUS_GET_DI;
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
                "Recuperer_reponse_module: Timeout module %d, enable=%d, started=%d, mode=%02d, "
                "transactionID=%06d, nbr_deconnect=%02d, last_reponse=%03ds ago, retente=in %03ds, date_next_eana=in %03ds",
                 module->modbus.id, module->modbus.enable, module->started, module->mode,
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
                   "Recuperer_reponse_module: bute = %d >= %d (sizeof(module->reponse)=%d, taille recue = %d)",
                    bute, sizeof(struct TRAME_MODBUS_REPONSE), sizeof(module->response), ntohs(module->response.taille) );
          Deconnecter_module( module );
          return;
        }

       cpt = read( module->connexion,
                   (unsigned char *)&module->response +
                                     module->nbr_oct_lu,
                    bute-module->nbr_oct_lu );
       if (cpt>=0)
        { module->nbr_oct_lu += cpt;
          if (module->nbr_oct_lu >= 
              TAILLE_ENTETE_MODBUS + ntohs(module->response.taille))
           { 
             Processer_trame( module );                                             /* Si l'on a trouvé une trame complète !! */
             module->nbr_oct_lu = 0;
           }
        }
       else
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                    "Recuperer_reponse_module: wrong trame ID for %d. Get %d, error %s",
                    module->modbus.id, cpt, strerror(errno) );
          Deconnecter_module ( module );
        }
      }
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du MODBUS                                                                                        */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct MODULE_MODBUS *module;
    GSList *liste;

    prctl(PR_SET_NAME, "W-MODBUS", 0, 0, 0 );
    memset( &Cfg_modbus, 0, sizeof(Cfg_modbus) );                                   /* Mise a zero de la structure de travail */
    Cfg_modbus.lib = lib;                                          /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_modbus.lib->TID = pthread_self();                                                   /* Sauvegarde du TID pour le pere */
    Modbus_Lire_config ();                                                  /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );
    Cfg_modbus.lib->Thread_run = TRUE;                                                                  /* Le thread tourne ! */

    g_snprintf( Cfg_modbus.lib->admin_prompt, sizeof(Cfg_modbus.lib->admin_prompt), "modbus" );
    g_snprintf( Cfg_modbus.lib->admin_help,   sizeof(Cfg_modbus.lib->admin_help),   "Manage Modbus system" );

    if (!Cfg_modbus.enable)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread is not enabled in config. Shutting Down %p",
                 pthread_self() );
       goto end;
     }

    Cfg_modbus.Modules_MODBUS = NULL;                                                         /* Init des variables du thread */

    if ( Charger_tous_MODBUS() == FALSE )                                                    /* Chargement des modules modbus */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "Run_modbus: No module MODBUS found -> stop" );
       Cfg_modbus.lib->Thread_run = FALSE;                                                      /* Le thread ne tourne plus ! */
     }

    while(lib->Thread_run == TRUE)                                                           /* On tourne tant que necessaire */
     { usleep(10000);

       if (lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "Run_modbus: SIGUSR1" );
          lib->Thread_sigusr1 = FALSE;
        }

       if (Cfg_modbus.reload == TRUE)
        { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "Run_thread: Reloading conf" );
          Decharger_tous_MODBUS();
          Charger_tous_MODBUS();
          Cfg_modbus.reload = FALSE;
        }

       if (Cfg_modbus.Modules_MODBUS == NULL ||                                     /* Si pas de module référencés, on attend */
           Modbus_is_actif() == FALSE)
        { sleep(2); continue; }

       liste = Cfg_modbus.Modules_MODBUS;
       while (liste && (lib->Thread_run == TRUE) && (Cfg_modbus.reload == FALSE) )
        { module = (struct MODULE_MODBUS *)liste->data;

          if ( module->modbus.enable == FALSE && module->started )                                  /* Module a deconnecter ! */
           { Deconnecter_module  ( module );
             liste = liste->next;                                          /* On prépare le prochain accès au prochain module */
             continue;
           }

          if ( module->modbus.enable == FALSE ||                       /* Si module DOWN ou si UP mais dans le delai de retry */
               Partage->top < module->date_retente )                               /* Si attente retente, on change de module */
           { liste = liste->next;                                          /* On prépare le prochain accès au prochain module */
             continue;
           }

/********************************************* Début de l'interrogation du module *********************************************/
          if ( ! module->started )                                                               /* Communication OK ou non ? */
           { if ( ! Connecter_module( module ) )
              { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
                          "Run_modbus: Module %03d DOWN. retrying in %ds", module->modbus.id, MODBUS_RETRY/10 );
                module->date_retente = Partage->top + MODBUS_RETRY;
              }
           }
          else
           { if ( module->request )                                                      /* Requete en cours pour ce module ? */
              { Recuperer_reponse_module ( module ); }
             else 
              { if (module->date_next_eana<Partage->top)                               /* Gestion décalée des I/O Analogiques */
                 { module->date_next_eana = Partage->top + MBUS_TEMPS_UPDATE_IO_ANA;                    /* Tous les 2 dixieme */
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
                                                module->do_check_eana = FALSE;                           /* Le check est fait */
                                                break;
                   
                 }
              }
           }
          liste = liste->next;                                             /* On prépare le prochain accès au prochain module */
        }
     }
    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE,
             "Run_thread: Preparing to stop . . . TID = %p", pthread_self() );
    Decharger_tous_MODBUS();
end:
    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE,
             "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_modbus.lib->Thread_run = FALSE;                                                         /* Le thread ne tourne plus ! */
    Cfg_modbus.lib->TID = 0;                                                  /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
