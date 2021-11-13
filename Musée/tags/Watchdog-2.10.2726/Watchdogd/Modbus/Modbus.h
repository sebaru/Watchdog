/**********************************************************************************************************/
/* Watchdogd/Include/Modbus.h   Header et constantes des modules MODBUS Watchdgo 2.0                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 13 jui 2007 16:14:03 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Modbus.h
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
 
#ifndef _MODBUS_H_
 #define _MODBUS_H_

 #define MBUS_READ_COIL                0x01
 #define MBUS_WRITE_MULTIPLE_COIL      0x0F
 #define MBUS_READ_REGISTER            0x04
 #define MBUS_WRITE_REGISTER           0x06
 #define MBUS_WRITE_MULTIPLE_REGISTER  0x10

 #define MBUS_TEMPS_UPDATE_IO_ANA  5               /* Rafraichssiement des I/Os ANA toutes les 5 dixièmes */

 enum
  { MODBUS_GET_DESCRIPTION,
    MODBUS_GET_FIRMWARE,
    MODBUS_INIT_WATCHDOG1,
    MODBUS_INIT_WATCHDOG2,
    MODBUS_INIT_WATCHDOG3,
    MODBUS_INIT_WATCHDOG4,
    MODBUS_GET_NBR_AI,
    MODBUS_GET_NBR_AO,
    MODBUS_GET_NBR_DI,
    MODBUS_GET_NBR_DO,
    MODBUS_GET_DI,
    MODBUS_GET_AI,
    MODBUS_SET_DO,
    MODBUS_SET_AO,
  };
 
 #define MODBUS_PORT_TCP    502                           /* Port de connexion TCP pour accès aux modules */
 #define MODBUS_RETRY       100                      /* 10 secondes entre chaque retry si pb de connexion */

 #define NOM_THREAD                "modbus"
 #define NOM_TABLE_MODULE_MODBUS   "modbus_modules"

 struct MODBUS_CONFIG                                             /* Communication entre DLS et la MODBUS */
  { struct LIBRAIRIE *lib;
    GSList *Modules_MODBUS;
    guint admin_start;                                                          /* Demande de deconnexion */
    guint admin_stop;                                                           /* Demande de deconnexion */
    gboolean enable;                                                           /* Thread enable at boot ? */
    gboolean reload;
  } Cfg_modbus;

 struct TRAME_MODBUS_REQUETE                                             /* Definition d'une trame MODBUS */
  { guint16 transaction_id;
    guint16 proto_id; /* -> 0 = MOBUS */
    guint16 taille; /* taille, en comptant le unit_id */
    guint8 unit_id; /* 0xFF */
    guint8 fct;
    guint16 adresse;
    union { guint16 nbr;
            guint16 valeur;
            guchar data[32];
          };
  };

 #define TAILLE_ENTETE_MODBUS   6              /* Nombre d'octet avant d'etre sur d'avoir la taille trame */
 struct TRAME_MODBUS_REPONSE                                             /* Definition d'une trame MODBUS */
  { guint16 transaction_id;
    guint16 proto_id; /* -> 0 = MOBUS */
    guint16 taille; /* taille, en comptant le unit_id */
    guint8 unit_id; /* 0xFF */
    guint8 fct;
    guint8 data[64]; /* max = 64 octets pour la description du firmware wago (registre 0x2023) */
  };

/********************************************* Gestion des modbus ******************************************/
 struct MODBUSDB
  { guint id;                                                 /* Numéro du module dans la base de données */
    gboolean enable;                                                       /* Le module doit-il tourner ? */
    guint watchdog;                       /* Le module doit-il etre auto-supervisé ? en dixeme de seconde */
    guint bit;                                       /* Bit interne B d'etat communication avec le module */
    gchar ip[32];                                                         /* Adresses IP du module MODBUS */
    gchar libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8];                             /* Libelle de la modbus */
    guint map_E;
    guint map_EA;
    guint map_A;
    guint map_AA;
  };

 struct MODULE_MODBUS
  { struct MODBUSDB modbus;

    gboolean started;                                                                  /* Est-il actif ?? */
    gint connexion;                                                                 /* FD de connexion IP */
    gint mode;                                                /* Mode dans le processus de connexion WAGO */
    gint nbr_oct_lu;                                                            /* Nombre d'octet deja lu */
    guint16 transaction_id;
    guint nbr_entree_ana;                               /* Nombre de entree analogique donnée par le wago */
    guint nbr_sortie_ana;                               /* Nombre de sortie analogique donnée par le wago */
    guint nbr_entree_tor;                                      /* Nombre de entree TOR donnée par le wago */
    guint nbr_sortie_tor;                                      /* Nombre de sortie TOR donnée par le wago */
    guint nbr_deconnect;
    guint date_retente;                            /* Prochaine date de raccrochage module en cas de DOWN */
    guint date_last_reponse;                                    /* Utilisé pour détecter un "DOWN module" */
    guint date_next_eana;                        /* Utilisé pour gérer les interrogations des bornes EANA */
    gboolean do_check_eana;                                       /* Interrogation des bornes EANA ou non */
    gboolean request;                /* Une requete a-t'elle été envoyée, et donc en attente de réponse ? */
    struct TRAME_MODBUS_REPONSE response;
  };

/*********************************************** Déclaration des prototypes *******************************/
 extern gboolean Modbus_Lire_config ( void );
 extern gint Ajouter_modbusDB ( struct MODBUSDB *modbus );
 extern gboolean Retirer_modbusDB ( struct MODBUSDB *modbus );
 extern gint Modifier_modbusDB( struct MODBUSDB *modbus );
#endif
/*--------------------------------------------------------------------------------------------------------*/

