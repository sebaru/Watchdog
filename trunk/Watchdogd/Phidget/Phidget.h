/******************************************************************************************************************************/
/* Watchdogd/Phidget/Phidget.h   Header et constantes des modules Phidget Watchdgo 3.0                                        */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    18.03.2021 21:59:33 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Phidget.h
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

#ifndef _PHIDGET_H_
 #define _PHIDGET_H_
 #include <libsoup/soup.h>
 #include <json-glib/json-glib.h>
 #include <phidget22.h>

 enum
  { PHIDGET_GET_DESCRIPTION,
    PHIDGET_GET_FIRMWARE,
    PHIDGET_INIT_WATCHDOG1,
    PHIDGET_INIT_WATCHDOG2,
    PHIDGET_INIT_WATCHDOG3,
    PHIDGET_INIT_WATCHDOG4,
    PHIDGET_GET_NBR_AI,
    PHIDGET_GET_NBR_AO,
    PHIDGET_GET_NBR_DI,
    PHIDGET_GET_NBR_DO,
    PHIDGET_GET_DI,
    PHIDGET_GET_AI,
    PHIDGET_SET_DO,
    PHIDGET_SET_AO,
  };

 #define PHIDGET_PORT_TCP    502                                               /* Port de connexion TCP pour accès aux modules */
 #define PHIDGET_RETRY       100                                          /* 10 secondes entre chaque retry si pb de connexion */

 #define NOM_THREAD                "phidget"

 struct PHIDGET_CONFIG                                                                 /* Communication entre DLS et la Phidget */
  { struct LIBRAIRIE *lib;
    GSList *Modules_Phidget;
  };

 struct TRAME_PHIDGET_REQUETE                                                                 /* Definition d'une trame Phidget */
  { guint16 transaction_id;
    guint16 proto_id; /* -> 0 = MOBUS */
    guint16 taille;   /* taille, à partir du unit_id */
    guint8 unit_id;   /* 0xFF */
    guint8 fct;
    guint16 adresse;
    union { guint16 nbr;
            guint16 valeur;
            guchar data[32];
          };
  };

 #define TAILLE_ENTETE_Phidget   6                                  /* Nombre d'octet avant d'etre sur d'avoir la taille trame */
 struct TRAME_PHIDGET_REPONSE                                                                 /* Definition d'une trame Phidget */
  { guint16 transaction_id;
    guint16 proto_id; /* -> 0 = MOBUS */
    guint16 taille;   /* taille, en comptant le unit_id */
    guint8 unit_id;   /* 0xFF */
    guint8 fct;
    guint8 data[64];  /* max = 64 octets pour la description du firmware wago (registre 0x2023) */
  };

/************************************************** Gestion des Phidget ********************************************************/
 struct PhidgetDB
  { guint id;                                                                     /* Numéro du module dans la base de données */
    gchar date_create[20];
    gboolean enable;                                                                           /* Le module doit-il tourner ? */
    guint watchdog;                                           /* Le module doit-il etre auto-supervisé ? en dixeme de seconde */
    gchar hostname[32];                                                                       /* Adresses IP du module Phidget */
    gchar tech_id[32];                                                                            /* Tech_id du module Phidget */
    gchar description[128];                                                                              /* Libelle du Phidget */
    gint max_request_par_sec;                                                        /* Nombre de requete maximum par seconde */
  };

 struct MODULE_Phidget
  { struct PhidgetDB Phidget;
    pthread_t TID;                                                                 /* Tid du thread gérant le module unitaire */
    gboolean started;                                                                                      /* Est-il actif ?? */
    gint connexion;                                                                                     /* FD de connexion IP */
    gint mode;                                                                    /* Mode dans le processus de connexion WAGO */
    gint nbr_oct_lu;                                                                                /* Nombre d'octet deja lu */
    guint16 transaction_id;
    guint nbr_entree_ana;                                                   /* Nombre de entree analogique donnée par le wago */
    guint nbr_sortie_ana;                                                   /* Nombre de sortie analogique donnée par le wago */
    guint nbr_entree_tor;                                                          /* Nombre de entree TOR donnée par le wago */
    guint nbr_sortie_tor;                                                          /* Nombre de sortie TOR donnée par le wago */
    guint nbr_deconnect;
    guint date_retente;                                                /* Prochaine date de raccrochage module en cas de DOWN */
    guint date_last_reponse;                                                        /* Utilisé pour détecter un "DOWN module" */
    guint date_next_eana;                                            /* Utilisé pour gérer les interrogations des bornes EANA */
    gboolean do_check_eana;                                                           /* Interrogation des bornes EANA ou non */
    gboolean request;                                    /* Une requete a-t'elle été envoyée, et donc en attente de réponse ? */
    gint last_top;                                                                           /* Nombre de requete par seconde */
    gint nbr_request;                                                                        /* Nombre de requete par seconde */
    gint nbr_request_par_sec;                                                                /* Nombre de requete par seconde */
    gint delai;                                                  /* delai d'attente pour limiter le nb de request par seconde */
    struct TRAME_PHIDGET_REPONSE response;
    gpointer *DI;                                                              /* Tableau dynamique d'accès aux bits internes */
    gpointer *AI;                                                              /* Tableau dynamique d'accès aux bits internes */
    gpointer *MSG_AI_OUT_OF_RANGE;                                             /* Tableau dynamique d'accès aux bits internes */
    gpointer *DO;                                                              /* Tableau dynamique d'accès aux bits internes */
    gpointer bit_comm;                                                                       /* Bit interne d'etat de la comm */
  };

/****************************************************** Déclaration des prototypes ********************************************/

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
