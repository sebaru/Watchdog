/**********************************************************************************************************/
/* Include/Reseaux_message.h:   Sous_tag de message pour watchdog 2.0 par lefevre Sebastien               */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _RESEAUX_MESSAGE_H_
 #define _RESEAUX_MESSAGE_H_

 #include "Cst_message.h"

 struct CMD_TYPE_MESSAGE
  { guint  id;
    guint  num;                                                    /* Numero du message dans la structure */
    gchar  libelle[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    gchar  libelle_audio[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    gchar  libelle_sms[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    gchar  objet[NBR_CARAC_OBJET_MSG_UTF8+1];
    guchar type;                                                       /* Etat, prealarme, defaut, alarme */
    gboolean not_inhibe;                          /* Flag pour la gestion par exemple de l'inhibition ... */
    gboolean sms;                                                                       /* Envoi de sms ? */
    guint  num_syn;                     /* Numéro du fichier syn correspondant(pas l'index dans la table) */
    guint  num_voc;                     /* Numéro du fichier syn correspondant(pas l'index dans la table) */
  };

 enum 
  { SSTAG_CLIENT_WANT_PAGE_MESSAGE,
    SSTAG_SERVEUR_ADDPROGRESS_MESSAGE,                         /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_MESSAGE_FIN,                     /* Ajout d'un groupe dans la liste cliente */

    SSTAG_CLIENT_ADD_MESSAGE,                              /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_MESSAGE_OK,                                      /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_MESSAGE,                                       /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_MESSAGE_OK,                                      /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_MESSAGE,                                 /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_MESSAGE_OK,             /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_MESSAGE,                            /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_MESSAGE_OK,                      /* Le serveur valide les nouvelles données */

    SSTAG_CLIENT_WANT_SYN_FOR_MESSAGE,                         /* Envoi des synoptiques pour les messages */
    SSTAG_SERVEUR_ADDPROGRESS_SYN_FOR_MESSAGE,
    SSTAG_SERVEUR_ADDPROGRESS_SYN_FOR_MESSAGE_FIN,

    SSTAG_CLIENT_TYPE_NUM_MNEMO_VOC,
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_VOC,
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

