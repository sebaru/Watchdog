/**********************************************************************************************************/
/* Include/Reseaux_connexion.h:   Sous_tag de connexion utilisé pour watchdog 2.0   par lefevre Sebastien */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 13:46:48 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

#ifndef _RESEAUX_CONNEXION_H_
 #define _RESEAUX_CONNEXION_H_

 #include "Cst_utilisateur.h"
 #include "Version.h"

 struct REZO_CLI_IDENT
  { gchar  nom [ NBR_CARAC_LOGIN_UTF8 + 1 ];
    gchar  password [ NBR_CARAC_LOGIN_UTF8 + 1 ];
    gchar  version [ TAILLE_VERSION + 1 ];
    time_t version_d;                                                    /* Version des données du client */
  };

 struct REZO_SRV_IDENT
  { gchar comment[80];
  };

 enum 
  { SSTAG_CLIENT_IDENT,                                               /* Le client s'identifie au serveur */

    SSTAG_SERVEUR_AUTORISE,                                       /* Le serveur autorise ou non le client */
    SSTAG_SERVEUR_REFUSE,                                         /* Le serveur autorise ou non le client */
    SSTAG_SERVEUR_ACCOUNT_DISABLED,                                /* Le compte utilisateur est desactivé */
    SSTAG_SERVEUR_ACCOUNT_EXPIRED,                                    /* Le compte utilisateur est expiré */
    SSTAG_SERVEUR_CHANGEPASS,                                  /* L'utilisateur doit changer son password */
    SSTAG_CLIENT_SETPASSWORD,
    SSTAG_SERVEUR_CLI_VALIDE,                       /* Le client est passé valide du point de vue serveur */
    SSTAG_CLIENT_OFF,                                                          /* Le client se deconnecte */
    SSTAG_SERVEUR_OFF,                                                             /* Le serveur se coupe */
    SSTAG_SERVEUR_PULSE
  };

#endif
/*--------------------------------------------------------------------------------------------------------*/

