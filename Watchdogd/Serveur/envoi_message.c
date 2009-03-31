/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_message.c        Configuration des messages de Watchdog v2.0                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 05 sep 2004 14:00:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>
 #include <pthread.h>

 #include "Reseaux.h"
 #include "Message_DB.h"
 #include "Erreur.h"
 #include "Config.h"
 #include "Client.h"

 #include "watchdogd.h"
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Preparer_envoi_message: convertit une structure MSG en structure CMD_SHOW_MESSAGE                      */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_MESSAGE *Preparer_envoi_msg ( struct MSGDB *msg )
  { struct CMD_SHOW_MESSAGE *rezo_msg;

    rezo_msg = (struct CMD_SHOW_MESSAGE *)g_malloc0( sizeof(struct CMD_SHOW_MESSAGE) );
    if (!rezo_msg) { return(NULL); }

    rezo_msg->id         = msg->id;
    rezo_msg->num        = msg->num;
    rezo_msg->type       = msg->type;
    rezo_msg->not_inhibe = msg->not_inhibe;
    rezo_msg->sms        = msg->sms;
    memcpy( &rezo_msg->libelle, msg->libelle, sizeof(rezo_msg->libelle) );
    memcpy( &rezo_msg->objet, msg->objet, sizeof(rezo_msg->objet) );
    return( rezo_msg );
  }
/**********************************************************************************************************/
/* Proto_editer_msg: Le client desire editer un msg                                                       */
/* Entrée: le client demandeur et le msg en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_message ( struct CLIENT *client, struct CMD_ID_MESSAGE *rezo_msg )
  { struct CMD_EDIT_MESSAGE edit_msg;
    struct MSGDB *msg;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    msg = Rechercher_messageDB_par_id( Config.log, Db_watchdog, rezo_msg->id );

    if (msg)
     { edit_msg.id         = msg->id;                                       /* Recopie des info editables */
       edit_msg.num        = msg->num;
       edit_msg.type       = msg->type;
       edit_msg.not_inhibe = msg->not_inhibe;
       edit_msg.sms        = msg->sms;
       memcpy( &edit_msg.libelle, msg->libelle, sizeof(edit_msg.libelle) );
       memcpy( &edit_msg.objet, msg->objet, sizeof(edit_msg.objet) );

       Envoi_client( client, TAG_MESSAGE, SSTAG_SERVEUR_EDIT_MESSAGE_OK,
                  (gchar *)&edit_msg, sizeof(struct CMD_EDIT_MESSAGE) );
       g_free(msg);                                                                 /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to locate message %s:\n%s"), rezo_msg->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_msg: Le client valide l'edition d'un msg                                          */
/* Entrée: le client demandeur et le msg en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_message ( struct CLIENT *client, struct CMD_EDIT_MESSAGE *rezo_msg )
  { struct MSGDB *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_messageDB ( Config.log, Db_watchdog, rezo_msg );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to edit message %s:\n%s"), rezo_msg->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_messageDB_par_id( Config.log, Db_watchdog, rezo_msg->id );
           if (result) 
            { struct CMD_SHOW_MESSAGE *msg;
              msg = Preparer_envoi_msg ( result );
              g_free(result);
              if (!msg)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             _("Not enough memory") );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_MESSAGE, SSTAG_SERVEUR_VALIDE_EDIT_MESSAGE_OK,
                                   (gchar *)msg, sizeof(struct CMD_SHOW_MESSAGE) );
                     g_free(msg);
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate message %s:\n%s"), rezo_msg->libelle, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_msg: Retrait du msg en parametre                                                         */
/* Entrée: le client demandeur et le msg en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_message ( struct CLIENT *client, struct CMD_ID_MESSAGE *rezo_msg )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_messageDB( Config.log, Db_watchdog, rezo_msg );

    if (retour)
     { Envoi_client( client, TAG_MESSAGE, SSTAG_SERVEUR_DEL_MESSAGE_OK,
                     (gchar *)rezo_msg, sizeof(struct CMD_ID_MESSAGE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to delete message %s:\n%s"), rezo_msg->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_msg: Un client nous demande d'ajouter un msg Watchdog                                    */
/* Entrée: le msg à créer                                                                                 */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_message ( struct CLIENT *client, struct CMD_ADD_MESSAGE *rezo_msg )
  { struct MSGDB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_messageDB ( Config.log, Db_watchdog, rezo_msg );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to add message %s:\n%s"), rezo_msg->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_messageDB_par_id( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate message %s:\n%s"), rezo_msg->libelle, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_SHOW_MESSAGE *msg;
              msg = Preparer_envoi_msg ( result );
              g_free(result);
              if (!msg)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             _("Not enough memory") );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_MESSAGE, SSTAG_SERVEUR_ADD_MESSAGE_OK,
                                   (gchar *)msg, sizeof(struct CMD_SHOW_MESSAGE) );
                     g_free(msg);
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_msgs: Envoi des msgs au client GID_MESSAGE                                                     */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_messages_thread ( struct CLIENT *client )
  { struct CMD_SHOW_MESSAGE *rezo_msg;
    struct CMD_ENREG nbr;
    struct MSGDB *msg;
    SQLHSTMT hquery;                                                   /* Requete SQL en cours d'emission */
    gint cpt;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    hquery = Recuperer_messageDB( Config.log, Db_watchdog );
    if (!hquery)                                                                 /* Si pas de histos (??) */
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading messages") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { msg = Recuperer_messageDB_suite( Config.log, Db_watchdog, hquery );
       if (!msg)
        { Envoi_client ( client, TAG_MESSAGE, SSTAG_SERVEUR_ADDPROGRESS_MESSAGE_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       rezo_msg = Preparer_envoi_msg( msg );
       g_free(msg);
       if (rezo_msg)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
          Envoi_client ( client, TAG_MESSAGE, SSTAG_SERVEUR_ADDPROGRESS_MESSAGE,
                         (gchar *)rezo_msg, sizeof(struct CMD_SHOW_MESSAGE) );
          g_free(rezo_msg);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
