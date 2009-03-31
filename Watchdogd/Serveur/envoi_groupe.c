/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_groupe.c        Configuration des groupes de Watchdog v2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                       jeu 30 déc 2004 14:27:29 CET */
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
/* Preparer_envoi_groupe: convertit une structure GROUPE en structure CMD_SHOW_GROUPE                     */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_GROUPE *Preparer_envoi_groupe ( struct GROUPEDB *groupe )
  { struct CMD_SHOW_GROUPE *rezo_groupe;

    rezo_groupe = (struct CMD_SHOW_GROUPE *)g_malloc0( sizeof(struct CMD_SHOW_GROUPE) );
    if (!rezo_groupe) { return(NULL); }

    rezo_groupe->id = groupe->id;
    memcpy( &rezo_groupe->nom, groupe->nom, sizeof(rezo_groupe->nom ) );
    memcpy( &rezo_groupe->commentaire, groupe->commentaire, sizeof(rezo_groupe->commentaire ) );
    return( rezo_groupe );
  }
/**********************************************************************************************************/
/* Proto_editer_groupe: Le client desire editer un groupe                                                 */
/* Entrée: le client demandeur et le groupe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_groupe ( struct CLIENT *client, struct CMD_ID_GROUPE *rezo_groupe )
  { struct CMD_EDIT_GROUPE edit_groupe;
    struct GROUPEDB *groupe;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = FALSE;
    groupe = NULL;
    if (rezo_groupe->id >= NBR_GROUPE_RESERVE)
     { groupe = Rechercher_groupeDB( Config.log, Db_watchdog, rezo_groupe->id );
       if (groupe) retour = TRUE;                                                          /* Autorisé !! */
     } 

    if (retour)
     { edit_groupe.id = groupe->id;                                         /* Recopie des info editables */
       memcpy( &edit_groupe.nom, groupe->nom, sizeof(edit_groupe.nom) );
       memcpy( &edit_groupe.commentaire, groupe->commentaire, sizeof(edit_groupe.commentaire) );

       Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_EDIT_GROUPE_OK,
                  (gchar *)&edit_groupe, sizeof(struct CMD_EDIT_GROUPE) );
       g_free(groupe);                                                              /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to locate group %s:\n%s"), rezo_groupe->nom, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_groupe: Le client valide l'edition d'un groupe                                    */
/* Entrée: le client demandeur et le groupe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_groupe ( struct CLIENT *client, struct CMD_EDIT_GROUPE *rezo_groupe )
  { struct GROUPEDB *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_groupeDB ( Config.log, Db_watchdog, rezo_groupe );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to edit group %s:\n%s"), rezo_groupe->nom, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_groupeDB( Config.log, Db_watchdog, rezo_groupe->id );
           if (result) 
            { struct CMD_SHOW_GROUPE *groupe;
              groupe = Preparer_envoi_groupe ( result );
              g_free(result);
              if (!groupe)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             _("Not enough memory") );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_VALIDE_EDIT_GROUPE_OK,
                                   (gchar *)groupe, sizeof(struct CMD_SHOW_GROUPE) );
                     g_free(groupe);
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate group %s:\n%s"), rezo_groupe->nom, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_groupe: Retrait du groupe en parametre                                                   */
/* Entrée: le client demandeur et le groupe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_groupe ( struct CLIENT *client, struct CMD_ID_GROUPE *rezo_groupe )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_groupeDB( Config.log, Db_watchdog, rezo_groupe );

    if (retour)
     { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_DEL_GROUPE_OK,
                     (gchar *)rezo_groupe, sizeof(struct CMD_ID_GROUPE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to delete group %s:\n%s"), rezo_groupe->nom, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_groupe: Un client nous demande d'ajouter un groupe Watchdog                              */
/* Entrée: le groupe à créer                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_groupe ( struct CLIENT *client, struct CMD_ADD_GROUPE *rezo_groupe )
  { struct GROUPEDB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_groupeDB ( Config.log, Db_watchdog, rezo_groupe );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to add group %s:\n%s"), rezo_groupe->nom, Db_watchdog->last_err);
                   printf("errrrrreur  %s\n", erreur.message );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_groupeDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to add group %s:\n%s"), rezo_groupe->nom, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_SHOW_GROUPE *groupe;
              groupe = Preparer_envoi_groupe ( result );
              g_free(result);
              if (!groupe)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             _("Not enough memory") );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADD_GROUPE_OK,
                                   (gchar *)groupe, sizeof(struct CMD_SHOW_GROUPE) );
                     g_free(groupe);
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_groupes: Envoi des groupes au client GID_USERS                                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoyer_groupes_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_SHOW_GROUPE *rezo_groupe;
    struct CMD_ENREG nbr;
    struct GROUPEDB *groupe;
    struct DB *Db_watchdog;
    SQLHSTMT hquery;
    Db_watchdog = client->Db_watchdog;

    hquery = Recuperer_groupesDB( Config.log, Db_watchdog );
    if (!hquery)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading groups") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for ( ; ; )
     { groupe = Recuperer_groupesDB_suite( Config.log, Db_watchdog, hquery );
       if (!groupe)                                                               /* Fin de traitement ?? */
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }
       rezo_groupe = Preparer_envoi_groupe( groupe );                     /* Sinon, on continue d'envoyer */
       g_free(groupe);

       if (rezo_groupe)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Envoi_client ( client, tag, sstag,
                         (gchar *)rezo_groupe, sizeof(struct CMD_SHOW_GROUPE) );
          g_free(rezo_groupe);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_groupes: Envoi des groupes au client GID_USERS                                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_groupes_thread ( struct CLIENT *client )
  { Envoyer_groupes_tag( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADDPROGRESS_GROUPE,
                                                  SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FIN );
    pthread_exit(NULL);
  }
/**********************************************************************************************************/
/* Envoyer_groupes: Envoi des groupes au client GID_USERS                                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_groupes_pour_util_thread ( struct CLIENT *client )
  { Envoyer_groupes_tag( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_UTIL,
                                                  SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_UTIL_FIN );
    pthread_exit(NULL);
  }
/**********************************************************************************************************/
/* Envoyer_groupes: Envoi des groupes au client GID_USERS                                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_groupes_pour_synoptique_thread ( struct CLIENT *client )
  { Envoyer_groupes_tag( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_SYNOPTIQUE,
                                              SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_SYNOPTIQUE_FIN );
    pthread_exit(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
