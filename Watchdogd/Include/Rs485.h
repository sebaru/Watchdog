/**********************************************************************************************************/
/* Watchdogd/Include/rs485.h   Header et constantes des modules rs485 Watchdgo 2.0                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 31 jui 2003 09:37:12 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
#ifndef _RS485_H_
 #define _RS485_H_

 #define FCT_IDENT      0x01
 #define FCT_ENTRE_TOR  0x02
 #define FCT_ENTRE_ANA  0x03
 #define FCT_SORTIE_TOR 0x04
 #define FCT_PING       0xFF
 
 #define NOM_TABLE_MODULE_RS485   "rs485"
 #define TAILLE_ENTETE  6
 #define TAILLE_DONNEES 10

 struct TRAME_RS485                                                       /* Definition d'une trame RS485 */
  { unsigned char dest;
    unsigned char source;
    unsigned char fonction;
    unsigned char taille;
    unsigned char donnees[TAILLE_DONNEES];
    unsigned char crc16_h;
    unsigned char crc16_l;
  };

 struct TRAME_RS485_IDENT
  { unsigned char version_major;
    unsigned char version_minor;
    unsigned char nbr_entre_ana;
    unsigned char nbr_entre_tor;
    unsigned char nbr_entre_choc;
    unsigned char nbr_sortie_tor;
  };

 struct MODULE_RS485
  { gint id;
    gboolean actif;
    gint ea_min, ea_max;
    gint e_min, e_max;
    gint ec_min, ec_max;
    gint s_min, s_max;
    gint sa_min, sa_max;

    time_t date_requete;
    time_t date_retente;
    time_t date_ana;
  };

 struct COM_RS485                                                 /* Communication entre DLS et la RS485 */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *Modules_RS485;
    gboolean reload;
    guint admin_del;                                                            /* Demande de deconnexion */
    guint admin_start;                                                          /* Demande de deconnexion */
    guint admin_stop;                                                           /* Demande de deconnexion */
    guint admin_add;                                                            /* Demande de deconnexion */
    guint admin_add_borne;                                                      /* Demande de deconnexion */
  };
/*********************************************** Déclaration des prototypes *******************************/
 extern void Run_rs485 ( void );                                                          /* Dans Rs485.c */

#endif
/*--------------------------------------------------------------------------------------------------------*/
