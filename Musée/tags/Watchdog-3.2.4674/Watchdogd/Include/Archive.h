/******************************************************************************************************************************/
/* Watchdogd/Include/Archive.h        Déclaration structure internes des archivages                                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          sam 08 jui 2006 12:02:36 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Archive.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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

 #define ARCHIVE_EA_TEMPS_SI_CONSTANT   600                                    /* Si constant, archivage toutes les 1 minutes */
 #define ARCHIVE_EA_TEMPS_SI_VARIABLE    50                                   /* Si variable, archivage toutes les 5 secondes */
 #define ARCHIVE_DEFAUT_RETENTION       400                              /* Nom de jours par défaut de retention des archives */
 #define ARCHIVE_DEFAULT_BUFFER_SIZE 500000

 #define NOM_TABLE_ARCH    "histo_bit"

 struct ARCHDB
  { guint  date_sec;                                                                                      /* Date de la photo */
    guint  date_usec;                                                                                     /* Date de la photo */
/*    guint  type;                                                                                 /* Type de bit: E ? B ? EA ? */
    gchar  nom[NBR_CARAC_ACRONYME_MNEMONIQUE_UTF8+1];
    gchar  tech_id[NBR_CARAC_PLUGIN_DLS_TECHID];
    gfloat valeur;                                                                           /* Valeur de l'entrée analogique */
  };

 struct COM_ARCH                                                                                   /* Communication vers ARCH */
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    GSList *liste_arch;                                                                   /* liste de struct ARCHDB a traiter */
    gint taille_arch;
    gint max_buffer_size;                                                   /* Taille max de la liste des archives avant drop */
    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_reload;                                              /* TRUE si le thread doit recharger sa configuration */
    gint  duree_retention;                                              /* Duree de retention des données d'archive, en jours */
    gint  archdb_port;
    gchar archdb_host    [ TAILLE_DB_HOST+1 ];                                  /* Nom du host de la base de donnes d'archive */
    gchar archdb_username[ TAILLE_DB_USERNAME+1 ];                           /* Nom de l'administrateur de la base de données */
    gchar archdb_database[ TAILLE_DB_DATABASE+1 ];                                          /* Chemin d'acces aux DB watchdog */
    gchar archdb_password[ TAILLE_DB_PASSWORD+1 ];                                          /* Mot de passe de connexion ODBC */
  };

/******************************************* Définitions des prototypes *******************************************************/
 extern void Run_arch ( void );                                                                             /* Dans Archive.c */
 extern gboolean Arch_Lire_config ( void );
 extern gint Arch_Clear_list ( void );
 extern void Ajouter_arch_by_nom( gchar *nom, gchar *tech_id, gfloat valeur );
 extern gboolean Ajouter_archDB ( struct DB *db, struct ARCHDB *arch );
 extern void Admin_arch_json ( gchar *commande, gchar **buffer_p, gint *taille_p );
 extern void Arch_Update_SQL_Partitions_thread ( void );
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
