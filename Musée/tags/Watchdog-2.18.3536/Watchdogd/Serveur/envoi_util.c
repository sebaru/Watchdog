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
/* Proto_editer_utilisateur: Le client desire editer un utilisateur                                       */
/* Entrée: le client demandeur et l'utilisateur en question                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_utilisateur ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *rezo_util )
  { struct CMD_TYPE_UTILISATEUR *util;

    util = Rechercher_utilisateurDB_by_id( rezo_util->id );

    if (util)
     { memset(util->salt, 0, sizeof(util->salt ) );                       /* RAZ des informations privées */
       memset(util->hash, 0, sizeof(util->hash ) );                       /* RAZ des informations privées */
       Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_EDIT_UTIL_OK,
                     (gchar *)util, sizeof(struct CMD_TYPE_UTILISATEUR) );
       g_free(util);                                                                /* liberation mémoire */
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
  { if (Modifier_utilisateurDB ( rezo_util ) == FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit user %s", rezo_util->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_VALIDE_EDIT_UTIL_OK,
                         (gchar *)rezo_util, sizeof(struct CMD_TYPE_UTILISATEUR) );
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_utilisateur: Retrait de l'utilisateur en parametre                                       */
/* Entrée: le client demandeur et le utilisateur en question                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_utilisateur ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *rezo_util )
  { gboolean retour;

    retour = Retirer_utilisateurDB( rezo_util );

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
  { struct CMD_TYPE_UTILISATEUR *util;
    gint id;

    id = Ajouter_utilisateurDB ( rezo_util );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add user %s", rezo_util->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { util = Rechercher_utilisateurDB_by_id( id );
           if (!util) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to load user %s", rezo_util->nom);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADD_UTIL_OK,
                            (gchar *)util, sizeof(struct CMD_TYPE_UTILISATEUR) );
              g_free(util);
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_utilisateurs: Envois des utilisateurs aux client GID_USERS                                     */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_utilisateurs_thread ( struct CLIENT *client )
  { struct CMD_TYPE_UTILISATEUR *util;
    struct CMD_ENREG nbr;
    struct DB *db;
    prctl(PR_SET_NAME, "W-EnvoiUTIL", 0, 0, 0 );

    if ( ! Recuperer_utilisateurDB( &db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d users", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for ( ; ; )
     { util = Recuperer_utilisateurDB_suite( &db );
       if (!util)
        { Envoi_client ( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADDPROGRESS_UTIL_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }
       memset(&util->salt, 0, sizeof(util->salt) );                       /* RAZ des informations privées */
       memset(&util->hash, 0, sizeof(util->hash) );                       /* RAZ des informations privées */
       Envoi_client ( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADDPROGRESS_UTIL,      /* Envoi des infos */
                      (gchar *)util, sizeof(struct CMD_TYPE_UTILISATEUR) );
       g_free(util);
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
