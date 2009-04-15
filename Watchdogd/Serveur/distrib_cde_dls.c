/**********************************************************************************************************/
/* Watchdogd/distrib_cde_dls.c        Distribution des commandes D.L.S                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 25 mai 2006 11:20:26 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>
 #include <string.h>
 #include <unistd.h>
 #include <time.h>

 #include "Erreur.h"
 #include "Config.h"
 #include "watchdogd.h"

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
/******************************************** Prototypes de fonctions *************************************/

/**********************************************************************************************************/
/* Envoyer_commande_dls: Gestion des envois de commande DLS                                               */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Envoyer_commande_dls ( int num )
  { pthread_mutex_lock( &Partage->com_ssrv_dls.synchro );
    Partage->com_ssrv_dls.liste_m = g_list_append ( Partage->com_ssrv_dls.liste_m, GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Partage->com_ssrv_dls.synchro );
  }
/*--------------------------------------------------------------------------------------------------------*/
