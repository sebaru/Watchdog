/**********************************************************************************************************/
/* Include/Cst_message.h        Déclaration des constantes dediées aux messages                           */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 11 aoû 2003 10:17:39 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _CST_MESSAGE_H_
 #define _CST_MESSAGE_H_

 #define NBR_CARAC_LIBELLE_MSG    50                                                  /* Attention au SMS */
 #define NBR_CARAC_LIBELLE_MSG_UTF8  (6*NBR_CARAC_LIBELLE_MSG)
 #define NBR_CARAC_OBJET_MSG      30
 #define NBR_CARAC_OBJET_MSG_UTF8    (6*NBR_CARAC_OBJET_MSG)
 enum
  { MSG_ETAT,                                                        /* Definitions des types de messages */
    MSG_PREALARME,
    MSG_DEFAUT,
    MSG_ALARME,
    MSG_VEILLE,
    NBR_TYPE_MSG
  };

/* #define MSG_CLIGNO       0
 #define MSG_FIXE         1
 #define MSG_OFF          2*/
#endif
/*--------------------------------------------------------------------------------------------------------*/
