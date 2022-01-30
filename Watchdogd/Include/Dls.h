/******************************************************************************************************************************/
/* Watchdogd/Include/Dls.h                  Définitions des constantes programme DLS                                          */
/*        projet Watchdog v2.0     par LEFEVRE Sebastien                                        sam 09 oct 2004 10:10:32 CEST */
/******************************************************************************************************************************/
/*
 * Dls.h
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

 #ifndef _DLS_H_
  #define _DLS_H_

 #include "Module_dls.h"

 #define NOM_TABLE_DLS         "dls"
 #define NBR_CARAC_TECHID      32
 #define NBR_CARAC_ACRONYME    64

 enum
  { MNEMO_BISTABLE,                                                                   /* Definitions des types de mnemoniques */
    MNEMO_MONOSTABLE,
    MNEMO_TEMPO,
    MNEMO_ENTREE,
    MNEMO_SORTIE,
    MNEMO_ENTREE_ANA,
    MNEMO_SORTIE_ANA,
    MNEMO_VISUEL,
    MNEMO_CPTH,
    MNEMO_CPT_IMP,
    MNEMO_REGISTRE,
    MNEMO_HORLOGE,
    MNEMO_MSG,
    MNEMO_BUS,
    MNEMO_DIGITAL_OUTPUT,
    MNEMO_WATCHDOG,
    NBR_TYPE_MNEMO
  };

 enum                                                                                  /* Code retour de la compilation D.L.S */
  { DLS_COMPIL_NEVER_COMPILED_YET,
    DLS_COMPIL_EXPORT_DB_FAILED,
    DLS_COMPIL_ERROR_LOAD_SOURCE,
    DLS_COMPIL_ERROR_LOAD_LOG,
    DLS_COMPIL_SYNTAX_ERROR,
    DLS_COMPIL_ERROR_FORK_GCC,
    DLS_COMPIL_OK,
    DLS_COMPIL_OK_WITH_WARNINGS,
    NBR_DLS_COMPIL_STATUS
  };

 #define NBR_CARAC_TECHID     32
 #define NBR_CARAC_ACRONYME   64
 #define NBR_CARAC_UNITE      32

 struct DLS_PLUGIN
  { gchar nom[128];
    gchar shortname[ NBR_CARAC_ACRONYME ];
    gchar tech_id[NBR_CARAC_TECHID];
    gchar package[130];
    gchar syn_parent_page[NBR_CARAC_TECHID];
    gchar syn_page[NBR_CARAC_TECHID];
    guint syn_id;                                           /* Numéro du fichier syn correspondant(pas l'index dans la table) */
    guint id;
    gboolean on;
    gchar compil_date[32];                                                                    /* Date de derniere compilation */
    guint compil_status;                                                                    /* Statut de derniere compilation */
    guint nbr_compil;                                                                         /* Nombre de compilation totale */
    guint nbr_ligne;                                                                               /* Nombre de ligne de code */
    gboolean debug;                                                                                /* Nombre de ligne de code */
    gboolean is_thread;

    gchar nom_fichier[60];                                                                                  /* Nom du fichier */
    time_t start_date;                                                                  /* time_t date de demarrage du plugin */
    void *handle;                                                                              /* Handle du fichier librairie */
    void (*go)(struct DLS_TO_PLUGIN *);                                                   /* Fonction de traitement du module */
    gdouble conso;                                                                       /* Consommation temporelle du plugin */
    gchar *(*version)(void);                                                       /* Retourne le numéro de version du plugin */
    struct DLS_TO_PLUGIN vars;
    GSList *Arbre_Comm;                         /* Liste tech_id des dependances du module pour le calcul de sa communication */
  };

 enum                                                                                  /* différent statut des temporisations */
  { DLS_TEMPO_NOT_COUNTING,                                                                 /* La tempo ne compte pas du tout */
    DLS_TEMPO_WAIT_FOR_DELAI_ON,                                       /* La tempo compte, en attendant le delai de mise à un */
    DLS_TEMPO_WAIT_FOR_MIN_ON,                                         /* Delai de MAU dépassé, en attente du creneau minimum */
    DLS_TEMPO_WAIT_FOR_MAX_ON,                                      /* Creneau minimum atteint, en attente du creneau maximum */
    DLS_TEMPO_WAIT_FOR_DELAI_OFF,                                /* Creneau max atteint, en attente du delai de remise a zero */
    DLS_TEMPO_WAIT_FOR_COND_OFF                                            /* Attend que la condition soit tombée avant reset */
  };

 struct DLS_TEMPO                                                                           /* Définition d'une temporisation */
  { gchar   acronyme[NBR_CARAC_ACRONYME];
    gchar   tech_id[NBR_CARAC_TECHID];
    gboolean init;                                   /* True si les données delai_on/off min_on/off ont bien été positionnées */
    guint status;                                                                               /* Statut de la temporisation */
    guint date_on;                                                              /* date a partir de laquelle la tempo sera ON */
    guint date_off;                                                            /* date a partir de laquelle la tempo sera OFF */
    gboolean state;
    guint delai_on;                                                     /* delai avant mise à un (fixé par option mnémonique) */
    guint delai_off;                                                  /* delai avant mise à zero (fixé par option mnémonique) */
    guint min_on;                            /* Durée minimale pendant laquelle la tempo sera ON (fixé par option mnémonique) */
    guint max_on;                            /* Durée maximale pendant laquelle la tempo sera ON (fixé par option mnémonique) */
    guint random;                                         /* Est-ce une tempo random ? si oui, est la dynamique max du random */
  };

 struct DLS_AI
  { gchar   acronyme[NBR_CARAC_ACRONYME];
    gchar   tech_id[NBR_CARAC_TECHID];
    gdouble min;
    gdouble max;
    guint   type;                                                                                  /* Type de gestion de l'EA */
    gchar   unite[NBR_CARAC_UNITE];                                                                                        /* Km, h, ° ... */
    gdouble valeur;
    guint   last_arch;                                                                         /* Date de la derniere archive */
    guint   inrange;
    guint   archivage;
   };

 struct DLS_AO
  { gchar   acronyme[NBR_CARAC_ACRONYME];
    gchar   tech_id[NBR_CARAC_TECHID];
    gdouble min;
    gdouble max;
    guint   type;                                                                                  /* Type de gestion de l'EA */
    gchar   unite[NBR_CARAC_UNITE];                                                                           /* Km, h, ° ... */
    gdouble valeur;
    guint   last_arch;                                                                         /* Date de la derniere archive */
  };

 struct SORTIE_TOR                                                                             /* Définition d'une sortie TOR */
  { gchar etat;                                                                                   /* Etat de la sortie 0 ou 1 */
    gint last_change;                                                                    /* Date du dernier changement d'etat */
  };

 struct DLS_WATCHDOG
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gint    top;
  };

 struct DLS_MONO
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gboolean etat;                                                                                      /* Etat actuel du bit */
    gboolean next_etat;                                                                       /*prochain etat calculé par DLS */
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_BI
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gint    groupe; /* Groupe 'radio' */
    gboolean etat;                                                                                      /* Etat actuel du bit */
    gboolean next_etat;                                                                       /*prochain etat calculé par DLS */
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_DI
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_DO
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_CI
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gint    valeur;
    gint    val_en_cours1;                                                    /* valeur en cours pour le calcul via les ratio */
    gdouble ratio;
    gdouble multi;
    guint   last_update;
    gint    imp_par_minute;
    gint    valeurs[60];                                                                              /* 60 dernieres valeurs */
    gchar   unite[NBR_CARAC_UNITE];
    gboolean etat;
    gint    archivage;
    guint   last_arch;
  };

 struct DLS_CH
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    guint valeur;
    guint last_arch;                                                     /* Date de dernier enregistrement en base de données */
    guint old_top;                                                                         /* Date de debut du comptage du CH */
    gboolean etat;
  };

 struct DLS_VISUEL
  { gchar    tech_id[NBR_CARAC_TECHID];
    gchar    acronyme[NBR_CARAC_ACRONYME];
    gchar    mode[32];
    gchar    color[16];
    gboolean cligno;
    gint     last_change;
    gint     changes;
    gchar    libelle[128]; /* libelle issu du plugin DLS */
  };

 struct DLS_MESSAGES
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gboolean etat;
    gboolean etat_update;
    gint groupe;
    gint last_change;
    gint last_on;
    gint changes;
  };

 struct DLS_MESSAGES_EVENT
  { struct DLS_MESSAGES *msg;
    gboolean etat;
  };

 struct DLS_REGISTRE
  { gchar   tech_id[NBR_CARAC_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME];
    gdouble valeur;
    gchar   unite[NBR_CARAC_UNITE];
    gint    archivage;
    guint   last_arch;                                                   /* Date de dernier enregistrement en base de données */
    gdouble pid_somme_erreurs;                                                                                /* Calcul PID KI*/
    gdouble pid_prev_erreur;                                                                                 /* Calcul PID KD */
  };

 struct DLS_SYN
  { gint syn_id;
    gboolean bit_comm;
    gboolean bit_defaut;
    gboolean bit_defaut_fixe;
    gboolean bit_alarme;
    gboolean bit_alarme_fixe;
    gboolean bit_veille_partielle;
    gboolean bit_veille_totale;
    gboolean bit_alerte;
    gboolean bit_alerte_fixe;
    gboolean bit_alerte_fugitive;
    gboolean bit_derangement;
    gboolean bit_derangement_fixe;
    gboolean bit_danger;
    gboolean bit_danger_fixe;
    GSList *Dls_plugins;                                                     /* Liste des plugins D.L.S associé au synoptique */
    GSList *Dls_sub_syns;                                                    /* Liste des plugins D.L.S associé au synoptique */
  };

 struct COM_DLS                                                                      /* Communication entre le serveur et DLS */
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    pthread_mutex_t synchro_traduction;                  /* Mutex pour interdire les traductions simultanées de plugins D.L.S */
    pthread_mutex_t synchro_data;                                      /* Mutex pour les acces concurrents à l'arbre des data */
    GSList *Dls_plugins;                                                                             /* Liste d'execution DLS */
    struct DLS_SYN *Dls_syns;                                                              /* Arbre de calcule des etats */
    struct ZMQUEUE *zmq_to_master;
    GSList *Set_Dls_DI_Edge_up;                                                 /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_DI_Edge_down;                                               /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_MONO_Edge_up;                                               /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_MONO_Edge_down;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_BI_Edge_up;                                               /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_BI_Edge_down;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Set_Dls_Data;                                                       /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_DI_Edge_up;                                               /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_DI_Edge_down;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_MONO_Edge_up;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_MONO_Edge_down;                                           /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_BI_Edge_up;                                             /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_BI_Edge_down;                                           /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_Data;                                               /* liste des Mxxx a désactiver à la fin du tour prg */

    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_debug;                                                             /* TRUE si le thread doit tout logguer */
    gboolean Thread_reload;                                              /* TRUE si le thread doit recharger sa configuration */
    gboolean Thread_reload_with_recompil;                       /* TRUE si le thread doit rebooter en recompilant les modules */
    guint temps_sched;                                          /* Delai d'attente DLS pour assurer 100 tours max par seconde */
    gboolean Top_check_horaire;                                                    /* True le controle horaire est réalisable */
  };

