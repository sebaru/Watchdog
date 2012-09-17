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
 static void Modbus_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Modbus_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_modbus.lib->Thread_debug = g_key_file_get_boolean ( gkf, "MODBUS", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */
    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Modbus_Liberer_config : Libere la mémoire allouer précédemment pour lire la config modbus              */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Modbus_Liberer_config ( void )
  { 
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

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Retirer_modbusDB: Database Connection Failed" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MODULE_MODBUS, modbus->id );

    retour = Lancer_requete_SQL ( Config.log, db, requete );               /* Execution de la requete SQL */
    Libere_DB_SQL( Config.log, &db );
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

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Ajouter_modifier_modbusDB: Database Connection Failed" );
       return(-1);
     }

    libelle = Normaliser_chaine ( Config.log, modbus->libelle );         /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Ajouter_modifier_modbusDB: Normalisation libelle impossible" );
       return(-1);
     }

    ip = Normaliser_chaine ( Config.log, modbus->ip );                   /* Formatage correct des chaines */
    if (!ip)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Ajouter_modifier_modbusDB: Normalisation ip impossible" );
       Libere_DB_SQL( Config.log, &db );
       g_free(libelle);
       return(-1);
     }

    if ( ajout == TRUE )
     { g_snprintf( requete, sizeof(requete),
                  "INSERT INTO %s(enable,ip,bit,watchdog,libelle,min_e_tor,min_e_ana,min_s_tor,min_s_ana) "
                  "VALUES ('%d','%s',%d,%d,'%s','%d','%d','%d','%d')",
                   NOM_TABLE_MODULE_MODBUS, modbus->enable, ip, modbus->bit, modbus->watchdog, libelle,
                   modbus->min_e_tor, modbus->min_e_ana, modbus->min_s_tor, modbus->min_s_ana
                 );
     }
    else
     { g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                  "UPDATE %s SET "             
                  "enable='%d',ip='%s',bit='%d',watchdog='%d',libelle='%s',"
                  "min_e_tor='%d',min_e_ana='%d',min_s_tor='%d',min_s_ana='%d'"
                  " WHERE id=%d",
                   NOM_TABLE_MODULE_MODBUS,
                   modbus->enable, ip, modbus->bit, modbus->watchdog, libelle,
                   modbus->min_e_tor, modbus->min_e_ana, modbus->min_s_tor, modbus->min_s_ana,
                   modbus->id );
      }
    g_free(ip);
    g_free(libelle);

    retour_sql = Lancer_requete_SQL ( Config.log, db, requete );               /* Lancement de la requete */
    if ( retour_sql == TRUE )                                                          /* Si pas d'erreur */
     { if (ajout==TRUE) retour = Recuperer_last_ID_SQL( Config.log, db );    /* Retourne le nouvel ID modbus */
       else retour = 0;
     }
    else retour = -1;
    Libere_DB_SQL( Config.log, &db );
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
                "SELECT id,enable,ip,bit,watchdog,libelle,min_e_tor,min_e_ana,min_s_tor,min_s_ana "
                " FROM %s ORDER BY libelle", NOM_TABLE_MODULE_MODBUS );

    return ( Lancer_requete_SQL ( Config.log, db, requete ) );             /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_modbusDB: Recupération de la liste des ids des modbuss                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct MODBUSDB *Recuperer_modbusDB_suite( struct DB *db )
  { struct MODBUSDB *modbus;

    Recuperer_ligne_SQL (Config.log, db);                              /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (Config.log, db);
       return(NULL);
     }

    modbus = (struct MODBUSDB *)g_try_malloc0( sizeof(struct MODBUSDB) );
    if (!modbus) Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR,
                          "Recuperer_modbusDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &modbus->libelle, db->row[5], sizeof(modbus->libelle) );
       memcpy( &modbus->ip,      db->row[2], sizeof(modbus->ip) );
       modbus->id        = atoi(db->row[0]);
       modbus->enable     = atoi(db->row[1]);
       modbus->bit       = atoi(db->row[3]);
       modbus->watchdog  = atoi(db->row[4]);
       modbus->min_e_tor = atoi(db->row[6]);
       modbus->min_e_ana = atoi(db->row[7]);
       modbus->min_s_tor = atoi(db->row[8]);
       modbus->min_s_ana = atoi(db->row[9]);
     }
    return(modbus);
  }
