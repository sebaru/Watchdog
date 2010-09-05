/**********************************************************************************************************/
/* Watchdogd/watchdogd.h      Déclarations générale watchdog                                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 11 avr 2009 12:23:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * watchdogd.h
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
 
 #ifndef _WATCHDOGD_H_
 #define _WATCHDOGD_H_

 #include <pthread.h>
 #include "Reseaux.h"

 #include "config.h"
 #include "Db.h"
 #include "Config.h"
 #include "Archive.h"
 #include "Audio.h"
 #include "Admin.h"
 #include "Client.h"
 #include "Cpth_DB.h"
 #include "Modbus.h"
 #include "Onduleur.h"
 #include "Rs485.h"
 #include "Scenario_DB.h"
 #include "Message_DB.h"
 #include "Camera_DB.h"
 #include "Sms.h"
 #include "Dls.h"
 #include "Histo_DB.h"
 #include "Synoptiques_DB.h"
 #include "Mnemonique_DB.h"
 #include "Icones_DB.h"
 #include "EntreeANA_DB.h"
 #include "Sous_serveur.h"

 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */

 enum
  { TOURNE,
    FIN,                                                                            /* Arret des serveurs */
    RELOAD,                                              /* Déchargement librairies/Config + Rechargement */
    REBOOT,                                                                       /* Relance des serveurs */
    CLEARREBOOT,                                                       /* Relance sans import des données */
  };
 
 #define EXIT_ERREUR       -1                                               /* Sortie sur erreur inconnue */
 #define EXIT_OK           0                                                            /* Sortie normale */
 #define EXIT_INACTIF      1                                             /* Un fils est mort d'inactivité */

 #define VERROU_SERVEUR              "watchdogd.lock"
 #define FICHIER_CERTIF_CA           "cacert.pem"
 #define FICHIER_CERTIF_SERVEUR      "serveursigne.pem"
 #define FICHIER_CERTIF_CLEF_SERVEUR "serveurkey.pem"
 #define FICHIER_CLEF_PUB_RSA        "watchdogd.pub.rsa" 
 #define FICHIER_CLEF_SEC_RSA        "watchdogd.sec.rsa" 
 #define FICHIER_FIFO_ADMIN_READ     "admin.fifo.read"
 #define FICHIER_FIFO_ADMIN_WRITE    "admin.fifo.write"
 #define FICHIER_EXPORT              "export.wdg"

 enum
  { TYPE_INFO_VIDE,                                                     /* Le fils a traité l'information */ 
    TYPE_INFO_NEW_HISTO,                                   /* Le fils doit traiter un nouveau message DLS */
    TYPE_INFO_DEL_HISTO,                                       /* Le fils doit traiter un message DLS OFF */
    TYPE_INFO_NEW_MOTIF                                         /* Le fils doit traiter un evenement Ixxx */
  };

 struct COM_MSRV                                        /* Communication entre DLS et le serveur Watchdog */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *liste_msg_repeat;                                       /* liste de struct MSGDB msg a envoyer */
    GList *liste_msg_on;                                           /* liste de struct MSGDB msg a envoyer */
    GList *liste_msg_off;                                          /* liste de struct MSGDB msg a envoyer */
    GList *liste_i;                                                /* liste de struct MSGDB msg a envoyer */
    gboolean reset_motion_detect;
  };

 struct ENTREE_ANA                             /* Traitement des entrées analogiques par le process rs485 */
  { struct CMD_TYPE_ENTREEANA cmd_type_eana;
    gdouble val_ech;
    gint    val;
    time_t  date;
    guint   inrange;
  };

 struct SORTIE_TOR
  { gchar etat;
    gint last_change;
    gint changes;
  };

 struct MESSAGES
  { gchar etat;
    gint last_change;
    gint changes;
    gint next_repeat;
  };
 struct COURBE
  { guint id;
    guint slot_id;
    guint type;
/*    time_t  date;*/
  };

 struct I_MOTIF
  { gint etat;
    gint rouge;
    gint vert;
    gint bleu;
    gint cligno;
    gint last_change;
    gint changes;
  };

 struct TEMPO
  { gint consigne;
  };

 struct PARTAGE                                                        /* Structure des données partagées */
  { gint taille_partage;
    gint shmid;
    gint jeton;
    guint top;                                                     /* Gestion des contraintes temporelles */
    guint top_cdg_plugin_dls;                                    /* Top de chien de garde des plugins DLS */
    guint audit_bit_interne_per_sec;     
    guint audit_bit_interne_per_sec_hold;
    guint audit_tour_dls_per_sec;     
    guint audit_tour_dls_per_sec_hold;
    guchar Arret;                                                  /* != 0 si l'arret systeme est demandé */

                                                                                /* Interfacage avec D.L.S */
    struct COM_MSRV com_msrv;                                                    /* Changement du à D.L.S */
    struct COM_RS485 com_rs485;                                                             /* Comm rs485 */
    struct COM_MODBUS com_modbus;                                              /* Comm vers thread modbus */
    struct COM_SMS com_sms;                                                              /* Comm msrv/sms */
    struct COM_DLS com_dls;                                                   /* Changement du au serveur */
    struct COM_ARCH com_arch;                                                  /* Com avec le thread ARCH */
    struct COM_AUDIO com_audio;                                               /* Com avec le thread AUDIO */
    struct COM_ONDULEUR com_onduleur;                                      /* Com avec le thread ONDULEUR */
    struct COM_ADMIN com_admin;                                               /* Com avec le thread ADMIN */

    struct CPT_HORAIRE ch [ NBR_COMPTEUR_H ];
    struct ENTREE_ANA ea [ NBR_ENTRE_ANA ];
    struct SCENARIO_DB scenario [ NBR_SCENARIO ];
    guchar m [ (NBR_BIT_MONOSTABLE>>3) + 1 ];                  /* Monostables du DLS (DLS=rw, Sserveur=r) */
    guchar e [ NBR_ENTRE_TOR>>3 ];
    struct SORTIE_TOR a [ NBR_SORTIE_TOR ];
    guchar b [ (NBR_BIT_BISTABLE>>3) + 1 ];                                                  /* Bistables */
    struct MESSAGES g [ NBR_MESSAGE_ECRITS ];                               /* Message vers veille et syn */
    struct I_MOTIF i[ NBR_BIT_CONTROLE ];                                           /* DLS=rw, Sserveur=r */
    struct TEMPO Tempo_R[NBR_TEMPO];

                                                                   /* Interfacage serveur -> sous-serveur */
    struct CMD_TYPE_HISTO    new_histo;                                   /* Envoi d'un histo aux clients */
    struct CMD_TYPE_HISTO    del_histo;                                  /* Destruction d'un histo client */
    struct CMD_ETAT_BIT_CTRL new_motif;                                        /* Changement d'etat motif */

    struct SOUS_SERVEUR *Sous_serveur;
    struct SOUS_SERVEUR ss_serveur;                                            /* !! Tableau dynamique !! */
  };

