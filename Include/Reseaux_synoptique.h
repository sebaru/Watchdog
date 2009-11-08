/**********************************************************************************************************/
/* Include/Reseaux_synoptique.h:   Sous_tag de la gestion synoptiques watchdog 2.0 par lefevre Sebastien  */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 10:44:55 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _RESEAUX_SYNOPTIQUE_H_
 #define _RESEAUX_SYNOPTIQUE_H_

 #include "Cst_synoptiques.h"

 struct CMD_SHOW_SYNOPTIQUE
  { guint id;                                                      /* Numero du message dans la structure */
    gchar  libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar  mnemo[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];
    guint  groupe;
  };
 struct CMD_EDIT_SYNOPTIQUE
  { guint id;                                                      /* Numero du message dans la structure */
    gchar  libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar  mnemo[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];
    guint  groupe;
  };

 struct CMD_ADD_SYNOPTIQUE
  { gchar  libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar  mnemo[NBR_CARAC_MNEMO_SYNOPTIQUE_UTF8+1];
    guint  groupe;
  };

 struct CMD_ID_SYNOPTIQUE
  { guint id;                                                      /* Numero du message dans la structure */
    gchar libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
  };

/************************************************* Tag de communication ***********************************/
 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE,                      /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FIN,                  /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_CREATE_PAGE_SYNOPTIQUE_OK,                          /* OK pour creer la page synoptique */
    SSTAG_CLIENT_WANT_PAGE_SYNOPTIQUE,
    SSTAG_CLIENT_WANT_GROUPE_FOR_SYNOPTIQUE,        /* Affichage des groupes pour appartenance synoptique */
    SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_SYNOPTIQUE,      /* Envoi des groupes pour l'edition utilisateur */
    SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_SYNOPTIQUE_FIN,                      /* Le transfert est terminé */
    SSTAG_CLIENT_ADD_SYNOPTIQUE,                           /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_SYNOPTIQUE_OK,                                   /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_SYNOPTIQUE,                                    /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_SYNOPTIQUE_OK,                                   /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_SYNOPTIQUE,                              /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_SYNOPTIQUE_OK,          /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_SYNOPTIQUE,                         /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_SYNOPTIQUE_OK,                   /* Le serveur valide les nouvelles données */      

  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