/**********************************************************************************************************/
/* Chercher_module_by_id : Recherche dans la liste de travail e module dont l'id est en paramètre         */
/* Entrée: l'id du module a retrouver                                                                     */
/* Sortie: le modules trouvé ou NULL si erreur                                                            */
/**********************************************************************************************************/
 static struct MODULE_MODBUS *Chercher_module_by_id ( gint id )
  { GSList *liste;
    liste = Cfg_modbus.Modules_MODBUS;
    while ( liste )
     { struct MODULE_MODBUS *module;
       module = ((struct MODULE_MODBUS *)liste->data);
       if (module->modbus.id == id) return(module);
       liste = liste->next;
     }
    return(NULL);
  }
/**********************************************************************************************************/
/* Charger_tous_Modbus: Requete la DB pour charger les modules et les bornes modbus                       */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_tous_MODBUS ( void  )
  { struct DB *db;
    gint cpt;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

/********************************************** Chargement des modules ************************************/
    if ( ! Recuperer_modbusDB( db ) )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    Cfg_modbus.Modules_MODBUS = NULL;
    cpt = 0;
    for ( ; ; )
     { struct MODULE_MODBUS *module;
       struct MODBUSDB *modbus;

       modbus = Recuperer_modbusDB_suite( db );
       if (!modbus) break;

       module = (struct MODULE_MODBUS *)g_try_malloc0( sizeof(struct MODULE_MODBUS) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR,
                   "Charger_tous_MODBUS: Erreur allocation mémoire struct MODULE_MODBUS" );
          g_free(modbus);
          Libere_DB_SQL( Config.log, &db );
          return(FALSE);
        }
       memcpy( &module->modbus, modbus, sizeof(struct MODBUSDB) );
       g_free(modbus);
       cpt++;                                              /* Nous avons ajouté un module dans la liste ! */
                                                                        /* Ajout dans la liste de travail */
       Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, 
                "Charger_tous_MODBUS: id=%d, enable=%d", module->modbus.id, module->modbus.enable );

       Cfg_modbus.Modules_MODBUS = g_slist_append ( Cfg_modbus.Modules_MODBUS, module );
     }
    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
             "Charger_tous_MODBUS: %d modules MODBUS found  !", cpt );
    Libere_DB_SQL( Config.log, &db );
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
    module->date_retente = 0;
    for ( cpt = module->modbus.min_e_ana; cpt<module->nbr_entree_ana; cpt++)
     { SEA_range( cpt, 0 ); }
    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "Deconnecter_module %d", module->modbus.id );
    SB( module->modbus.bit, 0 );                              /* Mise a zero du bit interne lié au module */
  }
