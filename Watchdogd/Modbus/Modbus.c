/**********************************************************************************************************/
/* Watchdogd/Modbus/Modbus.c  Gestion des modules MODBUS Watchdgo 2.0                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mer 15 avr 2009 14:42:38 CEST */
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

 static struct COMM_MODBUS                                /* Etat de la connexion avec les modules MODBUS */
  { gboolean started;
    gint connexion;                                                                 /* FD de connexion IP */
    gint nbr_oct_lu;
    guint16 transaction_id;
    guint borne_en_cours;
    guint nbr_deconnect;
    time_t date_retente;
    time_t date_last_reponse;
    gboolean request;
    struct TRAME_MODBUS_REPONSE response;
  } Comm_MODBUS [ NBR_ID_MODBUS ];

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */

/**********************************************************************************************************/
/* Charger_MODBUS: Requete la DB pour charger les modules et les bornes modbus                            */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gint Charger_MODBUS ( void  )
  { gchar requete[128];
    struct DB *db;
    gint cpt;
    
    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Info_c( Config.log, DEBUG_MODBUS, "Charger_modules_MODBUS: impossible d'ouvrir la Base de données",
               Config.db_database );
       return(0);
     }

    g_snprintf( requete, sizeof(requete), "SELECT id,ip,bit,watchdog FROM %s WHERE actif=1",
                NOM_TABLE_MODULE_MODBUS
              );

    if ( mysql_query ( db->mysql, requete ) )
     { Info_c( Config.log, DEBUG_MODBUS, "Charger_modules_MODBUS: requete failed",
               (char *)mysql_error(db->mysql) );
       Libere_DB_SQL( Config.log, &db );
       return(0);
     }

    db->result = mysql_use_result ( db->mysql );
    if ( ! db->result )
     { Info_c( Config.log, DEBUG_MODBUS, "Charger_modules_MODBUS: use_result failed",
               (char *) mysql_error(db->mysql) );
       Libere_DB_SQL( Config.log, &db );
       return(0);
     }

/********************************************** Chargement des modules ************************************/
    pthread_mutex_lock( &Partage->com_modbus.synchro );
    Partage->com_modbus.Modules_MODBUS = NULL;
    cpt = 0;
    while ((db->row = mysql_fetch_row(db->result)))
     { struct MODULE_MODBUS *module;

       module = (struct MODULE_MODBUS *)g_malloc0( sizeof(struct MODULE_MODBUS) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info( Config.log, DEBUG_MODBUS,
                "Charger_modules_MODBUS: Erreur allocation mémoire struct MODULE_MODBUS" );
          continue;
        }

       module->id       = atoi (db->row[0]);
       g_snprintf( module->ip, sizeof(module->ip), "%s", db->row[1] );
       module->bit      = atoi (db->row[2]);         /* Bit interne B d'etat communication avec le module */
       module->watchdog = atoi (db->row[3]);
       Partage->com_modbus.Modules_MODBUS = g_list_append ( Partage->com_modbus.Modules_MODBUS, module );
                                                                        /* Ajout dans la liste de travail */
       cpt++;                                              /* Nous avons ajouté un module dans la liste ! */
       Info_n( Config.log, DEBUG_MODBUS, "Charger_modules_MODBUS:  id       = ", module->id       );
       Info_c( Config.log, DEBUG_MODBUS, "                      -  ip       = ", module->ip       );
       Info_n( Config.log, DEBUG_MODBUS, "                      -  bit      = ", module->bit      );
       Info_n( Config.log, DEBUG_MODBUS, "                      -  watchdog = ", module->watchdog );
     }
    pthread_mutex_unlock( &Partage->com_modbus.synchro );
    mysql_free_result( db->result );
/******************************************* Chargement des bornes ****************************************/

    Libere_DB_SQL( Config.log, &db );
    return(cpt);
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_MODBUS ( void  )
  { pthread_mutex_lock( &Partage->com_modbus.synchro );
    if (Partage->com_modbus.Modules_MODBUS)
     { g_list_foreach( Partage->com_modbus.Modules_MODBUS, (GFunc) g_free, NULL );
       g_list_free( Partage->com_modbus.Modules_MODBUS );
       Partage->com_modbus.Modules_MODBUS = NULL;
     }
    if (Partage->com_modbus.Bornes_MODBUS)
     { g_list_foreach( Partage->com_modbus.Bornes_MODBUS, (GFunc) g_free, NULL );
       g_list_free( Partage->com_modbus.Bornes_MODBUS );
       Partage->com_modbus.Bornes_MODBUS = NULL;
     }
    pthread_mutex_unlock( &Partage->com_modbus.synchro );
  }

