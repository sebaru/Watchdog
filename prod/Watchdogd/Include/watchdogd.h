/******************************************************************************************************************************/
/* Watchdogd/watchdogd.h      Déclarations générales watchdog                                                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          sam 11 avr 2009 12:23:32 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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

 #include <glib.h>
 #include <pthread.h>
 #include <errno.h>

 #include "Reseaux.h"
 #include "config.h"
 #include "Db.h"
 #include "Config.h"
 #include "Archive.h"
 #include "Admin.h"
 #include "Message_DB.h"
 #include "Camera_DB.h"
 #include "Dls.h"
 #include "Histo_DB.h"
 #include "Synoptiques_DB.h"
 #include "Mnemonique_DB.h"
 #include "Icones_DB.h"
 #include "Proto_traductionDLS.h"

 extern struct PARTAGE *Partage;                                                 /* Accès aux données partagées des processes */

 #define EXIT_ERREUR       -1                                                                   /* Sortie sur erreur inconnue */
 #define EXIT_OK           0                                                                                /* Sortie normale */
 #define EXIT_INACTIF      1                                                                 /* Un fils est mort d'inactivité */

 #define VERROU_SERVEUR              "watchdogd.lock"
 #define FICHIER_EXPORT              "export.wdg"

 #define NUM_EA_SYS_BITS_PER_SEC        125                    /* Numéro d'EA de reference pour le nbr de bit dls par seconde */
 #define NUM_EA_SYS_TOUR_DLS_PER_SEC    124                   /* Numéro d'EA de reference pour le nbr de tour dls par seconde */
 #define NUM_EA_SYS_DLS_WAIT            123                      /* Numéro d'EA de reference pour le temps d'attente par tour */

 #define MAX_ENREG_QUEUE               1500                /* Nombre maximum d'enregistrement dans une queue de communication */

 struct LIBRAIRIE
  { pthread_t TID;                                                                                   /* Identifiant du thread */
    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    void *dl_handle;                                                                     /* handle de gestion de la librairie */
    gchar nom_fichier[128];                                                                 /* Nom de fichier de la librairie */
    gchar admin_prompt[32];                                                            /* Prompt auquel va répondre le thread */
    gchar admin_help[64];                                                              /* Designation de l'activité du thread */

    gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_debug;                                                    /* TRUE si le thread doit tourner en mode debug */
    gboolean Thread_sigusr1;                                                          /* TRUE si le thread doit gerer le USR1 */

    void (*Run_thread)( struct LIBRAIRIE *lib );                                  /* Fonction principale de gestion du thread */
                                                                                 /* Fonction de gestion des commandes d'admin */
    void (*Admin_command)( struct CONNEXION *connexion, gchar *ligne );
  };

 struct COM_DB                                                                 /* Interfaçage avec le code de gestion des BDD */
  { pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    GSList *Liste;                                                              /* Liste des requetes en cours de realisation */
  };

 struct COM_MSRV                                                            /* Communication entre DLS et le serveur Watchdog */
  { gboolean Thread_run;                                    /* TRUE si le thread tourne, FALSE pour lui demander de s'arreter */
    gboolean Thread_reboot;                                                  /* TRUE si le reboot doit suivre une RAZ mémoire */
    gboolean Thread_clear_reboot;                                            /* TRUE si le reboot doit suivre une RAZ mémoire */
    gboolean Thread_reload;                                              /* TRUE si le thread doit recharger sa configuration */
    gboolean Thread_sigusr1;                                                          /* TRUE si le thread doit gerer le USR1 */

    pthread_mutex_t synchro;                                                              /* Bit de synchronisation processus */
    GSList *liste_msg_repeat;                                                          /* liste de struct MSGDB msg a envoyer */
                                                                       /* Distribution aux threads (par systeme d'abonnement) */
    GSList *liste_msg;                                                                 /* liste de struct MSGDB msg a envoyer */
    GSList *liste_i;                                                             /* liste de I a traiter dans la distribution */
    GSList *liste_a;                                                             /* liste de A a traiter dans la distribution */
    GSList *liste_Event;                                                     /* liste de Event a traiter dans la distribution */

    pthread_mutex_t synchro_Liste_abonne_msg;                                             /* Bit de synchronisation processus */
    GSList *Liste_abonne_msg;                                                          /* liste de struct MSGDB msg a envoyer */

    GSList *Librairies;                                                        /* Liste des librairies chargées pour Watchdog */
  };

 struct PARTAGE                                                                            /* Structure des données partagées */
  { gint  taille_partage;
    gint  shmid;
    gchar version[16];
    time_t start_time;                                                                         /* Date de start de l'instance */
    guint top;                                                                         /* Gestion des contraintes temporelles */
    guint top_cdg_plugin_dls;                                                        /* Top de chien de garde des plugins DLS */
    guint audit_bit_interne_per_sec;     
    guint audit_bit_interne_per_sec_hold;
    guint audit_tour_dls_per_sec;     
    guint audit_tour_dls_per_sec_hold;
                                                                                                    /* Interfacage avec D.L.S */
    struct COM_DB com_db;                                                      /* Interfaçage avec le code de gestion des BDD */
    struct COM_MSRV com_msrv;                                                                        /* Changement du à D.L.S */
    struct COM_DLS com_dls;                                                                       /* Changement du au serveur */
    struct COM_ARCH com_arch;                                                                      /* Com avec le thread ARCH */
    struct COM_ADMIN com_admin;                                                                   /* Com avec le thread ADMIN */

    struct CPT_HORAIRE ch [ NBR_COMPTEUR_H ];
    struct CPT_IMP ci [ NBR_COMPTEUR_IMP ];
    struct ANALOG_INPUT ea [ NBR_ENTRE_ANA ];
    guchar m [ (NBR_BIT_MONOSTABLE>>3) + 1 ];                                      /* Monostables du DLS (DLS=rw, Sserveur=r) */
    struct DIGITAL_INPUT e [ NBR_ENTRE_TOR ];
    struct SORTIE_TOR a [ NBR_SORTIE_TOR ];
    guchar b [ (NBR_BIT_BISTABLE>>3) + 1 ];                                                                      /* Bistables */
    struct MESSAGES g [ NBR_MESSAGE_ECRITS ];                                                   /* Message vers veille et syn */
    struct I_MOTIF i[ NBR_BIT_CONTROLE ];                                                               /* DLS=rw, Sserveur=r */
    struct TEMPO Tempo_R[NBR_TEMPO];
    struct REGISTRE registre[NBR_REGISTRE];
  };

