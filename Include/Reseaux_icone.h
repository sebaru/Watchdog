/**********************************************************************************************************/
/* Include/Reseaux_icone.h:   Sous_tag de icone utilisé pour watchdog 2.0   par lefevre Sebastien         */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _RESEAUX_ICONE_H_
 #define _RESEAUX_ICONE_H_

 #include "Cst_icone.h"

 struct CMD_TYPE_CLASSE
  { guint id;                                                      /* Numero du message dans la structure */
    gchar libelle[ NBR_CARAC_CLASSE_ICONE_UTF8+1 ];
  };

 struct CMD_TYPE_ICONE
  { guint id;                                                      /* Numero du message dans la structure */
    gchar libelle[ NBR_CARAC_LIBELLE_ICONE_UTF8+1 ];
    guint id_classe;
    gchar nom_fichier[ 128 ];
  };

 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_ICONE,                           /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_ICONE_FIN,                       /* Ajout d'un groupe dans la liste cliente */
    SSTAG_CLIENT_WANT_PAGE_ICONE,                                         /* Le client veut la page Icone */
    SSTAG_SERVEUR_CREATE_PAGE_ICONE_OK,                        /* Le serveur repond OK pour creer la page */
    SSTAG_CLIENT_ADD_ICONE,                                /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_ICONE_WANT_FILE,       /* Le serveur a ajouté l'icone, il lui manque le fichier gif */
    SSTAG_CLIENT_ADD_ICONE_DEB_FILE,                       /* Le client commance à envoyer le fichier gif */
    SSTAG_CLIENT_ADD_ICONE_FILE,                               /* Le client continue d'envoyer le fichier */
    SSTAG_CLIENT_ADD_ICONE_FIN_FILE,                        /* Le client a terminé l'envoi du fichier gif */
    SSTAG_SERVEUR_ADD_ICONE_OK,                                       /* L'ajout de l'icone est un succes */

    SSTAG_CLIENT_DEL_ICONE,                                         /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_ICONE_OK,                                        /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_ICONE,                                   /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_ICONE_OK,               /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_ICONE,                              /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_ICONE_OK,                        /* Le serveur valide les nouvelles données */

    SSTAG_SERVEUR_ADDPROGRESS_CLASSE,                                          /* Envoi des classes icone */
    SSTAG_SERVEUR_ADDPROGRESS_CLASSE_FIN,                                      /* Envoi des classes icone */
    SSTAG_CLIENT_WANT_PAGE_CLASSE,
    SSTAG_CLIENT_ADD_CLASSE,                               /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_CLASSE_OK,                                       /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_CLASSE,                                        /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_CLASSE_OK,                                       /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_CLASSE,                                  /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_CLASSE_OK,              /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_CLASSE,                             /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_CLASSE_OK                        /* Le serveur valide les nouvelles données */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

