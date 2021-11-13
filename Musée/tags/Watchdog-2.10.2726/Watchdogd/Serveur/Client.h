/******************************************************************************************************************************/
/* Watchdogd/Include/Client.h      Définition d'un client watchdog                                                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                                           jeu 30 déc 2004 13:24:18 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Client.h
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 
 #ifndef _CLIENT_H_
 #define _CLIENT_H_

 #include <glib.h>
 #include <time.h>
 #include <pthread.h>
 
 #include "Reseaux.h"
 #include "Utilisateur_DB.h"

 #define TAILLE_MACHINE            30                                                    /* Taille max pour un nom d'hote DNS */
 #define TEMPS_UPDATE_CAPTEUR      20                                  /* Rafraichissement des capteurs toutes les 2 secondes */
 #define TEMPS_PULSE               20                                                           /* Envoi d'un pulse au client */

 enum
  { ENVOI_INTERNAL,                                                  /* Envoi des informations internes à la librairie Reseau */
    ATTENTE_CONNEXION_SSL,                                                               /* Veut-il crypter les connexions ?? */
    WAIT_FOR_IDENT,                                                    /* Permet de demander l'identification du client lourd */
    WAIT_FOR_NEWPWD,                                                        /* Si l'utilisateur doit changer son mot de passe */

    ENVOI_SYNCHRO,

    VALIDE_NON_ROOT,

    VALIDE,

    ENVOI_GROUPE_FOR_UTIL,
    ENVOI_MOTIF_ATELIER,                                                         /* Envoi des motifs associés à un synoptique */
    ENVOI_COMMENT_ATELIER,                                                     /* Envoi des comments associés à un synoptique */
    ENVOI_PASSERELLE_ATELIER,                                                  /* Envoi des passerelles dans l'atelier client */
    ENVOI_CAPTEUR_ATELIER,                                                         /* Envoi des capteur dans l'atelier client */
    ENVOI_CAMERA_SUP_ATELIER,                                                   /* Envoi des camera_sup dans l'atelier client */
    ENVOI_MOTIF_SUPERVISION,                                                             /* Envoi des motifs a la supervision */
    ENVOI_COMMENT_SUPERVISION,                                                         /* Envoi des comments à la supervision */
    ENVOI_PASSERELLE_SUPERVISION,                                             /* Envoi des infos passerelles à la supervision */
    ENVOI_PALETTE_SUPERVISION,                                                   /* Envoi des infos palettes à la supervision */
    ENVOI_CAPTEUR_SUPERVISION,                                                         /* Envoi des capteurs à la supervision */
    ENVOI_CAMERA_SUP_SUPERVISION,                                                    /* Envoi des camera_sup à la supervision */
    ENVOI_IXXX_SUPERVISION,
    ENVOI_GROUPE_FOR_SYNOPTIQUE,
    ENVOI_GROUPE_FOR_PROPRIETE_SYNOPTIQUE,                          /* Envoi des groupes pour la fenetre propriete synoptique */
    ENVOI_ICONE_FOR_ATELIER,
    ENVOI_CLASSE_FOR_ATELIER,
    ENVOI_SYNOPTIQUE_FOR_ATELIER,                                          /* Envoi de la liste des syn pour choix passerelle */
    ENVOI_SYNOPTIQUE_FOR_ATELIER_PALETTE,                                  /* Envoi de la liste des syn pour choix de palette */
    ENVOI_PALETTE_FOR_ATELIER_PALETTE,

    DECONNECTE                                                                                            /* Deconnexion SOFT */
  };

 struct CLIENT                                    /* Définition de la structure de travail et de gestion des clients distants */
  { gchar machine[TAILLE_MACHINE+1];                                              /* Le nom (ou adrIP) de sa machine distante */
    gint  ssrv_id;                                                                       /* Numero de sous serveur de gestion */
    X509 *certif;                                                                                           /* Certificat ssl */
    struct REZO_CLI_IDENT ident;                                                     /* Nom complet de l'utilisateur en cours */
    struct CMD_TYPE_UTILISATEUR *util;
    guchar mode;                                            /* Ce client est-il valide ou est-ce un gogol qui veut rentrer ?? */
    guchar defaut;                                                                                /* Defaut d'envoi au client */
    time_t date_connexion;                                                                                  /* Date connexion */
    guint  pulse;                                                                                          /* pulse du client */
    struct CONNEXION *connexion;                                           /* Connexion distante pour dialogue client-serveur */

    pthread_mutex_t mutex_struct_used;                    /* Zone critique: Compteurs du nombre d'utilisation de la structure */
    guint struct_used;                                                     /* Nombre de process utilisant la structure CLIENT */

/* Affichage initial */
    GSList *Liste_new_motif;                               /* Liste des changements de motifs a traiter et envoyer aux client */
    GSList *Liste_bit_syns;                                /* Ensemble des bits CTRL utilisés pour les synoptiques visualisés */
    GList *bit_init_syn;                           /* Ensemble des bits CTRL utilisés par le syn supervision en cours d'envoi */

    GList *bit_capteurs;                                   /* Ensemble des bits EAxx utilisés pour les synoptiques visualisés */
    GList *bit_init_capteur;                       /* Ensemble des bits CTRL utilisés par le syn supervision en cours d'envoi */

/* Communication des Evenements depuis SSRV vers les clients */
    GSList *Liste_events;                                                                       /* Liste des evenements recus */

/* Communication des Histo depuis SSRV vers les clients */
    GSList *Liste_histo;

/* Communication des fichiers de travail vers les clients */
    GSList *Liste_file;

/* Courbes en temps réel */
    struct CMD_TYPE_COURBE courbe;                                        /* Structure parametres Proto_ajouter_courbe_thread */
    GList *courbes;                                              /* Ensemble des entrees analogiques monitorées par le client */

    gint id_creation_plugin_dls;                                                 /* ID fichier du plugin en cours de creation */
    gint id_creation_message_mp3;                                              /* ID fichier message mp3 en cours de creation */
    gint classe_icone;                                                            /* Classe d'icone en cours de visualisation */
    gint date_next_send_capteur;                                             /* Date du prochain envoi des capteurs au client */
    struct CMD_CRITERE_HISTO_MSGS requete;                                       /* Pour la sauvegarde de la requete en cours */
    struct CMD_TYPE_SOURCE_DLS dls;                                         /* Pour la sauvegarde et compilation dls en cours */

    struct CMD_TYPE_SYNOPTIQUE syn;                      /* Structure du synoptique en cours d'envoi (atelier ou supervision) */

    struct CMD_HISTO_COURBE histo_courbe;                            /* Structure pour travailler sur les historiques courbes */
  };     

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
