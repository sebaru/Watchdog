/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_histo_hard.c        Requetes de l'ihstorique à la connexion client             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 13 mar 2004 18:41:14 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>
 #include <string.h>

 #include "Reseaux.h"
 #include "Histo_DB.h"
 #include "Erreur.h"
 #include "Config.h"
 #include "Client.h"

 #include "watchdogd.h"
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Preparer_envoi_histo: convertit une structure HISTO_HARD en structure CMD_SHOW_HISTO_HARD              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_HISTO_HARD *Preparer_envoi_histo ( struct HISTO_HARDDB *histo )
  { struct CMD_SHOW_HISTO_HARD *rezo_histo;

    rezo_histo = (struct CMD_SHOW_HISTO_HARD *)g_malloc0( sizeof(struct CMD_SHOW_HISTO_HARD) );
    if (!rezo_histo) { return(NULL); }
    rezo_histo->num         = histo->histo.msg.num;
    rezo_histo->type        = histo->histo.msg.type;
    rezo_histo->date_create_sec  = histo->histo.date_create_sec;
    rezo_histo->date_create_usec = histo->histo.date_create_usec;
    rezo_histo->date_fixe   = histo->histo.date_fixe;
    rezo_histo->date_fin    = histo->date_fin;
    memcpy( &rezo_histo->nom_ack, histo->histo.nom_ack, sizeof(rezo_histo->nom_ack) );
    memcpy( &rezo_histo->libelle, histo->histo.msg.libelle, sizeof(rezo_histo->libelle) );
    memcpy( &rezo_histo->objet,   histo->histo.msg.objet, sizeof(rezo_histo->objet) );
    return( rezo_histo );
  }
/**********************************************************************************************************/
/* Envoyer_histos: Envoi des histos au client GID_USERS                                                   */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Proto_envoyer_histo_hard_thread ( struct CLIENT *client )
  { struct CMD_SHOW_HISTO_HARD *rezo_histo;
    struct CMD_REQUETE_HISTO_HARD requete;
    struct HISTO_HARDDB *histo;
    struct DB *Db_watchdog;
    struct CMD_ENREG nbr;
    SQLHSTMT hquery;
    Db_watchdog = client->Db_watchdog;

    memcpy ( &requete, &client->requete, sizeof( requete ) );

    hquery = Rechercher_histo_hardDB( Config.log, Db_watchdog, &requete );
    if (!hquery)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading histo_hard") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for ( ; ; )
     { histo = Rechercher_histo_hardDB_suite( Config.log, Db_watchdog, hquery );
       if (!histo)
        { Envoi_client ( client, TAG_HISTO, SSTAG_SERVEUR_ADDPROGRESS_REQUETE_HISTO_HARD_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       rezo_histo = Preparer_envoi_histo( histo );
       rezo_histo->page_id = requete.page_id;              /* Envoie à la bonne page d'historique cliente */
       g_free(histo);
       if (rezo_histo)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          printf("Envoi_histo_hard: num %d, type %d nom_ack %s, objet %s, libelle %s\n",
                  rezo_histo->num,rezo_histo->type,rezo_histo->nom_ack,rezo_histo->objet,rezo_histo->libelle );
          Envoi_client ( client, TAG_HISTO, SSTAG_SERVEUR_ADDPROGRESS_REQUETE_HISTO_HARD,
                         (gchar *)rezo_histo, sizeof(struct CMD_SHOW_HISTO_HARD) );
          g_free(rezo_histo);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
