/******************************************************************************************************************************/
/* Include/Reseaux.h:   Utilisation/Gestion du reseaux pour watchdog 2.0                       par lefevre Sebastien          */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          jeu 13 mai 2004 12:01:49 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Reseaux.h
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

#ifndef _RESEAUX_H_
 #define _RESEAUX_H_

 #include <glib.h>
 #include <pthread.h>
 #include <openssl/ssl.h>
 #include "Erreur.h"
 #include "Reseaux_gtk_message.h"
 #include "Reseaux_connexion.h"
 #include "Reseaux_icone.h"
 #include "Reseaux_utilisateur.h"
 #include "Reseaux_synoptique.h"
 #include "Reseaux_dls.h"
 #include "Reseaux_message.h"
 #include "Reseaux_mnemonique.h"
 #include "Reseaux_supervision.h"
 #include "Reseaux_camera.h"
 #include "Reseaux_histo.h"
 #include "Reseaux_atelier.h"
 #include "Reseaux_courbe.h"
 #include "Reseaux_histo_courbe.h"
 #include "Reseaux_fichier.h"
 #include "Reseaux_admin.h"
 #include "Reseaux_satellite.h"

 #define TIMEOUT_BUFFER_PLEIN   30                              /* 1 seconde max d'attente de disponibilite du tampon d'envoi */

 struct ENTETE_CONNEXION
  { gint32  ss_tag;
    gint32  tag;
    gint32  taille_donnees;
  };

 #define  RECU_RIEN                0
 #define  RECU_OK                  1
 #define  RECU_EN_COURS            2
 #define  RECU_ERREUR              3
 #define  RECU_ERREUR_CONNRESET    4

 struct CONNEXION
  { struct ENTETE_CONNEXION entete;
    gchar  *donnees;
    gint32  index_entete;
    gint32  index_donnees;
    gint32  taille_bloc;
    gint32  socket;
    pthread_mutex_t mutex_write;                                  /* Zone critique: envoi des données au client par le reseau */
    time_t last_use;
    SSL    *ssl;
    struct LOG *log;
  };

 struct CMD_ENREG                                /* Pour le comptage et la description des données en cours de telechargement */
  { gint32 num;
    gchar comment[ 30 ];
  };

 enum
  { TAG_INTERNAL,                                                                         /* Gestion interne librairie Reseau */
    TAG_GTK_MESSAGE,                                                                                  /* Envoi de message GTK */
    TAG_FICHIER,                                                               /* Echange de fichiers entre serveur et client */
    TAG_CONNEXION,                                                                                  /* Gestion des connexions */
    TAG_ICONE,                                                                                          /* Gestion des icones */
    TAG_DLS,                                                                        /* Gestion des plugins et des sources DLS */
    TAG_UTILISATEUR,                                                                      /* Gestion des utilisateurs/groupes */
    TAG_SYNOPTIQUE,                                                                                /* Gestion des synoptiques */
    TAG_MNEMONIQUE,                                                                                /* Gestion des mnemoniques */
    TAG_MESSAGE,                                                                                      /* Gestion des messages */
    TAG_SUPERVISION,                                                                     /* Gestions de la partie supervision */
    TAG_HISTO,                                                                                          /* Gestions des histo */
    TAG_ATELIER,                                                                                     /* Gestions de l'atelier */
    TAG_COURBE,                                                                                       /* Gestions des courbes */
    TAG_HISTO_COURBE,                                                                     /* Gestions des historiques courbes */
    TAG_CAMERA,                                                                                         /* Gestion des camera */
    TAG_ADMIN,                                                    /* Utilisation des commandes d'admin depuis le client lourd */
    TAG_SATELLITE,                                                                             /* Echange vers les satellites */
  };

 enum
  { SSTAG_INTERNAL_PAQUETSIZE,                                                           /* Taille d'un bloc d'échange reseau */
    SSTAG_INTERNAL_SSLNEEDED,                                                          /* La communication doit passer en ssl */
    SSTAG_INTERNAL_SSLNEEDED_WITH_CERT,                                       /* SSL + authentification reseau via certificat */
    SSTAG_INTERNAL_END                                                                           /* Fin des echanges internes */
  };

/********************************************* Définitions des prototypes *****************************************************/
 extern gint Recevoir_reseau( struct CONNEXION *Connexion );
 extern struct CONNEXION *Nouvelle_connexion ( struct LOG *Log, gint socket, gint taille_bloc );
 extern gint Attendre_envoi_disponible ( struct CONNEXION *connexion );
 extern gint Envoyer_reseau( struct CONNEXION *connexion,
                             gint tag, gint ss_tag,
                             gchar *buffer, gint taille_buffer );
 extern void Fermer_connexion ( struct CONNEXION *connexion );
 extern gint Reseau_tag( struct CONNEXION *connexion );
 extern gint Reseau_ss_tag( struct CONNEXION *connexion );

 extern gchar *Nom_certif_signataire( X509 *certif );                                                           /* Dans Ssl.c */
 extern gchar *Nom_certif( X509 *certif );
#endif
/*----------------------------------------------------------------------------------------------------------------------------*/
