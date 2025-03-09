/******************************************************************************************************************************/
/* Watchdogd/Include/Dls.h                  Définitions des constantes programme DLS                                          */
/* Projet Abls-Habitat version 4.3                                                              sam 09 oct 2004 10:10:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Dls.h
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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

 #ifndef _DLS_H_
  #define _DLS_H_

 #include "Module_dls.h"

 #define NBR_CARAC_TECHID     32
 #define NBR_CARAC_ACRONYME   64
 #define NBR_CARAC_UNITE      32

 struct DLS_PLUGIN
  { gchar name[128];
    gchar shortname[ NBR_CARAC_ACRONYME ];
    gchar tech_id[NBR_CARAC_TECHID];
    gchar package[130];
    guint syn_id;                                           /* Numéro du fichier syn correspondant(pas l'index dans la table) */
    guint dls_id;
    gboolean enable;                                                                   /* TRUE si le plugin doit etre executé */
    gint  debug_time;                                                                                       /* Temps de debug */

    GSList *Dls_data_BI;                                                                               /* Liste des bistables */
    GSList *Dls_data_MONO;                                                                           /* Liste des monostables */
    GSList *Dls_data_DI;                                                                  /* Liste des entrees dynamiques TOR */
    GSList *Dls_data_DO;                                                                  /* Liste des sorties dynamiques TOR */
    GSList *Dls_data_AI;                                                                  /* Liste des entrees dynamiques ANA */
    GSList *Dls_data_AO;                                                                  /* Liste des sorties dynamiques ANA */
    GSList *Dls_data_MSG;                                                                               /* Liste des messages */
    GSList *Dls_data_CI;                                                                  /* Liste des compteurs d'impulsions */
    GSList *Dls_data_CH;                                                                      /* Liste des compteurs horaires */
    GSList *Dls_data_TEMPO;                                                                               /* Liste des tempos */
    GSList *Dls_data_VISUEL;                                                                    /* Liste des visuels (bits I) */
    GSList *Dls_data_REGISTRE;                                                                /* Liste des registres (bits R) */
    GSList *Dls_data_WATCHDOG;                                                                /* Liste des registres (bits R) */
    GSList *Dls_data_MESSAGE;                                                                /* Liste des messages (bits MSG) */
    GSList *Dls_data_HORLOGE;                                                            /* Liste des horloges (bits HORLOGE) */
    GSList *Thread_tech_ids;                                                             /* Liste des tech_id des dependances */

    time_t start_date;                                                                  /* time_t date de demarrage du plugin */
    void *handle;                                                                              /* Handle du fichier librairie */
    void (*go)(struct DLS_TO_PLUGIN *);                                                   /* Fonction de traitement du module */
    gdouble conso;                                                                       /* Consommation temporelle du plugin */
    gchar *(*version)(void);                                                       /* Retourne le numéro de version du plugin */
    void (*remap_all_alias)(struct DLS_TO_PLUGIN *);                                                 /* Set all Alias to NULL */
    void (*init_visuels)(struct DLS_TO_PLUGIN *);                                                    /* Set all Alias to NULL */
    struct DLS_TO_PLUGIN vars;
    GSList *Arbre_Comm;                         /* Liste tech_id des dependances du module pour le calcul de sa communication */
  };

 enum
  { MSG_ETAT,                                                                            /* Definitions des types de messages */
    MSG_ALERTE,
    MSG_DEFAUT,
    MSG_ALARME,
    MSG_VEILLE,
    MSG_NOTIF,
    MSG_DANGER,
    MSG_DERANGEMENT,
    NBR_TYPE_MSG
  };

 struct DLS_MESSAGE_EVENT
  { struct DLS_MESSAGE *msg;
    gboolean etat;
  };

 struct COM_DLS                                                                      /* Communication entre le serveur et DLS */
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    GSList *Dls_plugins;                                                                             /* Liste d'execution DLS */

    GSList *Set_Dls_DI_Edge_up;                                                /* liste des DIxxx a activer au debut tour prg */
    GSList *Set_Dls_DI_Edge_down;                                              /* liste des DIxxx a activer au debut tour prg */
    GSList *Set_Dls_MONO_Edge_up;                                               /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_MONO_Edge_down;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_BI_Edge_up;                                                 /* liste des Bxxx a activer au debut tour prg */
    GSList *Set_Dls_BI_Edge_down;                                               /* liste des Bxxx a activer au debut tour prg */
    GSList *Set_Dls_Data;                                                       /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_DI_Edge_up;                                               /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_DI_Edge_down;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_MONO_Edge_up;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_MONO_Edge_down;                                           /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_BI_Edge_up;                                               /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_BI_Edge_down;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_Data;                                               /* liste des Mxxx a désactiver à la fin du tour prg */
    GSList *HORLOGE_actives;                                         /* liste des HORLOGE actives au moment du tour programme */
    JsonNode *HORLOGE_ticks;                                                           /* Liste des horloges ticks a dérouler */

    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    guint temps_sched;                                          /* Delai d'attente DLS pour assurer 100 tours max par seconde */
    gboolean Top_check_horaire;                                                    /* True le controle horaire est réalisable */

    struct DLS_BI *sys_flipflop_5hz;
    struct DLS_BI *sys_flipflop_2hz;
    struct DLS_BI *sys_flipflop_1sec;
    struct DLS_BI *sys_flipflop_2sec;
    struct DLS_BI *sys_mqtt_connected;
    struct DLS_MONO *sys_top_5hz;
    struct DLS_MONO *sys_top_2hz;
    struct DLS_MONO *sys_top_1sec;
    struct DLS_MONO *sys_top_5sec;
    struct DLS_MONO *sys_top_10sec;
    struct DLS_MONO *sys_top_1min;
    struct DLS_AI *sys_bit_per_sec;
    struct DLS_AI *sys_tour_per_sec;
    struct DLS_AI *sys_dls_wait;
    struct DLS_AI *sys_maxrss;
  };

