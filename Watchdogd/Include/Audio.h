/**********************************************************************************************************/
/* Watchdogd/Include/Audio.h        Déclaration structure internes pour audio                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer 15 avr 2009 15:40:43 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _AUDIO_H_
 #define _AUDIO_H_

 struct COM_AUDIO                                                  /* Communication entre DLS et la RS485 */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *liste_audio;                                                   /* liste de message a prononcer */
    gboolean sigusr1;
  };

/*************************************** Définitions des prototypes ***************************************/
 extern void Run_audio ( void );                                                          /* Dans Audio.c */
 extern void Ajouter_audio( gint num );
#endif
/*--------------------------------------------------------------------------------------------------------*/
