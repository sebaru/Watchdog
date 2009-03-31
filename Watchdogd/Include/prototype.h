/**********************************************************************************************************/
/* Watchdogd/prototype.h      Définition d'un client watchdog                                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 03 jun 2003 10:39:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #ifndef _PROTOTYPE_H_
 #define _PROTOTYPE_H_

 #include <glib.h>
 #include <openssl/ssl.h>
 #include "Erreur.h"
 #include "Client.h"
/*---------------------------- Déclarations des prototypes de fonctions ----------------------------------*/

                                                                                       /* Dans Watchdog.c */
 extern void Charger_eana ( struct DB *Db_watchdog );
 extern void Charger_scenario ( struct DB *Db_watchdog );

 extern gint Activer_ecoute ( void );                                                    /* Dans ecoute.c */

 extern gboolean Activer_ecoute_admin ( void );                                           /* Dans Admin.c */
 extern void Desactiver_ecoute_admin ( void );
 extern void Gerer_fifo_admin ( void );

 extern struct PARTAGE *Shm_init ( void );                                                  /* Dans shm.c */
 extern gboolean Shm_stop ( struct PARTAGE *partage );

 extern void Gerer_jeton ( void );                                                      /* Dans process.c */
 extern void Gerer_zombie ( void );
 extern void Gerer_manque_process ( void );
 extern void Stopper_fils ( void );
 extern gboolean Demarrer_dls ( void );
 extern gboolean Demarrer_rs485 ( void );
 extern gboolean Demarrer_modbus ( void );
 extern gboolean Demarrer_sms ( void );
 extern gboolean Demarrer_arch ( void );

 extern void Gerer_arrive_MSGxxx_dls ( struct DB *Db_watchdog );                 /* Dans distrib_MSGxxx.c */

 extern void Gerer_arrive_Ixxx_dls ( void );                                       /* Dans distrib_Ixxx.c */

 extern void Run_serveur ( gint id );                                                   /* Dans serveur.c */

 extern SSL_CTX *Init_ssl ( void );                                                         /* Dans ssl.c */
 extern void Connecter_ssl( struct CLIENT *client );

 extern gchar *Init_db_watchdog( void );                                                 /* Dans initdb.c */
 
 extern void Run_dls ( void );                                                          /* Dans The_dls.c */
 
 extern gboolean Traduire_DLS( struct LOG *log, gint id );                               /* Dans Interp.c */

 extern void Run_rs485 ( void );                                                          /* Dans Rs485.c */

 extern void Run_sms ( void );                                                              /* Dans Sms.c */
 extern void Envoyer_sms ( gchar *libelle );

 extern void Run_arch ( void );                                                         /* Dans Archive.c */
 extern void Ajouter_arch( gint type, gint num, gint etat );

 extern void Run_modbus ( void );                                                        /* Dans Modbus.c */
 #endif
/*--------------------------------------------------------------------------------------------------------*/