/************************************************ Prototypes de fonctions *****************************************************/
 extern void Dls_Importer_plugins ( void );                                                                 /* Dans plugins.c */
 extern struct DLS_PLUGIN *Dls_Importer_un_plugin ( gchar *tech_id );
 extern void Dls_Reseter_all_bit_interne ( struct DLS_PLUGIN *plugin );
 extern gboolean Dls_auto_create_plugin( gchar *tech_id, gchar *description, gchar *package );
 extern void Dls_Decharger_plugins ( void );
 extern void Dls_Debug_plugin ( gchar *tech_id, gboolean actif );
 extern void Dls_Activer_plugin ( gchar *tech_id, gboolean actif );
 extern void Dls_foreach_plugins ( gpointer user_data, void (*do_plugin) (gpointer user_data, struct DLS_PLUGIN *) );
 extern void Dls_Acquitter_plugin ( gchar *tech_id );
 struct DLS_PLUGIN *Dls_get_plugin_by_tech_id ( gchar *tech_id );
 extern void Dls_run_archivage ( gpointer user_data, struct DLS_PLUGIN *plugin );

 extern void Run_dls ( void );                                                                              /* Dans The_dls.c */

 extern void Prendre_heure ( void );                                                                          /* Dans heure.c */

 extern void Dls_Save_Data_to_API ( struct DLS_PLUGIN *plugin );                                       /* Dans The_Dls_data.c */

                                                                                                         /* Dans The_dls_CI.c */
 extern void Dls_data_CI_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_CI_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_CI_export_to_API ( struct DLS_CI *bit );
                                                                                                         /* Dans The_dls_CH.c */
 extern void Dls_data_CH_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_CH_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_CH_export_to_API ( struct DLS_CH *bit );
                                                                                                         /* Dans The_dls_AI.c */
 extern void Dls_data_AI_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_AI_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_AI_to_json ( JsonNode *element, struct DLS_AI *bit );
 extern void Dls_data_set_AI ( struct DLS_AI *bit, gdouble valeur, gboolean in_range );
 extern gboolean Dls_data_set_AI_from_thread_ai ( JsonNode *request );
 extern void Dls_AI_export_to_API ( struct DLS_AI *bit );

                                                                                                         /* Dans The_dls_AO.c */
 extern void Dls_data_AO_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_AO_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_AO_to_json ( JsonNode *element, struct DLS_AO *bit );
 extern void Dls_AO_export_to_API ( struct DLS_AO *bit );
                                                                                                      /* Dans The_dls_TEMPO.c */
 extern void Dls_data_TEMPO_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_TEMPO_to_json ( JsonNode *element, struct DLS_TEMPO *bit );

                                                                                                   /* Dans The_dls_REGISTRE.c */
 extern void Dls_data_REGISTRE_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_REGISTRE_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_REGISTRE_export_to_API ( struct DLS_REGISTRE *bit );
                                                                                                         /* Dans The_dls_DI.c */
 extern void Dls_data_DI_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_DI_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_DI_to_json ( JsonNode *element, struct DLS_DI *bit );
 extern void Dls_data_set_DI ( struct DLS_DI *bit, gboolean valeur );
 extern gboolean Dls_data_set_DI_from_thread_di ( JsonNode *request );
 extern void Dls_DI_export_to_API ( struct DLS_DI *bit );
                                                                                                         /* Dans The_dls_DO.c */
 extern void Dls_data_DO_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_DO_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_DO_to_json ( JsonNode *element, struct DLS_DO *bit );
 extern void Dls_DO_export_to_API ( struct DLS_DO *bit );
                                                                                                       /* Dans The_dls_MONO.c */
 extern void Dls_data_MONO_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_MONO_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_MONO_export_to_API ( struct DLS_MONO *bit );
                                                                                                         /* Dans The_dls_BI.c */
 extern void Dls_data_BI_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_all_BI_to_json ( gpointer array, struct DLS_PLUGIN *plugin );
 extern void Dls_BI_export_to_API ( struct DLS_BI *bit );
                                                                                                    /* Dans The_dls_HORLOGE.c */
 extern void Dls_data_clear_HORLOGE ();
 extern void Dls_data_activer_horloge ( void );
 extern void Dls_Load_horloge_ticks ( void );
                                                                                                     /* Dans The_dls_VISUEL.c */
 extern void Dls_data_VISUEL_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_VISUEL_to_json ( JsonNode *RootNode, struct DLS_VISUEL *bit );

                                                                                                    /* Dans The_dls_MESSAGE.c */
 extern void Dls_data_MESSAGE_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_data_MESSAGE_free_all ( struct DLS_PLUGIN *plugin );
 extern void Dls_MESSAGE_to_json ( JsonNode *element, struct DLS_MESSAGE *bit );

                                                                                                   /* Dans The_dls_WATCHDOG.c */
 extern void Dls_data_WATCHDOG_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data );
 extern void Dls_WATCHDOG_to_json ( JsonNode *element, struct DLS_WATCHDOG *bit );
 extern gboolean Dls_data_set_WATCHDOG_from_thread_watchdog ( JsonNode *request );
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
