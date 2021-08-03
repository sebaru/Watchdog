/******************************************************************************************************************************/
/* Watchdogd/watchdogd.h      Déclarations générales watchdog                                                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          sam 11 avr 2009 12:23:32 CEST */
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
 #include <libsoup/soup.h>

/*---------------------------------------------------- dépendances -----------------------------------------------------------*/
 #include "Reseaux.h"
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

 struct LIBRAIRIE
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    void *dl_handle;                                                                     /* handle de gestion de la librairie */
    time_t start_time;
    gchar name[32];                                                                    /* Prompt auquel va répondre le thread */
    gchar description[64];                                                             /* Designation de l'activité du thread */
    gchar version[32];
    gchar nom_fichier[128];                                                                 /* Nom de fichier de la librairie */

    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_debug;                                                    /* TRUE si le thread doit tourner en mode debug */
    gboolean Thread_reload;                                                           /* TRUE si le thread doit gerer le USR1 */
    gboolean comm_status;                                                       /* Report local du status de la communication */
    gint     comm_next_update;  /* a virer */                         /* Date du prochain update Watchdog COMM vers le master */

    void (*Run_thread)( struct LIBRAIRIE *lib );                                  /* Fonction principale de gestion du thread */
                                                                                 /* Fonction de gestion des commandes d'admin */
    void *(*Admin_json)( struct LIBRAIRIE *lib, gpointer msg, const char *path, GHashTable *query, gint access_level );

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
    GSList *Dls_data_BOOL;                                                              /* Liste des bistables et monostables */
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
 extern gboolean Start_librairie ( struct LIBRAIRIE *lib );
 extern gboolean Stop_librairie ( struct LIBRAIRIE *lib );
 extern gboolean Reload_librairie_par_prompt ( gchar *prompt );
 extern struct LIBRAIRIE *Charger_librairie_par_prompt ( gchar *nom_fichier );
 extern gboolean Decharger_librairie_par_prompt ( gchar *nom_fichier );
 extern void Thread_init ( gchar *pr_name, gchar *classe, struct LIBRAIRIE *lib, gchar *version, gchar *description );
 extern void Thread_end ( struct LIBRAIRIE *lib );
 extern JsonNode *Thread_Listen_to_master ( struct LIBRAIRIE *lib );

 extern void Gerer_arrive_Axxx_dls ( void );                                                         /* Dans distrib_Events.c */

 extern void Gerer_arrive_MSGxxx_dls ( void );                                                       /* Dans distrib_MSGxxx.c */
 extern void Convert_libelle_dynamique ( gchar *local_tech_id, gchar *libelle, gint taille_max );

 extern void Gerer_arrive_Ixxx_dls ( void );                                                           /* Dans distrib_Ixxx.c */

 extern gboolean Send_mail ( gchar *sujet, gchar *dest, gchar *body );                                         /* dans mail.c */

 extern JsonNode *Http_Msg_to_Json ( SoupMessage *msg );                                                       /* Dans http.c */
 extern JsonNode *Http_Response_Msg_to_Json ( SoupMessage *msg );
 extern gint Http_Msg_status_code ( SoupMessage *msg );
 extern gchar *Http_Msg_reason_phrase ( SoupMessage *msg );

/*-------------------------------------------------- autres dépendances ------------------------------------------------------*/
 #include "Zmq.h"

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