/************************************************ Prototypes de fonctions *****************************************************/
 extern gboolean Recuperer_plugins_dlsDB( struct DB **db );                                                  /* Dans Dls_db.c */
 extern struct DLS_PLUGIN *Recuperer_plugins_dlsDB_suite( struct DB **db );
 extern struct DLS_PLUGIN *Rechercher_plugin_dlsDB( gchar *tech_id_src );
 extern gboolean Set_compil_status_plugin_dlsDB( gchar *tech_id_src, gint status, gchar *log_buffer );
 extern gboolean Get_source_dls_from_DB ( gchar *tech_id_src, gchar **result_buffer, gint *result_taille );
 extern gboolean Save_source_dls_to_DB( gchar *tech_id, gchar *buffer, gint taille );
 extern gboolean Dls_auto_create_plugin( gchar *tech_id, gchar *nom );

 extern void Dls_Charger_plugins ( gboolean compil );                                                       /* Dans plugins.c */
 extern void Dls_Decharger_plugins ( void );
 extern void Dls_Debug_plugin ( gchar *tech_id, gboolean actif );
 extern void Dls_Activer_plugin ( gchar *tech_id, gboolean actif );
 extern void Dls_Reseter_un_plugin ( gchar *tech_id );
 extern void Dls_foreach_plugins ( gpointer user_data, void (*do_plugin) (gpointer user_data, struct DLS_PLUGIN *) );
 extern void Dls_foreach_syns ( gpointer user_data, void (*do_syn)(gpointer user_data, struct DLS_SYN *) );
 extern void Dls_recalculer_arbre_comm ( void );
 extern void Dls_arbre_dls_syn_erase ( void );
 extern void Dls_recalculer_arbre_dls_syn ( void );
 extern void Dls_acquitter_plugin ( gchar *tech_id );
 extern void Dls_acquitter_synoptique ( gint syn_id );
 extern struct DLS_SYN *Dls_search_syn ( gint id );

 extern void Run_dls ( void );                                                                              /* Dans The_dls.c */
 extern void Dls_data_set_AI ( gchar *tech_id, gchar *acronyme, gpointer *ai_p, gdouble valeur, gboolean in_range );
 extern void Dls_data_set_DI ( struct DLS_TO_PLUGIN *vars, gchar *tech_id, gchar *acronyme, gpointer *di_p, gboolean valeur );
 extern struct DLS_BI *Dls_data_BI_lookup ( gchar *tech_id, gchar *acronyme );
 extern struct DLS_MONO *Dls_data_MONO_lookup ( gchar *tech_id, gchar *acronyme );
 extern struct DLS_MESSAGES *Dls_data_MSG_lookup ( gchar *tech_id, gchar *acronyme );
 extern gboolean Dls_data_get_MSG ( gchar *tech_id, gchar *acronyme, gpointer *msg_p );
 extern gboolean Dls_data_get_DO ( gchar *tech_id, gchar *acronyme, gpointer *dout_p );
 extern gboolean Dls_data_get_DO_up   ( gchar *tech_id, gchar *acronyme, gpointer *bool_p );
 extern gboolean Dls_data_get_DO_down ( gchar *tech_id, gchar *acronyme, gpointer *bool_p );
 extern void Envoyer_commande_dls_data ( gchar *tech_id, gchar *acronyme );
 extern void Dls_syn_vars_to_json ( gpointer user_data, struct DLS_SYN *tree );

 extern void Prendre_heure ( void );                                                                          /* Dans heure.c */
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
