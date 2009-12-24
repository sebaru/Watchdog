/**********************************************************************************************************/
/* Watchdogd/Include/Onduleur.h   Header et constantes des modules ONDULEUR Watchdgo 2.0                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 10 nov. 2009 16:35:20 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _ONDULEUR_H_
 #define _ONDULEUR_H_
 #include <upsclient.h>

 #define ONDULEUR_PORT_TCP    3493                        /* Port de connexion TCP pour accès aux modules */
 #define ONDULEUR_RETRY       9 /*1800*/                     /* 3 minutes entre chaque retry si pb de connexion */

 #define NOM_TABLE_MODULE_ONDULEUR   "onduleurs"

 struct COM_ONDULEUR                                                 /* Communication entre DLS et la RS485 */
  { pthread_mutex_t synchro;                                          /* Bit de synchronisation processus */
    GList *Modules_ONDULEUR;
    gboolean reload;
    guint admin_del;                                                            /* Demande de deconnexion */
    guint admin_start;                                                          /* Demande de deconnexion */
    guint admin_stop;                                                           /* Demande de deconnexion */
    guint admin_add;                                                            /* Demande de deconnexion */
  };

 struct MODULE_ONDULEUR
  { guint id;                                                 /* Numéro du module dans la base de données */
    gboolean actif;                                                        /* Le module doit-il tourner ? */
    gchar host[32];                                                     /* Adresses IP du module ONDULEUR */
    gchar ups[32];                                                      /* Adresses IP du module ONDULEUR */
    guint bit_comm;                                  /* Bit interne B d'etat communication avec le module */
    guint ea_ups_load;                                                     /* Numéro de l'EA pour le load */
    guint ea_ups_real_power;                                         /* Numéro de l'EA pour le real power */
    guint ea_battery_charge;                                    /* Numéro de l'EA pour la charge batterie */
    guint ea_input_voltage;                                        /* Numéro de l'EA pour l'input voltage */

    UPSCONN_t upsconn;                                                      /* Connexion UPS à l'onduleur */
    gboolean started;                                                                  /* Est-il actif ?? */
    guint nbr_deconnect;
    time_t date_retente;
  };

/*********************************************** Déclaration des prototypes *******************************/
 extern void Run_onduleur ( void );                                                    /* Dans Onduleur.c */
#endif
/*--------------------------------------------------------------------------------------------------------*/

