/**********************************************************************************************************/
/* Watchdogd/Include/Cpth_DB.h        Déclaration structure internes des compteurs horaire                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 14 fév 2006 15:18:10 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Cpth_DB.h
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
 
#ifndef _CPTH_H_
 #define _CPTH_H_

 #include "Db.h"

 #define NOM_TABLE_CPTH    "dls_cpth"

 struct CPTH_DB
  { guint  id_mnemo;                                                  /* Référence sur les id mnemoniques */
    guint  valeur;                                                                  /* Valeur du compteur */
    guint  num;                                                                     /* Numero du compteur */
  };

 struct CPT_HORAIRE
  { struct CPTH_DB cpthdb;
    guint last_arch;                                 /* Date de dernier enregistrement en base de données */
    guint old_top;                                                     /* Date de debut du comptage du CH */
    gboolean actif;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Charger_cpth ( void );
 extern void Updater_cpthDB ( void );
 extern struct CPTH_DB *Rechercher_cpthDB( struct LOG *log, struct DB *db, guint id );
#endif
/*--------------------------------------------------------------------------------------------------------*/
