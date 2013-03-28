/**********************************************************************************************************/
/* Watchdogd/master.c        Gestion des MASTER de Watchdog v2.0                                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                    lun. 18 févr. 2013 18:24:09 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Master.c
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
 
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <string.h>
 #include <unistd.h>
 #include <libsoup/soup.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Master.h"

/**********************************************************************************************************/
/* Master_Lire_config : Lit la config Watchdog et rempli la structure mémoire                             */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Master_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Master_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_master.lib->Thread_debug = g_key_file_get_boolean ( gkf, "MASTER", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */

    Cfg_master.enable = g_key_file_get_boolean ( gkf, "MASTER", "enable", NULL ); 
    Cfg_master.port   = g_key_file_get_integer ( gkf, "MASTER", "port", NULL );
    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Master_Liberer_config : Libere la mémoire allouer précédemment pour lire la config master              */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Master_Liberer_config ( void )
  {
  }
/**********************************************************************************************************/
/* Master_Gerer_message: Fonction d'abonné appellé lorsqu'un message est disponible.                      */
/* Entrée: une structure CMD_TYPE_HISTO                                                                   */
/* Sortie : Néant                                                                                         */
/**********************************************************************************************************/
 static void Master_Gerer_message ( struct CMD_TYPE_MESSAGE *msg )
  { gint taille;

    pthread_mutex_lock( &Cfg_master.lib->synchro );                      /* Ajout dans la liste a traiter */
    taille = g_slist_length( Cfg_master.Liste_message );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Master_Gerer_message: DROP message %d (length = %d > 150)", msg->num, taille);
       g_free(msg);
       return;
     }

    pthread_mutex_lock ( &Cfg_master.lib->synchro );
    Cfg_master.Liste_message = g_slist_append ( Cfg_master.Liste_message, msg );      /* Ajout a la liste */
    pthread_mutex_unlock ( &Cfg_master.lib->synchro );
  }
/**********************************************************************************************************/
/* Master_Gerer_sortie: Ajoute une demande d'envoi RF dans la liste des envois RFXCOM                     */
/* Entrées: le numéro de la sortie                                                                        */
/**********************************************************************************************************/
 void Master_Gerer_sortie( gint num_a )                                    /* Num_a est l'id de la sortie */
  { gint taille;

    pthread_mutex_lock( &Cfg_master.lib->synchro );              /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_master.Liste_sortie );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING,
                "Master_Gerer_sortie: DROP sortie %d (length = %d > 150)", num_a, taille );
       return;
     }

    pthread_mutex_lock( &Cfg_master.lib->synchro );       /* Ajout dans la liste de tell a traiter */
    Cfg_master.Liste_sortie = g_slist_prepend( Cfg_master.Liste_sortie, GINT_TO_POINTER(num_a) );
    pthread_mutex_unlock( &Cfg_master.lib->synchro );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_slaveDB: Recupération de la liste des ids des slave                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static gboolean Recuperer_slaveDB ( struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,bit_comm,libelle,enable,ea_min,ea_max,e_min,e_max,"
                "sa_min,sa_max,s_min,s_max"
                " FROM %s ORDER BY num", NOM_TABLE_SLAVES );

    return ( Lancer_requete_SQL ( Config.log, db, requete ) );             /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_slaveDB: Recupération de la liste des ids des slave                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct SLAVEDB *Recuperer_slaveDB_suite( struct DB *db )
  { struct SLAVEDB *slave;

    Recuperer_ligne_SQL (Config.log, db);                              /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( Config.log, db );
       return(NULL);
     }

    slave = (struct SLAVEDB *)g_try_malloc0( sizeof(struct SLAVEDB) );
    if (!slave) Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                          "Recuperer_slaveDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &slave->libelle, db->row[2], sizeof(slave->libelle) );
       slave->id                = atoi(db->row[0]);
       slave->bit_comm          = atoi(db->row[1]);
       slave->enable            = atoi(db->row[3]);
       slave->ea_min            = atoi(db->row[4]);
       slave->ea_max            = atoi(db->row[5]);
       slave->e_min             = atoi(db->row[6]);
       slave->e_max             = atoi(db->row[7]);
       slave->sa_min            = atoi(db->row[8]);
       slave->sa_max            = atoi(db->row[9]);
       slave->s_min             = atoi(db->row[10]);
       slave->s_max             = atoi(db->row[11]);
     }
    return(slave);
  }
/**********************************************************************************************************/
/* Charger_tous_slave: Requete la DB pour charger les modules et les bornes slave                         */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_tous_slave ( void  )
  { struct DB *db;

    db = Init_DB_SQL( Config.log );
    if ( !db )
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_WARNING, "Charger_tous_slave: Database Connection Failed" );
       return(-1);
     }