/*************************************** Définitions des prototypes ***************************************/
 extern gint Activer_ecoute ( void );                                                    /* Dans ecoute.c */

 extern struct PARTAGE *Shm_init ( void );                                                  /* Dans shm.c */
 extern gboolean Shm_stop ( struct PARTAGE *partage );

 extern void Gerer_jeton ( void );                                                      /* Dans process.c */
 extern void Gerer_zombie ( void );
 extern void Gerer_manque_process ( void );
 extern void Stopper_fils ( gint flag );
 extern gboolean Demarrer_dls ( void );
 extern gboolean Demarrer_rs485 ( void );
 extern gboolean Demarrer_modbus ( void );
 extern gboolean Demarrer_sms ( void );
 extern gboolean Demarrer_arch ( void );
 extern gboolean Demarrer_audio ( void );
 extern gboolean Demarrer_onduleur ( void );
 extern gboolean Demarrer_admin ( void );
 extern gboolean Demarrer_motion_detect ( void );

 extern void Gerer_arrive_MSGxxx_dls ( struct DB *Db_watchdog );                 /* Dans distrib_MSGxxx.c */
 extern void Gerer_message_repeat ( struct DB *Db_watchdog );

 extern void Gerer_arrive_Ixxx_dls ( void );                                       /* Dans distrib_Ixxx.c */

 extern SSL_CTX *Init_ssl ( void );                                                         /* Dans ssl.c */
 extern void Connecter_ssl( struct CLIENT *client );

 #endif
/*--------------------------------------------------------------------------------------------------------*/
