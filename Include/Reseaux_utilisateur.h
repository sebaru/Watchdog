/**********************************************************************************************************/
/* Include/Reseaux_utilisateur.h:   Sous_tag de utilisateur pour watchdog 2.0 par lefevre Sebastien       */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _RESEAUX_UTILISATEUR_H_
 #define _RESEAUX_UTILISATEUR_H_

 #include "Cst_utilisateur.h"

 struct CMD_UTIL_SETPASSWORD
  { guint  id;
    gchar  code_en_clair[ NBR_CARAC_LOGIN_UTF8+1 ];
  };
 struct CMD_SHOW_UTILISATEUR
  { guint  id;
    gchar  nom[ NBR_CARAC_LOGIN_UTF8+1 ];
    gchar  commentaire[ NBR_CARAC_COMMENTAIRE_UTF8+1 ];
  };
 struct CMD_EDIT_UTILISATEUR
  { guint    id;
    gchar    nom[ NBR_CARAC_LOGIN_UTF8+1 ];
    gboolean cansetpass;
    gboolean setpassnow;
    gchar    code_en_clair[ NBR_CARAC_LOGIN_UTF8+1 ];
    gchar    commentaire[ NBR_CARAC_COMMENTAIRE_UTF8+1 ];
    time_t   date_modif;
    time_t   date_expire;
    gboolean actif;
    gboolean expire;
    gboolean changepass;
    guint    gids[NBR_MAX_GROUPE_PAR_UTIL];
  };

 struct CMD_ADD_UTILISATEUR
  { gchar  nom[ NBR_CARAC_LOGIN_UTF8+1 ];
    gchar  commentaire[ NBR_CARAC_COMMENTAIRE_UTF8+1 ];
    gchar  code_en_clair[ NBR_CARAC_LOGIN_UTF8+1 ];
    time_t date_expire;
    gboolean expire;
    gboolean changepass;
    gboolean actif;
    gboolean cansetpass;
    guint    gids[NBR_MAX_GROUPE_PAR_UTIL];
  };
 struct CMD_ID_UTILISATEUR
  { guint  id;
    gchar  nom[ NBR_CARAC_LOGIN_UTF8+1 ];
  };

 struct CMD_SHOW_GROUPE
  { gchar  nom[ NBR_CARAC_LOGIN_UTF8+1 ];
    gchar  commentaire[ NBR_CARAC_COMMENTAIRE_UTF8+1 ];
    guint  id;
  };
 struct CMD_EDIT_GROUPE
  { gchar  nom[ NBR_CARAC_LOGIN_UTF8+1 ];
    gchar  commentaire[ NBR_CARAC_COMMENTAIRE_UTF8+1 ];
    guint  id;
  };

 struct CMD_ADD_GROUPE
  { gchar  nom[ NBR_CARAC_LOGIN_UTF8+1 ];
    gchar  commentaire[ NBR_CARAC_COMMENTAIRE_UTF8+1 ];
  };
 struct CMD_ID_GROUPE
  { gchar  nom[ NBR_CARAC_LOGIN_UTF8+1 ];
    guint  id;
  };

 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_UTIL,                       /* Ajout d'un utilisateur dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_UTIL_FIN,
    SSTAG_CLIENT_WANT_PAGE_UTIL,
    SSTAG_CLIENT_WANT_GROUPE_FOR_UTIL,
    SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_UTIL,            /* Envoi des groupes pour l'edition utilisateur */
    SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_UTIL_FIN,                            /* Le transfert est terminé */
    SSTAG_CLIENT_ADD_UTIL,                            /* Le client desire ajouter un utilisateur watchdog */
    SSTAG_SERVEUR_ADD_UTIL_OK,                                    /* L'ajout du utilisateur est un succes */

    SSTAG_CLIENT_DEL_UTIL,                                     /* Le client desire retirer un utilisateur */
    SSTAG_SERVEUR_DEL_UTIL_OK,                                    /* L'ajout du utilisateur est un succes */

    SSTAG_CLIENT_EDIT_UTIL,                               /* Le client demande l'edition d'un utilisateur */
    SSTAG_SERVEUR_EDIT_UTIL_OK,                /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_UTIL,                               /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_UTIL_OK,                         /* Le serveur valide les nouvelles données */

    SSTAG_SERVEUR_ADDPROGRESS_GROUPE,                          /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FIN,
    SSTAG_CLIENT_WANT_PAGE_GROUPE,
    SSTAG_CLIENT_ADD_GROUPE,                               /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_GROUPE_OK,                                       /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_GROUPE,                                        /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_GROUPE_OK,                                       /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_GROUPE,                                  /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_GROUPE_OK,              /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_GROUPE,                             /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_GROUPE_OK,                       /* Le serveur valide les nouvelles données */
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

