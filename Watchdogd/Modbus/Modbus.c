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
 
 #include <glib.h>
 #include <stdio.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/stat.h>
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

#ifdef bouh
 gchar *Mode_borne[NBR_MODE_BORNE+1] =
  { "input_TOR", "output_TOR", "input_ANA", "output_ANA", "unknown" };

/**********************************************************************************************************/
/* Charger_tous_MODBUS: Requete la DB pour charger les modules et les bornes modbus                       */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 gint Mode_borne_vers_id ( gchar *mode )
  { gint i;
    for (i = 0; i<NBR_MODE_BORNE; i++)
     { if ( ! strcmp ( mode, Mode_borne[i] ) ) break;
     }
    return(i);
  }
#endif
/**********************************************************************************************************/
/* Retirer_modbusDB: Elimination d'un module modbus                                                       */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_modbusDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_MODBUS *modbus )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MODULE_MODBUS, modbus->id );

    Lancer_requete_SQL ( log, db, requete );                               /* Execution de la requete SQL */

    g_snprintf( requete, sizeof(requete),
                "DELETE FROM %s WHERE module = %d", NOM_TABLE_BORNE_MODBUS, modbus->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Retirer_borne_modbusDB: Elimination d'une borne modbus                                                 */
/* Entrée: un log et une database et la borne a supprimer                                                 */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_borne_modbusDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_BORNE_MODBUS *borne_modbus )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),
                "DELETE FROM %s WHERE module = %d", NOM_TABLE_BORNE_MODBUS, borne_modbus->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_modbusDB: Ajout ou edition d'un modbus                                                         */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure modbus                        */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_modbusDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_MODBUS *modbus )
  { gchar requete[2048];
    gchar *libelle, *ip;

    libelle = Normaliser_chaine ( log, modbus->libelle );                /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_MODBUS, "Ajouter_modbusDB: Normalisation libelle impossible" );
       return(-1);
     }

    ip = Normaliser_chaine ( Config.log, modbus->ip );                   /* Formatage correct des chaines */
    if (!ip)
     { Info( Config.log, DEBUG_ADMIN, "Ajouter_modbusDB: Normalisation ip impossible" );
       Libere_DB_SQL( Config.log, &db );
       g_free(libelle);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s(actif,ip,bit,watchdog,libelle) VALUES ('%d','%s',%d,%d,'%s')",
                NOM_TABLE_MODULE_MODBUS, modbus->actif, ip, modbus->bit, modbus->watchdog, libelle
              );
    g_free(ip);
    g_free(libelle);

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Ajouter_modbusDB: Ajout ou edition d'un modbus                                                         */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure modbus                        */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_borne_modbusDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_BORNE_MODBUS *borne )
  { gchar requete[2048];

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s(module,type,adresse,min,nbr) VALUES ('%d','%d','%d','%d','%d')",
                NOM_TABLE_BORNE_MODBUS, borne->module, borne->type, borne->adresse, borne->min, borne->nbr
              );

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_modbusDB: Recupération de la liste des ids des modbuss                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_modbusDB ( struct LOG *log, struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,actif,ip,bit,watchdog,libelle"
                " FROM %s ORDER BY libelle", NOM_TABLE_MODULE_MODBUS );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_borne_modbusDB: Recupération de la liste des borne d'un module                               */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_borne_modbusDB ( struct LOG *log, struct DB *db, guint module )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,module,type,adresse,min,nbr"
                " FROM %s WHERE module='%d' ORDER BY adresse", NOM_TABLE_BORNE_MODBUS, module );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_modbusDB: Recupération de la liste des ids des modbuss                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_MODBUS *Recuperer_modbusDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_MODBUS *modbus;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    modbus = (struct CMD_TYPE_MODBUS *)g_malloc0( sizeof(struct CMD_TYPE_MODBUS) );
    if (!modbus) Info( log, DEBUG_MODBUS, "Recuperer_modbusDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( modbus->libelle, db->row[5], sizeof(modbus->libelle) );
       memcpy( modbus->ip,      db->row[2], sizeof(modbus->ip) );
       modbus->id        = atoi(db->row[0]);
       modbus->actif     = atoi(db->row[1]);
       modbus->bit       = atoi(db->row[3]);
       modbus->watchdog  = atoi(db->row[4]);
     }
    return(modbus);
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_modbusDB: Recupération de la liste des ids des modbuss                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_BORNE_MODBUS *Recuperer_borne_modbusDB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_BORNE_MODBUS *borne;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    borne = (struct CMD_TYPE_BORNE_MODBUS *)g_malloc0( sizeof(struct CMD_TYPE_BORNE_MODBUS) );
    if (!borne) Info( log, DEBUG_MODBUS, "Recuperer_borne_modbusDB_suite: Erreur allocation mémoire" );
    else
     { borne->id      = atoi(db->row[0]);
       borne->module  = atoi(db->row[1]);
       borne->type    = atoi(db->row[2]);
       borne->adresse = atoi(db->row[3]);
       borne->min     = atoi(db->row[4]);
       borne->nbr     = atoi(db->row[5]);
     }
    return(borne);
  }
