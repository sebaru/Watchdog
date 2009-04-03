/**********************************************************************************************************/
/* Watchdogd/watchdogd.h      Déclarations générale watchdog                                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                       lun 15 mar 2004 17:09:35 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #ifndef _WATCHDOGD_H_
 #define _WATCHDOGD_H_

 #include <pthread.h>
 #include "Reseaux.h"
 #include "Cst_entreeana.h"
 #include "Cst_dls.h"
 #include "Cpth_DB.h"
 #include "Scenario_DB.h"

 enum
  { TOURNE,
    FIN,                                                                            /* Arret des serveurs */
    RESTART,                                                                      /* Relance des serveurs */
    RELOAD,                                                     /* Déchargement librairies + Rechargement */
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

 enum
  { TYPE_INFO_VIDE,                                                     /* Le fils a traité l'information */ 
    TYPE_INFO_NEW_HISTO,                                   /* Le fils doit traiter un nouveau message DLS */
    TYPE_INFO_DEL_HISTO,                                       /* Le fils doit traiter un message DLS OFF */
    TYPE_INFO_NEW_MOTIF                                         /* Le fils doit traiter un evenement Ixxx */
  };

 struct COM_SSRV_DLS                                             /* Communication entre le serveur et DLS */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *liste_m;                                                           /* liste des Mxxx a activer */
    GList *liste_plugin_reset;                                            /* liste des plugins a resetter */
    GList *liste_plugin_on;                                                /* liste des plugins a allumer */
    GList *liste_plugin_off;                                              /* liste des plugins a eteindre */
    gboolean sigusr1;
  };

 struct SOUS_SERVEUR
  { pthread_t pid;
    gint nb_client;
    gboolean sigusr1;
    gboolean type_info;                                          /* Acquisition de l'information actuelle */
    GList *Clients;                                         /* La liste des clients qui se sont connectés */
  };

 struct COM_DLS_MSRV                                    /* Communication entre DLS et le serveur Watchdog */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *liste_msg_on;                                           /* liste de struct MSGDB msg a envoyer */
    GList *liste_msg_off;                                          /* liste de struct MSGDB msg a envoyer */
    GList *liste_i;                                                /* liste de struct MSGDB msg a envoyer */
  };

 struct COM_DLS_RS                                                 /* Communication entre DLS et la RS485 */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    gboolean sigusr1;
  };

 struct COM_MODBUS                                                 /* Communication entre DLS et la RS485 */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    gboolean sigusr1;
  };

 struct COM_MSRV_SMS                                                   /* Communication entre MSRV et SMS */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *liste_sms;                                              /* liste de struct MSGDB msg a envoyer */
    gboolean sigusr1;
  };

 struct COM_ARCH                                                               /* Communication vers ARCH */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *liste_arch;                                             /* liste de struct MSGDB msg a envoyer */
    gboolean sigusr1;
  };

 struct ENTREE_ANA                             /* Traitement des entrées analogiques par le process rs485 */
  { gdouble min;
    gdouble max;
    gdouble val_ech;
    gint    val;
    gint    unite;
    time_t  date;
    guint   inrange;
  };

 struct COURBE
  { guint id;
    guint slot_id;
    guint type;
/*    time_t  date;*/
  };

 struct CAPTEUR
  { gint    type;                                                              /* type du bit de controle */
    guint bit_controle;
    gdouble val_ech;
  };

 struct CPT_HORAIRE
  { struct CPTH_DB cpthdb;
    guint old_top;
    gboolean actif;
  };

 struct I_MOTIF
  { gint etat;
    gint rouge;
    gint vert;
    gint bleu;
    gint cligno;
  };

 struct TEMPO
  { gint consigne;
  };

 struct PARTAGE                                                        /* Structure des données partagées */
  { gint shmid;
    gint jeton;
    guint top;                                                     /* Gestion des contraintes temporelles */
    guint top_cdg_plugin_dls;                                    /* Top de chien de garde des plugins DLS */
    guint audit_bit_interne_per_sec;     
    guint audit_bit_interne_per_sec_hold;
    guchar Arret;                                                  /* != 0 si l'arret systeme est demandé */

                                                                                /* Interfacage avec D.L.S */
    struct COM_DLS_MSRV com_dls_msrv;                                            /* Changement du à D.L.S */
    struct COM_DLS_RS com_dls_rs;                                                          /* Comm rs/dls */
    struct COM_MODBUS com_modbus;                                              /* Comm vers thread modbus */
    struct COM_MSRV_SMS com_msrv_sms;                                                    /* Comm msrv/sms */
    struct COM_SSRV_DLS com_ssrv_dls;                                         /* Changement du au serveur */
    struct COM_ARCH com_arch;                                                  /* Com avec le thread ARCH */
    struct CPT_HORAIRE ch [ NBR_COMPTEUR_H ];
    struct ENTREE_ANA ea [ NBR_ENTRE_ANA ];
    struct SCENARIO_DB scenario [ NBR_SCENARIO ];
    guint  ea_histo [ NBR_ENTRE_ANA ][TAILLEBUF_HISTO_EANA];                        /* Stockage EA en int */
    guchar m [ (NBR_BIT_MONOSTABLE>>3) + 1 ];                  /* Monostables du DLS (DLS=rw, Sserveur=r) */
    guchar e [ NBR_ENTRE_TOR>>3 ];
    guchar a [ NBR_SORTIE_TOR>>3 ];
    guchar b [ (NBR_BIT_BISTABLE>>3) + 1 ];                                                  /* Bistables */
    guchar g [ (NBR_MESSAGE_ECRITS>>3) + 1 ];                               /* Message vers veille et syn */
    struct I_MOTIF i[ (NBR_BIT_CONTROLE>>3) + 1 ];                                  /* DLS=rw, Sserveur=r */
    struct TEMPO Tempo_R[NBR_TEMPO];

                                                                   /* Interfacage serveur -> sous-serveur */
    struct CMD_SHOW_HISTO    new_histo;                                   /* Envoi d'un histo aux clients */
    struct CMD_ID_HISTO      del_histo;                                  /* Destruction d'un histo client */
    struct CMD_ETAT_BIT_CTRL new_motif;                                        /* Changement d'etat motif */

    struct SOUS_SERVEUR *Sous_serveur;
    struct SOUS_SERVEUR ss_serveur;                                            /* !! Tableau dynamique !! */
  };

 #endif
/*--------------------------------------------------------------------------------------------------------*/
