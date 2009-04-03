/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_util.c        Configuration des utilisateurs de Watchdog v2.0                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 25 sep 2003 14:11:31 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>
 #include <string.h>

 #include "Reseaux.h"
 #include "Utilisateur_DB.h"
 #include "Erreur.h"
 #include "Config.h"
 #include "Client.h"

 #include "watchdogd.h"
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Preparer_envoi_util: convertit une structure UTILISATEUR en structure CMD_SHOW_UTILISATEUR             */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_UTILISATEUR *Preparer_envoi_utilisateur ( struct UTILISATEURDB *util )
  { struct CMD_SHOW_UTILISATEUR *rezo_util;

    rezo_util = (struct CMD_SHOW_UTILISATEUR *)g_malloc0( sizeof(struct CMD_SHOW_UTILISATEUR) );
    if (!rezo_util) { return(NULL); }

    rezo_util->id = util->id;
    memcpy( rezo_util->nom,         util->nom, sizeof(rezo_util->nom) );
    memcpy( rezo_util->commentaire, util->commentaire, sizeof(rezo_util->commentaire) );
    return( rezo_util );
  }
/**********************************************************************************************************/
/* Proto_editer_utilisateur: Le client desire editer un utilisateur                                       */
/* Entrée: le client demandeur et l'utilisateur en question                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_utilisateur ( struct CLIENT *client, struct CMD_ID_UTILISATEUR *rezo_util )
  { struct CMD_EDIT_UTILISATEUR edit_util;
    struct UTILISATEURDB *util;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = FALSE;
    util   = NULL;
    if (rezo_util->id >= NBR_UTILISATEUR_RESERVE)
     { util = Rechercher_utilisateurDB( Config.log, Db_watchdog, rezo_util->id );
       if (util) retour = TRUE;                                                            /* Autorisé !! */
     } 

    if (retour)
     { edit_util.id            = util->id;                                  /* Recopie des info editables */
       memcpy( &edit_util.nom, util->nom, sizeof(edit_util.nom) );
       memcpy( &edit_util.commentaire, util->commentaire, sizeof(edit_util.commentaire) );
       memcpy( &edit_util.gids, util->gids, sizeof(edit_util.gids) );
       g_snprintf( edit_util.code_en_clair, sizeof(edit_util.code_en_clair), "secret" );
       edit_util.cansetpass  = util->cansetpass;
       edit_util.date_modif  = util->date_modif;
       edit_util.date_expire = util->date_expire;
       edit_util.actif       = util->actif;
       edit_util.expire      = util->expire;
       edit_util.changepass  = util->changepass;

       Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_EDIT_UTIL_OK,
                     (gchar *)&edit_util, sizeof(struct CMD_EDIT_UTILISATEUR) );
       g_free(util);                                                                /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to locate user %s:\n%s"), rezo_util->nom, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_utilisateur: Le client valide l'edition d'un utilisateur                          */
/* Entrée: le client demandeur et le groupe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_utilisateur ( struct CLIENT *client, struct CMD_EDIT_UTILISATEUR *rezo_util )
  { struct UTILISATEURDB *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_utilisateurDB ( Config.log, Db_watchdog, Config.crypto_key, rezo_util );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to edit user %s:\n%s"), rezo_util->nom, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_utilisateurDB( Config.log, Db_watchdog, rezo_util->id );
           if (result) 
            { struct CMD_SHOW_UTILISATEUR *util;
              util = Preparer_envoi_utilisateur ( result );
              g_free(result);
              if (!util)
                { struct CMD_GTK_MESSAGE erreur;
                  g_snprintf( erreur.message, sizeof(erreur.message),
                               _("Not enough memory") );
                  Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                                (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
                }
              else { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_VALIDE_EDIT_UTIL_OK,
                                   (gchar *)util, sizeof(struct CMD_SHOW_UTILISATEUR) );
                     g_free(util);
                  }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate user %s:\n%s"), rezo_util->nom, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_utilisateur: Retrait de l'utilisateur en parametre                                       */
/* Entrée: le client demandeur et le utilisateur en question                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_utilisateur ( struct CLIENT *client, struct CMD_ID_UTILISATEUR *rezo_util )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_utilisateurDB( Config.log, Db_watchdog, rezo_util );

    if (retour)
     { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_DEL_UTIL_OK,
                     (gchar *)rezo_util, sizeof(struct CMD_ID_UTILISATEUR) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to delete user %s:\n%s"), rezo_util->nom, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_utilisateur: Un client nous demande d'ajouter un utilisateur Watchdog                    */
/* Entrée: le utilisateur à créer                                                                         */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_utilisateur ( struct CLIENT *client, struct CMD_ADD_UTILISATEUR *rezo_util )
  { struct UTILISATEURDB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_utilisateurDB ( Config.log, Db_watchdog, Config.crypto_key, rezo_util );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to add group %s:\n%s"), rezo_util->nom, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_utilisateurDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to add group %s:\n%s"), rezo_util->nom, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_SHOW_UTILISATEUR *util;
              util = Preparer_envoi_utilisateur ( result );
              g_free(result);
              if (!util)
                { struct CMD_GTK_MESSAGE erreur;
                  g_snprintf( erreur.message, sizeof(erreur.message),
                               _("Creation ok,\nnot enough memory to view.") );
                  Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                                (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
                }
              else { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADD_UTIL_OK,
                                   (gchar *)util, sizeof(struct CMD_SHOW_UTILISATEUR) );
                     g_free(util);
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_utilisateurs: Envois des utilisateurs aux client GID_USERS                                     */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_utilisateurs_thread ( struct CLIENT *client )
  { struct CMD_SHOW_UTILISATEUR *rezo_util;
    struct UTILISATEURDB *util;
    struct CMD_ENREG nbr;
    struct DB *Db_watchdog;
    SQLHSTMT hquery;
    Db_watchdog = client->Db_watchdog;

    hquery = Recuperer_utilsDB( Config.log, Db_watchdog );
    if (!hquery)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading %d users"), nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for ( ; ; )
     { util = Recuperer_utilsDB_suite( Config.log, Db_watchdog, hquery );
       if (!util)
        { Envoi_client ( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADDPROGRESS_UTIL_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       rezo_util = Preparer_envoi_utilisateur ( util );
       g_free(util);
       if (rezo_util)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Envoi_client ( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADDPROGRESS_UTIL,      /* Envoi des infos */
                         (gchar *)rezo_util, sizeof(struct CMD_SHOW_UTILISATEUR) );
          g_free(rezo_util);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