/**********************************************************************************************************/
/* Rechercher_modbusDB: Recupération du modbus dont le id est en parametre                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_MODBUS *Rechercher_modbusDB ( struct LOG *log, struct DB *db, guint id )
  { gchar requete[512];
    struct CMD_TYPE_MODBUS *modbus;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,actif,ip,bit,watchdog,libelle"
                " FROM %s WHERE id=%d",
                NOM_TABLE_MODULE_MODBUS, id );
       
    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }
       
    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_MODBUS, "Rechercher_modbusDB: MODBUS non trouvé dans la BDD", id );
       return(NULL);
     }
       
    modbus = g_malloc0( sizeof(struct CMD_TYPE_MODBUS) );
    if (!modbus)
     { Info( log, DEBUG_MODBUS, "Rechercher_modbusDB: Mem error" ); }
    else
     { memcpy( modbus->libelle, db->row[5], sizeof(modbus->libelle) );
       memcpy( modbus->ip,      db->row[2], sizeof(modbus->ip) );
       modbus->id        = atoi(db->row[0]);
       modbus->actif     = atoi(db->row[1]);
       modbus->bit       = atoi(db->row[3]);
       modbus->watchdog  = atoi(db->row[4]);
     }
    return(modbus);
  }
/**********************************************************************************************************/
/* Rechercher_modbusDB: Recupération du modbus dont le id est en parametre                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_BORNE_MODBUS *Rechercher_borne_modbusDB ( struct LOG *log, struct DB *db, guint id )
  { gchar requete[512];
    struct CMD_TYPE_BORNE_MODBUS *borne;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,module,type,adresse,min,nbr"
                " FROM %s WHERE id=%d",
                NOM_TABLE_BORNE_MODBUS, id );
       
    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }
       
    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_MODBUS, "Rechercher_borne_modbusDB: MODBUS non trouvé dans la BDD", id );
       return(NULL);
     }
       
    borne = (struct CMD_TYPE_BORNE_MODBUS *)g_malloc0( sizeof(struct CMD_TYPE_BORNE_MODBUS) );
    if (!borne) Info( log, DEBUG_MODBUS, "Recuperer_borne_modbusDB_suite: Erreur allocation mémoire" );
    else
     { borne->id      = atoi(db->row[0]);
       borne->module  = atoi(db->row[1]);
       borne->type    = atoi(db->row[2]);
       borne->adresse = atoi(db->row[3]);
       borne->min     = atoi(db->row[4]);
       borne->nbr     = atoi(db->row[5]);
     }
    return(borne);
  }
/**********************************************************************************************************/
/* Modifier_modbusDB: Modification d'un modbus Watchdog                                                   */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_modbusDB( struct LOG *log, struct DB *db, struct CMD_TYPE_MODBUS *modbus )
  { gchar *libelle, *ip;
    gchar requete[2048];

    libelle = Normaliser_chaine ( log, modbus->libelle );                /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_MODBUS, "Modifier_modbusDB: Normalisation libelle impossible" );
       return(-1);
     }

    ip = Normaliser_chaine ( Config.log, modbus->ip );                   /* Formatage correct des chaines */
    if (!ip)
     { Info( Config.log, DEBUG_ADMIN, "Modifier_modbusDB: Normalisation ip impossible" );
       Libere_DB_SQL( Config.log, &db );
       g_free(libelle);
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "actif='%d',ip='%s',bit='%d',watchdog='%d',libelle='%s'"
                " WHERE id=%d",
                NOM_TABLE_MODULE_MODBUS,
                modbus->actif, ip, modbus->bit, modbus->watchdog, libelle,
                modbus->id );
    g_free(libelle);
    g_free(ip);

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Modifier_borne_modbusDB: Modification d'une borne modbus Watchdog                                      */
/* Entrées: un log, une db, et une borne                                                                  */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 gboolean Modifier_borne_modbusDB( struct LOG *log, struct DB *db, struct CMD_TYPE_BORNE_MODBUS *borne )
  { gchar requete[2048];
    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "type='%d',adresse='%d',min='%d',nbr='%d'"
                " WHERE id=%d",
                NOM_TABLE_MODULE_MODBUS,
                borne->type, borne->adresse, borne->min, borne->nbr,
                borne->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Chercher_module_by_id : Recherche dans la liste de travail e module dont l'id est en paramètre         */
/* Entrée: l'id du module a retrouver                                                                     */
/* Sortie: le modules trouvé ou NULL si erreur                                                            */
/**********************************************************************************************************/
 static struct MODULE_MODBUS *Chercher_module_by_id ( gint id )
  { GList *liste;
    liste = Partage->com_modbus.Modules_MODBUS;
    while ( liste )
     { struct MODULE_MODBUS *module;
       module = ((struct MODULE_MODBUS *)liste->data);
       if (module->modbus.id == id) return(module);
       liste = liste->next;
     }
    return(NULL);
  }
/**********************************************************************************************************/
/* Charger_borne_module_MODBUS: Requete la DB pour charger les bornes d'un module MODBUS                  */
/* Entrée: la db, le module                                                                               */
/* Sortie: TRUE/FALSE                                                                                     */
/**********************************************************************************************************/
 static gboolean Charger_borne_module_MODBUS ( struct DB *db, struct MODULE_MODBUS *module  )
  { struct CMD_TYPE_BORNE_MODBUS *borne;

    if ( ! Recuperer_borne_modbusDB( Config.log, db, module->modbus.id ) )
     { Info( Config.log, DEBUG_MODBUS,
             "Charger_borne_module_MODBUS: Erreur recuperation borne" );
       return(FALSE);
     }

    for( ; ; )
     { borne = Recuperer_borne_modbusDB_suite( Config.log, db );
       if (!borne) break;

       module->Bornes = g_list_append ( module->Bornes, borne );        /* Ajout dans la liste de travail */
       Info_n( Config.log, DEBUG_MODBUS, "Charger_borne_module_MODBUS:  borne id = ", borne->id      );
       Info_n( Config.log, DEBUG_MODBUS, "                             module id = ", borne->module  );
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Charger_un_MODBUS: Charge un module dont l'id est en paramètre                                         */
/* Entrée: l'ID du module a charger                                                                       */
/* Sortie: TRUE si pas de souci, FALSE si erreur                                                          */
/**********************************************************************************************************/
 static gboolean Charger_un_MODBUS ( gint id  )
  { struct MODULE_MODBUS *module;
    struct CMD_TYPE_MODBUS *modbus;
    struct DB *db;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db) return(FALSE);

    module = (struct MODULE_MODBUS *)g_malloc0(sizeof(struct MODULE_MODBUS));
    if (!module)                                                      /* Si probleme d'allocation mémoire */
     { Libere_DB_SQL( Config.log, &db );
       Info( Config.log, DEBUG_MODBUS,
             "Charger_un_modbus: Erreur allocation mémoire struct MODULE_MODBUS" );
       return(FALSE);
     }

    modbus = Rechercher_modbusDB ( Config.log, db, id );
    if (!modbus)                                                      /* Si probleme d'allocation mémoire */
     { Libere_DB_SQL( Config.log, &db );
       Info( Config.log, DEBUG_MODBUS,
             "Charger_un_modbus: Erreur allocation mémoire struct CMD_TYPE_MODBUS" );
       g_free(module);
       return(FALSE);
     }

    memcpy( &module->modbus, modbus, sizeof(struct CMD_TYPE_MODBUS) );
    g_free(modbus);

    Info_n( Config.log, DEBUG_MODBUS, "Charger_modules_MODBUS:  id       = ", module->modbus.id       );
    Info_n( Config.log, DEBUG_MODBUS, "                      -  actif    = ", module->modbus.actif    );
    Info_c( Config.log, DEBUG_MODBUS, "                      -  ip       = ", module->modbus.ip       );
    Info_n( Config.log, DEBUG_MODBUS, "                      -  bit      = ", module->modbus.bit      );
    Info_n( Config.log, DEBUG_MODBUS, "                      -  watchdog = ", module->modbus.watchdog );

    if ( ! Charger_borne_module_MODBUS( db, module ) )
     { Libere_DB_SQL( Config.log, &db );
       Info( Config.log, DEBUG_MODBUS,
             "Charger_un_modbus: Erreur chargement bornes" );
       g_free(module);
       return(FALSE);
     }

    Partage->com_modbus.Modules_MODBUS = g_list_append ( Partage->com_modbus.Modules_MODBUS, module );

    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Charger_tous_MODBUS: Requete la DB pour charger les modules et les bornes modbus                       */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_tous_MODBUS ( void  )
  { struct DB *db;
    GList *liste;
    gint cpt;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db) return(FALSE);

/********************************************** Chargement des modules ************************************/
    if ( ! Recuperer_modbusDB( Config.log, db ) )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    Partage->com_modbus.Modules_MODBUS = NULL;
    cpt = 0;
    for ( ; ; )
     { struct MODULE_MODBUS *module;
       struct CMD_TYPE_MODBUS *modbus;

       modbus = Recuperer_modbusDB_suite( Config.log, db );
       if (!modbus) break;

       module = (struct MODULE_MODBUS *)g_malloc0( sizeof(struct MODULE_MODBUS) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info( Config.log, DEBUG_MODBUS,
                "Charger_tous_MODBUS: Erreur allocation mémoire struct MODULE_MODBUS" );
          g_free(modbus);
          Libere_DB_SQL( Config.log, &db );
          return(FALSE);
        }
       memcpy( &module->modbus, modbus, sizeof(struct CMD_TYPE_MODBUS) );
       g_free(modbus);
       cpt++;                                              /* Nous avons ajouté un module dans la liste ! */
                                                                        /* Ajout dans la liste de travail */
       Info_n( Config.log, DEBUG_MODBUS, "Charger_tous_MODBUS:  id    = ", module->modbus.id    );
       Info_n( Config.log, DEBUG_MODBUS, "                   -  actif = ", module->modbus.actif );

       Partage->com_modbus.Modules_MODBUS = g_list_append ( Partage->com_modbus.Modules_MODBUS, module );
     }

    liste = Partage->com_modbus.Modules_MODBUS;            /* Chargement des bornes associées aux modules */
    while(liste)
     { struct MODULE_MODBUS *module;
       module = (struct MODULE_MODBUS *)liste->data;
       Charger_borne_module_MODBUS( db, module );
       liste=liste->next;
     }

    Info_n( Config.log, DEBUG_MODBUS, "Charger_tous_MODBUS: module MODBUS found  !", cpt );
    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Deconnecter: Deconnexion du module                                                                     */
/* Entrée: un id                                                                                          */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Deconnecter_module ( struct MODULE_MODBUS *module )
  { if (!module) return;
    if (module->started == FALSE) return;

    close ( module->connexion );
    module->connexion = 0;
    module->started = FALSE;
    module->request = FALSE;
    module->nbr_deconnect++;
    module->date_retente = 0;
    Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Deconnecter_module", module->modbus.id );
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
     { Info( Config.log, DEBUG_MODBUS, "MODBUS: Connecter_module: DNS_Failed" );
       return(FALSE);
     }

    src.sin_family = host->h_addrtype;
    memcpy( (char*)&src.sin_addr, host->h_addr, host->h_length );                 /* On recopie les infos */
    src.sin_port = htons( MODBUS_PORT_TCP );                                /* Port d'attaque des modules */

    if ( (connexion = socket( AF_INET, SOCK_STREAM, 0)) == -1)                          /* Protocol = TCP */
     { Info( Config.log, DEBUG_MODBUS, "MODBUS: Connecter_module: Socket creation failed" );
       return(FALSE);
     }

    if (connect (connexion, (struct sockaddr *)&src, sizeof(src)) == -1)
     { Info_c( Config.log, DEBUG_MODBUS, "MODBUS: Connecter_module: connexion refused by module",
               module->modbus.ip );
       close(connexion);
       return(FALSE);
     }

    fcntl( connexion, F_SETFL, SO_KEEPALIVE | SO_REUSEADDR );
    module->connexion = connexion;                                          /* Sauvegarde du fd */
    module->date_last_reponse = time(NULL);
    module->borne_en_cours = module->Bornes;
    Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Connecter_module", module->modbus.id );
    SB( module->modbus.bit, 1 );                                 /* Mise a 1 du bit interne lié au module */

    return(TRUE);
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_un_MODBUS ( struct MODULE_MODBUS *module )
  { if (!module) return;
    Deconnecter_module  ( module );                                  /* Deconnexion du module en question */
    if (module->Bornes)
     { g_list_foreach( module->Bornes, (GFunc) g_free, NULL );
       g_list_free( module->Bornes );
     }
    Partage->com_modbus.Modules_MODBUS = g_list_remove ( Partage->com_modbus.Modules_MODBUS, module );
    g_free(module);
  }
/**********************************************************************************************************/
/* Decharger_tous_MODBUS: Decharge l'ensemble des modules MODBUS                                          */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Decharger_tous_MODBUS ( void  )
  { struct MODULE_MODBUS *module;
    while ( Partage->com_modbus.Modules_MODBUS )
     { module = (struct MODULE_MODBUS *)Partage->com_modbus.Modules_MODBUS->data;
       Decharger_un_MODBUS ( module );
     }
  }
/**********************************************************************************************************/
/* Modbus_is_actif: Renvoi TRUE si au moins un des modules modbus est actif                               */
/* Entrée: rien                                                                                           */
/* Sortie: TRUE/FALSE                                                                                     */
/**********************************************************************************************************/
 static gboolean Modbus_is_actif ( void )
  { GList *liste;
    liste = Partage->com_modbus.Modules_MODBUS;
    while ( liste )
     { struct MODULE_MODBUS *module;
       module = ((struct MODULE_MODBUS *)liste->data);

       if (module->modbus.actif) return(TRUE);
       liste = liste->next;
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Init_watchdog_modbus( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE_STOR requete;                            /* Definition d'une trame MODBUS */

    memset (&requete, 0, sizeof(struct TRAME_MODBUS_REQUETE_STOR) );

    requete.transaction_id = 0;
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( 0x100A );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;
    requete.valeur         = htons( 0x0000 );                          /* 5 secondes avant coupure sortie */

    if ( write ( module->connexion, &requete, sizeof(requete) )                    /* Envoi de la requete */
         != sizeof (requete) )
     { Info_n( Config.log, DEBUG_MODBUS,
               "MODBUS: Init_watchdog_modbus: stop watchdog failed", module->modbus.id );
       Deconnecter_module( module );
     }

    module->transaction_id++;
    requete.transaction_id = 0;
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( 0x1009 );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;
    requete.valeur         = htons( 0x0001 );                          /* 5 secondes avant coupure sortie */

    if ( write ( module->connexion, &requete, sizeof(requete) )                    /* Envoi de la requete */
         != sizeof (requete) )
     { Info_n( Config.log, DEBUG_MODBUS,
               "MODBUS: Init_watchdog_modbus: close modbus tcp on watchdog failed", module->modbus.id );
       Deconnecter_module( module );
     }

    module->transaction_id++;
    requete.transaction_id = 0;
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( 0x1000 );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;
    requete.valeur         = htons( module->modbus.watchdog );                          /* coupure sortie */

    if ( write ( module->connexion, &requete, sizeof(requete) )                    /* Envoi de la requete */
         != sizeof (requete) )
     { Info_n( Config.log, DEBUG_MODBUS,
               "MODBUS: Init_watchdog_modbus: init watchdog timer failed", module->modbus.id );
       Deconnecter_module( module );
     }

    requete.transaction_id = 0;
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( 0x100A );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;
    requete.valeur         = htons( 0x0001 );                                              /* Start Timer */

    if ( write ( module->connexion, &requete, sizeof(requete) )                    /* Envoi de la requete */
         != sizeof (requete) )
     { Info_n( Config.log, DEBUG_MODBUS,
               "MODBUS: Init_watchdog_modbus: watchdog start failed", module->modbus.id );
       Deconnecter_module( module );
     }

    module->transaction_id=1;
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_borne_input_tor( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE_ETOR requete;                            /* Definition d'une trame MODBUS */
    struct CMD_TYPE_BORNE_MODBUS *borne;
    borne = (struct CMD_TYPE_BORNE_MODBUS *)module->borne_en_cours->data;
    memset (&requete, 0, sizeof(struct TRAME_MODBUS_REQUETE_ETOR) );

    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( borne->adresse );
    requete.nbr            = htons( borne->nbr );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_ENTRE_TOR;

    if ( write ( module->connexion, &requete, sizeof(requete) )                    /* Envoi de la requete */
         != sizeof (requete) )
     { Deconnecter_module( module ); }
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_borne_input_ana( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE_EANA requete;                            /* Definition d'une trame MODBUS */
    struct CMD_TYPE_BORNE_MODBUS *borne;
    borne = (struct CMD_TYPE_BORNE_MODBUS *)module->borne_en_cours->data;
    memset (&requete, 0, sizeof(struct TRAME_MODBUS_REQUETE_EANA) );

    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( borne->adresse );
    requete.nbr            = htons( borne->nbr );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_ENTRE_ANA;

    if ( write ( module->connexion, &requete, sizeof(requete) )                    /* Envoi de la requete */
         != sizeof (requete) )
     { Deconnecter_module( module ); }
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_borne_output_tor( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE_STOR requete;                            /* Definition d'une trame MODBUS */
    struct CMD_TYPE_BORNE_MODBUS *borne;
    gint cpt_a, valeur;
    borne = (struct CMD_TYPE_BORNE_MODBUS *)module->borne_en_cours->data;

    memset (&requete, 0, sizeof(struct TRAME_MODBUS_REQUETE_STOR) );

    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( borne->adresse );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;

    switch ( borne->nbr )
     { case 8:                                                           /* Bornes a 8 sorties !! */
               requete.taille         = htons( 0x0006 );        /* taille, en comptant le unit_id */
               cpt_a = borne->min;
               valeur = 0;
               if ( A(cpt_a++) ) valeur |=   1;
               if ( A(cpt_a++) ) valeur |=   4;
               if ( A(cpt_a++) ) valeur |=  16;
               if ( A(cpt_a++) ) valeur |=  64;
               if ( A(cpt_a++) ) valeur |=   2;
               if ( A(cpt_a++) ) valeur |=   8;
               if ( A(cpt_a++) ) valeur |=  32;
               if ( A(cpt_a++) ) valeur |= 128;
               requete.valeur = htons( valeur );
               break;
       default: Info_n( Config.log, DEBUG_MODBUS,
                        "MODBUS: Interroger_borne_output_tor: borne InputTOR non gérée", borne->nbr
                      );
     }

    if ( write ( module->connexion, &requete, sizeof(requete) )                    /* Envoi de la requete */
         != sizeof (requete) )
     { Deconnecter_module( module ); }
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_borne( struct MODULE_MODBUS *module )
  { struct CMD_TYPE_BORNE_MODBUS *borne;
    borne = (struct CMD_TYPE_BORNE_MODBUS *)module->borne_en_cours->data;
    switch (borne->type)
     { case BORNE_INPUT_TOR:  Interroger_borne_input_tor( module );
            break;

       case BORNE_INPUT_ANA:  Interroger_borne_input_ana( module );
            break;

       case BORNE_OUTPUT_TOR: Interroger_borne_output_tor( module );                /* Borne de sortie ?? */
            break;

       default: Info(Config.log, DEBUG_MODBUS, "MODBUS: Interroger_borne: type de borne non reconnu" );
     }

    module->nbr_oct_lu = 0;
    module->request = TRUE;                                                  /* Une requete a été envoyée */
  }
/**********************************************************************************************************/
/* Recuperer_borne: Recupere les informations d'une borne MODBUS                                          */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Processer_trame( struct MODULE_MODBUS *module )
  { struct CMD_TYPE_BORNE_MODBUS *borne;
    borne = (struct CMD_TYPE_BORNE_MODBUS *)module->borne_en_cours->data;

    if (ntohs(module->response.transaction_id) != module->transaction_id)             /* Mauvaise reponse */
     { if (ntohs(module->response.transaction_id))              /* Reponse aux trames d'initialisation ?? */
        { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Processer_trame: wrong transaction_id  attendu",
                  module->transaction_id );
          Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Processer_trame: wrong transaction_id  reponse",
                  ntohs(module->response.transaction_id) );
        }
       else
        { Info_n( Config.log, DEBUG_MODBUS,
                  "MODBUS: Processer_trame: trame d'init recue", module->modbus.id );
        }
                                            /* On laisse tomber la trame recue, et on attends la suivante */
       memset (&module->response, 0, sizeof(struct TRAME_MODBUS_REPONSE) );
       return;
     }

    if ( (guint16) module->response.proto_id )
     { Info( Config.log, DEBUG_MODBUS, "MODBUS: Processer_trame: wrong proto_id" );
       Deconnecter_module( module );
     }

    else
     { int nbr, cpt_e;
       module->date_last_reponse = time(NULL);                                 /* Estampillage de la date */
       nbr = module->response.nbr;
       switch ( module->response.fct )
        { case MBUS_ENTRE_TOR:                                       /* Quelles type de borne d'entrées ? */
               switch ( borne->nbr )
                { case 8:                                                        /* Bornes a 8 entrées !! */
                          if (nbr != 1) break;         /* Si nous n'avons pas recu le bon nombre d'octets */
                          cpt_e = borne->min;
                          SE( cpt_e++, ( module->response.data[0] & 1  ) );
                          SE( cpt_e++, ( module->response.data[0] & 4  ) );
                          SE( cpt_e++, ( module->response.data[0] & 16 ) );
                          SE( cpt_e++, ( module->response.data[0] & 64 ) );
                          SE( cpt_e++, ( module->response.data[0] & 2  ) );
                          SE( cpt_e++, ( module->response.data[0] & 8  ) );
                          SE( cpt_e++, ( module->response.data[0] & 32 ) );
                          SE( cpt_e++, ( module->response.data[0] & 128) );
                          break;
                  default: Info_n( Config.log, DEBUG_MODBUS,
                                   "MODBUS: Processer_trame: borne InputTOR non gérée",
                                   borne->nbr
                                 );
                }
               break;
          case MBUS_SORTIE_TOR:                                      /* Quelles type de borne de sortie ? */
               break;
          case MBUS_ENTRE_ANA:                                       /* Quelles type de borne d'entrées ? */
               switch ( borne->nbr )
                { case 4: { guint reponse;                                       /* Bornes a 4 entrées !! */
                            if (nbr != 8) break;       /* Si nous n'avons pas recu le bon nombre d'octets */

                            cpt_e = borne->min;
                            if ( ! (module->response.data[1] & 0x03) )
                             { reponse = module->response.data[0] << 5;
                               reponse |= module->response.data[1] >> 3;
                               SEA( cpt_e++, reponse );
                             }
                            else SEA( cpt_e++, 0 );
                            if ( ! (module->response.data[3] & 0x03) )
                             { reponse = module->response.data[2] << 5;
                               reponse |= module->response.data[3] >> 3;
                               SEA( cpt_e++, reponse );
                             }
                            else SEA( cpt_e++, 0 );
                            if ( ! (module->response.data[5] & 0x03) )
                             { reponse = module->response.data[4] << 5;
                               reponse |= module->response.data[5] >> 3;
                               SEA( cpt_e++, reponse );
                             }
                            else SEA( cpt_e++, 0 );
                            if ( ! (module->response.data[7] & 0x03) )
                             { reponse = module->response.data[6] << 5;
                               reponse |= module->response.data[7] >> 3;
                               SEA( cpt_e++, reponse );
                             }
                            else SEA( cpt_e++, 0 );

                          /*SEA( cpt_e++, ( Comm_MODBUS[id_module].response.data[0] & 1  ) );*/
/*{ int cpt;
             printf("Entrée:\n");
             for (cpt=Config.module_modbus[id_module].borne[id_borne].min; cpt<Config.module_modbus[id_module].borne[id_borne].min+8; cpt++)
 		    { printf("E%d = %d ", cpt, E(cpt) );
 		    }
             printf("\n");
}*/

                          }
                          break;
                  default: Info_n( Config.log, DEBUG_MODBUS,
                                   "MODBUS: Processer_trame: borne InputANA non gérée", 
                                   borne->nbr
                                 );
                }
               break;


          case 0x80 + MBUS_ENTRE_TOR:
               Info( Config.log, DEBUG_MODBUS, "MODBUS: Processer_trame: Erreur ENTRE_TOR" );
               break;
          case 0x80 + MBUS_SORTIE_TOR:
               Info( Config.log, DEBUG_MODBUS, "MODBUS: Processer_trame: Erreur SORTIE_TOR" );
               break;
          case 0x80 + MBUS_ENTRE_ANA:
               Info( Config.log, DEBUG_MODBUS, "MODBUS: Processer_trame: Erreur ENTRE_ANA" );
               break;
          default: Info( Config.log, DEBUG_MODBUS, "MODBUS: Processer_trame: fct inconnu" );
        }
     }

    module->request = FALSE;                                                 /* Une requete a été traitée */
    module->transaction_id++;
    memset (&module->response, 0, sizeof(struct TRAME_MODBUS_REPONSE) );
  }
/**********************************************************************************************************/
/* Recuperer_borne: Recupere les informations d'une borne MODBUS                                          */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Recuperer_borne( struct MODULE_MODBUS *module )
  { fd_set fdselect;
    struct timeval tv;
    gint retval, cpt;

    if (module->date_last_reponse + 10 < time(NULL))                     /* Detection attente trop longue */
     { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Recuperer_borne: Pb reponse module, deconnexion",
               module->modbus.id );
       Deconnecter_module( module );
       return;
     }

    FD_ZERO(&fdselect);
    FD_SET(module->connexion, &fdselect );
    tv.tv_sec = 0;
    tv.tv_usec= 100;                                                           /* Attente d'un caractere */
    retval = select(module->connexion+1, &fdselect, NULL, NULL, &tv );

    if ( retval>=0 && FD_ISSET(module->connexion, &fdselect) )
     { int bute;
       if (module->nbr_oct_lu<TAILLE_ENTETE_MODBUS)
            { bute = TAILLE_ENTETE_MODBUS; }
       else { bute = TAILLE_ENTETE_MODBUS + ntohs(module->response.taille); }

       cpt = read( module->connexion,
                   (unsigned char *)&module->response +
                                     module->nbr_oct_lu,
                    bute-module->nbr_oct_lu );
       if (cpt>0)
        { module->nbr_oct_lu += cpt;
          if (module->nbr_oct_lu >= 
              TAILLE_ENTETE_MODBUS + ntohs(module->response.taille))
           { 
             pthread_mutex_lock( &Partage->com_dls.synchro );
             Processer_trame( module );                         /* Si l'on a trouvé une trame complète !! */
             pthread_mutex_unlock( &Partage->com_dls.synchro );
             module->nbr_oct_lu = 0;
           }
         } 
        else
         { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Recuperer_borne: wrong trame", module->modbus.id );
           Deconnecter_module ( module );
         }
      }
  }
/**********************************************************************************************************/
/* Main: Fonction principale du MODBUS                                                                    */
/**********************************************************************************************************/
 void Run_modbus ( void )
  { struct MODULE_MODBUS *module;
    GList *liste;

    prctl(PR_SET_NAME, "W-MODBUS", 0, 0, 0 );
    Info( Config.log, DEBUG_MODBUS, "MODBUS: demarrage" );

    Partage->com_modbus.Modules_MODBUS = NULL;                            /* Init des variables du thread */

    if ( Charger_tous_MODBUS() == FALSE )                                /* Chargement des modules modbus */
     { Info( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: No module MODBUS found -> stop" );
       pthread_exit(GINT_TO_POINTER(-1));
     }

    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { usleep(1000);
       sched_yield();

       if (Partage->com_modbus.reload == TRUE)
        { Info( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Reloading conf" );
          Decharger_tous_MODBUS();
          Charger_tous_MODBUS();
          Partage->com_modbus.reload = FALSE;
        }

       if (Partage->com_modbus.admin_del)
        { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Deleting module",
                  Partage->com_modbus.admin_del );
          module = Chercher_module_by_id ( Partage->com_modbus.admin_del );
          Decharger_un_MODBUS ( module );
          Partage->com_modbus.admin_del = 0;
        }

       if (Partage->com_modbus.admin_del_borne)
        { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Deleting une borne",
                  Partage->com_modbus.admin_del_borne );
          module = Chercher_module_by_id ( Partage->com_modbus.admin_del_borne );
          Decharger_un_MODBUS ( module );
          Charger_un_MODBUS ( Partage->com_modbus.admin_del_borne );
          Partage->com_modbus.admin_del_borne = 0;
        }

       if (Partage->com_modbus.admin_add)
        { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Adding module",
                  Partage->com_modbus.admin_add );
          Charger_un_MODBUS ( Partage->com_modbus.admin_add );
          Partage->com_modbus.admin_add = 0;
        }

       if (Partage->com_modbus.admin_add_borne)
        { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Adding une borne",
                  Partage->com_modbus.admin_add_borne );
          module = Chercher_module_by_id ( Partage->com_modbus.admin_del_borne );
          Decharger_un_MODBUS ( module );
          Charger_un_MODBUS ( Partage->com_modbus.admin_add_borne );
          Partage->com_modbus.admin_add_borne = 0;
        }

       if (Partage->com_modbus.admin_start)
        { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Starting module",
                  Partage->com_modbus.admin_start );
          module = Chercher_module_by_id ( Partage->com_modbus.admin_start );
          if (module) module->modbus.actif = 1;
          Partage->com_modbus.admin_start = 0;
        }

       if (Partage->com_modbus.admin_stop)
        { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Stoping module",
                  Partage->com_modbus.admin_stop );
          module = Chercher_module_by_id ( Partage->com_modbus.admin_stop );
          if (module) module->modbus.actif = 0;
          Deconnecter_module  ( module );
          Partage->com_modbus.admin_stop = 0;
        }

       if (Partage->com_modbus.Modules_MODBUS == NULL ||        /* Si pas de module référencés, on attend */
           Modbus_is_actif() == FALSE)
        { sleep(2); continue; }

       liste = Partage->com_modbus.Modules_MODBUS;
       while (liste)
        { module = (struct MODULE_MODBUS *)liste->data;
          if ( module->modbus.actif != TRUE || 
               Partage->top < module->date_retente )           /* Si attente retente, on change de module */
           { liste = liste->next;                      /* On prépare le prochain accès au prochain module */
             continue;
           }

/*********************************** Début de l'interrogation du module ***********************************/
          if ( ! module->started )                                           /* Communication OK ou non ? */
           { if ( Connecter_module( module ) )
              { if ( module->modbus.watchdog )
                 { Init_watchdog_modbus(module); }
                module->date_retente = 0;;
                module->started = TRUE;
              }
             else
              { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Module DOWN", module->modbus.id );
                module->date_retente = Partage->top + MODBUS_RETRY;
              }
           }
          else
           { if ( module->request )                                  /* Requete en cours pour ce module ? */
              { Recuperer_borne ( module ); }
             else
              {                                        /* Si pas de requete, on passe a la borne suivante */
                module->borne_en_cours = module->borne_en_cours->next;
                if ( ! module->borne_en_cours )                                      /* Tour des bornes ? */
                 { module->borne_en_cours = module->Bornes; }

/***************************** Début de l'interrogation de la borne du module *****************************/
                Interroger_borne ( module );
             }
           }
          liste = liste->next;                         /* On prépare le prochain accès au prochain module */
        }
     }

    Decharger_tous_MODBUS();
    Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
