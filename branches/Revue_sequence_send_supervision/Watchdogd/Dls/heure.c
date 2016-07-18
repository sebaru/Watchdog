/**********************************************************************************************************/
/* Watchdogd/Dls/heure.c    ->    fonctions appelées par les plugins D.L.S                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                       lun 22 déc 2003 16:46:02 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * heure.c
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
 
 #include <time.h>

 #include "Module_dls.h"                /* Inclusion des prototypes de fonctions de controle/commande DLS */

 static int nbr_heure, nbr_minute;                                 /* Gestion des demarrages à heure fixe */
 static int num_jour_semaine;                                             /* Numéro du jour de la semaine */
/**********************************************************************************************************/
/* Prendre_heure: Prend la date/heure actuelle de la machine                                              */
/* Entrée: rien                                                                                           */
/* Sortie: les variables globales de gestion de l'heure sont mises à jour                                 */
/**********************************************************************************************************/
 void Prendre_heure ( void )
  { struct tm tm;
    time_t temps;

    time(&temps);
    localtime_r( &temps, &tm );
    nbr_heure = tm.tm_hour;
    nbr_minute = tm.tm_min;
    num_jour_semaine = tm.tm_wday;
  }
/**********************************************************************************************************/
/* Heure: renvoie TRUE si l'heure actuelle a changée (une fois par minute donc) et vaut les parametres    */
/* Entrée: heure et minute attendue                                                                       */
/* Sortie: rien ou pas rien                                                                               */
/**********************************************************************************************************/
 int Jour_semaine ( int jour )
  { return( num_jour_semaine == jour ); }
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
