/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_histo.c        Envoi de l'ihstorique à la connexion client                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 13 mar 2004 18:41:14 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>
 #include <string.h>
 #include <pthread.h>

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
/* Preparer_envoi_histo: convertit une structure HISTO en structure CMD_SHOW_HISTO                        */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_HISTO *Preparer_envoi_histo ( struct HISTODB *histo )
  { struct CMD_SHOW_HISTO *rezo_histo;

    rezo_histo = (struct CMD_SHOW_HISTO *)g_malloc0( sizeof(struct CMD_SHOW_HISTO) );
    if (!rezo_histo) { return(NULL); }

    rezo_histo->id               = histo->msg.num;
    rezo_histo->type             = histo->msg.type;
    rezo_histo->date_create_sec  = histo->date_create_sec;
    rezo_histo->date_create_usec = histo->date_create_usec;
    rezo_histo->date_fixe        = histo->date_fixe;
    memcpy( &rezo_histo->nom_ack, histo->nom_ack, sizeof(rezo_histo->nom_ack) );
    memcpy( &rezo_histo->objet, histo->msg.objet, sizeof(rezo_histo->objet) );
    memcpy( &rezo_histo->libelle, histo->msg.libelle, sizeof(rezo_histo->libelle) );
    return( rezo_histo );
  }
/**********************************************************************************************************/
/* Proto_acquitter_histo: le client demande l'acquittement d'un histo                                     */
/* Entrée: le client demandeur et le histo en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_acquitter_histo ( struct CLIENT *client, struct CMD_ID_HISTO *rezo_histo )
  { struct CMD_EDIT_HISTO edit_histo;
    struct HISTODB *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    result = Rechercher_histoDB( Config.log, Db_watchdog, rezo_histo->id );
    if (!result) return;
    if (result->date_fixe)                                                            /* Deja acquitté ?? */
     { g_free(result);
       return;
     }
    g_free(result);

    edit_histo.id = rezo_histo->id;               /* On renseigne la structure de modification de l'histo */
    time( (time_t *)&edit_histo.date_fixe );
    memcpy( edit_histo.nom_ack, client->util->nom, sizeof(edit_histo.nom_ack) );

    retour = Modifier_histoDB ( Config.log, Db_watchdog, &edit_histo );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to ack histo %d:\n%s"), rezo_histo->id, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_histoDB( Config.log, Db_watchdog, rezo_histo->id );
           if (result) 
            { struct CMD_SHOW_HISTO *histo;
              histo = Preparer_envoi_histo ( result );
              g_free(result);
              if (!histo)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             _("Not enough memory") );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_HISTO, SSTAG_SERVEUR_ACK_HISTO,
                                   (gchar *)histo, sizeof(struct CMD_SHOW_HISTO) );
                     g_free(histo);
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate histo %d:\n%s"), rezo_histo->id, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_histos: Envoi des histos au client GID_USERS                                                   */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_histo_thread ( struct CLIENT *client )
  { struct CMD_SHOW_HISTO *rezo_histo;
    struct HISTODB *histo;
    SQLHSTMT hquery;                                                   /* Requete SQL en cours d'emission */
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    hquery = Recuperer_histoDB( Config.log, Db_watchdog );
    if (!hquery)
     { Client_mode( client, VALIDE );         /* Le client est maintenant valide aux yeux du sous-serveur */
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    for( ; ; )
     { histo = Recuperer_histoDB_suite( Config.log, Db_watchdog, hquery );
       if (!histo)
        { Envoi_client ( client, TAG_HISTO, SSTAG_SERVEUR_ADDPROGRESS_HISTO_FIN, NULL, 0 );
          Info( Config.log, DEBUG_INFO, "Envoyer_histo: fin" );
          Client_mode( client, VALIDE );      /* Le client est maintenant valide aux yeux du sous-serveur */
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit( NULL );
        }

       rezo_histo = Preparer_envoi_histo( histo );
       g_free(histo);
       if (rezo_histo)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
          Info_c( Config.log, DEBUG_INFO, "Envoyer_histo: ", rezo_histo->libelle );
          Envoi_client ( client, TAG_HISTO, SSTAG_SERVEUR_ADDPROGRESS_HISTO,
                          (gchar *)rezo_histo, sizeof(struct CMD_SHOW_HISTO) );
          g_free(rezo_histo);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
