/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_groupe.c        Configuration des groupes de Watchdog v2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 21:23:05 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_groupe.c
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
/* Preparer_envoi_groupe: convertit une structure GROUPE en structure CMD_TYPE_GROUPE                     */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_TYPE_GROUPE *Preparer_envoi_groupe ( struct CMD_TYPE_GROUPE *groupe )
  { struct CMD_TYPE_GROUPE *rezo_groupe;

    rezo_groupe = (struct CMD_TYPE_GROUPE *)g_try_malloc0( sizeof(struct CMD_TYPE_GROUPE) );
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
 void Proto_editer_groupe ( struct CLIENT *client, struct CMD_TYPE_GROUPE *rezo_groupe )
  { struct CMD_TYPE_GROUPE edit_groupe;
    struct CMD_TYPE_GROUPE *groupe;

    if (rezo_groupe->id < NBR_GROUPE_RESERVE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Edition of built-in group %s forbidden", rezo_groupe->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     } 

    groupe = Rechercher_groupeDB( rezo_groupe->id );

    if (groupe)
     { edit_groupe.id = groupe->id;                                         /* Recopie des info editables */
       memcpy( &edit_groupe.nom, groupe->nom, sizeof(edit_groupe.nom) );
       memcpy( &edit_groupe.commentaire, groupe->commentaire, sizeof(edit_groupe.commentaire) );

       Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_EDIT_GROUPE_OK,
                  (gchar *)&edit_groupe, sizeof(struct CMD_TYPE_GROUPE) );
       g_free(groupe);                                                              /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate group %s", rezo_groupe->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_groupe: Le client valide l'edition d'un groupe                                    */
/* Entrée: le client demandeur et le groupe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_groupe ( struct CLIENT *client, struct CMD_TYPE_GROUPE *rezo_groupe )
  { struct CMD_TYPE_GROUPE *result;
    gboolean retour;

    retour = Modifier_groupeDB ( rezo_groupe );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit group %s", rezo_groupe->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_groupeDB( rezo_groupe->id );
           if (result) 
            { struct CMD_TYPE_GROUPE *groupe;
              groupe = Preparer_envoi_groupe ( result );
              g_free(result);
              if (!groupe)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_VALIDE_EDIT_GROUPE_OK,
                                   (gchar *)groupe, sizeof(struct CMD_TYPE_GROUPE) );
                     g_free(groupe);
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate group %s", rezo_groupe->nom);
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
 void Proto_effacer_groupe ( struct CLIENT *client, struct CMD_TYPE_GROUPE *rezo_groupe )
  { gboolean retour;

    retour = Retirer_groupeDB( rezo_groupe );

    if (retour)
     { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_DEL_GROUPE_OK,
                     (gchar *)rezo_groupe, sizeof(struct CMD_TYPE_GROUPE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete group %s", rezo_groupe->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_groupe: Un client nous demande d'ajouter un groupe Watchdog                              */
/* Entrée: le groupe à créer                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_groupe ( struct CLIENT *client, struct CMD_TYPE_GROUPE *rezo_groupe )
  { struct CMD_TYPE_GROUPE *result;
    gint id;

    id = Ajouter_groupeDB ( rezo_groupe );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add group %s", rezo_groupe->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_groupeDB( id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to add group %s", rezo_groupe->nom );
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_TYPE_GROUPE *groupe;
              groupe = Preparer_envoi_groupe ( result );
              g_free(result);
              if (!groupe)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_UTILISATEUR, SSTAG_SERVEUR_ADD_GROUPE_OK,
                                   (gchar *)groupe, sizeof(struct CMD_TYPE_GROUPE) );
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
  { struct CMD_TYPE_GROUPE *rezo_groupe;
    struct CMD_ENREG nbr;
    struct CMD_TYPE_GROUPE *groupe;
    struct DB *db;
    prctl(PR_SET_NAME, "W-EnvoiGrp", 0, 0, 0 );

    if (! Recuperer_groupesDB( &db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       return;
    }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d groups", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for ( ; ; )
     { groupe = Recuperer_groupesDB_suite( &db );
       if (!groupe)                                                               /* Fin de traitement ?? */
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          return;
        }
       rezo_groupe = Preparer_envoi_groupe( groupe );                     /* Sinon, on continue d'envoyer */
       g_free(groupe);

       if (rezo_groupe)
        { Envoi_client ( client, tag, sstag,
                         (gchar *)rezo_groupe, sizeof(struct CMD_TYPE_GROUPE) );
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
  { Envoyer_groupes_tag( client, TAG_SYNOPTIQUE, SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_SYNOPTIQUE,
                                                 SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_SYNOPTIQUE_FIN );
    pthread_exit(NULL);
  }
/**********************************************************************************************************/
/* Envoyer_groupes: Envoi des groupes au client GID_USERS                                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_groupes_pour_propriete_synoptique_thread ( struct CLIENT *client )
  { Envoyer_groupes_tag( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_PROPRIETE_SYNOPTIQUE,
                                              SSTAG_SERVEUR_ADDPROGRESS_GROUPE_FOR_PROPRIETE_SYNOPTIQUE_FIN );
    pthread_exit(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