/********************************************** Chargement des modules ************************************/
    if ( ! Recuperer_slaveDB( db ) )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    Cfg_master.Slaves = NULL;
    for ( ; ; )
     { struct SLAVE *module;
       struct SLAVEDB *slave;

       slave = Recuperer_slaveDB_suite( db );
       if (!slave) break;

       module = (struct SLAVE *)g_try_malloc0( sizeof(struct SLAVE) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_ERR,
                    "Charger_tous_slave: Erreur allocation mémoire struct SLAVE" );
          g_free(slave);
          Libere_DB_SQL( Config.log, &db );
          return(FALSE);
        }
       memcpy( &module->slave, slave, sizeof(struct SLAVEDB) );
       if (module->slave.enable) module->started = TRUE;          /* Si enable at boot... et bien Start ! */
       g_free(slave);
                                                                        /* Ajout dans la liste de travail */
       pthread_mutex_lock ( &Cfg_master.lib->synchro );
       Cfg_master.Slaves = g_slist_prepend ( Cfg_master.Slaves, module );
       pthread_mutex_unlock ( &Cfg_master.lib->synchro );

       Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
                 "Charger_tous_slave: id = %d, enable = %d", module->slave.id, module->slave.enable );
     }
    pthread_mutex_lock ( &Cfg_master.lib->synchro );
    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
              "Charger_tous_slave: %03d module SLAVES found  !", g_slist_length(Cfg_master.Slaves) );
    pthread_mutex_unlock ( &Cfg_master.lib->synchro );

    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_tous_slave ( void  )
  { struct SLAVE *module;

    pthread_mutex_lock ( &Cfg_master.lib->synchro );
    while ( Cfg_master.Slaves )
     { module = (struct SLAVE *)Cfg_master.Slaves->data;
       Cfg_master.Slaves = g_slist_remove ( Cfg_master.Slaves, module );
       g_free(module);
     }
    pthread_mutex_unlock ( &Cfg_master.lib->synchro );
  }
/**********************************************************************************************************/
/* Master Callback : Renvoi une reponse suite a une demande d'un slave (appellée par libsoup)             */
/* Entrées : le contexte, le message, l'URL                                                               */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static void Master_CB (SoupServer        *server,  SoupMessage       *msg, 
                        const char        *path,  GHashTable        *query,
                        SoupClientContext *client, gpointer           user_data)
  {
    soup_message_set_status (msg, 0 ); /* On renvoie 0 all is good */
  }  
/**********************************************************************************************************/
/* Run_thread: Thread principal                                                                           */
/* Entrée: une structure LIBRAIRIE                                                                        */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { prctl(PR_SET_NAME, "W-MASTER", 0, 0, 0 );
    memset( &Cfg_master, 0, sizeof(Cfg_master) );               /* Mise a zero de la structure de travail */
    Cfg_master.lib = lib;                      /* Sauvegarde de la structure pointant sur cette librairie */
    Master_Lire_config ();                              /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );

    g_snprintf( Cfg_master.lib->admin_prompt, sizeof(Cfg_master.lib->admin_prompt), "master" );
    g_snprintf( Cfg_master.lib->admin_help,   sizeof(Cfg_master.lib->admin_help),   "Manage communications with Slaves Watchdog" );

    if (!Cfg_master.enable)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread not enable in config. Shutting Down %d", pthread_self() );
       goto end;
     }

    Cfg_master.context = g_main_context_new ();
    Cfg_master.server  = soup_server_new ( SOUP_SERVER_PORT, Cfg_master.port,
                                           SOUP_SERVER_ASYNC_CONTEXT, Cfg_master.context,
                                           NULL
                                         );
    if (!Cfg_master.server)
     { Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: SoupServer creation error. Shutting Down %d", pthread_self() );
       goto end;
     }

    Charger_tous_slave();
    soup_server_add_handler ( Cfg_master.server, "/set_internal_bit", Master_CB, NULL, NULL );
    soup_server_run_async   ( Cfg_master.server );

    Abonner_distribution_message ( Master_Gerer_message );      /* Abonnement à la diffusion des messages */
    Abonner_distribution_sortie  ( Master_Gerer_sortie );        /* Abonnement à la diffusion des sorties */

    Cfg_master.lib->Thread_run = TRUE;                                              /* Le thread tourne ! */
    while(Cfg_master.lib->Thread_run == TRUE)                            /* On tourne tant que necessaire */
     { usleep(10000);
       sched_yield();

       if (Cfg_master.lib->Thread_sigusr1)                                /* A-t'on recu un signal USR1 ? */
        { int nbr_msg, nbr_sortie;

          Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO, "Run_thread: SIGUSR1" );
          pthread_mutex_lock( &Cfg_master.lib->synchro );     /* On recupere le nombre de msgs en attente */
          nbr_msg    = g_slist_length(Cfg_master.Liste_message);
          nbr_sortie = g_slist_length(Cfg_master.Liste_sortie);
          pthread_mutex_unlock( &Cfg_master.lib->synchro );
          Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_INFO,
                    "Run_thread: In Queue : %d MSGS, %d A", nbr_msg, nbr_sortie );
          Cfg_master.lib->Thread_sigusr1 = FALSE;
        }

       g_main_context_iteration ( Cfg_master.context, FALSE );
     }

    Desabonner_distribution_sortie  ( Master_Gerer_sortie ); /* Desabonnement de la diffusion des sorties */
    Desabonner_distribution_message ( Master_Gerer_message );/* Desabonnement de la diffusion des messages */

    soup_server_disconnect ( Cfg_master.server);
    g_main_context_unref (Cfg_master.context );

    Decharger_tous_slave();

end:
    Master_Liberer_config();                                  /* Liberation de la configuration du thread */
    Info_new( Config.log, Cfg_master.lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_master.lib->TID = 0;                              /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
