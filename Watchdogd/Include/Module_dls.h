/**********************************************************************************************************/
/* Watchdog/Include/Module_dls.h -> Déclaration des prototypes de fonctions                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 31 jui 2003 11:49:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #ifndef _MODULE_DLS_H_
 #define _MODULE_DLS_H_

 extern char *TRdetail( int num ); /* bidouille a virer */

 extern int E( int num );
 extern int B( int num );
 extern int M( int num );
 extern int TR( int num );
 extern int TRbarre( int num );
 extern int EA_ech_inf( double val, int num );
 extern int EA_ech_sup( double val, int num );
 extern int EA_ech_inf_egal( double val, int num );
 extern int EA_ech_sup_egal( double val, int num );
 extern void SI( int num, int etat, int rouge, int vert, int bleu, int cligno );
 extern void SB( int num, int etat );
 extern void STR( int num, int etat );
 extern void SCH( int num, int etat );
 extern void SM( int num, int etat );
 extern void SA( int num, int etat );
 extern void MSG( int num, int etat );
                           
 extern int Heure( int heure, int minute );                                    /* Tester l'heure actuelle */
 extern int Heure_avant( int heure, int minute );
 extern int Heure_apres( int heure, int minute );
 #endif
/*-------------------------------------------------------------------------------------------------------*/
