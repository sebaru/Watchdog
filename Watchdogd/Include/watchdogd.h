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
 #include <string.h>
 #include <openssl/ssl.h>
 #include <libsoup/soup.h>
 #include <uuid/uuid.h>

/*---------------------------------------------------- dépendances -----------------------------------------------------------*/
 #include "Json.h"
 #include "Process.h"
 #include "Db.h"
 #include "config.h"
 #include "Dls.h"
 #include "Http.h"
 #include "Config.h"
 #include "Archive.h"
 #include "Message_DB.h"
 #include "Histo_DB.h"
 #include "Synoptiques_DB.h"
 #include "Proto_traductionDLS.h"
 #include "Mnemonique_DB.h"

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
                                                                       /* Distribution aux threads (par systeme d'abonnement) */
    GSList *liste_msg;                                                                 /* liste de struct MSGDB msg a envoyer */
    GSList *liste_visuel;                                            /* liste de I (dynamique) a traiter dans la distribution */
    GSList *Liste_DO;                                                            /* liste de A a traiter dans la distribution */
    GSList *Liste_AO;                                                            /* liste de A a traiter dans la distribution */
    GSList *Librairies;                                                        /* Liste des librairies chargées pour Watchdog */
    GSList *Subprocess;                                                        /* Liste des librairies chargées pour Watchdog */
    SoupSession *API_session;
    SoupWebsocketConnection *API_websocket;
    gint last_master_ping;                                                    /* Gere le dernier ping du master vers le slave */
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
    struct COM_ARCH com_arch;                                                                      /* Com avec le thread ARCH */

    JsonNode *Maps_root;                                                                   /* Json Array de tous les mappings */
    GTree *Maps_from_thread;                                                          /* GTree des mappings thread vers local */
    GTree *Maps_to_thread;                                                            /* GTree des mappings local vers thread */

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
 extern void MSRV_Remap( void );                                                                          /* Dans Watchdogd.c */

 extern struct PARTAGE *Shm_init ( void );                                                                      /* Dans shm.c */
 extern gboolean Shm_stop ( struct PARTAGE *partage );

 extern void Stopper_fils ( void );                                                                         /* Dans process.c */
 extern gboolean Demarrer_dls ( void );
 extern gboolean Demarrer_http ( void );
 extern gboolean Demarrer_arch ( void );
 extern void Charger_librairies ( void );
 extern void Decharger_librairies ( void );
 extern void SubProcess_send_comm_to_master_new ( struct SUBPROCESS *module, gboolean etat );
 extern void SubProcess_init ( struct SUBPROCESS *module, gint sizeof_vars );
 extern void SubProcess_end ( struct SUBPROCESS *module );

 extern void Gerer_arrive_Axxx_dls ( void );                                                         /* Dans distrib_Events.c */

 extern void Gerer_arrive_MSGxxx_dls ( void );                                                       /* Dans distrib_MSGxxx.c */
 extern void Convert_libelle_dynamique ( gchar *local_tech_id, gchar *libelle, gint taille_max );

 extern void Gerer_arrive_Ixxx_dls ( void );                                                           /* Dans distrib_Ixxx.c */

 extern gboolean Send_mail ( gchar *sujet, gchar *dest, gchar *body );                                         /* dans mail.c */

 extern gboolean MSRV_Map_to_thread ( JsonNode *key );
 extern gboolean MSRV_Map_from_thread ( JsonNode *key );

 extern void UUID_New ( gchar *target );                                                                       /* Dans uuid.c */
 extern void UUID_Load ( gchar *thread, gchar *target );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
