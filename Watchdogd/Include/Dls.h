/**********************************************************************************************************/
/* Watchdogd/Include/Dls.h                  Définitions des constantes programme DLS                      */
/*        projet Watchdog v2.0     par LEFEVRE Sebastien                    sam 09 oct 2004 10:10:32 CEST */
/**********************************************************************************************************/

 #ifndef _DLS_H_
  #define _DLS_H_

 #include <glib.h>
 #include "Reseaux.h"
 #include "Db.h"
 #include "Cst_dls.h"

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
    GList *Liste_A;
    GList *Liste_MSG;
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
 extern int E( int num );
 extern int EA_inrange( int num );
 extern double EA_ech( int num );
 extern int EA_ech_inf( double val, int num );
 extern int EA_ech_inf_egal( double val, int num );
 extern int EA_ech_sup( double val, int num );
 extern int EA_ech_sup_egal( double val, int num );
 extern int A( int num );
 extern int B( int num );
 extern int M( int num );
 extern int TR( int num );
 extern char *TRdetail( int num );
 extern int TRbarre( int num );
 extern void SE( int num, int etat );
 extern void SEA( int num, int val_int, int inrange );
 extern void SB( int num, int etat );
 extern void SM( int num, int etat );
 extern void SI( int num, int etat, int rouge, int vert, int bleu, int cligno );
 extern void STR( int num, int cons );
 extern void SA( int num, int etat );
 extern void SCH( int num, int etat );
 extern void MSG( int num, int etat );
 
 extern void Prendre_heure ( void );                                                      /* Dans heure.c */ 
 #endif
/*--------------------------------------------------------------------------------------------------------*/