/************************************************ Définitions des prototypes **************************************************/
 extern void Charger_config_bit_interne( void );                                                          /* Dans Watchdogd.c */
 extern gint Activer_ecoute ( void );                                                                        /* Dans ecoute.c */

 extern struct PARTAGE *_init ( void );                                                                         /* Dans shm.c */
 extern struct PARTAGE *Shm_init ( void );
 extern gboolean Shm_stop ( struct PARTAGE *partage );
 extern void *w_malloc0( gint size, gchar *justification );
 extern void w_free( void *ptr, gchar *justification );

 extern void Stopper_fils ( gint flag );                                                                    /* Dans process.c */
 extern gboolean Demarrer_dls ( void );
 extern gboolean Demarrer_arch ( void );
 extern gboolean Demarrer_admin ( void );
 extern void Charger_librairies ( void );
 extern void Decharger_librairies ( void );
 extern gboolean Start_librairie ( struct LIBRAIRIE *lib );
 extern gboolean Stop_librairie ( struct LIBRAIRIE *lib );
 extern struct LIBRAIRIE *Charger_librairie_par_prompt ( gchar *nom_fichier );
 extern gboolean Decharger_librairie_par_prompt ( gchar *nom_fichier );

 extern void Gerer_arrive_Events ( void );                                                           /* Dans distrib_Events.c */
 extern void Abonner_distribution_events ( void (*Gerer_event) (struct CMD_TYPE_MSRV_EVENT *event), gchar *thread );
 extern void Desabonner_distribution_events ( void (*Gerer_event) (struct CMD_TYPE_MSRV_EVENT *event) );
 extern void Send_Event ( gchar *instance, gchar *thread, guint type, gchar *objet, gfloat val_float );
 extern void Envoyer_Event_msrv( struct CMD_TYPE_MSRV_EVENT *event );
 extern void Gerer_arrive_Axxx_dls ( void );
 
 extern void Gerer_arrive_MSGxxx_dls ( void );                                                       /* Dans distrib_MSGxxx.c */
 extern void Gerer_histo_repeat ( void );
 extern void Abonner_distribution_histo ( void (*Gerer_histo) (struct CMD_TYPE_HISTO *histo) );
 extern void Desabonner_distribution_histo ( void (*Gerer_histo) (struct CMD_TYPE_HISTO *histo) );

 extern void Gerer_arrive_Ixxx_dls ( void );                                                           /* Dans distrib_Ixxx.c */
 extern void Abonner_distribution_motif ( void (*Gerer_motif) (gint num) );
 extern void Desabonner_distribution_motif ( void (*Gerer_motif) (gint num) );

 #endif
/*----------------------------------------------------------------------------------------------------------------------------*/
