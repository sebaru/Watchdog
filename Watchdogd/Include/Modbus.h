/**********************************************************************************************************/
/* Watchdogd/Include/Modbus.h   Header et constantes des modules MODBUS Watchdgo 2.0                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 13 jui 2007 16:14:03 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _MODBUS_H_
 #define _MODBUS_H_

 #define MBUS_ENTRE_TOR  0x01
 #define MBUS_ENTRE_ANA  0x04
 #define MBUS_SORTIE_TOR 0x06
 #define MBUS_SORTIE_ANA 0x06
 
 #define MODBUS_PORT_TCP    502                           /* Port de connexion TCP pour accès aux modules */
 #define MODBUS_RETRY       10                       /* 10 secondes entre chaque retry si pb de connexion */

 #define NOM_TABLE_MODULE_MODBUS   "modbus_modules"
 #define NOM_TABLE_BORNE_MODBUS    "modbus_bornes"

 enum
  { BORNE_INPUT_TOR,
    BORNE_OUTPUT_TOR,
    BORNE_INPUT_ANA,
    BORNE_OUTPUT_ANA,
    NBR_MODE_BORNE
  };

 extern gchar *Mode_borne[NBR_MODE_BORNE];


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

 struct TRAME_MODBUS_REQUETE_STOR                                        /* Definition d'une trame MODBUS */
  { guint16 transaction_id;
    guint16 proto_id; /* -> 0 = MOBUS */
    guint16 taille; /* taille, en comptant le unit_id */
    guint8 unit_id; /* 0xFF */
    guint8 fct;
    guint16 adresse;
    guint16 valeur;
  };

 #define TAILLE_ENTETE_MODBUS   6              /* Nombre d'octet avant d'etre sur d'avoir la taille trame */
 struct TRAME_MODBUS_REPONSE                                             /* Definition d'une trame MODBUS */
  { guint16 transaction_id;
    guint16 proto_id; /* -> 0 = MOBUS */
    guint16 taille; /* taille, en comptant le unit_id */
    guint8 unit_id; /* 0xFF */
    guint8 fct;
    guint8 nbr;
    guint8 data[16]; /* max = 8*2octets ANA */
  };

 struct BORNE_MODBUS
  { gint id;
    gint type;
    gint adresse;
    gint min;
    gint nbr;
  };

 struct MODULE_MODBUS
  { guint id;                                                 /* Numéro du module dans la base de données */
    guint watchdog;                       /* Le module doit-il etre auto-supervisé ? en dixeme de seconde */
    guint bit;                                       /* Bit interne B d'etat communication avec le module */
    gchar ip[32];                                                         /* Adresses IP du module MODBUS */

    gboolean started;                                                                  /* Est-il actif ?? */
    gint connexion;                                                                 /* FD de connexion IP */
    gint nbr_oct_lu;                                                            /* Nombre d'octet deja lu */
    guint16 transaction_id;
    guint nbr_deconnect;
    time_t date_retente;
    time_t date_last_reponse;
    gboolean request;
    struct TRAME_MODBUS_REPONSE response;
    GList *Bornes;
    GList *borne_en_cours;
  };

/*********************************************** Déclaration des prototypes *******************************/
 extern void Run_modbus ( void );                                                        /* Dans Modbus.c */

#endif
/*--------------------------------------------------------------------------------------------------------*/

