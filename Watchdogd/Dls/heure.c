/**********************************************************************************************************/
/* Watchdogd/Dls/heure.c    ->    fonctions appelées par les plugins D.L.S                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                       lun 22 déc 2003 16:46:02 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <time.h>

 #include "Module_dls.h"                /* Inclusion des prototypes de fonctions de controle/commande DLS */

 static int nbr_heure, nbr_minute;                                 /* Gestion des demarrages à heure fixe */

/**********************************************************************************************************/
/* Prendre_heure: Prend la date/heure actuelle de la machine                                              */
/* Entrée: rien                                                                                           */
/* Sortie: les variables globales de gestion de l'heure sont mises à jour                                 */
/**********************************************************************************************************/
 void Prendre_heure ( void )
  { struct tm *tm;
    time_t temps;

    time(&temps);
    tm = localtime( &temps );
    nbr_heure = tm->tm_hour;
    nbr_minute = tm->tm_min;
  }
/**********************************************************************************************************/
/* Heure: renvoie TRUE si l'heure actuelle a changée (une fois par minute donc) et vaut les parametres    */
/* Entrée: heure et minute attendue                                                                       */
/* Sortie: rien ou pas rien                                                                               */
/**********************************************************************************************************/
 int Heure ( int heure, int minute )
  { return( nbr_heure==heure && nbr_minute==minute ); }
/**********************************************************************************************************/
/* Heure: renvoie TRUE si l'heure actuelle a changée (une fois par minute donc) et vaut les parametres    */
/* Entrée: heure et minute attendue                                                                       */
/* Sortie: rien ou pas rien                                                                               */
/**********************************************************************************************************/
 int Heure_apres ( int heure, int minute )
  { return( (heure<nbr_heure || (heure==nbr_heure && minute<nbr_minute)) ); }
/**********************************************************************************************************/
/* Heure: renvoie TRUE si l'heure actuelle a changée (une fois par minute donc) et vaut les parametres    */
/* Entrée: heure et minute attendue                                                                       */
/* Sortie: rien ou pas rien                                                                               */
/**********************************************************************************************************/
 int Heure_avant ( int heure, int minute )
  { return( (heure>nbr_heure || (heure==nbr_heure && minute>nbr_minute)) ); }
/*--------------------------------------------------------------------------------------------------------*/
