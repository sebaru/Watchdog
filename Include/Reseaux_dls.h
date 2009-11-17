/**********************************************************************************************************/
/* Include/Reseaux_dls.h:   Sous_tag de dls utilisé pour watchdog 2.0   par lefevre Sebastien             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _RESEAUX_DLS_H_
 #define _RESEAUX_DLS_H_

 #include "Cst_dls.h"

 struct CMD_TYPE_PLUGIN_DLS
  { gchar nom[ NBR_CARAC_PLUGIN_DLS_UTF8 + 1 ];
    guint id;
    guint on;
  };
 
 struct CMD_TYPE_SOURCE_DLS
  { guint id;
    guint taille;                                 /* Taille des données qui suivent dans le paquet reseau */
  };

 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_PLUGIN_DLS,                                            /* Envoi des plugins */
    SSTAG_SERVEUR_ADDPROGRESS_PLUGIN_DLS_FIN,
    SSTAG_CLIENT_WANT_PAGE_DLS,
    SSTAG_CLIENT_ADD_PLUGIN_DLS,                      /* Le client desire ajouter un utilisateur watchdog */
    SSTAG_SERVEUR_ADD_PLUGIN_DLS_OK,                              /* L'ajout du utilisateur est un succes */

    SSTAG_CLIENT_DEL_PLUGIN_DLS,                               /* Le client desire retirer un utilisateur */
    SSTAG_SERVEUR_DEL_PLUGIN_DLS_OK,                              /* L'ajout du utilisateur est un succes */

    SSTAG_CLIENT_EDIT_PLUGIN_DLS,                              /* Le client demande l'edition d'un plugin */
    SSTAG_SERVEUR_EDIT_PLUGIN_DLS_OK,          /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_PLUGIN_DLS,                         /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_PLUGIN_DLS_OK,                   /* Le serveur valide les nouvelles données */

    SSTAG_CLIENT_EDIT_SOURCE_DLS,                         /* Le client demande l'edition d'un utilisateur */
    SSTAG_SERVEUR_EDIT_SOURCE_DLS_OK,          /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_SERVEUR_SOURCE_DLS,                                                         /* Envoi du prg DLS */
    SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS_DEB,                     /* Le client renvoie les données editées */
    SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS,                         /* Le client renvoie les données editées */
    SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS_FIN,                     /* Le client renvoie les données editées */

    SSTAG_CLIENT_WANT_TYPE_NUM_MNEMO,
    SSTAG_SERVEUR_TYPE_NUM_MNEMO,
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

