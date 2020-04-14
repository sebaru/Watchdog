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

 #include "Reseaux.h"
 #include "Db.h"
 #include "Module_dls.h"

 #define NOM_TABLE_DLS         "dls"

 #define NBR_BIT_BISTABLE_RESERVED     40                      /* Nombre de bits bistables reservés pour le système B00 - B39 */

 struct PLUGIN_DLS
  { struct CMD_TYPE_PLUGIN_DLS plugindb;
    gchar nom_fichier[60];                                                                                  /* Nom du fichier */
    time_t start_date;                                                                  /* time_t date de demarrage du plugin */
    void *handle;                                                                              /* Handle du fichier librairie */
    void (*go)(struct DLS_TO_PLUGIN *);                                                   /* Fonction de traitement du module */
    float conso;                                                                         /* Consommation temporelle du plugin */
    gchar *(*version)(void);                                                       /* Retourne le numéro de version du plugin */
    struct DLS_TO_PLUGIN vars;
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
  { gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
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
  { gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gfloat  min;
    gfloat  max;
    guint   type;                                                                                  /* Type de gestion de l'EA */
    gchar   unite[NBR_CARAC_UNITE_MNEMONIQUE_UTF8+1];                                                         /* Km, h, ° ... */
    gfloat  val_ech;
    gfloat  val_avant_ech;
    guint   last_arch;                                                                         /* Date de la derniere archive */
    guint   inrange;
  };

 struct DLS_AO
  { gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gfloat  min;
    gfloat  max;
    guint   type;                                                                                  /* Type de gestion de l'EA */
    gchar   unite[NBR_CARAC_UNITE_MNEMONIQUE_UTF8+1];                                                         /* Km, h, ° ... */
    gfloat  val_ech;
    gfloat  val_avant_ech;
    guint   last_arch;                                                                         /* Date de la derniere archive */
  };

 struct DIGITAL_INPUT
  { gboolean etat;
  };

 struct ANALOG_INPUT
  { struct CMD_TYPE_MNEMO_AI confDB;
    gfloat  val_ech;
    gfloat  val_avant_ech;
    guint   last_arch;                                                                         /* Date de la derniere archive */
    guint   inrange;
  };

 struct SORTIE_TOR                                                                             /* Définition d'une sortie TOR */
  { gchar etat;                                                                                   /* Etat de la sortie 0 ou 1 */
    gint last_change;                                                                    /* Date du dernier changement d'etat */
  };

 struct DLS_BOOL
  { gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_DI
  { gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_DO
  { gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_CI
  { gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gint    valeur;
    gint    val_en_cours1;                                                    /* valeur en cours pour le calcul via les ratio */
    gfloat  ratio;
    gfloat  multi;
    guint   last_update;
    gint    imp_par_minute;
    gint    valeurs[60];                                                                              /* 60 dernieres valeurs */
    gchar   unite[32];
    gboolean etat;
  };

 struct DLS_CH
  { gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    guint valeur;
    guint last_arch;                                                     /* Date de dernier enregistrement en base de données */
    guint old_top;                                                                         /* Date de debut du comptage du CH */
    gboolean etat;
  };

 struct DLS_VISUEL
  { gchar    tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gchar    acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gint     mode;
    gchar    color[16];
    gboolean cligno;
    gint     last_change;
    gint     changes;
  };

 struct DLS_MESSAGES
  { gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gboolean etat;
    gint last_change;
    gint changes;
  };

 struct DLS_MESSAGES_EVENT
  { struct DLS_MESSAGES *msg;
    gboolean etat;
  };

 struct DLS_REGISTRE
  { gchar    tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gchar    acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gfloat   valeur;
    gboolean archivage;
    gchar   unite[32];
    guint last_arch;                                                     /* Date de dernier enregistrement en base de données */
  };

 struct I_MOTIF
  { gint etat;
    gint rouge;
    gint vert;
    gint bleu;
    gint cligno;
    gint last_change;
    gint changes;
  };

 struct DLS_TREE
  { struct CMD_TYPE_SYN_VARS syn_vars;
    GSList *Liste_plugin_dls;                                                /* Liste des plugins D.L.S associé au synoptique */
    GSList *Liste_dls_tree;                                               /* Liste des sous_synoptiques associés au synoptique */
  };

 struct COM_DLS                                                                      /* Communication entre le serveur et DLS */
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    pthread_mutex_t synchro_traduction;                  /* Mutex pour interdire les traductions simultanées de plugins D.L.S */
    struct DLS_TREE *Dls_tree;                                                                       /* Arbre d'execution DLS */
    pthread_mutex_t synchro_data;                                      /* Mutex pour les acces concurrents à l'arbre des data */
    struct ZMQUEUE *zmq_to_master;
    GSList *Set_M;                                                              /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_M;                                                      /* liste des Mxxx a désactiver à la fin du tour prg */
    GSList *Set_Dls_Data;                                                       /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_Data;                                               /* liste des Mxxx a désactiver à la fin du tour prg */

    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_debug;                                                             /* TRUE si le thread doit tout logguer */
    gboolean Thread_reload;                                              /* TRUE si le thread doit recharger sa configuration */
    gboolean Compil_at_boot;                                            /* True si DLS doit compiler les plugins au démarrage */
    guint admin_start;                                                                              /* Demande de deconnexion */
    guint admin_stop;                                                                               /* Demande de deconnexion */
    guint temps_sched;                                          /* Delai d'attente DLS pour assurer 100 tours max par seconde */
    gboolean Top_check_horaire;                                                    /* True le controle horaire est réalisable */
  };

/************************************************ Prototypes de fonctions *****************************************************/
 extern gboolean Retirer_plugin_dlsDB( struct CMD_TYPE_PLUGIN_DLS *dls );                                    /* Dans Dls_db.c */
 extern gint Ajouter_plugin_dlsDB( struct CMD_TYPE_PLUGIN_DLS *dls );
 extern gboolean Recuperer_plugins_dlsDB( struct DB **db );
 extern gboolean Recuperer_plugins_dlsDB_by_syn( struct DB **db_retour, gint syn_id );
 extern struct CMD_TYPE_PLUGIN_DLS *Recuperer_plugins_dlsDB_suite( struct DB **db );
 extern struct CMD_TYPE_PLUGIN_DLS *Rechercher_plugin_dlsDB( gint id );
 extern gboolean Modifier_plugin_dlsDB( struct CMD_TYPE_PLUGIN_DLS *dls );
 extern gboolean Set_compil_status_plugin_dlsDB( gint id, gint status, gchar *log_buffer );
 extern gboolean Get_source_dls_from_DB ( gint id, gchar **result_buffer, gint *result_taille );
 extern gboolean Save_source_dls_to_DB( gint id, gchar *buffer, gint taille );
 extern gboolean Dls_auto_create_plugin( gchar *tech_id, gchar *nom );

 extern void Charger_plugins ( void );                                                                      /* Dans plugins.c */
 extern void Decharger_plugins ( void );
 extern void Decharger_plugin_by_id ( gint id );
 extern gint Compiler_source_dls( gboolean reset, gint id, gchar *buffer, gint taille_buffer );
 extern void Activer_plugin_by_id ( gint id, gboolean actif );
 extern void Reseter_un_plugin ( gint id );                                                                 /* Dans plugins.c */

 extern void Run_dls ( void );                                                                              /* Dans The_dls.c */
 extern int A( int num );
 extern int EA_inrange( int num );
 extern void SB_SYS( int num, int etat );
 extern void SE( int num, int etat );
 extern void Dls_data_set_R  ( gchar *tech_id, gchar *acronyme, gpointer *r_p, float val );
 extern void Dls_data_set_AI ( gchar *tech_id, gchar *acronyme, gpointer *ai_p, float val_avant_ech, gboolean in_range );
 extern void Dls_data_set_DI ( gchar *tech_id, gchar *acronyme, gpointer *di_p, gboolean valeur );
 extern gboolean Dls_data_get_MSG ( gchar *tech_id, gchar *acronyme, gpointer *msg_p );
 extern gboolean Dls_data_get_DO ( gchar *tech_id, gchar *acronyme, gpointer *dout_p );
 extern gboolean Dls_data_get_DO_up   ( gchar *tech_id, gchar *acronyme, gpointer *bool_p );
 extern gboolean Dls_data_get_DO_down ( gchar *tech_id, gchar *acronyme, gpointer *bool_p );
 extern gint Dls_data_get_VISUEL ( gchar *tech_id, gchar *acronyme, gpointer *visu_p );
 extern void SEA( int num, float val_avant_ech );
 extern void SEA_range( int num, int range );
 extern void SEA_ech( int num, float val_ech );
 extern void Envoyer_commande_dls ( int num );
 extern void Envoyer_commande_dls_data ( gchar *tech_id, gchar *acronyme );
 extern void Dls_foreach ( void *user_data,
                           void (*do_plugin) (void *user_data, struct PLUGIN_DLS *),
                           void (*do_tree)   (void *user_data, struct DLS_TREE *) );

 extern void Prendre_heure ( void );                                                                          /* Dans heure.c */
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
