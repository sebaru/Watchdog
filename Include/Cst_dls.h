/**********************************************************************************************************/
/* Include/Cst_dls.h         definition des constantes DLS partagées entre client et serveur              */
/* Projet WatchDog version 2.0       Gestion d'habitat                     lun. 23 nov. 2009 20:25:07 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Cst_dls.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
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

 #ifndef _CST_DLS_H_
 #define _CST_DLS_H_

 #define NBR_CARAC_PLUGIN_DLS       40                                             /* Nom d'un plugin DLS */
 #define NBR_CARAC_PLUGIN_DLS_UTF8  (6*NBR_CARAC_PLUGIN_DLS)

 #define NBR_BIT_DLS           10000

 #define NBR_ENTRE_TOR         256
 #define NBR_ENTRE_ANA         16
 #define NBR_SORTIE_TOR        256
 #define NBR_BIT_CONTROLE      NBR_BIT_DLS  /* Ixxx */
 #define NBR_BIT_BISTABLE      NBR_BIT_DLS  /* Bxxx */
 #define NBR_BIT_MONOSTABLE    NBR_BIT_DLS  /* Mxxx */
 #define NBR_TEMPO             128
 #define NBR_MNEMONIQUE        NBR_BIT_DLS
 #define NBR_MESSAGE_ECRITS    NBR_BIT_DLS
 #define NBR_COMPTEUR_H        100
 #define NBR_SCENARIO          100
 #define NBR_CAMERA            16

/*********************************************** prototypes de fonctions **********************************/
 #endif
/*--------------------------------------------------------------------------------------------------------*/
