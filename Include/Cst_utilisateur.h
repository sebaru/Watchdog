/**********************************************************************************************************/
/* Include/Cst_utilisateur.h         definition des gid et uid reserve du systeme                         */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:50:09 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #ifndef _CST_UTILISATEUR_H_
 #define _CST_UTILISATEUR_H_

 #include "Erreur.h"

 #define NBR_MAX_GROUPE_PAR_UTIL  50

 #define NBR_CARAC_LOGIN             16                           /* Idem pour la longueur max du password */
 #define NBR_CARAC_LOGIN_UTF8        (NBR_CARAC_LOGIN * 6)
 #define NBR_CARAC_COMMENTAIRE       40
 #define NBR_CARAC_COMMENTAIRE_UTF8  (NBR_CARAC_COMMENTAIRE * 6)

/*********************************************** prototypes de fonctions **********************************/
 #endif
/*--------------------------------------------------------------------------------------------------------*/
