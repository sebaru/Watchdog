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

 #include <gnutls/gnutls.h>
 #include <gnutls/x509.h>
 #include <openssl/ssl.h>
 #include <openssl/err.h>
 #include <openssl/rand.h>

 #define NOM_THREAD                       "satellite"

 #define SATELLITE_DEFAUT_PORT            5558
 #define SATELLITE_DEFAUT_HOST            "localhost"
 #define SATELLITE_DEFAUT_FILE_CA         "cacert.pem"
 #define SATELLITE_DEFAUT_FILE_SERVER     "satellite.pem"
 #define SATELLITE_DEFAUT_FILE_KEY        "satellitekey.pem"
 #define SATELLITE_TIME_NEXT_RETRY        300                /* 30 secondes pour se reconnecter au master */

 struct SATELLITE_CONFIG
  { struct LIBRAIRIE *lib;
    gboolean Thread_reload;                          /* TRUE si le thread doit recharger sa configuration */
    gboolean enable;                              /* True si la config indique que le thread doit tourner */
    GSList *liste_Events;                                          /* liste de struct MSGDB msg a envoyer */
    gchar master_host[80];                                                /* Nom DNS du l'instance MASTER */
    guint master_port;                                         /* Port TCP d'attaque de l'instance MASTER */
    gchar ssl_file_cert[80];
    gchar ssl_file_key[80];
    gchar ssl_file_ca[80];
    guchar Mode;
    struct CONNEXION *Connexion;
    X509 *master_certif;
    SSL_CTX *Ssl_ctx;
    guint date_next_retry;
  } Cfg_satellite;

 enum
  { SAT_DISCONNECTED,
    SAT_RETRY_CONNECT,
    SAT_ATTENTE_INTERNAL,
    SAT_ATTENTE_CONNEXION_SSL,
    SAT_WAIT_FOR_CONNECTED,
    SAT_CONNECTED,
  };
/*************************************** Définitions des prototypes ***************************************/
 extern gboolean Satellite_Lire_config ( void );
 extern void Satellite_Deconnecter_sale ( void );
 extern void Satellite_Deconnecter ( void );
 extern void Satellite_Ecouter_maitre ( void );
 extern void Satellite_Gerer_protocole_connexion ( struct CONNEXION *connexion );
 extern gboolean Satellite_Connecter_ssl ( void );
 extern gboolean Satellite_Connecter ( void );
 extern void Satellite_Envoyer_maitre ( gint tag, gint ss_tag, gchar *buffer, gint taille );

#endif
/*--------------------------------------------------------------------------------------------------------*/
