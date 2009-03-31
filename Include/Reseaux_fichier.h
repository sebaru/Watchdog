/**********************************************************************************************************/
/* Include/Reseaux_fichier.h:   Sous_tag de l'fichier pour watchdog 2.0 par lefevre Sebastien             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _RESEAUX_FICHIER_H_
 #define _RESEAUX_FICHIER_H_

 struct CMD_FICHIER
  { gchar nom[80];
  };
 struct CMD_VERSION
  { time_t version;
  };

 enum 
  { SSTAG_SERVEUR_DEL_FICHIER,
    SSTAG_SERVEUR_FICHIER,
    SSTAG_SERVEUR_VERSION,
  };
#endif
/*--------------------------------------------------------------------------------------------------------*/

