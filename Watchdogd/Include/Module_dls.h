/**********************************************************************************************************/
/* Watchdog/Include/Module_dls.h -> D�claration des prototypes de fonctions                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 31 jui 2003 11:49:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Module_dls.h
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
 
 #ifndef _MODULE_DLS_H_
 #define _MODULE_DLS_H_

 extern int E( int num );
 extern int B( int num );
 extern int M( int num );
 extern int TR( int num );
 extern int TRCount( int num );
 extern int EA_ech_inf( double val, int num );
 extern int EA_ech_sup( double val, int num );
 extern int EA_ech_inf_egal( double val, int num );
 extern int EA_ech_sup_egal( double val, int num );
 extern float CI( int num );
 extern void SI( int num, int etat, int rouge, int vert, int bleu, int cligno );
 extern void SB( int num, int etat );
 extern void STR( int num, int etat );
 extern void SCH( int num, int etat );
 extern void SCI( int num, int etat, int reset, int ratio );
 extern void SM( int num, int etat );
 extern void SA( int num, int etat );
 extern void MSG( int num, int etat );
                           
 extern int Heure( int heure, int minute );                                    /* Tester l'heure actuelle */
 extern int Heure_avant( int heure, int minute );
 extern int Heure_apres( int heure, int minute );
 extern int Jour_semaine( int jour );                 /* Sommes nous le jour de la semaine en parametre ? */
 #endif 
/*-------------------------------------------------------------------------------------------------------*/
