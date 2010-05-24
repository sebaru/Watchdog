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

 #define ONDULEUR_PORT_TCP    3493                        /* Port de connexion TCP pour accès aux modules */
 #define ONDULEUR_RETRY       1800                     /* 3 minutes entre chaque retry si pb de connexion */

 #define NOM_TABLE_MODULE_ONDULEUR   "onduleurs"

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
  { guint id;                                                 /* Numéro du module dans la base de données */
    gboolean actif;                                                        /* Le module doit-il tourner ? */
    gchar host[32];                                                     /* Adresses IP du module ONDULEUR */
    gchar ups[32];                                                      /* Adresses IP du module ONDULEUR */
    guint bit_comm;                                  /* Bit interne B d'etat communication avec le module */
    guint ea_ups_load;                                                     /* Numéro de l'EA pour le load */
    guint ea_ups_real_power;                                         /* Numéro de l'EA pour le real power */
    guint ea_battery_charge;                                    /* Numéro de l'EA pour la charge batterie */
    guint ea_input_voltage;                                        /* Numéro de l'EA pour l'input voltage */

    UPSCONN_t upsconn;                                                      /* Connexion UPS à l'onduleur */
    gboolean started;                                                                  /* Est-il actif ?? */
    guint nbr_deconnect;
    time_t date_retente;
  };

/*********************************************** Déclaration des prototypes *******************************/
 extern void Run_onduleur ( void );                                                    /* Dans Onduleur.c */
#endif
/*--------------------------------------------------------------------------------------------------------*/

