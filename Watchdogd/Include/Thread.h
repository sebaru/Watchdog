/******************************************************************************************************************************/
/* Watchdogd/include/thread.h      Déclarations générales de gestion des threads                                              */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                                30.01.2022 12:46:36 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * thread.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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

 struct THREAD
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    void *dl_handle;                                                                     /* handle de gestion de la librairie */
    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_debug;                                                    /* TRUE si le thread doit tourner en mode debug */
    SoupSession *Soup_session;
    struct mosquitto *MQTT_session;
    GSList *MQTT_messages;
    JsonNode *config;                               /* Pointeur vers un element du tableau lib->config spécifique a ce thread */
    gboolean comm_status;                                                       /* Report local du status de la communication */
    gint     comm_next_update;                                        /* Date du prochain update Watchdog COMM vers le master */
    JsonNode *ai_nbr_tour_par_sec;                                                                        /* Tour par seconde */
    JsonNode *IOs;
    gint nbr_tour;
    gint nbr_tour_par_sec;
    gint nbr_tour_top;
    gint nbr_tour_delai;
    gint telemetrie_top;
    gint hour_top;
    void *vars;                                                               /* Pointeur vers les variables de run du module */
    void (*Run_thread)( struct THREAD *module );                          /* Fonction principale de gestion du module */
  };

/************************************************ Définitions des prototypes **************************************************/
 extern void Stopper_fils ( void );                                                                          /* Dans thread.c */
 extern gboolean Demarrer_dls ( void );
 extern gboolean Demarrer_http ( void );
 extern gboolean Demarrer_arch ( void );
 extern void Charger_librairies ( void );
 extern void Decharger_librairies ( void );
 extern void Thread_Start_one_thread (JsonArray *array, guint index_, JsonNode *element, gpointer user_data );
 extern void Thread_Stop_one_thread ( JsonNode *element );
 extern void Thread_Push_API_message ( JsonNode *request );
 extern void Thread_Set_debug ( JsonNode *request );
 extern void Thread_send_comm_to_master ( struct THREAD *module, gboolean etat );
 extern void Thread_loop ( struct THREAD *module );
 extern void Thread_init ( struct THREAD *module, gint sizeof_vars );
 extern void Thread_end ( struct THREAD *module );
 extern gboolean Thread_every_hour ( struct THREAD *module );

 extern JsonNode *Mnemo_create_thread_DI ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle );
 extern JsonNode *Mnemo_create_thread_DO ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gboolean mono );
 extern JsonNode *Mnemo_create_thread_AI ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage );
 extern JsonNode *Mnemo_create_thread_AO ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle, gchar *unite, gint archivage );
 extern JsonNode *Mnemo_create_thread_WATCHDOG ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle );
 extern JsonNode *Mnemo_create_thread_HORLOGE ( struct THREAD *module, gchar *thread_acronyme, gchar *libelle );
 extern void Mnemo_create_thread_HORLOGE_tick ( struct THREAD *module, JsonNode *bit, gint heure, gint minute );
 extern void Mnemo_delete_thread_HORLOGE_tick ( struct THREAD *module, JsonNode *bit );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
