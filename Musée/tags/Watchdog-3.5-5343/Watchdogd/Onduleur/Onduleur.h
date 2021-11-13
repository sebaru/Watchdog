/**********************************************************************************************************/
/* Watchdogd/Onduleur/Onduleur.h   Header et constantes des modules UPS Watchdgo 2.0                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 10 nov. 2009 16:35:20 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Onduleur.h
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

#ifndef _UPS_H_
 #define _UPS_H_
 #include <json-glib/json-glib.h>
 #include <upsclient.h>

 #define NOM_THREAD      "ups"

 #define UPS_PORT_TCP    3493                                                 /* Port de connexion TCP pour accès aux modules */
 #define UPS_RETRY       1800                                              /* 3 minutes entre chaque retry si pb de connexion */
 #define UPS_POLLING      100                                      /* Si tout va bien, on s'y connecte toutes les 10 secondes */

 struct UPS_CONFIG                                                    /* Communication entre DLS et l'UPS */
  { struct LIBRAIRIE *lib;
    GSList *Modules_UPS;
    void *zmq_to_master;                                             /* Envoi des events au master si l'instance est un slave */
    void *zmq_from_bus;                                              /* Envoi des events au master si l'instance est un slave */
  };

 struct MODULE_UPS
  { gboolean enable;                                                                   /* Le module doit-il tourner au boot ? */
    gchar tech_id[32];                                                                                   /* Tech_id du module */
    gchar description[80];                                                                                 /* Libelle associé */
    gchar host[32];                                                                              /* Adresses IP du module UPS */
    gchar name[32];                                                                               /* Nom de l'UPS sur le HOST */
    gchar admin_username[32];                                                                             /* Username associé */
    gchar admin_password[32];                                                                             /* Password associé */
    gchar date_create[32];                                                                                /* Date de creation */
    gpointer bit_comm;                                                            /* Pointer de raccourci pour le bit de comm */
    gint  nbr_connexion;                                                              /* Nombre de connexion OK dans le temps */
    UPSCONN_t upsconn;                                                                               /* Connexion UPS à l'ups */
    gboolean started;                                                                                      /* Est-il actif ?? */
    time_t date_next_connexion;
  };

/************************************************* Déclaration des prototypes *************************************************/

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/

