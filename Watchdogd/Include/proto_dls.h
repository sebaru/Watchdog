/**********************************************************************************************************/
/* Watchdog/Include/proto_dls.h  -> Déclaration des prototypes de fonctions                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven sep 13 13:59:27 CEST 2002 */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #ifndef _PROTO_DLS_H_
 #define _PROTO_DLS_H_

 struct PLUGIN_DLS_DL
  { gchar nom_fichier[60];                                                              /* Nom du fichier */
    gint actif;
    void *handle;                                                          /* Handle du fichier librairie */
    void (*go)(void);                                                 /* Fonction de traitement du module */
    float conso;                                                     /* Consommation temporelle du plugin */
    gint id;                                                                      /* Numero du plugin DLS */
  };

/***************************** Déclarations des prototypes utilisés dans DLS ******************************/
 extern void SE( int num, int etat );
 extern void SEA( int num, int val_int, int inrange );
 extern int A( int num );
 extern int E( int num );
 extern int EA_int( int num );
 extern int EA_inrange( int num );
 extern double EA_ech( int num );
 extern void Reseter_un_plugin ( gint id );
 extern void Retirer_plugins ( void );
 extern void Charger_plugins ( void );
 extern void Lister_plugins ( void );
 extern void Activer_plugins ( gint num, gboolean actif );
 
 extern void Prendre_heure ( void );                                                      /* Dans heure.c */ 
 #endif
/*--------------------------------------------------------------------------------------------------------*/
