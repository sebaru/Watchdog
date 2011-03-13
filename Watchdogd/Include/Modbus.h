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

 #define MBUS_READ_COIL      0x01
 #define MBUS_ENTRE_ANA      0x04
 #define MBUS_SORTIE_TOR     0x06
 #define MBUS_SORTIE_ANA     0x06
 #define MBUS_READ_REGISTER  0x04
 #define MBUS_WRITE_REGISTER 0x06

 enum
  { MODBUS_GET_DESCRIPTION,
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
  };
 
 #define MODBUS_PORT_TCP    502                           /* Port de connexion TCP pour accès aux modules */
 #define MODBUS_RETRY       100                      /* 10 secondes entre chaque retry si pb de connexion */

 #define NOM_TABLE_MODULE_MODBUS   "modbus_modules"
 #define NOM_TABLE_BORNE_MODBUS    "modbus_bornes"

 struct COM_MODBUS                                                 /* Communication entre DLS et la MODBUS */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *Modules_MODBUS;
    gboolean reload;
    guint admin_del;                                                            /* Demande de deconnexion */
    guint admin_start;                                                          /* Demande de deconnexion */
    guint admin_stop;                                                           /* Demande de deconnexion */
    guint admin_add;                                                            /* Demande de deconnexion */
    guint admin_module_reload;                                                  /* Demande de deconnexion */
  };

 struct TRAME_MODBUS_REQUETE_ETOR                                        /* Definition d'une trame MODBUS */
  { guint16 transaction_id;
    guint16 proto_id; /* -> 0 = MOBUS */
    guint16 taille; /* taille, en comptant le unit_id */
    guint8 unit_id; /* 0xFF */
    guint8 fct;
    guint16 adresse;
    guint16 nbr;
  };
 #define TRAME_MODBUS_REQUETE_EANA TRAME_MODBUS_REQUETE_ETOR

 struct TRAME_MODBUS_REQUETE                                             /* Definition d'une trame MODBUS */
  { guint16 transaction_id;
    guint16 proto_id; /* -> 0 = MOBUS */
    guint16 taille; /* taille, en comptant le unit_id */
    guint8 unit_id; /* 0xFF */
    guint8 fct;
    guint16 adresse;
    union { guint16 nbr;
            guint16 valeur;
          };
  };

 #define TAILLE_ENTETE_MODBUS   6              /* Nombre d'octet avant d'etre sur d'avoir la taille trame */
 struct TRAME_MODBUS_REPONSE                                             /* Definition d'une trame MODBUS */
  { guint16 transaction_id;
    guint16 proto_id; /* -> 0 = MOBUS */
    guint16 taille; /* taille, en comptant le unit_id */
    guint8 unit_id; /* 0xFF */
    guint8 fct;
    guint8 data[64]; /* max = 64 octets description wago */
  };

 struct MODULE_MODBUS
  { struct CMD_TYPE_MODBUS modbus;

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
    time_t date_retente;                           /* Prochaine date de raccrochage module en cas de DOWN */
    time_t date_last_reponse;                                   /* Utilisé pour détecter un "DOWN module" */
    time_t date_next_eana;                       /* Utilisé pour gérer les interrogations des bornes EANA */
    gboolean do_check_eana;                                       /* Interrogation des bornes EANA ou non */
    gboolean request;                /* Une requete a-t'elle été envoyée, et donc en attente de réponse ? */
    struct TRAME_MODBUS_REPONSE response;
    GList *Bornes;                                             /* La liste des bornes associées au module */
    GList *borne_en_cours;                                           /* La borne en cours d'interrogation */
  };

/*********************************************** Déclaration des prototypes *******************************/
 extern void Run_modbus ( void );                                                        /* Dans Modbus.c */
 extern struct CMD_TYPE_MODBUS *Rechercher_modbusDB ( struct LOG *log, struct DB *db, guint id );
 extern struct CMD_TYPE_MODBUS *Recuperer_modbusDB_suite( struct LOG *log, struct DB *db );
 extern gboolean Recuperer_modbusDB ( struct LOG *log, struct DB *db );
 extern gint Ajouter_modbusDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_MODBUS *modbus );
 extern gboolean Retirer_modbusDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_MODBUS *modbus );
 extern gboolean Modifier_modbusDB( struct LOG *log, struct DB *db, struct CMD_TYPE_MODBUS *modbus );
 extern struct CMD_TYPE_BORNE_MODBUS *Rechercher_borne_modbusDB ( struct LOG *log, struct DB *db, guint id );
 extern struct CMD_TYPE_BORNE_MODBUS *Recuperer_borne_modbusDB_suite( struct LOG *log, struct DB *db );
 extern gboolean Recuperer_borne_modbusDB ( struct LOG *log, struct DB *db, guint module );
 extern gint Ajouter_borne_modbusDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_BORNE_MODBUS *modbus );
 extern gboolean Retirer_borne_modbusDB ( struct LOG *log, struct DB *db, struct CMD_TYPE_BORNE_MODBUS *modbus );
 extern gboolean Modifier_borne_modbusDB( struct LOG *log, struct DB *db, struct CMD_TYPE_BORNE_MODBUS *modbus );
#endif
/*--------------------------------------------------------------------------------------------------------*/

