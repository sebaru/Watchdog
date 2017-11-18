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
 #define TEMPS_UPDATE_CADRAN      20                                  /* Rafraichissement des cadrans toutes les 2 secondes */
 #define TEMPS_PULSE               20                                                           /* Envoi d'un pulse au client */

 enum
  { ENVOI_INTERNAL,                                                  /* Envoi des informations internes à la librairie Reseau */
    ATTENTE_CONNEXION_SSL,                                                               /* Veut-il crypter les connexions ?? */
    WAIT_FOR_IDENT,                                                    /* Permet de demander l'identification du client lourd */
    WAIT_FOR_NEWPWD,                                                        /* Si l'utilisateur doit changer son mot de passe */

    VALIDE,

    ENVOI_ICONE_FOR_ATELIER,
    ENVOI_CLASSE_FOR_ATELIER,
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

    GSList *Liste_bit_cadrans;                            /* Ensemble des bits EAxx utilisés pour les synoptiques visualisés */

/* Communication des Evenements depuis SSRV vers les clients */
    GSList *Liste_events;                                                                       /* Liste des evenements recus */

/* Communication des Histo depuis SSRV vers les clients */
    GSList *Liste_histo;

/* Communication des fichiers de travail vers les clients */
    GSList *Liste_file;

    gchar *Source_DLS_new;                                                    /* Nouvelle source DLS en cours de récupération */
    guint taille_Source_DLS_new;                                                     /* Taille en mémoire du buffer ci dessus */
    gint id_creation_message_mp3;                                              /* ID fichier message mp3 en cours de creation */
    gint classe_icone;                                                            /* Classe d'icone en cours de visualisation */
    gint date_next_send_cadran;                                             /* Date du prochain envoi des cadrans au client */
    struct CMD_CRITERE_HISTO_MSGS requete;                                       /* Pour la sauvegarde de la requete en cours */
    struct CMD_TYPE_SOURCE_DLS dls;                                         /* Pour la sauvegarde et compilation dls en cours */

    struct CMD_TYPE_SYNOPTIQUE *syn_to_send;             /* Structure du synoptique en cours d'envoi (atelier ou supervision) */
  };     

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
