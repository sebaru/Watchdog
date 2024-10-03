/******************************************************************************************************************************/
/* Watchdogd/Dls/heure.c    ->    fonctions appelées par les plugins D.L.S                                                    */
/* Projet Abls-Habitat version 4.2       Gestion d'habitat                                       lun 22 déc 2003 16:46:02 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * heure.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2024 - Sebastien LEFEVRE
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

 #include "watchdogd.h"
 #include "Module_dls.h"                                    /* Inclusion des prototypes de fonctions de controle/commande DLS */

 static int nbr_heure, nbr_minute;                                                     /* Gestion des demarrages à heure fixe */
 static int num_jour_semaine;                                                                 /* Numéro du jour de la semaine */
/******************************************************************************************************************************/
/* Prendre_heure: Prend la date/heure actuelle de la machine. Appelée toutes les minutes par DLS                              */
/* Entrée: rien                                                                                                               */
/* Sortie: les variables globales de gestion de l'heure sont mises à jour                                                     */
/******************************************************************************************************************************/
 void Prendre_heure ( void )
  { struct tm tm;
    time_t temps;

    time(&temps);
    localtime_r( &temps, &tm );
    nbr_heure = tm.tm_hour;                    /* Sinon sauvegarde dans les variables, et edge_up=1 pendant un tour programme */
    nbr_minute = tm.tm_min;
    num_jour_semaine = tm.tm_wday;
    Partage->com_dls.Top_check_horaire = TRUE;
  }
/******************************************************************************************************************************/
/* Heure: renvoie TRUE si le jour actuel est celui en parametre                                                               */
/* Entrée: le jour a tester                                                                                                   */
/* Sortie: TRUE / FALSE                                                                                                       */
/******************************************************************************************************************************/
 int Jour_semaine ( int jour )
  { return( num_jour_semaine == jour ); }
/******************************************************************************************************************************/
/* Heure: renvoie TRUE si l'heure actuelle est celle en paramètre                                                             */
/* Entrée: heure et minute attendue                                                                                           */
/* Sortie: TRUE / FALSE                                                                                                       */
/******************************************************************************************************************************/
 int Heure ( int heure, int minute )
  { return ( nbr_heure==heure && nbr_minute==minute && Partage->com_dls.Top_check_horaire ); }
/******************************************************************************************************************************/
/* Heure: renvoie TRUE si l'heure actuelle est future à celle en parametre                                                    */
/* Entrée: heure et minute attendue                                                                                           */
/* Sortie: TRUE / FALSE                                                                                                       */
/******************************************************************************************************************************/
 int Heure_apres ( int heure, int minute )
  { return( (heure<nbr_heure || (heure==nbr_heure && minute<nbr_minute)) ); }
/******************************************************************************************************************************/
/* Heure: renvoie TRUE si l'heure actuelle est antérieure a celle en parametre                                                */
/* Entrée: heure et minute attendue                                                                                           */
/* Sortie: TRUE / FALSE                                                                                                       */
/******************************************************************************************************************************/
 int Heure_avant ( int heure, int minute )
  { return( (heure>nbr_heure || (heure==nbr_heure && minute>nbr_minute)) ); }
/******************************************************************************************************************************/
/* Heure: renvoie TRUE si l'heure actuelle est future à celle en parametre                                                    */
/* Entrée: heure et minute attendue                                                                                           */
/* Sortie: TRUE / FALSE                                                                                                       */
/******************************************************************************************************************************/
 int Heure_apres_egal ( int heure, int minute )
  { return( (heure<nbr_heure || (heure==nbr_heure && minute<=nbr_minute)) ); }
/******************************************************************************************************************************/
/* Heure: renvoie TRUE si l'heure actuelle est antérieure a celle en parametre                                                */
/* Entrée: heure et minute attendue                                                                                           */
/* Sortie: TRUE / FALSE                                                                                                       */
/******************************************************************************************************************************/
 int Heure_avant_egal ( int heure, int minute )
  { return( (heure>nbr_heure || (heure==nbr_heure && minute>=nbr_minute)) ); }
/*----------------------------------------------------------------------------------------------------------------------------*/
