/**********************************************************************************************************/
/* Client/client.h      Déclarations générale watchdog client                                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      mar 03 jun 2003 10:39:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #ifndef _CLIENT_H_
 #define _CLIENT_H_
 #include <glib.h>

 #include "Cst_utilisateur.h"
 #include "Reseaux.h"
 #include "Config_cli.h"

 #define PROGRAMME          "Watchdog-client"

 #define EXIT_ERREUR       -1                                               /* Sortie sur erreur inconnue */
 #define EXIT_OK           0                                                            /* Sortie normale */
 #define EXIT_INACTIF      1                                             /* Un fils est mort d'inactivité */

 #define REPERTOIR_CONF    ".watchdog"         /* Repertoire ou sont stocké les fichiers de configuration */
 #define FICHIER_CERTIF_CA      "cacert.pem"                 /* Certificat de l'autorite de certification */
 #define FICHIER_CERTIF_CLIENT       "clientsigne.pem"
 #define FICHIER_CERTIF_CLEF_CLIENT  "clientkey.pem"
 
 enum
  { INERTE,
    ATTENTE_CONNEXION_SSL,
    ENVOI_IDENT,
    ATTENTE_AUTORISATION,
    CONNECTE,
    VALIDE,
    ENVOI_GIF
  };

 struct CLIENT
  { guint32 mode;
    guint32 id;
    GList *gids;
    struct CONNEXION *connexion;
    gchar user[NBR_CARAC_LOGIN_UTF8+1]; /* encore utile ??? */
    gchar password[NBR_CARAC_LOGIN_UTF8+1]; /* idem */
    gchar serveur[TAILLE_NOM_SERVEUR+1]; /* idem */
    struct
     { gint32 id_fichier_gif;                  /* Identificateur de fichier gif en cours d'envoi au serveur */
       gchar *buffer;                                        /* Doit contenir aussi l'entete CMD_ID_ICONE */
     } transfert;
  };
 #endif
/*--------------------------------------------------------------------------------------------------------*/
