/**********************************************************************************************************/
/* Watchdogd/Include/Onduleur.h   Header et constantes des modules ONDULEUR Watchdgo 2.0                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 10 nov. 2009 16:35:20 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Onduleur.h
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
 
#ifndef _ONDULEUR_H_
 #define _ONDULEUR_H_
 #include <upsclient.h>
 #include "Reseaux.h"

 #define ONDULEUR_PORT_TCP    3493                        /* Port de connexion TCP pour accès aux modules */
 #define ONDULEUR_RETRY       1800                     /* 3 minutes entre chaque retry si pb de connexion */

 #define NOM_TABLE_ONDULEUR   "onduleurs"

 struct COM_ONDULEUR                                               /* Communication entre DLS et la RS485 */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *Modules_ONDULEUR;
    gboolean reload;
    guint admin_del;                                                            /* Demande de deconnexion */
    guint admin_start;                                                          /* Demande de deconnexion */
    guint admin_stop;                                                           /* Demande de deconnexion */
    guint admin_add;                                                            /* Demande de deconnexion */
  };

 struct MODULE_ONDULEUR
  { struct CMD_TYPE_ONDULEUR onduleur;

    UPSCONN_t upsconn;                                                      /* Connexion UPS à l'onduleur */
    gboolean started;                                                                  /* Est-il actif ?? */
    guint nbr_deconnect;
    time_t date_retente;
  };

/*********************************************** Déclaration des prototypes *******************************/
 extern void Run_onduleur ( void );                                                    /* Dans Onduleur.c */
 extern gboolean Retirer_onduleurDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_ONDULEUR *onduleur );
 extern gint Ajouter_onduleurDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_ONDULEUR *onduleur );
 extern gboolean Recuperer_onduleurDB ( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_ONDULEUR *Recuperer_onduleurDB_suite( struct LOG *log, struct DB *db );
 extern struct CMD_TYPE_ONDULEUR *Rechercher_onduleurDB ( struct LOG *log, struct DB *db, guint num );
 extern struct CMD_TYPE_ONDULEUR *Rechercher_onduleurDB_par_id ( struct LOG *log, struct DB *db, guint id );
 extern gboolean Modifier_onduleurDB_set_start( struct LOG *log, struct DB *db, gint id, gint start );
 extern gboolean Modifier_onduleurDB( struct LOG *log, struct DB *db, struct CMD_TYPE_ONDULEUR *onduleur );
#endif
/*--------------------------------------------------------------------------------------------------------*/

