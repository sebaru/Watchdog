/**********************************************************************************************************/
/* Watchdogd/Include/Archive.h        Déclaration structure internes des archivages                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 08 jui 2006 12:02:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Archive.h
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
 
#ifndef _ARCHIVAGE_DB_H_
 #define _ARCHIVAGE_DB_H_

 #define ARCHIVE_EA_TEMPS_SI_CONSTANT  3000                /* Si constant, archivage toutes les 5 minutes */
 #define ARCHIVE_EA_TEMPS_SI_VARIABLE    50               /* Si variable, archivage toutes les 5 secondes */

 #define NUM_EA_SYS_ARCHREQUEST         126  /* Numéro d'EA de reference pour le nbr de request archivage */

 #define NOM_TABLE_ARCH    "histo_bit"

 struct ARCHDB
  { guint  date_sec;                                                                  /* Date de la photo */
    guint  date_usec;                                                                 /* Date de la photo */
    guint  type;                                                             /* Type de bit: E ? B ? EA ? */
    guint  num;                                            /* Numero de l'entrée analogique photographiée */
    gfloat valeur;                                                       /* Valeur de l'entrée analogique */
  };

 struct COM_ARCH                                                               /* Communication vers ARCH */
  { pthread_t TID;                                                               /* Identifiant du thread */
    pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GSList *liste_arch;                                               /* liste de struct ARCHDB a traiter */
    gint taille_arch;
    gboolean Thread_run;                /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    gboolean Thread_sigusr1;                                      /* TRUE si le thread doit gerer le USR1 */
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Run_arch ( void );                                                         /* Dans Archive.c */
 extern gint Arch_Clear_list ( void );
  extern void Ajouter_arch( gint type, gint num, gfloat valeur );
 extern void Ajouter_archDB ( struct DB *db, struct ARCHDB *arch );
 extern gboolean Recuperer_archDB ( struct LOG *log, struct DB *db, guint type, guint num,
                                    time_t date_deb, time_t date_fin );
 extern struct ARCHDB *Recuperer_archDB_suite( struct LOG *log, struct DB *db );

#endif
/*--------------------------------------------------------------------------------------------------------*/
