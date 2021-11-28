/******************************************************************************************************************************/
/* Watchdogd/watchdogd.h      Déclarations générales watchdog                                                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 11 avr 2009 12:23:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * watchdogd.h
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

 #ifndef _WATCHDOGD_H_
 #define _WATCHDOGD_H_

 #include <glib.h>
 #include <pthread.h>
 #include <string.h>
 #include <errno.h>
 #include <openssl/ssl.h>
 #include <libsoup/soup.h>
 #include <uuid/uuid.h>

/*---------------------------------------------------- dépendances -----------------------------------------------------------*/
 #include "Json.h"
 #include "Db.h"
 #include "config.h"
 #include "Dls.h"
 #include "Config.h"
 #include "Archive.h"
 #include "Message_DB.h"
 #include "Histo_DB.h"
 #include "Synoptiques_DB.h"
 #include "Mnemonique_DB.h"
 #include "Proto_traductionDLS.h"

 extern struct PARTAGE *Partage;                                                 /* Accès aux données partagées des processes */

 #define EXIT_ERREUR       -1                                                                   /* Sortie sur erreur inconnue */
 #define EXIT_OK           0                                                                                /* Sortie normale */
 #define EXIT_INACTIF      1                                                                 /* Un fils est mort d'inactivité */

 #define VERROU_SERVEUR              "watchdogd.lock"
 #define FICHIER_EXPORT              "export.wdg"

 struct SUBPROCESS
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    struct PROCESS *lib;
    JsonNode *config;
    gboolean comm_status;                                                       /* Report local du status de la communication */
    gint     comm_next_update;                                        /* Date du prochain update Watchdog COMM vers le master */
    void *zmq_from_bus;                                                                       /* handle d"ecoute du BUS local */
    void *zmq_to_master;                                                                           /* handle d"envoiau master */
    gchar zmq_buffer[1024];                                                     /* Buffer de reception des messages du master */
    void *vars;                                                               /* Pointeur vers les variables de run du module */
  };

 struct PROCESS
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    gchar uuid[37];                                                                            /* Unique Identifier du thread */
    void *dl_handle;                                                                     /* handle de gestion de la librairie */
    time_t start_time;
    gchar name[32];                                                                    /* Prompt auquel va répondre le thread */
    gchar description[64];                                                             /* Designation de l'activité du thread */
    gchar version[32];
    gchar nom_fichier[128];                                                                 /* Nom de fichier de la librairie */
    gint  database_version;                                            /* Version du schema de base de données pour ce thread */
    JsonNode *config;

    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_debug;                                                    /* TRUE si le thread doit tourner en mode debug */
    gboolean Thread_reload;                                                           /* TRUE si le thread doit gerer le USR1 */

    void (*Run_process)( struct PROCESS *lib );                                  /* Fonction principale de gestion du thread */
    void (*Run_subprocess)( struct SUBPROCESS *module );                          /* Fonction principale de gestion du module */
                                                                                 /* Fonction de gestion des commandes d'admin */
    void *(*Admin_json)( struct PROCESS *lib, gpointer msg, const char *path, GHashTable *query, gint access_level );
    void *(*Admin_config)( struct PROCESS *lib, gpointer msg, JsonNode *RootNode );

    GSList *modules;                                                                           /* Liste des modules du thread */
    gboolean comm_status;                                                       /* Report local du status de la communication */
    gint     comm_next_update;                                        /* Date du prochain update Watchdog COMM vers le master */
    void *zmq_from_bus;                                                                       /* handle d"ecoute du BUS local */
    void *zmq_to_master;                                                                           /* handle d"envoiau master */
    gchar zmq_buffer[1024];                                                     /* Buffer de reception des messages du master */
  };

 struct COM_DB                                                                 /* Interfaçage avec le code de gestion des BDD */
  { pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    GSList *Liste;                                                              /* Liste des requetes en cours de realisation */
  };

 struct COM_MSRV                                                            /* Communication entre DLS et le serveur Watchdog */
  { gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
                                                                       /* Distribution aux threads (par systeme d'abonnement) */
    GSList *liste_msg;                                                                 /* liste de struct MSGDB msg a envoyer */
    GSList *liste_visuel;                                            /* liste de I (dynamique) a traiter dans la distribution */
    GSList *Liste_DO;                                                            /* liste de A a traiter dans la distribution */
    GSList *Liste_AO;                                                            /* liste de A a traiter dans la distribution */
    struct ZMQUEUE *zmq_to_bus;                                                      /* Message Queue des evenements Watchdog */
    union
     { struct ZMQUEUE *zmq_to_slave;                                                         /* Message Queue vers les slaves */
       struct ZMQUEUE *zmq_to_master;
     };

    GSList *Librairies;                                                        /* Liste des librairies chargées pour Watchdog */
    gint last_master_ping;                                                    /* Gere le dernier ping du master vers le slave */
  };

 struct PARTAGE                                                                            /* Structure des données partagées */
  { gint  taille_partage;
    gchar version[16];
    time_t start_time;                                                                         /* Date de start de l'instance */
    void *zmq_ctx;                                                    /* Contexte d'échange inter-thread et message queue ZMQ */
    guint top;                                                                         /* Gestion des contraintes temporelles */
    guint top_cdg_plugin_dls;                                                        /* Top de chien de garde des plugins DLS */
    guint audit_bit_interne_per_sec;
    guint audit_bit_interne_per_sec_hold;
    guint audit_tour_dls_per_sec;
    guint audit_tour_dls_per_sec_hold;
                                                                                                    /* Interfacage avec D.L.S */
    struct COM_DB com_db;                                                      /* Interfaçage avec le code de gestion des BDD */
    struct COM_MSRV com_msrv;                                                                        /* Changement du à D.L.S */
    struct COM_DLS com_dls;                                                                       /* Changement du au serveur */
    struct COM_ARCH com_arch;                                                                      /* Com avec le thread ARCH */

    GSList *Dls_data_TEMPO;                                                                               /* Liste des tempos */
    GSList *Dls_data_MONO;                                                                           /* Liste des monostables */
    GSList *Dls_data_BI;                                                                               /* Liste des bistables */
    GSList *Dls_data_DI;                                                                  /* Liste des entrees dynamiques TOR */
    GSList *Dls_data_DO;                                                                  /* Liste des sorties dynamiques TOR */
    GSList *Dls_data_AI;                                                                  /* Liste des entrees dynamiques ANA */
    GSList *Dls_data_AO;                                                                  /* Liste des sorties dynamiques ANA */
    GSList *Dls_data_MSG;                                                                               /* Liste des messages */
    GSList *Dls_data_CI;                                                                  /* Liste des compteurs d'impulsions */
    GSList *Dls_data_CH;                                                                      /* Liste des compteurs horaires */
    GSList *Dls_data_VISUEL;                                                                    /* Liste des visuels (bits I) */
    GSList *Dls_data_REGISTRE;                                                                /* Liste des registres (bits R) */
    GSList *Dls_data_WATCHDOG;                                                                /* Liste des registres (bits R) */
  };

