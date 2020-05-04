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

 #include "Reseaux.h"
 #include "Zmq.h"
 #include "Json.h"
 #include "config.h"
 #include "Db.h"
 #include "Config.h"
 #include "Archive.h"
 #include "Admin.h"
 #include "Message_DB.h"
 #include "Camera_DB.h"
 #include "Dls.h"
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

 #define MAX_ENREG_QUEUE               1500      /* (a virer) Nombre maximum d'enregistrement dans une queue de communication */

 struct LIBRAIRIE
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    void *dl_handle;                                                                     /* handle de gestion de la librairie */
    gchar nom_fichier[128];                                                                 /* Nom de fichier de la librairie */
    gchar admin_prompt[32];                                                            /* Prompt auquel va répondre le thread */
    gchar admin_help[64];                                                              /* Designation de l'activité du thread */

    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_debug;                                                    /* TRUE si le thread doit tourner en mode debug */
    gboolean Thread_reload;                                                           /* TRUE si le thread doit gerer le USR1 */

    void (*Run_thread)( struct LIBRAIRIE *lib );                                  /* Fonction principale de gestion du thread */
                                                                                 /* Fonction de gestion des commandes d'admin */
    void *(*Admin_json)( gchar *commande, gchar **buffer, gint *taille_buf );
  };

 struct COM_DB                                                                 /* Interfaçage avec le code de gestion des BDD */
  { pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    GSList *Liste;                                                              /* Liste des requetes en cours de realisation */
  };

 struct COM_MSRV                                                            /* Communication entre DLS et le serveur Watchdog */
  { gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_reboot;                                                  /* TRUE si le reboot doit suivre une RAZ mémoire */
    gboolean Thread_clear_reboot;                                            /* TRUE si le reboot doit suivre une RAZ mémoire */
    gboolean Thread_reload;                                              /* TRUE si le thread doit recharger sa configuration */

    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    GSList *liste_msg_repeat;                                                          /* liste de struct MSGDB msg a envoyer */
                                                                       /* Distribution aux threads (par systeme d'abonnement) */
    GSList *liste_msg;                                                                 /* liste de struct MSGDB msg a envoyer */
    GSList *liste_i;                                                             /* liste de I a traiter dans la distribution */
    GSList *liste_new_i;                                             /* liste de I (dynamique) a traiter dans la distribution */
    GSList *Liste_DO;                                                            /* liste de A a traiter dans la distribution */
    GSList *Liste_AO;                                                            /* liste de A a traiter dans la distribution */
    struct ZMQUEUE *zmq_msg;                                                           /* Message Queue des messages Watchdog */
    struct ZMQUEUE *zmq_motif;                                                           /* Message Queue des motifs Watchdog */
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

    struct ANALOG_INPUT ea [ NBR_ENTRE_ANA ];
    guchar m [ (NBR_BIT_MONOSTABLE>>3) + 1 ];                                      /* Monostables du DLS (DLS=rw, Sserveur=r) */
    struct DIGITAL_INPUT e [ NBR_ENTRE_TOR ];
    struct SORTIE_TOR a [ NBR_SORTIE_TOR ];
    guchar b [ (NBR_BIT_BISTABLE>>3) + 1 ];                                                                      /* Bistables */
    struct I_MOTIF i[ NBR_BIT_CONTROLE ];                                                               /* DLS=rw, Sserveur=r */
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
  };

/************************************************ Définitions des prototypes **************************************************/
 extern void Charger_config_bit_interne( void );                                                          /* Dans Watchdogd.c */
 extern gint Activer_ecoute ( void );                                                                        /* Dans ecoute.c */

 extern struct PARTAGE *_init ( void );                                                                         /* Dans shm.c */
 extern struct PARTAGE *Shm_init ( void );
 extern gboolean Shm_stop ( struct PARTAGE *partage );
 extern void *w_malloc0( gint size, gchar *justification );
 extern void w_free( void *ptr, gchar *justification );

 extern void Stopper_fils ( void );                                                                         /* Dans process.c */
 extern gboolean Demarrer_dls ( void );
 extern gboolean Demarrer_arch ( void );
 extern void Charger_librairies ( void );
 extern void Decharger_librairies ( void );
 extern gboolean Start_librairie ( struct LIBRAIRIE *lib );
 extern gboolean Stop_librairie ( struct LIBRAIRIE *lib );
 extern struct LIBRAIRIE *Charger_librairie_par_prompt ( gchar *nom_fichier );
 extern gboolean Decharger_librairie_par_prompt ( gchar *nom_fichier );

 extern void Gerer_arrive_Axxx_dls ( void );                                                         /* Dans distrib_Events.c */

 extern void Gerer_arrive_MSGxxx_dls ( void );                                                       /* Dans distrib_MSGxxx.c */
 extern void Gerer_histo_repeat ( void );

 extern void Gerer_arrive_Ixxx_dls ( void );                                                           /* Dans distrib_Ixxx.c */

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
