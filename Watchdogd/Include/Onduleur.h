/**********************************************************************************************************/
/* Watchdogd/Include/Onduleur.h   Header et constantes des modules ONDULEUR Watchdgo 2.0                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar. 10 nov. 2009 16:35:20 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _ONDULEUR_H_
 #define _ONDULEUR_H_
 #include <upsclient.h>

 #define ONDULEUR_PORT_TCP    3493                        /* Port de connexion TCP pour accès aux modules */
 #define ONDULEUR_RETRY       1800                    /* 3 moinutes entre chaque retry si pb de connexion */

 #define NOM_TABLE_MODULE_ONDULEUR   "modbus_onduleurs"

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
    guint bit;                                       /* Bit interne B d'etat communication avec le module */
    gchar host[32];                                                     /* Adresses IP du module ONDULEUR */

    UPSCONN_t *upsconn;                                                        /* Connexion UPS à l'onduleur */
    gboolean started;                                                                  /* Est-il actif ?? */
    guint nbr_deconnect;
    time_t date_retente;
  };

/*********************************************** Déclaration des prototypes *******************************/
 extern void Run_onduleur ( void );                                                    /* Dans Onduleur.c */
#endif
/*--------------------------------------------------------------------------------------------------------*/

