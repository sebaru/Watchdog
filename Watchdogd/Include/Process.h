/******************************************************************************************************************************/
/* Watchdogd/process.h      Déclarations générales watchdog                                                                   */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    30.01.2022 12:46:36 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * process.h
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

 #ifndef _PROCESS_H_
 #define _PROCESS_H_

 #include <glib.h>
 #include <pthread.h>
 #include <string.h>
 #include <errno.h>
 #include <json-glib/json-glib.h>

 struct SUBPROCESS
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    struct PROCESS *lib;
    JsonNode *config;                               /* Pointeur vers un element du tableau lib->config spécifique a ce thread */
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
    void *(*Admin_config)( struct PROCESS *lib, gpointer msg, JsonNode *RootNode );

    GSList *modules;                                                                           /* Liste des modules du thread */
    gboolean comm_status;                                                       /* Report local du status de la communication */
    gint     comm_next_update;                                        /* Date du prochain update Watchdog COMM vers le master */
    void *zmq_from_bus;                                                                       /* handle d"ecoute du BUS local */
    void *zmq_to_master;                                                                           /* handle d"envoiau master */
    gchar zmq_buffer[1024];                                                     /* Buffer de reception des messages du master */
  };

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
