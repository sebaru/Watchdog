/**********************************************************************************************************/
/* Watchdogd/Modbus/Modbus.c  Gestion des modules MODBUS Watchdgo 2.0                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 16 avr 2009 16:27:30 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Modbus.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - 
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

 #include "Erreur.h"
 #include "Config.h"
 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
 #include "proto_dls.h"                                                             /* Acces a A(x), E(x) */   
 #include "Modbus.h"

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
/**********************************************************************************************************/
/* Charger_tous_MODBUS: Requete la DB pour charger les modules et les bornes modbus                       */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static struct MODULE_MODBUS *Chercher_module_by_id ( gint id )
  { GList *liste;
    liste = Partage->com_modbus.Modules_MODBUS;
    while ( liste )
     { struct MODULE_MODBUS *module;
       module = ((struct MODULE_MODBUS *)liste->data);
       if (module->id == id) return(module);
       liste = liste->next;
     }
    return(NULL);
  }
/**********************************************************************************************************/
/* Charger_tous_MODBUS: Requete la DB pour charger les modules et les bornes modbus                       */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_un_MODBUS_DB ( struct MODULE_MODBUS *module, gint id  )
  { gchar requete[128];
    struct DB *db;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db) return(FALSE);

/********************************************** Chargement des modules ************************************/
    g_snprintf( requete, sizeof(requete), "SELECT actif,ip,bit,watchdog FROM %s WHERE id=%d",
                NOM_TABLE_MODULE_MODBUS, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    while ( Recuperer_ligne_SQL (Config.log, db) )
     { module->id       = id;
       module->actif    = atoi (db->row[0]);
       g_snprintf( module->ip, sizeof(module->ip), "%s", db->row[1] );
       module->bit      = atoi (db->row[2]);         /* Bit interne B d'etat communication avec le module */
       module->watchdog = atoi (db->row[3]);
                                                                        /* Ajout dans la liste de travail */
       Info_n( Config.log, DEBUG_MODBUS, "Charger_modules_MODBUS:  id       = ", module->id       );
       Info_n( Config.log, DEBUG_MODBUS, "                      -  actif    = ", module->actif    );
       Info_c( Config.log, DEBUG_MODBUS, "                      -  ip       = ", module->ip       );
       Info_n( Config.log, DEBUG_MODBUS, "                      -  bit      = ", module->bit      );
       Info_n( Config.log, DEBUG_MODBUS, "                      -  watchdog = ", module->watchdog );
     }
    Liberer_resultat_SQL ( Config.log, db );
/******************************************* Chargement des bornes ****************************************/
    g_snprintf( requete, sizeof(requete), "SELECT id,type,adresse,min,nbr FROM %s WHERE module=%d",
                NOM_TABLE_BORNE_MODBUS, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    while ( Recuperer_ligne_SQL (Config.log, db) )
     { struct BORNE_MODBUS *borne;

       borne = (struct BORNE_MODBUS *)g_malloc0( sizeof(struct BORNE_MODBUS) );
       if (!borne)                                                   /* Si probleme d'allocation mémoire */
        { Info( Config.log, DEBUG_MEM,
                "Charger_modules_MODBUS: Erreur allocation mémoire struct BORNE_MODBUS" );
          continue;
        }

       borne->id       = atoi (db->row[0]);
       borne->type     = atoi (db->row[1]);
       borne->adresse  = atoi (db->row[2]);
       borne->min      = atoi (db->row[3]);
       borne->nbr      = atoi (db->row[4]);

       module->Bornes = g_list_append ( module->Bornes, borne );        /* Ajout dans la liste de travail */
       Info_n( Config.log, DEBUG_MODBUS, "Charger_un_MODBUS_DB:  borne id = ", borne->id      );
       Info_n( Config.log, DEBUG_MODBUS, "                           type = ", borne->type    );
       Info_n( Config.log, DEBUG_MODBUS, "                        adresse = ", borne->adresse );
       Info_n( Config.log, DEBUG_MODBUS, "                            min = ", borne->min     );
       Info_n( Config.log, DEBUG_MODBUS, "                            nbr = ", borne->nbr     );
     }
    Liberer_resultat_SQL ( Config.log, db );

    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Charger_tous_MODBUS: Requete la DB pour charger les modules et les bornes modbus                            */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_tous_MODBUS ( void  )
  { gchar requete[128];
    struct DB *db;
    gint cpt;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db) return(FALSE);

/********************************************** Chargement des modules ************************************/
    g_snprintf( requete, sizeof(requete), "SELECT id FROM %s",
                NOM_TABLE_MODULE_MODBUS
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    Partage->com_modbus.Modules_MODBUS = NULL;
    cpt = 0;
    while ( Recuperer_ligne_SQL (Config.log, db) )
     { struct MODULE_MODBUS *module;

       module = (struct MODULE_MODBUS *)g_malloc0( sizeof(struct MODULE_MODBUS) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info( Config.log, DEBUG_MEM,
                "Charger_modules_MODBUS: Erreur allocation mémoire struct MODULE_MODBUS" );
          continue;
        }

       Charger_un_MODBUS_DB( module, atoi (db->row[0]) );
                                                                        /* Ajout dans la liste de travail */
       Partage->com_modbus.Modules_MODBUS = g_list_append ( Partage->com_modbus.Modules_MODBUS, module );
       cpt++;                                              /* Nous avons ajouté un module dans la liste ! */
       Info_n( Config.log, DEBUG_MODBUS, "Charger_modules_MODBUS:  id       = ", module->id       );
       Info_n( Config.log, DEBUG_MODBUS, "                      -  actif    = ", module->actif    );
       Info_c( Config.log, DEBUG_MODBUS, "                      -  ip       = ", module->ip       );
       Info_n( Config.log, DEBUG_MODBUS, "                      -  bit      = ", module->bit      );
       Info_n( Config.log, DEBUG_MODBUS, "                      -  watchdog = ", module->watchdog );
     }
    Liberer_resultat_SQL ( Config.log, db );
    Info_n( Config.log, DEBUG_INFO, "MODBUS: Run_modbus: module MODBUS found  !", cpt );

    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Charger_une_borne_MODBUS ( gint id )
  { struct MODULE_MODBUS *module;
    struct BORNE_MODBUS *borne;
    gchar requete[128];
    struct DB *db;
    
    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db) return;

    g_snprintf( requete, sizeof(requete), "SELECT id,type,adresse,min,nbr,module FROM %s WHERE id=%d",
                NOM_TABLE_BORNE_MODBUS, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }

    if ( Recuperer_ligne_SQL (Config.log, db) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }

    borne = (struct BORNE_MODBUS *)g_malloc0( sizeof(struct BORNE_MODBUS) );
    if (!borne)                                                   /* Si probleme d'allocation mémoire */
     { Info( Config.log, DEBUG_MEM,
             "Charger_modules_MODBUS: Erreur allocation mémoire struct BORNE_MODBUS" );
       Libere_DB_SQL( Config.log, &db );
       return;
     }

    borne->id       = atoi (db->row[0]);
    borne->type     = atoi (db->row[1]);
    borne->adresse  = atoi (db->row[2]);
    borne->min      = atoi (db->row[3]);
    borne->nbr      = atoi (db->row[4]);

    module = Chercher_module_by_id ( atoi(db->row[5]) );
    if (module)
     { module->Bornes = g_list_append ( module->Bornes, borne ); }      /* Ajout dans la liste de travail */
    else g_free(borne);                                                   /* Sinon, on oublie, tant pis ! */
    Liberer_resultat_SQL ( Config.log, db );
    Libere_DB_SQL( Config.log, &db );
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Charger_un_MODBUS ( gint id )
  { struct MODULE_MODBUS *module;
    
    module = (struct MODULE_MODBUS *)g_malloc0( sizeof(struct MODULE_MODBUS) );
    if (!module)                                                   /* Si probleme d'allocation mémoire */
     { Info( Config.log, DEBUG_MEM,
            "Charger_un_MODBUS: Erreur allocation mémoire struct MODULE_MODBUS" );
       return;
     }
    Charger_un_MODBUS_DB ( module, id );
    Partage->com_modbus.Modules_MODBUS = g_list_append ( Partage->com_modbus.Modules_MODBUS, module );
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_un_MODBUS ( struct MODULE_MODBUS *module )
  { if (!module) return;
    if (module->Bornes)
     { g_list_foreach( module->Bornes, (GFunc) g_free, NULL );
       g_list_free( module->Bornes );
     }
    Partage->com_modbus.Modules_MODBUS = g_list_remove ( Partage->com_modbus.Modules_MODBUS, module );
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_tous_MODBUS ( void  )
  { struct MODULE_MODBUS *module;
    while ( Partage->com_modbus.Modules_MODBUS )
     { module = (struct MODULE_MODBUS *)Partage->com_modbus.Modules_MODBUS->data;
       Decharger_un_MODBUS ( module );
     }
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
    module->date_retente = time(NULL) + MODBUS_RETRY;
    Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Deconnecter_module", module->id );
    SB( module->bit, 0 );                                     /* Mise a zero du bit interne lié au module */
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

    if ( !(host = gethostbyname( module->ip )) )                                  /* On veut l'adresse IP */
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
               module->ip );
       close(connexion);
       return(FALSE);
     }

    fcntl( connexion, F_SETFL, SO_KEEPALIVE | SO_REUSEADDR );
    module->connexion = connexion;                                          /* Sauvegarde du fd */
    module->date_last_reponse = time(NULL);
    Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Connecter_module", module->id );
    SB( module->bit, 1 );                                        /* Mise a 1 du bit interne lié au module */

    return(TRUE);
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Init_watchdog_modbus( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE_STOR requete;                            /* Definition d'une trame MODBUS */

    memset (&requete, 0, sizeof(struct TRAME_MODBUS_REQUETE_STOR) );

    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( 0x100A );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;
    requete.valeur         = htons( 0x0000 );                          /* 5 secondes avant coupure sortie */

    if ( write ( module->connexion, &requete, sizeof(requete) )                    /* Envoi de la requete */
         != sizeof (requete) )
     { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Init_watchdog_modbus: stop watchdog failed", module->id );
       Deconnecter_module( module );
     }

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( 0x1009 );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;
    requete.valeur         = htons( 0x0001 );                          /* 5 secondes avant coupure sortie */

    if ( write ( module->connexion, &requete, sizeof(requete) )                    /* Envoi de la requete */
         != sizeof (requete) )
     { Info_n( Config.log, DEBUG_MODBUS,
               "MODBUS: Init_watchdog_modbus: close modbus tcp on watchdog failed", module->id );
       Deconnecter_module( module );
     }

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( 0x1000 );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;
    requete.valeur         = htons( module->watchdog );                                 /* coupure sortie */

    if ( write ( module->connexion, &requete, sizeof(requete) )                    /* Envoi de la requete */
         != sizeof (requete) )
     { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Init_watchdog_modbus: init watchdog timer failed", module->id );
       Deconnecter_module( module );
     }

    module->transaction_id++;
    requete.transaction_id = htons(module->transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( 0x100A );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;
    requete.valeur         = htons( 0x0001 );                                              /* Start Timer */

    if ( write ( module->connexion, &requete, sizeof(requete) )                    /* Envoi de la requete */
         != sizeof (requete) )
     { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Init_watchdog_modbus: watchdog start failed", module->id );
       Deconnecter_module( module );
     }

    module->transaction_id++;
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_borne_input_tor( struct MODULE_MODBUS *module )
  { struct TRAME_MODBUS_REQUETE_ETOR requete;                            /* Definition d'une trame MODBUS */
    struct BORNE_MODBUS *borne;
    borne = (struct BORNE_MODBUS *)module->borne_en_cours->data;
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
    struct BORNE_MODBUS *borne;
    borne = (struct BORNE_MODBUS *)module->borne_en_cours->data;
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
    struct BORNE_MODBUS *borne;
    gint cpt_a, valeur;
    borne = (struct BORNE_MODBUS *)module->borne_en_cours->data;

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
  { struct BORNE_MODBUS *borne;
    borne = (struct BORNE_MODBUS *)module->borne_en_cours->data;
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
  { struct BORNE_MODBUS *borne;
    borne = (struct BORNE_MODBUS *)module->borne_en_cours->data;

    if (module->transaction_id != ntohs(module->response.transaction_id))
     { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Processer_trame: wrong transaction_id  attendu",
               module->transaction_id );
       Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Processer_trame: wrong transaction_id  reponse",
               module->response.transaction_id );
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
       module->date_last_reponse = time(NULL);                          /* Estampillage de la date */
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
                               SEA( cpt_e++, reponse, 1 );
                             }
                            else SEA( cpt_e++, 0, 0 );
                            if ( ! (module->response.data[3] & 0x03) )
                             { reponse = module->response.data[2] << 5;
                               reponse |= module->response.data[3] >> 3;
                               SEA( cpt_e++, reponse, 1 );
                             }
                            else SEA( cpt_e++, 0, 0 );
                            if ( ! (module->response.data[5] & 0x03) )
                             { reponse = module->response.data[4] << 5;
                               reponse |= module->response.data[5] >> 3;
                               SEA( cpt_e++, reponse, 1 );
                             }
                            else SEA( cpt_e++, 0, 0 );
                            if ( ! (module->response.data[7] & 0x03) )
                             { reponse = module->response.data[6] << 5;
                               reponse |= module->response.data[7] >> 3;
                               SEA( cpt_e++, reponse, 1 );
                             }
                            else SEA( cpt_e++, 0, 0 );

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

    module->request = FALSE;                                          /* Une requete a été traitée */
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

    if (module->date_last_reponse + 10 < time(NULL))              /* Detection attente trop longue */
     { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Recuperer_borne: Pb reponse module, deconnexion",
               module->id );
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
             Processer_trame( module );                         /* Si l'on a trouvé une trame complète !! */
             module->nbr_oct_lu = 0;
           }
         } 
        else
         { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Recuperer_borne: wrong trame", module->id );
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

    if ( Charger_tous_MODBUS() == FALSE )                                    /* Chargement des modules modbus */
     { Info( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: No module MODBUS found -> stop" );
       pthread_exit(GINT_TO_POINTER(-1));
     }

    liste = Partage->com_modbus.Modules_MODBUS;
    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { time_t date;                                           /* On veut parler au prochain module MODBUS */
       sched_yield();

       if (Partage->com_modbus.reload == TRUE)
        { Info( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Reloading conf" );
          Decharger_tous_MODBUS();
          Charger_tous_MODBUS();
          liste = Partage->com_modbus.Modules_MODBUS;
          Partage->com_modbus.reload = FALSE;
        }

       if (Partage->com_modbus.admin_del)
        { Info( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Deleting module" );
          module = Chercher_module_by_id ( Partage->com_modbus.admin_del );
          Deconnecter_module  ( module );
          Decharger_un_MODBUS ( module );
          Partage->com_modbus.admin_del = 0;
        }

       if (Partage->com_modbus.admin_add)
        { Info( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Adding module" );
          Charger_un_MODBUS ( Partage->com_modbus.admin_add );
          Partage->com_modbus.admin_add = 0;
        }

       if (Partage->com_modbus.admin_add_borne)
        { Info( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Adding module une borne" );
          Charger_une_borne_MODBUS ( Partage->com_modbus.admin_add_borne );
          Partage->com_modbus.admin_add_borne = 0;
        }

       if (Partage->com_modbus.admin_start)
        { Info( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Starting module" );
          module = Chercher_module_by_id ( Partage->com_modbus.admin_start );
          module->actif = 1;
          Partage->com_modbus.admin_start = 0;
        }

       if (Partage->com_modbus.admin_stop)
        { Info( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Stoping module" );
          module = Chercher_module_by_id ( Partage->com_modbus.admin_stop );
          Deconnecter_module  ( module );
          module->actif = 0;
          Partage->com_modbus.admin_stop = 0;
        }

       if (liste == NULL)                                 /* L'admin peut deleter les modules un par un ! */
        { sleep(1); continue; }                        /* Si pas de module, on ne sollicite pas le proc ! */

       liste = liste->next;
       if (!liste)                                       /* On vient de faire un tour de tous les modules */
        { liste = Partage->com_modbus.Modules_MODBUS; }

       module = (struct MODULE_MODBUS *)liste->data;
       if (module->actif != TRUE) { continue; }

/*********************************** Début de l'interrogation du module ***********************************/
       date = time(NULL);                                                 /* On recupere l'heure actuelle */
       if ( date < module->date_retente )                      /* Si attente retente, on change de module */
        { continue;
        }

       if ( ! module->started )                                              /* Communication OK ou non ? */
        { if ( Connecter_module( module ) )
           { if ( module->watchdog )
              { Init_watchdog_modbus(module); }
             module->started = TRUE;
           }
          else
           { Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Module DOWN", module->id );
             module->date_retente = date + MODBUS_RETRY;
             continue;
           }
        }

       if ( module->request )                                        /* Requete en cours pour ce module ? */
        { Recuperer_borne ( module );
          continue;
        }

                                                       /* Si pas de requete, on passe a la borne suivante */
       module->borne_en_cours = module->borne_en_cours->next;
       if ( ! module->borne_en_cours)                                                /* Tour des bornes ? */
        { module->borne_en_cours = module->Bornes; }

/***************************** Début de l'interrogation de la borne du module ******************************/
       Interroger_borne ( module );
     }

    Decharger_tous_MODBUS();
    Info_n( Config.log, DEBUG_MODBUS, "MODBUS: Run_modbus: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
