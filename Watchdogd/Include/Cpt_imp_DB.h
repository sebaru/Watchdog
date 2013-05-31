/**********************************************************************************************************/
/* Watchdogd/Include/Cpt_imp_DB.h        Déclaration structure internes des compteurs horaire             */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 07 déc. 2010 17:19:44 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Cpt_imp_DB.h
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
 
#ifndef _CPT_IMP_H_
 #define _CPT_IMP_H_

 #include "Db.h"

 #define NOM_TABLE_CPT_IMP    "dls_cpt_imp"

 struct CPT_IMP
  { struct CMD_TYPE_OPTION_COMPTEUR_IMP cpt_impdb;
    gboolean actif;                                                       /* Mémorisation de l'etat du CI */
    gfloat val_en_cours1;                                 /* valeur en cours pour le calcul via les ratio */
    gfloat val_en_cours2;                     /* valeur en cours avant interprétation selon le type de CI */
    time_t last_update;                               /* date de derniere update de la valeur du compteur */
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Charger_cpt_imp ( void );
 extern void Updater_cpt_impDB ( void );
 extern gboolean Recuperer_cpt_impDB ( struct DB **db );
 extern struct CMD_TYPE_OPTION_COMPTEUR_IMP *Recuperer_cpt_impDB_suite( struct DB **db );
 extern struct CMD_TYPE_OPTION_COMPTEUR_IMP *Rechercher_cpt_impDB( guint id );
 extern gboolean Modifier_cpt_impDB( struct CMD_TYPE_OPTION_COMPTEUR_IMP *cpt_imp );
#endif
/*--------------------------------------------------------------------------------------------------------*/
