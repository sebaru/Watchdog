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
    gint start;                     /* 1 si les bits internes "start" du plugins doivent etre positionnés */
    void *handle;                                                          /* Handle du fichier librairie */
    void (*go)(int);                                                  /* Fonction de traitement du module */
    float conso;                                                     /* Consommation temporelle du plugin */
    gint id;                                                                      /* Numero du plugin DLS */
  };

/***************************** Déclarations des prototypes utilisés dans DLS ******************************/
 extern void Run_dls ( void );                                                          /* Dans The_dls.c */
 extern int E( int num );
 extern int EA_inrange( int num );
 extern int EA_int( int num );
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
 extern void Reseter_un_plugin ( gint id );
 extern void Retirer_plugins ( void );
 extern void Charger_plugins ( void );
 extern void Lister_plugins ( void );
 extern void Activer_plugins ( gint num, gboolean actif );
 
 extern void Prendre_heure ( void );                                                      /* Dans heure.c */ 
 #endif
/*--------------------------------------------------------------------------------------------------------*/