/************************************************ Définitions des prototypes **************************************************/
 extern void Charger_config_bit_interne( void );                                                          /* Dans Watchdogd.c */
 extern gint Activer_ecoute ( void );                                                                        /* Dans ecoute.c */

 extern struct PARTAGE *_init ( void );                                                                         /* Dans shm.c */
 extern struct PARTAGE *Shm_init ( void );
 extern gboolean Shm_stop ( struct PARTAGE *partage );

 extern void Stopper_fils ( void );                                                                         /* Dans process.c */
 extern gboolean Demarrer_dls ( void );
 extern gboolean Demarrer_arch ( void );
 extern void Charger_librairies ( void );
 extern void Decharger_librairies ( void );
 extern gboolean Process_start ( struct PROCESS *lib );
 extern gboolean Process_stop ( struct PROCESS *lib );
 extern gboolean Process_reload_by_uuid ( gchar *uuid );
 extern gboolean Process_set_debug ( gchar *uuid, gboolean debug );
 extern void Process_set_database_version ( struct PROCESS *lib, gint version );
 extern void Thread_init ( gchar *pr_name, gchar *classe, struct PROCESS *lib, gchar *version, gchar *description );
 extern void Thread_end ( struct PROCESS *lib );
 extern JsonNode *Thread_Listen_to_master ( struct PROCESS *lib );
 extern void Thread_send_comm_to_master ( struct PROCESS *lib, gboolean etat );
 extern JsonNode *SubProcess_Listen_to_master_new ( struct SUBPROCESS *module );
 extern void SubProcess_send_comm_to_master_new ( struct SUBPROCESS *module, gboolean etat );
 extern void Process_Load_one_subprocess (JsonArray *array, guint index_, JsonNode *element, gpointer user_data );
 extern void Process_Unload_one_subprocess ( struct SUBPROCESS *module, struct PROCESS *lib );
 extern void SubProcess_init ( struct SUBPROCESS *module, gint sizeof_vars );
 extern void SubProcess_end ( struct SUBPROCESS *module );

 extern void Gerer_arrive_Axxx_dls ( void );                                                         /* Dans distrib_Events.c */

 extern void Gerer_arrive_MSGxxx_dls ( void );                                                       /* Dans distrib_MSGxxx.c */
 extern void Convert_libelle_dynamique ( gchar *local_tech_id, gchar *libelle, gint taille_max );

 extern void Gerer_arrive_Ixxx_dls ( void );                                                           /* Dans distrib_Ixxx.c */

 extern gboolean Send_mail ( gchar *sujet, gchar *dest, gchar *body );                                         /* dans mail.c */

 extern JsonNode *Http_Msg_to_Json ( SoupMessage *msg );                                                       /* Dans http.c */
 extern JsonNode *Http_Response_Msg_to_Json ( SoupMessage *msg );
 extern gint Http_Msg_status_code ( SoupMessage *msg );
 extern gchar *Http_Msg_reason_phrase ( SoupMessage *msg );

 extern void New_uuid ( gchar *target );                                                                       /* Dans uuid.c */
/*-------------------------------------------------- autres dépendances ------------------------------------------------------*/
 #include "Zmq.h"

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
