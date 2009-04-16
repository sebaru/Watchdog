/**********************************************************************************************************/
/* Watchdogd/Include/Admin.h        Déclaration structure internes pour admin                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer 15 avr 2009 15:40:43 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _ADMIN_H_
 #define _ADMIN_H_


 #define NOM_SOCKET "socket.wdg"

 struct CLIENT_ADMIN
  { gint connexion;
  };

 struct COM_ADMIN                                                  /* Communication entre DLS et la RS485 */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    gboolean sigusr1; 
    gint ecoute;                                                       /* Socket d'ecoute du thread ADMIN */
    GList *Clients;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Run_admin ( void );                                                          /* Dans Audio.c */
#endif
/*--------------------------------------------------------------------------------------------------------*/
