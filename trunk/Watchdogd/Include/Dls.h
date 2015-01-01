/**********************************************************************************************************/
/* Watchdogd/Include/Dls.h                  Définitions des constantes programme DLS                      */
/*        projet Watchdog v2.0     par LEFEVRE Sebastien                    sam 09 oct 2004 10:10:32 CEST */
/**********************************************************************************************************/
/*
 * Dls.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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

 struct PLUGIN_DLS
  { struct CMD_TYPE_PLUGIN_DLS plugindb;
    gchar nom_fichier[60];                                                              /* Nom du fichier */
    gint starting;                  /* 1 si les bits internes "start" du plugins doivent etre positionnés */
    void *handle;                                                          /* Handle du fichier librairie */
    void (*go)(int);                                                  /* Fonction de traitement du module */
    float conso;                                                     /* Consommation temporelle du plugin */
  };

 enum
  { DLS_COMPIL_ERROR_LOAD_SOURCE,
    DLS_COMPIL_ERROR_LOAD_LOG,
    DLS_COMPIL_ERROR_TRAD,
    DLS_COMPIL_ERROR_FORK_GCC,
    DLS_COMPIL_OK_WITH_WARNINGS,
    DLS_COMPIL_OK
  };

 enum                                                              /* différent statut des temporisations */
  { TEMPO_NOT_COUNTING,                                                 /* La tempo ne compte pas du tout */
    TEMPO_WAIT_FOR_DELAI_ON,                       /* La tempo compte, en attendant le delai de mise à un */
    TEMPO_WAIT_FOR_MIN_ON,                         /* Delai de MAU dépassé, en attente du creneau minimum */
    TEMPO_WAIT_FOR_MAX_ON,                      /* Creneau minimum atteint, en attente du creneau maximum */
    TEMPO_WAIT_FOR_DELAI_OFF,                /* Creneau max atteint, en attente du delai de remise a zero */
    TEMPO_WAIT_FOR_COND_OFF                           /* Atteint que la condition soit tombée avant reset */
  };

 struct TEMPO                                                           /* Définition d'une temporisation */
  { struct CMD_TYPE_OPTION_TEMPO option_tempo;
                                                                            /* Variables de travail (run) */
    guint status;                                                           /* Statut de la temporisation */
    guint date_on;                                          /* date a partir de laquelle la tempo sera ON */
    guint date_off;                                        /* date a partir de laquelle la tempo sera OFF */
    gboolean state;
  };

 struct COM_DLS                                             /* Communication entre le serveur et DLS */
  { pthread_t TID;                                                               /* Identifiant du thread */
    pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    pthread_mutex_t synchro_ea_access;                /* Bit de synchro pour read/write les bits internes */
    GSList *Plugins;                                                  /* Liste des plugins chargés de DLS */
    GSList *Set_M;                                          /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_M;                                  /* liste des Mxxx a désactiver à la fin du tour prg */
    GSList *Set_E;                                          /* liste des Exxx a activer au debut tour prg */
    GSList *Reset_E;                                  /* liste des Exxx a désactiver à la fin du tour prg */
    GSList *liste_plugin_reset;                                           /* liste des plugins a resetter */
    gboolean Thread_run;                /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    gboolean Thread_sigusr1;                                      /* TRUE si le thread doit gerer le USR1 */
    guint admin_start;                                                          /* Demande de deconnexion */
    guint admin_stop;                                                           /* Demande de deconnexion */
    guint temps_sched;
  };

/*************************************** Prototypes de fonctions ******************************************/
 extern gboolean Retirer_plugin_dlsDB( struct CMD_TYPE_PLUGIN_DLS *dls );
 extern gint Ajouter_plugin_dlsDB( struct CMD_TYPE_PLUGIN_DLS *dls );
 extern gboolean Recuperer_plugins_dlsDB( struct DB **db );
 extern struct CMD_TYPE_PLUGIN_DLS *Recuperer_plugins_dlsDB_suite( struct DB **db );
 extern struct CMD_TYPE_PLUGIN_DLS *Rechercher_plugin_dlsDB( gint id );
 extern gboolean Modifier_plugin_dlsDB( struct CMD_TYPE_PLUGIN_DLS *dls );

 extern void Reseter_un_plugin ( gint id );
 extern void Decharger_un_plugin_by_id ( gint id );
 extern void Decharger_plugins ( void );
 extern void Charger_plugins ( void );
 extern void Activer_plugin_by_id ( gint id, gboolean actif );
 extern gint Compiler_source_dls( gboolean new, gboolean reset, gint id, gchar *buffer, gint taille_buffer );

 extern void Run_dls ( void );                                                          /* Dans The_dls.c */
 extern int EA_inrange( int num );
 extern float EA_ech( int num );
 extern int A( int num );
 extern char *Tdetail( int num );
 extern void SEA( int num, float val_avant_ech );
 extern void SEA_range( int num, int range );
 extern void SEA_ech( int num, float val_ech );
 extern void Envoyer_entree_dls( int num, int etat, gboolean furtif );
 extern void Envoyer_commande_dls ( int num );

 extern void Prendre_heure ( void );                                                      /* Dans heure.c */ 
 #endif
/*--------------------------------------------------------------------------------------------------------*/
