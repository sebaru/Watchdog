/**********************************************************************************************************/
/* Watchdogd/client.h      Définition d'un client watchdog                                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                       jeu 30 déc 2004 13:24:18 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #ifndef _CLIENT_H_
 #define _CLIENT_H_

 #include <glib.h>
 #include <time.h>
 #include <pthread.h>
 
 #include "Reseaux.h"
 #include "Utilisateur_DB.h"

 #define TAILLE_MACHINE  30                                          /* Taille max pour un nom d'hote DNS */

 enum
  { ATTENTE_CONNEXION_SSL,                                           /* Veut-il crypter les connexions ?? */
    ATTENTE_IDENT,                                /* Permet de demander l'identification de l'utilisateur */
    ENVOI_AUTORISATION,                                                 /* Envoi de l'autorisation ou pas */

    ATTENTE_NEW_PASSWORD,                               /* Si l'utilisateur doit changer son mot de passe */

    ENVOI_DONNEES,                                        /* Envoi des données GIF... au client si besoin */
    ENVOI_HISTO,
    ENVOI_INHIB,
    ENVOI_PALETTE,

    VALIDE_NON_ROOT,

    ENVOI_GIF,
    VALIDE,

    ENVOI_GROUPE_FOR_UTIL,
    ENVOI_DLS,
    ENVOI_SYNOPTIQUE,
    ENVOI_MNEMONIQUE,
    ENVOI_MNEMONIQUE_FOR_COURBE,
    ENVOI_MNEMONIQUE_FOR_HISTO_COURBE,                 /* Envoi des infos mnemo pour la page histo_courbe */
    ENVOI_MOTIF_ATELIER,                                     /* Envoi des motifs associés à un synoptique */
    ENVOI_COMMENT_ATELIER,                                 /* Envoi des comments associés à un synoptique */
    ENVOI_PASSERELLE_ATELIER,                              /* Envoi des passerelles dans l'atelier client */
    ENVOI_CAPTEUR_ATELIER,                                     /* Envoi des capteur dans l'atelier client */
    ENVOI_MOTIF_SUPERVISION,                                         /* Envoi des motifs a la supervision */
    ENVOI_COMMENT_SUPERVISION,                                     /* Envoi des comments à la supervision */
    ENVOI_PASSERELLE_SUPERVISION,                         /* Envoi des infos passerelles à la supervision */
    ENVOI_PALETTE_SUPERVISION,                               /* Envoi des infos palettes à la supervision */
    ENVOI_CAPTEUR_SUPERVISION,                                     /* Envoi des capteurs à la supervision */
    ENVOI_IXXX_SUPERVISION,
    ENVOI_GROUPE_FOR_SYNOPTIQUE,
    ENVOI_GROUPE_FOR_PROPRIETE_SYNOPTIQUE,      /* Envoi des groupes pour la fenetre propriete synoptique */
    ENVOI_ICONE,
    ENVOI_ICONE_FOR_ATELIER,
    ENVOI_CLASSE,
    ENVOI_CLASSE_FOR_ATELIER,
    ENVOI_SYNOPTIQUE_FOR_ATELIER,                      /* Envoi de la liste des syn pour choix passerelle */
    ENVOI_SYNOPTIQUE_FOR_ATELIER_PALETTE,              /* Envoi de la liste des syn pour choix de palette */
    ENVOI_PALETTE_FOR_ATELIER_PALETTE,
    ENVOI_HISTO_HARD,
    ENVOI_ENTREEANA,

    ENVOI_SOURCE_DLS,
    ENVOI_SCENARIO,                                                       /* Envoi des scenario au client */
    ENVOI_SCENARIO_SUP,                                                   /* Envoi des scenario au client */

    DECONNECTE                                                                        /* Deconnexion SOFT */
  };

 struct LISTE_FICH
  { gint taille;
    gchar fichier_absolu[80];
    gchar fichier[40];
  };

 struct CLIENT                /* Définition de la structure de travail et de gestion des clients distants */
  { gchar machine[TAILLE_MACHINE+1];                          /* Le nom (ou adrIP) de sa machine distante */
    struct DB *Db_watchdog;                                               /* Lien avec la base de données */
    struct REZO_CLI_IDENT ident;                                 /* Nom complet de l'utilisateur en cours */
    struct UTILISATEURDB *util;
    guchar mode;                        /* Ce client est-il valide ou est-ce un gogol qui veut rentrer ?? */
    guchar defaut;                                                            /* Defaut d'envoi au client */
    time_t        seconde;                                                        /* Seconde de connexion */
    guint  pulse;                                                                      /* pulse du client */
    struct
     { int fd;                                        /* descripteur du fichier actuellement en transfert */
       gchar *buffer;                                                        /* Buffer d'envoi de donnees */
       gint en_cours;
       gint index;                                    /* Pour la lecture 'propre' de capteurs DLS en UTF8 */
       GList *fichiers;                                                        /* Une GList de LISTE_FICH */
       gint taille;                                               /* Taille totale des fichiers à envoyer */
     } transfert;
    struct CONNEXION *connexion;                       /* Connexion distante pour dialogue client-serveur */

    pthread_mutex_t mutex_write;              /* Zone critique: envoi des données au client par le reseau */
    gint Id_serveur;                                               /* Numero du serveur servant ce client */

    pthread_mutex_t mutex_struct_used;/* Zone critique: Compteurs du nombre d'utilisation de la structure */
    guint struct_used;                                 /* Nombre de process utilisant la structure CLIENT */

/* Affichage initial */
    GList *bit_syns;                   /* Ensemble des bits CTRL utilisés pour les synoptiques visualisés */
    GList *bit_init_syn;       /* Ensemble des bits CTRL utilisés par le syn supervision en cours d'envoi */

    GList *bit_capteurs;               /* Ensemble des bits EAxx utilisés pour les synoptiques visualisés */
    GList *bit_init_capteur;   /* Ensemble des bits CTRL utilisés par le syn supervision en cours d'envoi */

/* Courbes */
    GList *courbes;                          /* Ensemble des entrees analogiques monitorées par le client */

    gint id_creation_plugin_dls;                             /* ID fichier du plugin en cours de creation */
    gint classe_icone;                                        /* Classe d'icone en cours de visualisation */
    gint num_supervision;                                 /* Numéro du synoptique en cours de supervision */
    struct CMD_WANT_SCENARIO_MOTIF sce;                     /* numéro du monostable du scenario a envoyer */
    struct CMD_REQUETE_HISTO_HARD requete;                   /* Pour la sauvegarde de la requete en cours */
    struct CMD_EDIT_SOURCE_DLS dls;                  /* Pour la sauvegarde de la compilation dls en cours */

    struct CMD_ID_COURBE courbe;                      /* Structure parametres Proto_ajouter_courbe_thread */
    struct CMD_ID_SYNOPTIQUE syn;                     /* Structure parametres Proto_ajouter_courbe_thread */

    struct CMD_HISTO_COURBE histo_courbe;        /* Structure pour travailler sur les historiques courbes */
  };     

 #endif
/*--------------------------------------------------------------------------------------------------------*/
