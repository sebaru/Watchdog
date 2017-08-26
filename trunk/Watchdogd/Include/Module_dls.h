/******************************************************************************************************************************/
/* Watchdogd/Include/Module_dls.h -> Déclaration des prototypes de fonctions                                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          jeu 31 jui 2003 11:49:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
 #include <glib.h>

 extern void Dls_print_debug ( gint id, gint *Tableau_bit, gint *Tableau_num, gfloat *Tableau_val );
 extern int E( int num );
 extern int B( int num );
 extern int M( int num );
 extern int T( int num );
 extern int EA_ech_inf( float val, int num );
 extern int EA_ech_sup( float val, int num );
 extern int EA_ech_inf_egal( float val, int num );
 extern int EA_ech_sup_egal( float val, int num );
 extern void SEA( int num, float val_avant_ech );
 extern void SR( int num, float val );
 extern float EA_ech( int num );
 extern float R( int num );
 extern float CI( int num );
 extern void SI( int num, int etat, int rouge, int vert, int bleu, int cligno );
 extern void SB( int num, int etat );
 extern void ST( int num, int etat );
 extern void SCH( int num, int etat, int reset );
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
