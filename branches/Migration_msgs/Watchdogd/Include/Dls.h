/******************************************************************************************************************************/
/* Watchdogd/Include/Dls.h                  D�finitions des constantes programme DLS                                          */
/*        projet Watchdog v2.0     par LEFEVRE Sebastien                                        sam 09 oct 2004 10:10:32 CEST */
/******************************************************************************************************************************/
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

 #define NBR_BIT_BISTABLE_RESERVED     40                      /* Nombre de bits bistables reserv�s pour le syst�me B00 - B39 */

 struct PLUGIN_DLS
  { struct CMD_TYPE_PLUGIN_DLS plugindb;
    gchar nom_fichier[60];                                                                                  /* Nom du fichier */
    time_t start_date;                                                                  /* time_t date de demarrage du plugin */
    void *handle;                                                                              /* Handle du fichier librairie */
    void (*go)(struct DLS_TO_PLUGIN *);                                                   /* Fonction de traitement du module */
    float conso;                                                                         /* Consommation temporelle du plugin */
    gint (*Get_Tableau_bit)(gint);                                             /* Fonction d'identification des bits utilis�s */
    gint (*Get_Tableau_num)(gint);                                             /* Fonction d'identification des bits utilis�s */
    gint (*Get_Tableau_msg)(gint);                                             /* Fonction d'identification des bits utilis�s */
    struct DLS_TO_PLUGIN vars;
  };

 enum                                                                                  /* diff�rent statut des temporisations */
  { DLS_TEMPO_NOT_COUNTING,                                                                 /* La tempo ne compte pas du tout */
    DLS_TEMPO_WAIT_FOR_DELAI_ON,                                       /* La tempo compte, en attendant le delai de mise � un */
    DLS_TEMPO_WAIT_FOR_MIN_ON,                                         /* Delai de MAU d�pass�, en attente du creneau minimum */
    DLS_TEMPO_WAIT_FOR_MAX_ON,                                      /* Creneau minimum atteint, en attente du creneau maximum */
    DLS_TEMPO_WAIT_FOR_DELAI_OFF,                                /* Creneau max atteint, en attente du delai de remise a zero */
    DLS_TEMPO_WAIT_FOR_COND_OFF                                            /* Attend que la condition soit tomb�e avant reset */
  };

 struct DLS_TEMPO                                                                           /* D�finition d'une temporisation */
  { struct CMD_TYPE_MNEMO_TEMPO confDB;
    gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    guint status;                                                                               /* Statut de la temporisation */
    guint date_on;                                                              /* date a partir de laquelle la tempo sera ON */
    guint date_off;                                                            /* date a partir de laquelle la tempo sera OFF */
    gboolean state;
  };

 struct DIGITAL_INPUT
  { gboolean etat;
  };

 struct ANALOG_INPUT
  { struct CMD_TYPE_MNEMO_AI confDB;
    gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gfloat  val_ech;
    gfloat  val_avant_ech;
    guint   last_arch;                                                                         /* Date de la derniere archive */
    guint   inrange;
  };

 struct CPT_IMP
  { struct CMD_TYPE_MNEMO_CPT_IMP confDB;
    gboolean actif;                                                                           /* M�morisation de l'etat du CI */
    gfloat val_en_cours1;                                                     /* valeur en cours pour le calcul via les ratio */
    gfloat val_en_cours2;                                         /* valeur en cours avant interpr�tation selon le type de CI */
    time_t last_update;                                                   /* date de derniere update de la valeur du compteur */
    guint last_arch;                                                     /* Date de dernier enregistrement en base de donn�es */
  };

 struct CPT_HORAIRE
  { struct CMD_TYPE_MNEMO_CPT_H confDB;
    guint last_arch;                                 /* Date de dernier enregistrement en base de donn�es */
    guint old_top;                                                     /* Date de debut du comptage du CH */
    gboolean actif;
  };

 struct SORTIE_TOR                                                                             /* D�finition d'une sortie TOR */
  { gchar etat;                                                                                   /* Etat de la sortie 0 ou 1 */
    gint last_change;                                                                    /* Date du dernier changement d'etat */
    gint changes;                               /* Compte le nombre de changes afin de ne pas depasser une limite par seconde */
  };

 struct DLS_BOOL
  { gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gboolean etat;
    gboolean edge_up;
    gboolean edge_down;
  };

 struct DLS_MESSAGES
  { gchar   acronyme[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gchar   tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gboolean etat;
    gint last_change;
    gint changes;
    gint next_repeat;
    /*gboolean persist;*/
  };

 struct MESSAGES_EVENT
  { struct DLS_MESSAGES *msg;
    gchar etat;
  };

 struct REGISTRE
  { struct CMD_TYPE_MNEMO_REGISTRE confDB;
    gfloat val;
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
    GSList *Liste_plugin_dls;                                                /* Liste des plugins D.L.S associ� au synoptique */
    GSList *Liste_dls_tree;                                               /* Liste des sous_synoptiques associ�s au synoptique */
  };

 struct COM_DLS                                                                      /* Communication entre le serveur et DLS */
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    pthread_mutex_t synchro_traduction;                  /* Mutex pour interdire les traductions simultan�es de plugins D.L.S */
    struct DLS_TREE *Dls_tree;                                                                       /* Arbre d'execution DLS */
    pthread_mutex_t synchro_data;                                      /* Mutex pour les acces concurrents � l'arbre des data */
    GSList *Set_M;                                                              /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_M;                                                      /* liste des Mxxx a d�sactiver � la fin du tour prg */
    GSList *Set_Dls_Data;                                                       /* liste des Mxxx a activer au debut tour prg */
    GSList *Reset_Dls_Data;                                               /* liste des Mxxx a d�sactiver � la fin du tour prg */

    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_reload;                                              /* TRUE si le thread doit recharger sa configuration */
    guint admin_start;                                                                              /* Demande de deconnexion */
    guint admin_stop;                                                                               /* Demande de deconnexion */
    guint temps_sched;
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
 extern void Dls_data_set_AI ( gchar *tech_id, gchar *acronyme, gpointer **ai_p, float val_avant_ech );
 extern gboolean Dls_data_get_MSG ( gchar *tech_id, gchar *acronyme, gpointer **msg_p );
 extern void SEA( int num, float val_avant_ech );
 extern void SEA_range( int num, int range );
 extern void SEA_ech( int num, float val_ech );
 extern void Envoyer_commande_dls ( int num );
 extern void Envoyer_commande_dls_data ( gchar *nom, gchar *owner );
 extern void Dls_foreach ( void *user_data, 
                           void (*do_plugin) (void *user_data, struct PLUGIN_DLS *),
                           void (*do_tree)   (void *user_data, struct DLS_TREE *) );

 extern void Prendre_heure ( void );                                                                          /* Dans heure.c */ 
 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
