/**********************************************************************************************************/
/* Include/Reseaux_camera.h:   Sous_tag de gestion des camera pour watchdog 2.0 par lefevre Sebastien     */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 10:44:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _RESEAUX_CAMERA_H_
 #define _RESEAUX_CAMERA_H_

 #include "Cst_camera.h"

/******************************************** Gestion des camera ******************************************/
 struct CMD_TYPE_CAMERA
  { guint   id;                                                                 /* ID unique de la camera */
    gchar   libelle[NBR_CARAC_LIBELLE_MNEMONIQUE_UTF8];                           /* Libelle de la camera */
    gchar   location[NBR_CARAC_LOCATION_CAMERA_UTF8];                             /* Libelle de la camera */
    gint    type;                                                            /* petite, moyenne, grande ? */
  };

/************************************************* Tag de communication ***********************************/
 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_CAMERA,                          /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_CAMERA_FIN,                      /* Ajout d'un groupe dans la liste cliente */
    SSTAG_CLIENT_WANT_PAGE_CAMERA,

    SSTAG_CLIENT_EDIT_CAMERA,                                 /* Le client demande l'edition d'une camera */
    SSTAG_SERVEUR_EDIT_CAMERA_OK,              /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_CAMERA,                             /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_CAMERA_OK,                       /* Le serveur valide les nouvelles données */      
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