/**********************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                           */
/* Entrée: une nom et un password                                                                         */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                          */
/**********************************************************************************************************/
 static gboolean Connecter_module ( struct MODULE_MODBUS *module )
  { struct sockaddr_in src;                                            /* Données locales: pas le serveur */
    struct hostent *host;
    int connexion;

    if ( !(host = gethostbyname( module->modbus.ip )) )                           /* On veut l'adresse IP */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Connecter_module: DNS_Failed for %s", module->modbus.ip );
       return(FALSE);
     }

    src.sin_family = host->h_addrtype;
    memcpy( (char*)&src.sin_addr, host->h_addr, host->h_length );                 /* On recopie les infos */
    src.sin_port = htons( MODBUS_PORT_TCP );                                /* Port d'attaque des modules */

    if ( (connexion = socket( AF_INET, SOCK_STREAM, 0)) == -1)                          /* Protocol = TCP */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Connecter_module: Socket creation failed" );
       return(FALSE);
     }

    if (connect (connexion, (struct sockaddr *)&src, sizeof(src)) == -1)
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE,
                "Connecter_module: connexion refused by module %s", module->modbus.ip );
       close(connexion);
       return(FALSE);
     }

    fcntl( connexion, F_SETFL, SO_KEEPALIVE | SO_REUSEADDR );
    module->connexion = connexion;                                          /* Sauvegarde du fd */
    module->date_last_reponse = Partage->top;
    module->transaction_id=1;
    module->mode = MODBUS_GET_DESCRIPTION;
    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
             "Connecter_module %d (%s)", module->modbus.id, module->modbus.ip );

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
    g_free(module);
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
               "Init_watchdog_modbus: stop watchdog failed for %d (error %d)", module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
               "Init_watchdog_modbus: stop watchdog OK for %d", module->modbus.id );
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
               "Init_watchdog_modbus: close modbus tcp on watchdog failed for %d (error %d)",
               module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
               "Init_watchdog_modbus: close modbus tcp on watchdog OK for %d", module->modbus.id );
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
               "Init_watchdog_modbus: init watchdog timer failed for %d (error %d)",
               module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
               "Init_watchdog_modbus: init watchdog timer OK for %d", module->modbus.id );
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
                "Init_watchdog_modbus: watchdog start failed for %d (error %d)",
                 module->modbus.id, retour );
       Deconnecter_module( module );
     }
    else
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_DEBUG,
                "Init_watchdog_modbus: watchdog start OK for %d", module->modbus.id );
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
    gint cpt_a, cpt_poid, cpt_byte, cpt, taille;

    memset(&requete, 0, sizeof(requete) );                           /* Mise a zero globale de la requete */
    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    taille                 = 0x0006 + (module->nbr_sortie_tor/8 + 1);
    requete.taille         = htons( taille );                                                   /* taille */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.fct            = MBUS_WRITE_MULTIPLE_COIL;
    requete.adresse        = 0x00;
    requete.nbr            = htons( module->nbr_sortie_tor );                                /* bit count */
    requete.data[2]        = (module->nbr_sortie_tor/8);                                    /* Byte count */
    cpt_a = module->modbus.min_s_tor;
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
    cpt_a = module->modbus.min_s_ana;
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

    if (ntohs(module->response.transaction_id) != module->transaction_id)             /* Mauvaise reponse */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                "Processer_trame: wrong transaction_id  attendu %d, recu %d",
                 module->transaction_id, ntohs(module->response.transaction_id) );
                                            /* On laisse tomber la trame recue, et on attends la suivante */
       memset (&module->response, 0, sizeof(struct TRAME_MODBUS_REPONSE) );
       return;
     }

    if ( (guint16) module->response.proto_id )
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Processer_trame: wrong proto_id" );
       Deconnecter_module( module );
     }
    else
     { int cpt_e, cpt_byte, cpt_poid, cpt;
       gint16 chaine[17];
       module->date_last_reponse = Partage->top;                               /* Estampillage de la date */
       SB( module->modbus.bit, 1 );                              /* Mise a 1 du bit interne lié au module */
       if ( module->response.fct >=0x80 )
        { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING,
                   "Processer_trame: Erreur Reponse Module %d, Error %d, Exception code %d",
                    module->modbus.id, module->response.fct, (int)module->response.data[0] );
          Deconnecter_module( module );
        }
       else switch (module->mode)
        { case MODBUS_GET_DI:
               cpt_e = module->modbus.min_e_tor;
               for ( cpt_poid = 1, cpt_byte = 1, cpt = 0; cpt<module->nbr_entree_tor; cpt++)
                { SE( cpt_e, ( module->response.data[ cpt_byte ] & cpt_poid ) );
                  cpt_e++;
                  cpt_poid = cpt_poid << 1;
                  if (cpt_poid == 256) { cpt_byte++; cpt_poid = 1; }
                }
               module->mode = MODBUS_GET_AI;
               break;
          case MODBUS_GET_AI:
               cpt_e = module->modbus.min_e_ana;
               for ( cpt = 0; cpt<module->nbr_entree_ana; cpt++)
                { switch(Partage->ea[cpt_e].cmd_type_eana.type)
                   { case ENTREEANA_WAGO_750455:
                          if ( ! (module->response.data[ 2*cpt + 2 ] & 0x03) )
                           { int reponse;
                             reponse  = module->response.data[ 2*cpt + 1 ] << 5;
                             reponse |= module->response.data[ 2*cpt + 2 ] >> 3;
                             SEA( cpt_e, reponse );
                             SEA_range( cpt_e, 1 );
                           }
                          else SEA_range( cpt_e, 0 );
                          break;
                     case ENTREEANA_WAGO_750461:
                           { int reponse;
                             reponse  = module->response.data[ 2*cpt + 1 ] << 8;
                             reponse |= module->response.data[ 2*cpt + 2 ];
                             if (reponse < -2000.0 || reponse >= 8500.0) { SEA_range( cpt_e, 0 ); }
                             else { SEA( cpt_e, reponse );
                                    SEA_range( cpt_e, 1 );
                                  }
                           }
                          break;
                     default : SEA_range( cpt_e, 0 );
                               SEA( cpt_e, 0 );
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
               memset ( chaine, 0, sizeof(chaine) );
               chaine[0] = ntohs( *(gint16 *)((gchar *)&module->response.data +  1) );
               chaine[1] = ntohs( *(gint16 *)((gchar *)&module->response.data +  3) );
               chaine[2] = ntohs( *(gint16 *)((gchar *)&module->response.data +  5) );
               chaine[3] = ntohs( *(gint16 *)((gchar *)&module->response.data +  7) );
               chaine[4] = ntohs( *(gint16 *)((gchar *)&module->response.data +  9) );
               chaine[5] = ntohs( *(gint16 *)((gchar *)&module->response.data + 11) );
               chaine[6] = ntohs( *(gint16 *)((gchar *)&module->response.data + 13) );
               chaine[7] = ntohs( *(gint16 *)((gchar *)&module->response.data + 15) );

               Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO,
                         "Processer_trame: Get Description %s", (gchar *) chaine );
               module->mode = MODBUS_INIT_WATCHDOG1;
               break;
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
/**********************************************************************************************************/
/* Recuperer_borne: Recupere les informations d'une borne MODBUS                                          */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Recuperer_reponse_module( struct MODULE_MODBUS *module )
  { fd_set fdselect;
    struct timeval tv;
    gint retval, cpt;

    if (module->date_last_reponse + 100 < Partage->top)                  /* Detection attente trop longue */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_WARNING, "Recuperer_borne: Pb reponse module %d, deconnexion",
                 module->modbus.id );
       Deconnecter_module( module );
       return;
     }

    FD_ZERO(&fdselect);
    FD_SET(module->connexion, &fdselect );
    tv.tv_sec = 0;
    tv.tv_usec= 1000;                                                          /* Attente d'un caractere */
    retval = select(module->connexion+1, &fdselect, NULL, NULL, &tv );

    if ( retval>0 && FD_ISSET(module->connexion, &fdselect) )
     { int bute;
       if (module->nbr_oct_lu<TAILLE_ENTETE_MODBUS)
            { bute = TAILLE_ENTETE_MODBUS; }
       else { bute = TAILLE_ENTETE_MODBUS + ntohs(module->response.taille); }

       cpt = read( module->connexion,
                   (unsigned char *)&module->response +
                                     module->nbr_oct_lu,
                    bute-module->nbr_oct_lu );
       if (cpt>=0)
        { module->nbr_oct_lu += cpt;
          if (module->nbr_oct_lu >= 
              TAILLE_ENTETE_MODBUS + ntohs(module->response.taille))
           { 
             Processer_trame( module );                         /* Si l'on a trouvé une trame complète !! */
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
/**********************************************************************************************************/
/* Main: Fonction principale du MODBUS                                                                    */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct MODULE_MODBUS *module;
    GSList *liste;

    prctl(PR_SET_NAME, "W-MODBUS", 0, 0, 0 );
    memset( &Cfg_modbus, 0, sizeof(Cfg_modbus) );               /* Mise a zero de la structure de travail */
    Cfg_modbus.lib = lib;                      /* Sauvegarde de la structure pointant sur cette librairie */
    Modbus_Lire_config ();                              /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );
    Cfg_modbus.lib->Thread_run = TRUE;                                              /* Le thread tourne ! */

    g_snprintf( Cfg_modbus.lib->admin_prompt, sizeof(Cfg_modbus.lib->admin_prompt), "modbus" );
    g_snprintf( Cfg_modbus.lib->admin_help,   sizeof(Cfg_modbus.lib->admin_help),   "Manage Modbus system" );

    Cfg_modbus.Modules_MODBUS = NULL;                                     /* Init des variables du thread */

    if ( Charger_tous_MODBUS() == FALSE )                                /* Chargement des modules modbus */
     { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_ERR, "Run_modbus: No module MODBUS found -> stop" );
       Cfg_modbus.lib->Thread_run = FALSE;                                  /* Le thread ne tourne plus ! */
     }

    while(lib->Thread_run == TRUE)                                       /* On tourne tant que necessaire */
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

       if (Cfg_modbus.admin_start)
        { module = Chercher_module_by_id ( Cfg_modbus.admin_start );
          if (module) module->modbus.enable = 1;
          Cfg_modbus.admin_start = 0;
        }

       if (Cfg_modbus.admin_stop)
        { module = Chercher_module_by_id ( Cfg_modbus.admin_stop );
          if (module) module->modbus.enable = 0;
          Deconnecter_module  ( module );
          Cfg_modbus.admin_stop = 0;
        }

       if (Cfg_modbus.Modules_MODBUS == NULL ||                 /* Si pas de module référencés, on attend */
           Modbus_is_actif() == FALSE)
        { sleep(2); continue; }

       liste = Cfg_modbus.Modules_MODBUS;
       while (liste)
        { module = (struct MODULE_MODBUS *)liste->data;
          if ( module->modbus.enable != TRUE || 
               Partage->top < module->date_retente )           /* Si attente retente, on change de module */
           { liste = liste->next;                      /* On prépare le prochain accès au prochain module */
             continue;
           }

/*********************************** Début de l'interrogation du module ***********************************/
          if ( ! module->started )                                           /* Communication OK ou non ? */
           { if ( Connecter_module( module ) )
              { module->date_retente = 0;
                module->started = TRUE;
              }
             else
              { Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_INFO, "Run_modbus: Module DOWN %03d", module->modbus.id );
                module->date_retente = Partage->top + MODBUS_RETRY;
              }
           }
          else
           { if ( module->request )                                  /* Requete en cours pour ce module ? */
              { Recuperer_reponse_module ( module ); }
             else 
              { if (module->date_next_eana<Partage->top)           /* Gestion décalée des I/O Analogiques */
                 { module->date_next_eana = Partage->top + MBUS_TEMPS_UPDATE_IO_ANA;/* Tous les 2 dixieme */
                   module->do_check_eana = TRUE;
                 }
                switch (module->mode)
                 { case MODBUS_GET_DESCRIPTION: Interroger_description( module ); break;
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
                                                module->do_check_eana = FALSE;       /* Le check est fait */
                                                break;
                   
                 }
              }
           }
          liste = liste->next;                         /* On prépare le prochain accès au prochain module */
        }
     }

    Decharger_tous_MODBUS();
    Modbus_Liberer_config();                                  /* Liberation de la configuration de Modbus */

    Info_new( Config.log, Cfg_modbus.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_modbus.lib->TID = 0;                              /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
