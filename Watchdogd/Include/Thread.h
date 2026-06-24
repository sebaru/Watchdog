/******************************************************************************************************************************/
/* Watchdogd/include/thread.h      Déclarations générales de gestion des threads                                              */
/* Projet Abls-Habitat version 4.7       Gestion d'habitat                                                30.01.2022 12:46:36 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * thread.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2026 - Sébastien LEFÈVRE
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

 #ifndef _THREAD_H_
 #define _THREAD_H_

 #include <glib.h>
 #include <pthread.h>
 #include <string.h>
 #include <errno.h>
 #include <json-glib/json-glib.h>
 #include <mosquitto.h>

 #define THREAD_MQTT_RECONNECT_DELAY  300

 struct THREAD
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    void *dl_handle;                                                                     /* handle de gestion de la librairie */
    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    struct mosquitto *MQTT_session;
    gboolean MQTT_connected;                                       /* Report du status de la communication vers le MQTT local */
    GSList  *MQTT_messages;
    gint     MQTT_next_top_connect;                                                          /* Date de prochaine reconnexion */
    JsonNode *config;                                         /* Pointeur vers l'ensemble de la config spécifique a ce thread */
    JsonNode *ai_nbr_tour_par_sec;                                                                        /* Tour par seconde */
    JsonNode *IOs;
    gint nbr_tour;
    gint nbr_tour_par_sec;
    gint nbr_tour_top;
    gint nbr_tour_delai;
    gint telemetrie_top;
    gint hour_top;
    void (*Run_thread)( struct THREAD *module );                                  /* Fonction principale de gestion du module */

    guint sizeof_vars;
    GRWLock sub_threads_lock;
    GSList *sub_threads;                                                        /* Liste des sous-threads pour chaque tech_id */
    GSList *cur_thread_link;                                                                 /* Link GSLIST du Thread courant */
    struct SUB_THREAD *cur_thread;                                                     /* Pointeur vers le sub_thread courant */
  };

 struct SUB_THREAD                                                                         /* composants du champ sub_threads */
  { gchar    thread_tech_id[32];                                                                     /* Tech_id du sub_thread */
    gchar    description[128];                                                                       /* description du thread */ 
    gpointer vars;                                                    /* Pointeur vers la zone mémoire vars du current thread */
    gboolean stopped;                                                                     /* TRUE si le sub_thread est arrêté */
    gboolean stopping;                                                          /* TRUE si le sub_thread est en cours d'arrêt */
    gboolean comm_status;                                                       /* Report local du status de la communication */
    gint     comm_next_update;                                        /* Date du prochain update Watchdog COMM vers le master */
  };
/************************************************ Définitions des prototypes **************************************************/
 extern gboolean Demarrer_dls ( void );                                                                      /* Dans thread.c */
 extern void Stopper_dls ( void );
 extern void Thread_Start_all ( void );
 extern void Thread_Start_by_thread_tech_id ( gchar *thread_tech_id );
 extern void Thread_Stop_all ( void );
 extern void Thread_Stop_by_thread_tech_id ( gchar *thread_tech_id );
 extern void Thread_Restart ( gchar *thread_classe, gchar *thread_tech_id );
 extern void Thread_send_comm_to_master ( struct THREAD *module, gboolean etat );
 extern void Thread_loop ( struct THREAD *module );
 extern void Thread_init ( struct THREAD *module, gint sizeof_vars );
 extern void Thread_end ( struct THREAD *module );
 extern gboolean Thread_every_hour ( struct THREAD *module );

 extern JsonNode *Mnemo_create_thread_DI ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle );
 extern JsonNode *Mnemo_create_thread_CI ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage );
 extern JsonNode *Mnemo_create_thread_DO ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gboolean mono );
 extern JsonNode *Mnemo_create_thread_AI ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage );
 extern JsonNode *Mnemo_create_thread_AO ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage );
 extern JsonNode *Mnemo_create_thread_WATCHDOG ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle );
 extern JsonNode *Mnemo_create_thread_HORLOGE ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle );
 extern void Mnemo_create_thread_HORLOGE_tick ( struct THREAD *module, JsonNode *bit, gint heure, gint minute );
 extern void Mnemo_delete_thread_HORLOGE_tick ( struct THREAD *module, JsonNode *bit );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
