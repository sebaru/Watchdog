/******************************************************************************************************************************/
/* Include/Reseaux_message.h:   Sous_tag de message pour watchdog 2.0 par lefevre Sebastien                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Reseaux_message.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifndef _RESEAUX_MESSAGE_H_
 #define _RESEAUX_MESSAGE_H_

 #define NBR_CARAC_LIBELLE_MSG    100                                                 /* Attention au SMS */
 #define NBR_CARAC_LIBELLE_MSG_UTF8  (2*NBR_CARAC_LIBELLE_MSG)

 enum
  { MSG_ETAT,                                                        /* Definitions des types de messages */
    MSG_ALERTE,
    MSG_DEFAUT,
    MSG_ALARME,
    MSG_VEILLE,
    MSG_ATTENTE,
    MSG_DANGER,
    NBR_TYPE_MSG
  };

 enum
  { MSG_SMS_NONE,
    MSG_SMS_YES,
    MSG_SMS_GSM_ONLY,
    MSG_SMS_SMSBOX_ONLY,
    NBR_TYPE_MSG_SMS
  };

 struct CMD_TYPE_MESSAGE
  { guint  id;
    guint  num;                                                                        /* Numero du message dans la structure */
    guint  dls_id;                                                           /* Numéro ID du plugin D.L.S rattaché au message */
    gchar  dls_shortname[NBR_CARAC_PLUGIN_DLS_UTF8+1];
    gchar  libelle[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    gchar  libelle_sms[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    guint  syn_id;                                                             /* Numéro ID du synoptique rattaché au message */
    gchar  syn_groupe[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    gchar  syn_page[NBR_CARAC_PAGE_SYNOPTIQUE_UTF8+1];
    gchar  syn_libelle[NBR_CARAC_LIBELLE_SYNOPTIQUE_UTF8+1];
    guchar type;                                                                           /* Etat, prealarme, defaut, alarme */
    gboolean enable;                                                  /* Flag pour la gestion par exemple de l'inhibition ... */
    guint  sms;                                                                                             /* Envoi de sms ? */
    gboolean audio;                                                                             /* Activation message audio ? */
    guint  bit_audio;                                                         /* Numéro du Monostable associé au profil vocal */
    gchar  libelle_audio[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    guint  time_repeat;                                               /* Temps entre deux répétitions (si non nul) en minutes */
    gboolean persist;                                                                               /* Persistence du message */
    gboolean is_mp3;                                                            /* Un mp3 a-t'il été chargé pour ce message ? */
  };

 struct CMD_TYPE_MESSAGES
  { guint nbr_messages;                                                     /* Nombre de structure CMD_TYPE_MESSAGE suivantes */
    struct CMD_TYPE_MESSAGE msg[];
  };
 
 struct CMD_TYPE_MESSAGE_MP3                                                       /* Structure pour l'échange du fichier mp3 */
  { guint num;
    guint taille;                                                     /* Taille des données qui suivent dans le paquet reseau */
  };

 enum 
  { SSTAG_CLIENT_WANT_PAGE_MESSAGE,
    SSTAG_SERVEUR_CREATE_PAGE_MESSAGE_OK,                                                    /* Affichage de la page onduleur */
    SSTAG_SERVEUR_ADDPROGRESS_MESSAGE,                                             /* Ajout d'un groupe dans la liste cliente */
    SSTAG_SERVEUR_ADDPROGRESS_MESSAGE_FIN,                                         /* Ajout d'un groupe dans la liste cliente */

    SSTAG_CLIENT_ADD_MESSAGE,                                                  /* Le client desire ajouter un groupe watchdog */
    SSTAG_SERVEUR_ADD_MESSAGE_OK,                                                          /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_DEL_MESSAGE,                                                           /* Le client desire retirer un groupe */
    SSTAG_SERVEUR_DEL_MESSAGE_OK,                                                          /* L'ajout du groupe est un succes */

    SSTAG_CLIENT_EDIT_MESSAGE,                                                     /* Le client demande l'edition d'un groupe */
    SSTAG_SERVEUR_EDIT_MESSAGE_OK,                                 /* Le serveur accepte et envoi les données correspondantes */
    SSTAG_CLIENT_VALIDE_EDIT_MESSAGE,                                                /* Le client renvoie les données editées */
    SSTAG_SERVEUR_VALIDE_EDIT_MESSAGE_OK,                                          /* Le serveur valide les nouvelles données */

    SSTAG_CLIENT_WANT_DLS_FOR_MESSAGE,                                             /* Envoi des synoptiques pour les messages */
    SSTAG_SERVEUR_ADDPROGRESS_DLS_FOR_MESSAGE,
    SSTAG_SERVEUR_ADDPROGRESS_DLS_FOR_MESSAGE_FIN,

    SSTAG_CLIENT_TYPE_NUM_MNEMO_VOC,
    SSTAG_SERVEUR_TYPE_NUM_MNEMO_VOC,
  };

#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
