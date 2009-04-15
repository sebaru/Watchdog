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

/*********************************************** Déclaration des prototypes *******************************/
 extern void Rs485_state ( int id, gchar *chaine, int size );
 extern void Run_rs485 ( void );                                                          /* Dans Rs485.c */

#endif
/*--------------------------------------------------------------------------------------------------------*/
