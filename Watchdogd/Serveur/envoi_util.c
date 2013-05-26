/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_util.c        Configuration des utilisateurs de Watchdog v2.0                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 25 sep 2003 14:11:31 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_util.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */
 
 #include <glib.h>
 #include <sys/prctl.h>
 #include <string.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Preparer_envoi_util: convertit une structure UTILISATEUR en structure CMD_TYPE_UTILISATEUR             */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_TYPE_UTILISATEUR *Preparer_envoi_utilisateur ( struct UTILISATEURDB *util )
  { struct CMD_TYPE_UTILISATEUR *rezo_util;

    rezo_util = (struct CMD_TYPE_UTILISATEUR *)g_try_malloc0( sizeof(struct CMD_TYPE_UTILISATEUR) );
    if (!rezo_util) { return(NULL); }

    rezo_util->id = util->id;
    memcpy( &rezo_util->nom,         util->nom, sizeof(rezo_util->nom) );
    memcpy( &rezo_util->commentaire, util->commentaire, sizeof(rezo_util->commentaire) );
    return( rezo_util );
  }
/**********************************************************************************************************/
/* Proto_editer_utilisateur: Le client desire editer un utilisateur                                       */
/* Entrée: le client demandeur et l'utilisateur en question                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_utilisateur ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *rezo_util )
  { struct CMD_TYPE_UTILISATEUR edit_util;
    struct UTILISATEURDB *util;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    util = Rechercher_utilisateurDB( Config.log, Db_watchdog, rezo_util->id );

    if (util)
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
                     (gchar *)&edit_util, sizeof(struct CMD_TYPE_UTILISATEUR) );
       g_free(util);                                                                /* liberation mémoire */
       Client_mode( client, ENVOI_GROUPE_FOR_UTIL );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Permission Denied for user %s (%d)", rezo_util->nom, rezo_util->id );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_utilisateur: Le client valide l'edition d'un utilisateur                          */
/* Entrée: le client demandeur et le groupe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_utilisateur ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *rezo_util )
  { struct UTILISATEURDB *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_utilisateurDB ( Config.log, Db_watchdog, Config.crypto_key, rezo_util );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit user %s", rezo_util->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_utilisateurDB( Config.log, Db_watchdog, rezo_util->id );
           if (result) 
            { struct CMD_TYPE_UTILISATEUR *util;
              util = Preparer_envoi_utilisateur ( result );
              g_free(result);
              if (!util)
                { struct CMD_GTK_MESSAGE erreur;
                  g_snprintf( erreur.message, sizeof(erreur.message),
                               "Not enough memory" );
                  Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                                (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
                }
              else { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_VALIDE_EDIT_UTIL_OK,
                                   (gchar *)util, sizeof(struct CMD_TYPE_UTILISATEUR) );
                     g_free(util);
                  }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Permission Denied for user %s (%d)", rezo_util->nom, rezo_util->id );
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
 void Proto_effacer_utilisateur ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *rezo_util )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_utilisateurDB( Config.log, Db_watchdog, rezo_util );

    if (retour)
     { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_DEL_UTIL_OK,
                     (gchar *)rezo_util, sizeof(struct CMD_TYPE_UTILISATEUR) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete user %s", rezo_util->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_utilisateur: Un client nous demande d'ajouter un utilisateur Watchdog                    */
/* Entrée: le utilisateur à créer                                                                         */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_utilisateur ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *rezo_util )
  { struct UTILISATEURDB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_utilisateurDB ( Config.log, Db_watchdog, Config.crypto_key, rezo_util );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add group %s", rezo_util->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_utilisateurDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to add group %s", rezo_util->nom);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_TYPE_UTILISATEUR *util;
              util = Preparer_envoi_utilisateur ( result );
              g_free(result);
              if (!util)
                { struct CMD_GTK_MESSAGE erreur;
                  g_snprintf( erreur.message, sizeof(erreur.message),
                               "Creation ok,\nnot enough memory to view." );
                  Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                                (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
                }
              else { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADD_UTIL_OK,
                                   (gchar *)util, sizeof(struct CMD_TYPE_UTILISATEUR) );
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
  { struct CMD_TYPE_UTILISATEUR *rezo_util;
    struct UTILISATEURDB *util;
    struct CMD_ENREG nbr;
    struct DB *db;
    prctl(PR_SET_NAME, "W-EnvoiUTIL", 0, 0, 0 );

    db = Init_DB_SQL();       
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */
    if ( ! Recuperer_utilsDB( Config.log, db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d users", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for ( ; ; )
     { util = Recuperer_utilsDB_suite( Config.log, db );
       if (!util)
        { Envoi_client ( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADDPROGRESS_UTIL_FIN, NULL, 0 );
          Libere_DB_SQL( &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }
       rezo_util = Preparer_envoi_utilisateur ( util );
       g_free(util);
       if (rezo_util)
        { Envoi_client ( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADDPROGRESS_UTIL,      /* Envoi des infos */
                         (gchar *)rezo_util, sizeof(struct CMD_TYPE_UTILISATEUR) );
          g_free(rezo_util);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
