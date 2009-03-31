/**********************************************************************************************************/
/* Include/Reseaux_gtk_message.h:   Type de message utilisé pour watchdog 2.0   par lefevre Sebastien     */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _RESEAUX_GTK_MESSAGE_H_
 #define _RESEAUX_GTK_MESSAGE_H_

 #define NBR_CARAC_MSGERREUR         100                        /* 80 caractères de messages d'erreur max */
 #define NBR_CARAC_MSGERREUR_UTF8    (NBR_CARAC_MSGERREUR*6)

 struct CMD_GTK_MESSAGE
  { gchar message[NBR_CARAC_MSGERREUR_UTF8+1];
  };

 enum 
  { SSTAG_SERVEUR_ERREUR,
    SSTAG_SERVEUR_WARNING,
    SSTAG_SERVEUR_INFO,
    SSTAG_SERVEUR_NBR_ENREG,                                     /* Envoi du nombre d'objet a telecharger */    
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/
