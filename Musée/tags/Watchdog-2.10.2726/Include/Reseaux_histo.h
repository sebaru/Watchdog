/**********************************************************************************************************/
/* Include/Reseaux_histo.h:   Sous_tag de histo pour watchdog 2.0 par lefevre Sebastien                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Reseaux_histo.h
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

#ifndef _RESEAUX_HISTO_H_
 #define _RESEAUX_HISTO_H_

 struct CMD_TYPE_HISTO
  { guint id;                                                        /* id unique dans la base de données */
    gboolean alive;                                             /* Le message est-il encore d'actualité ? */
    struct CMD_TYPE_MESSAGE msg;
    gchar nom_ack [ NBR_CARAC_LOGIN_UTF8 + 1 ];
    guint date_create_sec;
    guint date_create_usec;
    guint date_fixe;
    guint date_fin;
  };

 struct CMD_RESPONSE_HISTO_MSGS
  { struct CMD_TYPE_HISTO histo;
    gint32 page_id;                    /* Numéro de la page du client sur laquelle afficher les résultats */
  };

 struct CMD_CRITERE_HISTO_MSGS
  { guint  num;                                                                /* Numero unique historique */
    gchar  libelle[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    gchar  groupage[NBR_CARAC_LIBELLE_MSG_UTF8+1];
    guint  type;                                                       /* Etat, prealarme, defaut, alarme */
    guint  date_create_min;
    guint  date_create_max;
    gchar  nom_ack[NBR_CARAC_LOGIN_UTF8+1];
    gint32 page_id;
  };

 enum 
  { SSTAG_SERVEUR_ADDPROGRESS_HISTO,
    SSTAG_SERVEUR_ADDPROGRESS_HISTO_FIN,
    SSTAG_SERVEUR_SHOW_HISTO,
    SSTAG_SERVEUR_DEL_HISTO,
    
    SSTAG_CLIENT_ACK_HISTO,
    SSTAG_SERVEUR_ACK_HISTO,
    
    SSTAG_CLIENT_REQUETE_HISTO_MSGS,          /* Le client fait une requete sur l'historique de smessages */
    SSTAG_SERVEUR_ADDPROGRESS_REQUETE_HISTO_MSGS,                     /* Reponse à une requete histo_hard */
    SSTAG_SERVEUR_ADDPROGRESS_REQUETE_HISTO_MSGS_FIN,
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

