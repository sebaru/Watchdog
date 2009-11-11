/**********************************************************************************************************/
/* Watchdogd/Onduleur/Onduleur.c  Gestion des modules ONDULEUR Watchdgo 2.0                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 10 nov. 2009 15:56:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Onduleur.c
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
 #include <sys/prctl.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <upsclient.h>

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */

/**********************************************************************************************************/
/* Charger_tous_ONDULEUR: Requete la DB pour charger les modules onduleur                                 */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static struct MODULE_ONDULEUR *Chercher_module_by_id ( gint id )
  { GList *liste;
    liste = Partage->com_onduleur.Modules_ONDULEUR;
    while ( liste )
     { struct MODULE_ONDULEUR *module;
       module = ((struct MODULE_ONDULEUR *)liste->data);
       if (module->id == id) return(module);
       liste = liste->next;
     }
    return(NULL);
  }
/**********************************************************************************************************/
/* Charger_tous_ONDULEUR: Requete la DB pour charger les modules onduleur                                 */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_un_ONDULEUR_DB ( struct MODULE_ONDULEUR *module, gint id  )
  { gchar requete[128];
    struct DB *db;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db) return(FALSE);

/********************************************** Chargement des modules ************************************/
    g_snprintf( requete, sizeof(requete), "SELECT host,ups,bit_comm,actif,"
                                          "ea_ups_load,ea_ups_real_power,ea_battery_charge,ea_input_voltage"
                                          " FROM %s WHERE id=%d",
                NOM_TABLE_MODULE_ONDULEUR, id
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    while ( Recuperer_ligne_SQL (Config.log, db) )
     { g_snprintf( module->host, sizeof(module->host), "%s", db->row[0] );
       g_snprintf( module->ups,  sizeof(module->ups),  "%s", db->row[1] );
       module->id                = id;
       module->bit_comm          = atoi(db->row[2] );
       module->actif             = atoi(db->row[3] );
       module->ea_ups_load       = atoi(db->row[4] );
       module->ea_ups_real_power = atoi(db->row[5] );
       module->ea_battery_charge = atoi(db->row[6] );
       module->ea_input_voltage  = atoi(db->row[7] );
                                                                        /* Ajout dans la liste de travail */
       Info_n( Config.log, DEBUG_ONDULEUR, "Charger_modules_ONDULEUR:  id    = ", module->id   );
       Info_c( Config.log, DEBUG_ONDULEUR, "                        -  host  = ", module->host );
     }
    Liberer_resultat_SQL ( Config.log, db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Charger_tous_ONDULEUR: Requete la DB pour charger les modules onduleur                                 */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_tous_ONDULEUR ( void  )
  { gchar requete[128];
    struct DB *db;
    gint cpt;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db) return(FALSE);

/********************************************** Chargement des modules ************************************/
    g_snprintf( requete, sizeof(requete), "SELECT id FROM %s",
                NOM_TABLE_MODULE_ONDULEUR
              );

    if ( Lancer_requete_SQL ( Config.log, db, requete ) == FALSE )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    Partage->com_onduleur.Modules_ONDULEUR = NULL;
    cpt = 0;
    while ( Recuperer_ligne_SQL (Config.log, db) )
     { struct MODULE_ONDULEUR *module;

       module = (struct MODULE_ONDULEUR *)g_malloc0( sizeof(struct MODULE_ONDULEUR) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info( Config.log, DEBUG_MEM,
                "Charger_tous_ONDULEUR: Erreur allocation mémoire struct MODULE_ONDULEUR" );
          continue;
        }
       Charger_un_ONDULEUR_DB( module, atoi (db->row[0]) );
       cpt++;                                              /* Nous avons ajouté un module dans la liste ! */
                                                                        /* Ajout dans la liste de travail */
       Partage->com_onduleur.Modules_ONDULEUR = g_list_append ( Partage->com_onduleur.Modules_ONDULEUR, module );
     }
    Liberer_resultat_SQL ( Config.log, db );
    Info_n( Config.log, DEBUG_INFO, "Charger_tous_ONDULEUR: module ONDULEUR found  !", cpt );

    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Charger_un_ONDULEUR ( gint id )
  { struct MODULE_ONDULEUR *module;
    
    module = (struct MODULE_ONDULEUR *)g_malloc0( sizeof(struct MODULE_ONDULEUR) );
    if (!module)                                                   /* Si probleme d'allocation mémoire */
     { Info( Config.log, DEBUG_MEM,
            "Charger_un_ONDULEUR: Erreur allocation mémoire struct MODULE_ONDULEUR" );
       return;
     }
    pthread_mutex_lock( &Partage->com_onduleur.synchro );
    Charger_un_ONDULEUR_DB ( module, id );
    Partage->com_onduleur.Modules_ONDULEUR = g_list_append ( Partage->com_onduleur.Modules_ONDULEUR, module );
    pthread_mutex_unlock( &Partage->com_onduleur.synchro );
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_un_ONDULEUR ( struct MODULE_ONDULEUR *module )
  { if (!module) return;
    pthread_mutex_lock( &Partage->com_onduleur.synchro );
    Partage->com_onduleur.Modules_ONDULEUR = g_list_remove ( Partage->com_onduleur.Modules_ONDULEUR, module );
    g_free(module);
    pthread_mutex_unlock( &Partage->com_onduleur.synchro );
  }
/**********************************************************************************************************/
/* Decharger_tous_ONDULEUR: Decharge l'ensemble des modules ONDULEUR                                          */
/* Entrée: rien                                                                                           */
/* Sortie: rien                                                                                           */
/**********************************************************************************************************/
 static void Decharger_tous_ONDULEUR ( void  )
  { struct MODULE_ONDULEUR *module;
    while ( Partage->com_onduleur.Modules_ONDULEUR )
     { module = (struct MODULE_ONDULEUR *)Partage->com_onduleur.Modules_ONDULEUR->data;
       Decharger_un_ONDULEUR ( module );
     }
  }
/**********************************************************************************************************/
/* Deconnecter: Deconnexion du module                                                                     */
/* Entrée: un id                                                                                          */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Deconnecter_module ( struct MODULE_ONDULEUR *module )
  { if (!module) return;
    if (module->started == FALSE) return;

    upscli_disconnect( &module->upsconn );
    module->started = FALSE;
    module->nbr_deconnect++;
    module->date_retente = 0;
    Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Deconnecter_module", module->id );
    SB( module->bit_comm, 0 );                                /* Mise a zero du bit interne lié au module */
  }
/**********************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                           */
/* Entrée: une nom et un password                                                                         */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                          */
/**********************************************************************************************************/
 static gboolean Connecter_module ( struct MODULE_ONDULEUR *module )
  { int connexion;

    if ( (connexion = upscli_connect( &module->upsconn, module->host, ONDULEUR_PORT_TCP, UPSCLI_CONN_TRYSSL)) == -1 )
     { Info_c( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Connecter_module: connexion refused by module",
               (char *)upscli_strerror(&module->upsconn) );
       return(FALSE);
     }

    Info_c( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Connecter_module", module->host );
    SB( module->bit_comm, 1 );                                   /* Mise a 1 du bit interne lié au module */

    return(TRUE);
  }
/**********************************************************************************************************/
/* Modbus_is_actif: Renvoi TRUE si au moins un des modules modbus est actif                               */
/* Entrée: rien                                                                                           */
/* Sortie: TRUE/FALSE                                                                                     */
/**********************************************************************************************************/
 static gboolean Onduleur_is_actif ( void )
  { GList *liste;
    liste = Partage->com_onduleur.Modules_ONDULEUR;
    while ( liste )
     { struct MODULE_ONDULEUR *module;
       module = ((struct MODULE_ONDULEUR *)liste->data);

       if (module->actif) return(TRUE);
       liste = liste->next;
     }
    return(FALSE);
  }
/**********************************************************************************************************/
/* Interroger_onduleur: Interrogation d'un onduleur                                                       */
/* Entrée: identifiants des modules et borne                                                              */
/* Sortie: ?                                                                                              */
/**********************************************************************************************************/
 static void Interroger_onduleur( struct MODULE_ONDULEUR *module )
  { const char *query[] = { "VAR", "Evo1750", "ups.load" };
    char **answer;
    guint numa;
    int retour;

    retour = upscli_get( &module->upsconn, 3, query, &numa, &answer);
    if (retour == -1)
     { Info_c( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Interroger_module: Wrong ANSWER",
               (char *)upscli_strerror(&module->upsconn) );
       Deconnecter_module ( module );
       module->date_retente = Partage->top + ONDULEUR_RETRY;        /* On ne retentera que dans longtemps */
       return;
     }
    else
     {     Info_c( Config.log, DEBUG_ONDULEUR, "ONDULEUR: load", answer[3] );

     }
    module->date_retente = Partage->top + ONDULEUR_RETRY / 3;               /* Ce n'est pas du temps réel */
  }

/**********************************************************************************************************/
/* Main: Fonction principale du ONDULEUR                                                                  */
/**********************************************************************************************************/
 void Run_onduleur ( void )
  { struct MODULE_ONDULEUR *module;
    GList *liste;

    prctl(PR_SET_NAME, "W-ONDULEUR", 0, 0, 0 );
    Info( Config.log, DEBUG_ONDULEUR, "ONDULEUR: demarrage" );

    Partage->com_onduleur.Modules_ONDULEUR = NULL;                            /* Init des variables du thread */

    if ( Charger_tous_ONDULEUR() == FALSE )                                /* Chargement des modules onduleur */
     { Info( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: No module ONDULEUR found -> stop" );
       pthread_exit(GINT_TO_POINTER(-1));
     }

    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { usleep(1000);
       sched_yield();

       if (Partage->com_onduleur.reload == TRUE)
        { Info( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Reloading conf" );
          Decharger_tous_ONDULEUR();
          Charger_tous_ONDULEUR();
          Partage->com_onduleur.reload = FALSE;
        }

       if (Partage->com_onduleur.admin_del)
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Deleting module",
                  Partage->com_onduleur.admin_del );
          module = Chercher_module_by_id ( Partage->com_onduleur.admin_del );
          Deconnecter_module  ( module );
          Decharger_un_ONDULEUR ( module );
          Partage->com_onduleur.admin_del = 0;
        }

       if (Partage->com_onduleur.admin_add)
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Adding module",
                  Partage->com_onduleur.admin_add );
          Charger_un_ONDULEUR ( Partage->com_onduleur.admin_add );
          Partage->com_onduleur.admin_add = 0;
        }

       if (Partage->com_onduleur.admin_start)
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Starting module",
                  Partage->com_onduleur.admin_start );
          module = Chercher_module_by_id ( Partage->com_onduleur.admin_start );
          if (module) module->actif = 1;
          Partage->com_onduleur.admin_start = 0;
        }

       if (Partage->com_onduleur.admin_stop)
        { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Stoping module",
                  Partage->com_onduleur.admin_stop );
          module = Chercher_module_by_id ( Partage->com_onduleur.admin_stop );
          if (module) module->actif = 0;
          Deconnecter_module  ( module );
          Partage->com_onduleur.admin_stop = 0;
        }

       if (Partage->com_onduleur.Modules_ONDULEUR == NULL ||        /* Si pas de module référencés, on attend */
           Onduleur_is_actif() == FALSE)
        { sleep(2); continue; }

       liste = Partage->com_onduleur.Modules_ONDULEUR;
       while (liste)
        { module = (struct MODULE_ONDULEUR *)liste->data;
          if ( module->actif != TRUE || 
               Partage->top < module->date_retente )           /* Si attente retente, on change de module */
           { liste = liste->next;                      /* On prépare le prochain accès au prochain module */
             continue;
           }

/*********************************** Début de l'interrogation du module ***********************************/
          if ( ! module->started )                                           /* Communication OK ou non ? */
           { if ( Connecter_module( module ) )
              { module->date_retente = 0;;
                module->started = TRUE;
              }
             else
              { Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Module DOWN", module->id );
                module->date_retente = Partage->top + ONDULEUR_RETRY;
              }
           }
          else
           { Interroger_onduleur ( module );
           }
          liste = liste->next;                         /* On prépare le prochain accès au prochain module */
        }
     }

    Decharger_tous_ONDULEUR();
    Info_n( Config.log, DEBUG_ONDULEUR, "ONDULEUR: Run_onduleur: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
