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
 
 #define NBR_ID_MODBUS             4
 #define NBR_ID_MODBUS_BORNE       8

 #define MODBUS_PORT_TCP    502                           /* Port de connexion TCP pour accès aux modules */
 #define MODBUS_RETRY       5                         /* 2 secondes entre chaque retry si pb de connexion */

 enum
  { BORNE_INPUT_TOR,
    BORNE_OUTPUT_TOR,
    BORNE_INPUT_ANA,
    BORNE_OUTPUT_ANA
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

 struct MODULE_MODBUS_BORNE
  { gboolean actif;
    gint type;
    guint16 adresse;
    guint16 min;
    guint16 nbr;
  };

 struct MODULE_MODBUS
  { gboolean actif;
    guint watchdog;                       /* Le module doit-il etre auto-supervisé ? en dixeme de seconde */
    guint bit;                                       /* Bit interne B d'etat communication avec le module */
    gchar ip[32];                                                         /* Adresses IP du module MODBUS */
    struct MODULE_MODBUS_BORNE borne[NBR_ID_MODBUS_BORNE];
  };

/*********************************************** Déclaration des prototypes *******************************/
 extern void Modbus_state ( int id, gchar *chaine, int size );

#endif
/*--------------------------------------------------------------------------------------------------------*/

