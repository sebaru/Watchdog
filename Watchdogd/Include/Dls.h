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
  { gchar nom[ NBR_CARAC_PLUGIN_DLS_UTF8 + 1 ];
    gchar nom_fichier[60];                                                              /* Nom du fichier */
    guint id;                                                                     /* Numero du plugin DLS */
    guint on;
    gint starting;                  /* 1 si les bits internes "start" du plugins doivent etre positionnés */
    void *handle;                                                          /* Handle du fichier librairie */
    void (*go)(int);                                                  /* Fonction de traitement du module */
    float conso;                                                     /* Consommation temporelle du plugin */
  };


 struct COM_DLS                                             /* Communication entre le serveur et DLS */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *Plugins;                                                   /* Liste des plugins chargés de DLS */
    GList *liste_m;                                                           /* liste des Mxxx a activer */
    GList *liste_plugin_reset;                                            /* liste des plugins a resetter */
    gboolean reload;
    guint admin_start;                                                          /* Demande de deconnexion */
    guint admin_stop;                                                           /* Demande de deconnexion */
    guint temps_sched;
  };

/*************************************** Prototypes de fonctions ******************************************/
 extern gboolean Retirer_plugin_dlsDB( struct LOG *log, struct DB *db, struct CMD_TYPE_PLUGIN_DLS *dls );
 extern gint Ajouter_plugin_dlsDB( struct LOG *log, struct DB *db, struct CMD_TYPE_PLUGIN_DLS *dls );
 extern gboolean Recuperer_plugins_dlsDB( struct LOG *log, struct DB *db );
 extern struct PLUGIN_DLS *Recuperer_plugins_dlsDB_suite( struct LOG *log, struct DB *db );
 extern struct PLUGIN_DLS *Rechercher_plugin_dlsDB( struct LOG *log, struct DB *db, gint id );
 extern gboolean Modifier_plugin_dlsDB( struct LOG *log, struct DB *db, struct CMD_TYPE_PLUGIN_DLS *dls );

 extern void Reseter_un_plugin ( gint id );
 extern void Decharger_un_plugin_by_id ( gint id );
 extern void Decharger_plugins ( void );
 extern void Charger_plugins ( void );
 extern void Activer_plugin_by_id ( gint id, gboolean actif );

 extern void Run_dls ( void );                                                          /* Dans The_dls.c */
 extern int EA_inrange( int num );
 extern double EA_ech( int num );
 extern int A( int num );
 extern char *TRdetail( int num );
 extern void SE( int num, int etat );
 extern void SEA( int num, int val_int, int inrange );
 
 extern void Prendre_heure ( void );                                                      /* Dans heure.c */ 
 #endif
/*--------------------------------------------------------------------------------------------------------*/
