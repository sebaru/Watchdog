/**********************************************************************************************************/
/* Include/Reseaux_entreeana.h:   Sous_tag de entreeana pour watchdog 2.0 par lefevre Sebastien           */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _RESEAUX_ENTREEANA_H_
 #define _RESEAUX_ENTREEANA_H_

 #include "Cst_entreeana.h"

 struct CMD_TYPE_ENTREEANA
  { guint  id_mnemo;                                                        /* Id unique du mnemo associé */
    guint  unite;                                                                         /* Km, h, ° ... */
    guint  num;                                                                         /* Numero de l'EA */
    gfloat min;
    gfloat max;
    gchar libelle[ NBR_CARAC_LIBELLE_ENTREEANA_UTF8+1 ];
  };

 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA,                       /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FIN,
    SSTAG_CLIENT_WANT_PAGE_ENTREEANA,

    SSTAG_CLIENT_EDIT_ENTREEANA,                               /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_ENTREEANA_OK,           /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_ENTREEANA,                          /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_ENTREEANA_OK,                    /* Le serveur valide les nouvelles données */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

