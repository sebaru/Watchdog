/******************************************************************************************************************************/
/* Watchdogd/watchdogd.h      Déclarations générales watchdog                                                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          sam 11 avr 2009 12:23:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * watchdogd.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2023 - Sebastien Lefevre
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
 #include <string.h>
 #include <openssl/ssl.h>
 #include <libsoup/soup.h>
 #include <uuid/uuid.h>

/*---------------------------------------------------- dépendances -----------------------------------------------------------*/
 #include "Erreur.h"
 #include "Json.h"
 #include "Dls.h"
 #include "Thread.h"
 #include "config.h"
 #include "Http.h"
 #include "Config.h"
 #include "Archive.h"

 extern struct PARTAGE *Partage;                                                 /* Accès aux données partagées des processes */

 #define EXIT_ERREUR       -1                                                                   /* Sortie sur erreur inconnue */
 #define EXIT_OK           0                                                                                /* Sortie normale */
 #define EXIT_INACTIF      1                                                                 /* Un fils est mort d'inactivité */

 #define VERROU_SERVEUR              "watchdogd.lock"

 struct COM_DB                                                                 /* Interfaçage avec le code de gestion des BDD */
  { pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    GSList *Liste;                                                              /* Liste des requetes en cours de realisation */
  };

 struct COM_MSRV                                                            /* Communication entre DLS et le serveur Watchdog */
  { gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    pthread_t TID_api_sync;                                                                          /* Identifiant du thread */
    pthread_t TID_arch_sync;                                                                         /* Identifiant du thread */
                                                                       /* Distribution aux threads (par systeme d'abonnement) */
    GSList *liste_msg;                                                                 /* liste de struct MSGDB msg a envoyer */
    GSList *liste_visuel;                                            /* liste de I (dynamique) a traiter dans la distribution */
    GSList *Liste_DO;                                                            /* liste de A a traiter dans la distribution */
    GSList *Liste_AO;                                                            /* liste de A a traiter dans la distribution */
    GSList *Threads;                                                               /* Liste des Threads chargés pour Watchdog */
    struct mosquitto *MQTT_local_session;                                                /* Session MQTT vers le broker local */
    struct mosquitto *MQTT_API_session;                                                    /* Session MQTT vers le broker API */
    gboolean MQTT_connected;                                                         /* TRUE si la connexion au broker est OK */
    GSList *MQTT_messages;                                                               /* Liste des messages recus via MQTT */
  };

 struct PARTAGE                                                                            /* Structure des données partagées */
  { gint  taille_partage;
    gchar version[16];
    time_t start_time;                                                                         /* Date de start de l'instance */
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
    struct COM_HTTP com_http;                                                                       /* Zone mémoire pour HTTP */

    pthread_mutex_t archive_liste_sync;                                                   /* Bit de synchronisation processus */
    GSList *archive_liste;                                                                /* liste de struct ARCHDB a traiter */
    gint archive_liste_taille;

    pthread_mutex_t abonnements_synchro;                                                  /* Bit de synchronisation processus */
    GSList *abonnements;                                                               /* Abonnements aux entrées analogiques */

    JsonNode *Maps_root;                                                                   /* Json Array de tous les mappings */
    GTree *Maps_from_thread;                                                          /* GTree des mappings thread vers local */
    GTree *Maps_to_thread;                                                            /* GTree des mappings local vers thread */
  };

/************************************************ Définitions des prototypes **************************************************/
 extern void MSRV_Remap( void );                                                                          /* Dans Watchdogd.c */

 extern struct PARTAGE *Shm_init ( void );                                                                      /* Dans shm.c */
 extern gboolean Shm_stop ( struct PARTAGE *partage );

 extern void Gerer_arrive_Axxx_dls ( void );                                                         /* Dans distrib_Events.c */

 extern void Convert_libelle_dynamique ( gchar *local_tech_id, gchar *libelle, gint taille_max );

 extern void API_Send_ARCHIVE ( void );                                                                     /* Dans api_xxx.c */
 extern void API_Clear_ARCHIVE ( void );
 extern void API_Send_Abonnements ( void );
 extern void Run_api_sync ( void );
 extern JsonNode *Http_Post_to_global_API ( gchar *URI, JsonNode *RootNode );
 extern JsonNode *Http_Get_from_global_API ( gchar *URI, gchar *format, ... );

 extern gboolean Send_mail ( gchar *sujet, gchar *dest, gchar *body );                                         /* dans mail.c */

 extern gboolean MSRV_Map_to_thread ( JsonNode *key );
 extern gboolean MSRV_Map_from_thread ( JsonNode *key );
 extern void MSRV_Agent_upgrade_to ( gchar *branche );

 extern void UUID_New ( gchar *target );                                                                       /* Dans uuid.c */
 extern void UUID_Load ( gchar *thread, gchar *target );

 extern void Http_Add_Agent_signature ( SoupMessage *msg, gchar *buf, gint buf_size );                  /* Dans http_common.c */
 extern SoupSession *HTTP_New_session ( gchar *user_agent );
 extern gboolean Http_Accept_certificate ( SoupMessage* self, GTlsCertificate* tls_peer_certificate, GTlsCertificateFlags tls_peer_errors, gpointer user_data );
 extern JsonNode *Http_Send_json_request_from_agent ( SoupMessage *soup_msg, JsonNode *RootNode );
 extern JsonNode *Http_Send_json_request_from_thread ( struct THREAD *module, SoupMessage *soup_msg, JsonNode *RootNode );
 extern void Http_Send_json_response ( SoupServerMessage *msg, gint code, gchar *message, JsonNode *RootNode );

 extern void MQTT_Send_to_topic ( struct mosquitto *mqtt_session, gchar *topic, gchar *tag, JsonNode *node );
 extern void MQTT_Send_AI ( struct THREAD *module, JsonNode *thread_ai, gdouble valeur, gboolean in_range );
 extern void MQTT_Send_DI ( struct THREAD *module, JsonNode *thread_di, gboolean etat );
 extern void MQTT_Send_DI_pulse ( struct THREAD *module, gchar *thread_tech_id, gchar *thread_acronyme );
 extern void MQTT_Send_WATCHDOG ( struct THREAD *module, gchar *thread_acronyme, gint consigne );
 extern void MQTT_Subscribe ( struct mosquitto *mqtt_session, gchar *topic );
 extern void MQTT_Send_to_API ( gchar *topic, JsonNode *node );

 extern gboolean MQTT_Start_MQTT_API ( void );
 extern void MQTT_Stop_MQTT_API ( void );
 extern void MQTT_Send_MSGS_to_API ( void );
 extern void MQTT_Send_visuels_to_API ( void );

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
