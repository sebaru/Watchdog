/**********************************************************************************************************/
/* Watchdogd/Satellite/Satellite.h        Déclaration structure internes des SATELLITE                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                    lun. 18 févr. 2013 20:06:29 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * satellite.h
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
 
#ifndef _SATELLITE_H_
 #define _SATELLITE_H_

 #define SATELLITE_DEFAUT_FILE_CA         "cacert.pem"
 #define SATELLITE_DEFAUT_FILE_SERVER     "serveursigne.pem"
 #define SATELLITE_DEFAUT_FILE_KEY        "serveurkey.pem"
 #define SATELLITE_DEFAUT_MAX_CONNEXION   100

 struct SATELLITE_CONFIG
  { struct LIBRAIRIE *lib;
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    gboolean enable;                              /* True si la config indique que le thread doit tourner */
    GSList *Liste_entretor;                                        /* liste de struct MSGDB msg a envoyer */
    GSList *Liste_entreeANA;                                       /* liste de struct MSGDB msg a envoyer */
    gchar send_to_url[128];
    gchar https_file_cert[80];
    gchar https_file_key[80];
    gchar https_file_ca[80];
    gchar *received_buffer;                                /* Buffer de reception de la reponse du master */
    gint received_size;                                                           /* Nombre d'octet recus */
 } Cfg_satellite;

/*************************************** Définitions des prototypes ***************************************/
#endif
/*--------------------------------------------------------------------------------------------------------*/