#ifdef bouh
/**********************************************************************************************************/
/* Deconnecter: Deconnexion du module                                                                     */
/* Entrée: un id                                                                                          */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Deconnecter_module ( guint id )
  { if (Comm_MODBUS[id].started == FALSE) return;

    close ( Comm_MODBUS[id].connexion );
    Comm_MODBUS[id].connexion = 0;
    Comm_MODBUS[id].started = FALSE;
    Comm_MODBUS[id].request = FALSE;
    Comm_MODBUS[id].nbr_deconnect++;
    Comm_MODBUS[id].date_retente = time(NULL) + MODBUS_RETRY;
    Info_n( Config.log, DEBUG_INFO, "MODBUS: Deconnecter_module", id );
    SB( Config.module_modbus[id].bit, 0 );                    /* Mise a zero du bit interne lié au module */
  }
/**********************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                           */
/* Entrée: une nom et un password                                                                         */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                          */
/**********************************************************************************************************/
 static gboolean Connecter_module ( guint id )
  { struct sockaddr_in src;                                            /* Données locales: pas le serveur */
    struct hostent *host;
    int connexion;

    if ( !(host = gethostbyname( Config.module_modbus[id].ip )) )                 /* On veut l'adresse IP */
     { Info( Config.log, DEBUG_INFO, "MODBUS: Connecter_module: DNS_Failed" );
       return(FALSE);
     }

    src.sin_family = host->h_addrtype;
    memcpy( (char*)&src.sin_addr, host->h_addr, host->h_length );                 /* On recopie les infos */
    src.sin_port = htons( MODBUS_PORT_TCP );                                /* Port d'attaque des modules */

    if ( (connexion = socket( AF_INET, SOCK_STREAM, 0)) == -1)                          /* Protocol = TCP */
     { Info( Config.log, DEBUG_INFO, "MODBUS: Connecter_module: Socket creation failed" );
       return(FALSE);
     }

    if (connect (connexion, (struct sockaddr *)&src, sizeof(src)) == -1)
     { Info_c( Config.log, DEBUG_INFO, "MODBUS: Connecter_module: connexion refused by module",
               Config.module_modbus[id].ip );
       close(connexion);
       return(FALSE);
     }

    fcntl( connexion, F_SETFL, SO_KEEPALIVE | SO_REUSEADDR );
    Comm_MODBUS[ id ].connexion = connexion;                                          /* Sauvegarde du fd */
    Comm_MODBUS[ id ].date_last_reponse = time(NULL);
    Info_n( Config.log, DEBUG_INFO, "MODBUS: Connecter_module", id );
    SB( Config.module_modbus[id].bit, 1 );                       /* Mise a 1 du bit interne lié au module */

    return(TRUE);
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Init_watchdog_modbus( guint id_module )
  { struct TRAME_MODBUS_REQUETE_STOR requete;                            /* Definition d'une trame MODBUS */

    memset (&requete, 0, sizeof(struct TRAME_MODBUS_REQUETE_STOR) );

    requete.transaction_id = htons(Comm_MODBUS[id_module].transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( 0x100A );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;
    requete.valeur         = htons( 0x0000 );                          /* 5 secondes avant coupure sortie */

    if ( write ( Comm_MODBUS[id_module].connexion,                         /* Envoi de la requete */
                 &requete,
                 sizeof (struct TRAME_MODBUS_REQUETE_STOR) )
         != sizeof (struct TRAME_MODBUS_REQUETE_STOR) )
     { Info_n( Config.log, DEBUG_INFO, "MODBUS: Init_watchdog_modbus: stop watchdog failed", id_module );
       Deconnecter_module( id_module );
     }

    Comm_MODBUS[id_module].transaction_id++;
    requete.transaction_id = htons(Comm_MODBUS[id_module].transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( 0x1009 );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;
    requete.valeur         = htons( 0x0001 );                          /* 5 secondes avant coupure sortie */

    if ( write ( Comm_MODBUS[id_module].connexion,                         /* Envoi de la requete */
                 &requete,
                 sizeof (struct TRAME_MODBUS_REQUETE_STOR) )
         != sizeof (struct TRAME_MODBUS_REQUETE_STOR) )
     { Info_n( Config.log, DEBUG_INFO,
               "MODBUS: Init_watchdog_modbus: close modbus tcp on watchdog failed", id_module );
       Deconnecter_module( id_module );
     }

    Comm_MODBUS[id_module].transaction_id++;
    requete.transaction_id = htons(Comm_MODBUS[id_module].transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( 0x1000 );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;
    requete.valeur         = htons( Config.module_modbus[id_module].watchdog );         /* coupure sortie */

    if ( write ( Comm_MODBUS[id_module].connexion,                                 /* Envoi de la requete */
                 &requete,
                 sizeof (struct TRAME_MODBUS_REQUETE_STOR) )
         != sizeof (struct TRAME_MODBUS_REQUETE_STOR) )
     { Info_n( Config.log, DEBUG_INFO, "MODBUS: Init_watchdog_modbus: init watchdog timer failed", id_module );
       Deconnecter_module( id_module );
     }

    Comm_MODBUS[id_module].transaction_id++;
    requete.transaction_id = htons(Comm_MODBUS[id_module].transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( 0x100A );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;
    requete.valeur         = htons( 0x0001 );                                              /* Start Timer */

    if ( write ( Comm_MODBUS[id_module].connexion,                                 /* Envoi de la requete */
                 &requete,
                 sizeof (struct TRAME_MODBUS_REQUETE_STOR) )
         != sizeof (struct TRAME_MODBUS_REQUETE_STOR) )
     { Info_n( Config.log, DEBUG_INFO, "MODBUS: Init_watchdog_modbus: watchdog start failed", id_module );
       Deconnecter_module( id_module );
     }

    Comm_MODBUS[id_module].transaction_id++;
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_borne_input_tor( guint id_module, guint id_borne )
  { struct TRAME_MODBUS_REQUETE_ETOR requete;                            /* Definition d'une trame MODBUS */
    struct MODULE_MODBUS_BORNE *borne;

    memset (&requete, 0, sizeof(struct TRAME_MODBUS_REQUETE_ETOR) );
    borne = &Config.module_modbus[id_module].borne[id_borne];

    requete.transaction_id = htons(Comm_MODBUS[id_module].transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( borne->adresse );
    requete.nbr            = htons( borne->nbr );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_ENTRE_TOR;

/*{ int cpt;
             printf("Trame envoyée:\n");
             for (cpt=0; cpt<sizeof(requete); cpt++)
 		    { printf("%02X ", (unsigned char)*((unsigned char *)&requete + cpt) );
 		    }
             printf("\n");
}*/
    if ( write ( Comm_MODBUS[id_module].connexion,                                 /* Envoi de la requete */
                 &requete,
                 sizeof(requete) )
                 != sizeof(requete) )
     { Deconnecter_module( id_module ); }
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_borne_input_ana( guint id_module, guint id_borne )
  { struct TRAME_MODBUS_REQUETE_EANA requete;                            /* Definition d'une trame MODBUS */
    struct MODULE_MODBUS_BORNE *borne;

    memset (&requete, 0, sizeof(struct TRAME_MODBUS_REQUETE_EANA) );
    borne = &Config.module_modbus[id_module].borne[id_borne];

    requete.transaction_id = htons(Comm_MODBUS[id_module].transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( borne->adresse );
    requete.nbr            = htons( borne->nbr );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_ENTRE_ANA;

/*{ int cpt;
             printf("Trame envoyée:\n");
             for (cpt=0; cpt<sizeof(requete); cpt++)
 		    { printf("%02X ", (unsigned char)*((unsigned char *)&requete + cpt) );
 		    }
             printf("\n");
}*/
    if ( write ( Comm_MODBUS[id_module].connexion,                                 /* Envoi de la requete */
                 &requete,
                 sizeof(requete) )
                 != sizeof(requete) )
     { Deconnecter_module( id_module ); }
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_borne_output_tor( guint id_module, guint id_borne )
  { struct TRAME_MODBUS_REQUETE_STOR requete;                            /* Definition d'une trame MODBUS */
    struct MODULE_MODBUS_BORNE *borne;
    gint cpt_a, valeur;

    memset (&requete, 0, sizeof(struct TRAME_MODBUS_REQUETE_STOR) );
    borne = &Config.module_modbus[id_module].borne[id_borne];

    requete.transaction_id = htons(Comm_MODBUS[id_module].transaction_id);
    requete.proto_id       = 0x00;                                                        /* -> 0 = MOBUS */
    requete.unit_id        = 0x00;                                                                /* 0xFF */
    requete.adresse        = htons( borne->adresse );
    requete.taille         = htons( 0x0006 );                           /* taille, en comptant le unit_id */
    requete.fct            = MBUS_SORTIE_TOR;

    switch ( borne->nbr )
     { case 8:                                                           /* Bornes a 8 sorties !! */
               requete.taille         = htons( 0x0006 );        /* taille, en comptant le unit_id */
               cpt_a = Config.module_modbus[id_module].borne[id_borne].min;
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
       default: Info_n( Config.log, DEBUG_INFO,
                        "MODBUS: Interroger_borne_output_tor: borne InputTOR non gérée", borne->nbr
                      );
     }

/*{ int cpt;
             printf("Trame envoyée: %d\n", requete.valeur);
             for (cpt=0; cpt<sizeof(requete); cpt++)
 		    { printf("%02X ", (unsigned char)*((unsigned char *)&requete + cpt) );
 		    }
             printf("\n");
}*/
    if ( write ( Comm_MODBUS[id_module].connexion,                         /* Envoi de la requete */
                 &requete,
                 sizeof (struct TRAME_MODBUS_REQUETE_STOR) )
         != sizeof (struct TRAME_MODBUS_REQUETE_STOR) )
     { Deconnecter_module( id_module ); }
  }
/**********************************************************************************************************/
/* Interroger_borne: Interrogation d'une borne du module                                                  */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_borne( guint id_module, guint id_borne )
  { struct MODULE_MODBUS_BORNE *borne;
    borne = &Config.module_modbus[id_module].borne[id_borne];

    switch (borne->type)
     { case BORNE_INPUT_TOR:  Interroger_borne_input_tor( id_module, id_borne );
            break;

       case BORNE_INPUT_ANA:  Interroger_borne_input_ana( id_module, id_borne );
            break;

       case BORNE_OUTPUT_TOR: Interroger_borne_output_tor( id_module, id_borne );   /* Borne de sortie ?? */
            break;

       default: Info(Config.log, DEBUG_INFO, "MODBUS: Interroger_borne: type de borne non reconnu" );
     }

    Comm_MODBUS[id_module].nbr_oct_lu = 0;
    Comm_MODBUS[id_module].request = TRUE;                                   /* Une requete a été envoyée */
  }
/**********************************************************************************************************/
/* Recuperer_borne: Recupere les informations d'une borne MODBUS                                          */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Processer_trame( guint id_module, guint id_borne )
  { if (Comm_MODBUS[id_module].transaction_id != ntohs(Comm_MODBUS[id_module].response.transaction_id))
     { Info_n( Config.log, DEBUG_INFO, "MODBUS: Processer_trame: wrong transaction_id  attendu",
               Comm_MODBUS[id_module].transaction_id );
       Info_n( Config.log, DEBUG_INFO, "MODBUS: Processer_trame: wrong transaction_id  reponse",
               ntohs(Comm_MODBUS[id_module].response.transaction_id) );
                                            /* On laisse tomber la trame recue, et on attends la suivante */
       memset (&Comm_MODBUS[id_module].response, 0, sizeof(struct TRAME_MODBUS_REPONSE) );
       return;
     }

    if ( (guint16) Comm_MODBUS[id_module].response.proto_id )
     { Info( Config.log, DEBUG_INFO, "MODBUS: Processer_trame: wrong proto_id" );
       Deconnecter_module( id_module );
     }

    else
     { int nbr, cpt_e;
       Comm_MODBUS[id_module].date_last_reponse = time(NULL);                  /* Estampillage de la date */
       nbr = Comm_MODBUS[id_module].response.nbr;
       switch ( Comm_MODBUS[id_module].response.fct )
        { case MBUS_ENTRE_TOR:                                       /* Quelles type de borne d'entrées ? */
               switch ( Config.module_modbus[id_module].borne[id_borne].nbr )
                { case 8:                                                        /* Bornes a 8 entrées !! */
                          if (nbr != 1) break;         /* Si nous n'avons pas recu le bon nombre d'octets */
                          cpt_e = Config.module_modbus[id_module].borne[id_borne].min;
                          SE( cpt_e++, ( Comm_MODBUS[id_module].response.data[0] & 1  ) );
                          SE( cpt_e++, ( Comm_MODBUS[id_module].response.data[0] & 4  ) );
                          SE( cpt_e++, ( Comm_MODBUS[id_module].response.data[0] & 16 ) );
                          SE( cpt_e++, ( Comm_MODBUS[id_module].response.data[0] & 64 ) );
                          SE( cpt_e++, ( Comm_MODBUS[id_module].response.data[0] & 2  ) );
                          SE( cpt_e++, ( Comm_MODBUS[id_module].response.data[0] & 8  ) );
                          SE( cpt_e++, ( Comm_MODBUS[id_module].response.data[0] & 32 ) );
                          SE( cpt_e++, ( Comm_MODBUS[id_module].response.data[0] & 128) );
/*{ int cpt;
             printf("Entrée:\n");
             for (cpt=Config.module_modbus[id_module].borne[id_borne].min; cpt<Config.module_modbus[id_module].borne[id_borne].min+8; cpt++)
 		    { printf("E%d = %d ", cpt, E(cpt) );
 		    }
             printf("\n");
}*/

                          break;
                  default: Info_n( Config.log, DEBUG_INFO,
                                   "MODBUS: Processer_trame: borne InputTOR non gérée",
                                   Config.module_modbus[id_module].borne[id_borne].nbr
                                 );
                }
               break;
          case MBUS_SORTIE_TOR:                                      /* Quelles type de borne de sortie ? */
               break;
          case MBUS_ENTRE_ANA:                                       /* Quelles type de borne d'entrées ? */
               switch ( Config.module_modbus[id_module].borne[id_borne].nbr )
                { case 4: { guint reponse;                                       /* Bornes a 4 entrées !! */
                            if (nbr != 8) break;       /* Si nous n'avons pas recu le bon nombre d'octets */

                            cpt_e = Config.module_modbus[id_module].borne[id_borne].min;
                            if ( ! (Comm_MODBUS[id_module].response.data[1] & 0x03) )
                             { reponse = Comm_MODBUS[id_module].response.data[0] << 5;
                               reponse |= Comm_MODBUS[id_module].response.data[1] >> 3;
                               SEA( cpt_e++, reponse, 1 );
                             }
                            else SEA( cpt_e++, 0, 0 );
                            if ( ! (Comm_MODBUS[id_module].response.data[3] & 0x03) )
                             { reponse = Comm_MODBUS[id_module].response.data[2] << 5;
                               reponse |= Comm_MODBUS[id_module].response.data[3] >> 3;
                               SEA( cpt_e++, reponse, 1 );
                             }
                            else SEA( cpt_e++, 0, 0 );
                            if ( ! (Comm_MODBUS[id_module].response.data[5] & 0x03) )
                             { reponse = Comm_MODBUS[id_module].response.data[4] << 5;
                               reponse |= Comm_MODBUS[id_module].response.data[5] >> 3;
                               SEA( cpt_e++, reponse, 1 );
                             }
                            else SEA( cpt_e++, 0, 0 );
                            if ( ! (Comm_MODBUS[id_module].response.data[7] & 0x03) )
                             { reponse = Comm_MODBUS[id_module].response.data[6] << 5;
                               reponse |= Comm_MODBUS[id_module].response.data[7] >> 3;
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
                  default: Info_n( Config.log, DEBUG_INFO,
                                   "MODBUS: Processer_trame: borne InputANA non gérée", 
                                   Config.module_modbus[id_module].borne[id_borne].nbr
                                 );
                }
               break;


          case 0x80 + MBUS_ENTRE_TOR:
               Info( Config.log, DEBUG_INFO, "MODBUS: Processer_trame: Erreur ENTRE_TOR" );
               break;
          case 0x80 + MBUS_SORTIE_TOR:
               Info( Config.log, DEBUG_INFO, "MODBUS: Processer_trame: Erreur SORTIE_TOR" );
               break;
          case 0x80 + MBUS_ENTRE_ANA:
               Info( Config.log, DEBUG_INFO, "MODBUS: Processer_trame: Erreur ENTRE_ANA" );
               break;
          default: Info( Config.log, DEBUG_INFO, "MODBUS: Processer_trame: fct inconnu" );
        }
     }

    Comm_MODBUS[id_module].request = FALSE;                                  /* Une requete a été traitée */
    Comm_MODBUS[id_module].transaction_id++;
    memset (&Comm_MODBUS[id_module].response, 0, sizeof(struct TRAME_MODBUS_REPONSE) );
  }
/**********************************************************************************************************/
/* Recuperer_borne: Recupere les informations d'une borne MODBUS                                          */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Recuperer_borne( guint id_module, guint id_borne )
  { fd_set fdselect;
    struct timeval tv;
    gint retval, cpt;

    if (Comm_MODBUS[id_module].date_last_reponse + 10 < time(NULL))      /* Detection attente trop longue */
     { Info_n( Config.log, DEBUG_INFO, "MODBUS: Recuperer_borne: Pb reponse module, deconnexion", id_module );
       Deconnecter_module( id_module );
       return;
     }

    FD_ZERO(&fdselect);
    FD_SET(Comm_MODBUS[id_module].connexion, &fdselect );
    tv.tv_sec = 0;
    tv.tv_usec= 100;                                                           /* Attente d'un caractere */
    retval = select(Comm_MODBUS[id_module].connexion+1, &fdselect, NULL, NULL, &tv );

    if ( retval>=0 && FD_ISSET(Comm_MODBUS[id_module].connexion, &fdselect) )
     { int bute;
       if (Comm_MODBUS[id_module].nbr_oct_lu<TAILLE_ENTETE_MODBUS)
            { bute = TAILLE_ENTETE_MODBUS; }
       else { bute = TAILLE_ENTETE_MODBUS + ntohs(Comm_MODBUS[id_module].response.taille); }

       cpt = read( Comm_MODBUS[id_module].connexion,
                   (unsigned char *)&Comm_MODBUS[id_module].response +
                                     Comm_MODBUS[id_module].nbr_oct_lu,
                    bute-Comm_MODBUS[id_module].nbr_oct_lu );
       if (cpt>0)
        { Comm_MODBUS[id_module].nbr_oct_lu += cpt;
          if (Comm_MODBUS[id_module].nbr_oct_lu >= 
              TAILLE_ENTETE_MODBUS + ntohs(Comm_MODBUS[id_module].response.taille))
           { 
             Processer_trame( id_module, id_borne );
             Comm_MODBUS[id_module].nbr_oct_lu = 0;
           }
         } 
        else
         { Info_n( Config.log, DEBUG_FORK, "MODBUS: Recuperer_borne: wrong trame", id_module );
           Deconnecter_module ( id_module );
         }
      }

  }
/**********************************************************************************************************/
/* Nbr_borne: Renvoie le nombre total de borne definit dans la config                                     */
/* Entrée: Néant                                                                                          */
/**********************************************************************************************************/
 static gint Nbr_borne ( void )
  { gint i, j, cpt_borne;
    cpt_borne = 0;
    for (i=0; i<NBR_ID_MODBUS; i++)
     { if (Config.module_modbus[i].actif)
        { for (j=0; j<NBR_ID_MODBUS_BORNE; j++)
           { if (Config.module_modbus[i].borne[j].actif) cpt_borne++;
           }
        }
     }
    return(cpt_borne);
  }
#endif
/**********************************************************************************************************/
/* Modbus_state : Renvoie l'etat du modbus num id en tant que chaine de caractere                         */
/* Entrée : Le numéro du module et la chaine a remplir                                                    */
/**********************************************************************************************************/
 void Modbus_state ( int id, gchar *chaine, int size )
  { g_snprintf( chaine, size,
                " MODBUS[%02d] - Running = %d, transaction_id = %d, request = %d, nbr_deconnect = %d\n", id,
                Comm_MODBUS[id].started, Comm_MODBUS[id].transaction_id,
                Comm_MODBUS[id].request, Comm_MODBUS[id].nbr_deconnect );
  } 
/**********************************************************************************************************/
/* Main: Fonction principale du MODBUS                                                                    */
/**********************************************************************************************************/
 void Run_modbus ( void )
  { guint id_en_cours;
    guint cpt;

    prctl(PR_SET_NAME, "W-MODBUS", 0, 0, 0 );
    Info( Config.log, DEBUG_FORK, "MODBUS: demarrage" );

    if ( (cpt = Charger_MODBUS()) == 0 )                                 /* Chargement des modules modbus */
     { Info( Config.log, DEBUG_INFO, "MODBUS: Run_modbus: No module MODBUS found -> stop" );
       pthread_exit(GINT_TO_POINTER(-1));
     }
    else Info_n( Config.log, DEBUG_INFO, "MODBUS: Run_modbus:  modules MODBUS found  !", cpt );
       

    memset( &Comm_MODBUS, 0, sizeof( Comm_MODBUS ) );

#ifdef bouh
    Db_watchdog = ConnexionDB( Config.log, Config.db_name,           /* Connexion en tant que user normal */
                               Config.db_admin_username, Config.db_password );

    if (!Db_watchdog)
     { Info_c( Config.log, DEBUG_DB, "MODBUS: Run_modbus: Unable to open database (dsn)", Config.db_name );
       close(fd_modbus);
       pthread_exit(GINT_TO_POINTER(-1));
     }


/*    nbr_oct_lu = 0; */
    id_en_cours = 0;
/*    attente_reponse = FALSE;*/
    memset (&Comm_MODBUS, 0, sizeof(Comm_MODBUS) );

    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { time_t date;                                           /* On veut parler au prochain module MODBUS */
       date = time(NULL);                                                 /* On recupere l'heure actuelle */

       if (Partage->com_modbus.sigusr1)
        { int i;
          Partage->com_modbus.sigusr1 = FALSE;
               Info( Config.log, DEBUG_INFO, "MODBUS: Run_modbus: SIGUSR1" );
          for (i=0; i<NBR_ID_MODBUS; i++)
           { Info_n( Config.log, DEBUG_INFO, "       ------ module_modbus", i );
             Info_n( Config.log, DEBUG_INFO, "                    started", Comm_MODBUS[i].started );
             Info_n( Config.log, DEBUG_INFO, "             transaction_id", Comm_MODBUS[i].transaction_id );
             Info_n( Config.log, DEBUG_INFO, "                    request", Comm_MODBUS[i].request );
             Info_n( Config.log, DEBUG_INFO, "              nbr_deconnect", Comm_MODBUS[i].nbr_deconnect );
           }
        }

       do                                                   /* Recherche du prochain module a questionner */
        { id_en_cours++;
          if (id_en_cours>=NBR_ID_MODBUS)                /* On vient de faire un tour de tous les modules */
           { id_en_cours = 0; }
        } while( ! Config.module_modbus[id_en_cours].actif );

/*********************************** Début de l'interrogation du module ***********************************/
       if ( date < Comm_MODBUS[ id_en_cours ].date_retente )   /* Si attente retente, on change de module */
        { usleep(1);
          sched_yield();
          continue;
        }

       if ( ! Comm_MODBUS[ id_en_cours ].started )                           /* Communication OK ou non ? */
        { if ( Connecter_module( id_en_cours ) )
           { if ( Config.module_modbus[id_en_cours].watchdog )
              { Init_watchdog_modbus(id_en_cours); }
             Comm_MODBUS[ id_en_cours ].started = TRUE;
           }
          else
           { Info_n( Config.log, DEBUG_INFO, "MODBUS: Run_modbus: Module DOWN", id_en_cours );
             Comm_MODBUS[ id_en_cours ].date_retente = date + MODBUS_RETRY;
             continue;
           }
        }

       if ( Comm_MODBUS[id_en_cours].request )                       /* Requete en cours pour ce module ? */
        { Recuperer_borne ( id_en_cours, Comm_MODBUS[id_en_cours].borne_en_cours );
          continue;
        }

                                                       /* Si pas de requete, on passe a la borne suivante */
       do                                                /* Recherche de la prochaine borne a questionner */
        { Comm_MODBUS[id_en_cours].borne_en_cours++;
          if (Comm_MODBUS[id_en_cours].borne_en_cours>=NBR_ID_MODBUS_BORNE)          /* Tour des bornes ? */
           { Comm_MODBUS[id_en_cours].borne_en_cours = 0; }
        } while( ! Config.module_modbus[id_en_cours].borne[Comm_MODBUS[id_en_cours].borne_en_cours].actif );


/***************************** Début de l'interrogation de la borne du module ******************************/
    /* printf("Interroger borne   %d:%d  request=%d\n", id_en_cours, Comm_MODBUS[id_en_cours].borne_en_cours, Comm_MODBUS[id_en_cours].request );*/
       Interroger_borne ( id_en_cours, Comm_MODBUS[id_en_cours].borne_en_cours );
     }
#endif
    Decharger_MODBUS();
    Info_n( Config.log, DEBUG_FORK, "MODBUS: Run_modbus: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
