/**********************************************************************************************************/
/* Watchdogd/Include/Sms.h        Déclaration structure internes des SMS                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mer 15 avr 2009 15:46:31 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _SMS_H_
 #define _SMS_H_

 struct COM_MSRV_SMS                                                   /* Communication entre MSRV et SMS */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *liste_sms;                                              /* liste de struct MSGDB msg a envoyer */
    gboolean sigusr1;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Run_sms ( void );                                                              /* Dans Sms.c */
 extern void Envoyer_sms ( gchar *libelle );

#endif
/*--------------------------------------------------------------------------------------------------------*/
