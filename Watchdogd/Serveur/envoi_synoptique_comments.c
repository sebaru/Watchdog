/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_comments.c        Envoi des comments aux clients                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 11 avr 2009 10:50:24 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique_comments.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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
 #include <bonobo/bonobo-i18n.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

 #include "Reseaux.h"
 #include "Synoptiques_DB.h"
 #include "Erreur.h"
 #include "Config.h"
 #include "Client.h"

 #include "watchdogd.h"
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Preparer_envoi_comment: convertit une structure COMMENTDB en structure CMD_SHOW_COMMENT                */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_COMMENT *Preparer_envoi_comment ( struct COMMENTDB *comment )
  { struct CMD_SHOW_COMMENT *rezo_comment;

    rezo_comment = (struct CMD_SHOW_COMMENT *)g_malloc0( sizeof(struct CMD_SHOW_COMMENT) );
    if (!rezo_comment) { return(NULL); }

    memcpy( rezo_comment->libelle, comment->libelle, sizeof(comment->libelle) );     /* Recopie structure */
    memcpy( rezo_comment->font, comment->font, sizeof(comment->font) );      /* Recopie dans la structure */
    rezo_comment->id         = comment->id;
    rezo_comment->syn_id     = comment->syn_id;
    rezo_comment->position_x = comment->position_x;                          /* en abscisses et ordonnées */
    rezo_comment->position_y = comment->position_y;
    rezo_comment->rouge      = comment->rouge;
    rezo_comment->vert       = comment->vert;
    rezo_comment->bleu       = comment->bleu;
    return( rezo_comment );
  }
/**********************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                         */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_comment_atelier ( struct CLIENT *client, struct CMD_ID_COMMENT *rezo_comment )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
    Info( Config.log, DEBUG_INFO, "MSRV: demande d'effacement comment" );
    retour = Retirer_commentDB( Config.log, Db_watchdog, rezo_comment );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_COMMENT_OK,
                     (gchar *)rezo_comment, sizeof(struct CMD_ID_COMMENT) );
       Info( Config.log, DEBUG_INFO, "MSRV: effacement comment OK" );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to delete comment %s:\n%s"), rezo_comment->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Info( Config.log, DEBUG_INFO, "MSRV: effacement comment NOK" );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                               */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_comment_atelier ( struct CLIENT *client, struct CMD_ADD_COMMENT *rezo_comment )
  { struct COMMENTDB *result;
    struct DB *Db_watchdog;
    gint id;
    Info( Config.log, DEBUG_INFO, "MSRV: demande d'ajout comment" );
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_commentDB ( Config.log, Db_watchdog, rezo_comment );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to add comment %s:\n%s"), rezo_comment->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Info( Config.log, DEBUG_INFO, "MSRV: ajout comment NOK" );
     }
    else { result = Rechercher_commentDB( Config.log, Db_watchdog, id );
       Info_c( Config.log, DEBUG_INFO, "MSRV: ajout comment3", rezo_comment->libelle );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate comment %s:\n%s"), rezo_comment->libelle, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
              Info( Config.log, DEBUG_INFO, "MSRV: ajout comment NOK (2)" );
            }
           else
            { struct CMD_SHOW_COMMENT *comment;
              comment = Preparer_envoi_comment( result );
              g_free(result);
              if (!comment)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             _("Not enough memory") );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
                 Info( Config.log, DEBUG_INFO, "MSRV: ajout comment NOK (3)" );
               }
              else { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_COMMENT_OK,
                                   (gchar *)comment, sizeof(struct CMD_SHOW_COMMENT) );
                     g_free(comment);
                     Info_c( Config.log, DEBUG_INFO, "MSRV: ajout comment OK", rezo_comment->libelle );
                   }
            }
         }
       Info_c( Config.log, DEBUG_INFO, "MSRV: ajout comment4", rezo_comment->libelle );
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_comment_atelier ( struct CLIENT *client, struct CMD_EDIT_COMMENT *rezo_comment )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
Info( Config.log, DEBUG_INFO, "Debut valider_editer_comment_atelier" );
    retour = Modifier_commentDB ( Config.log, Db_watchdog, rezo_comment );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to save comment %s:\n%s"), rezo_comment->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
Info( Config.log, DEBUG_INFO, "Fin valider_editer_comment_atelier" );
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_comment_atelier_thread ( struct CLIENT *client )
  { struct CMD_SHOW_COMMENT *rezo_comment;
    struct CMD_ENREG nbr;
    struct COMMENTDB *comment;
    struct DB *Db_watchdog;
    SQLHSTMT hquery;
    Db_watchdog = client->Db_watchdog;

    hquery = Recuperer_commentDB( Config.log, Db_watchdog, client->syn.id );
    if (!hquery) { Client_mode( client, ENVOI_PASSERELLE_ATELIER );
                   Unref_client( client );                            /* Déréférence la structure cliente */
                   pthread_exit ( NULL );                                        /* Si pas de histos (??) */
                 }

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading comments") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { comment = Recuperer_commentDB_suite( Config.log, Db_watchdog, hquery );
       if (!comment)
        { Client_mode( client, ENVOI_PASSERELLE_ATELIER );
          Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        } 

       rezo_comment = Preparer_envoi_comment( comment );
       g_free(comment);
       if (rezo_comment)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
          Info_c( Config.log, DEBUG_INFO, "THR Envoyer_comment_atelier: comment LIB", rezo_comment->libelle );
          Info_n( Config.log, DEBUG_INFO, "THR Envoyer_comment_atelier: comment ID ", rezo_comment->id );
          Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT,
                         (gchar *)rezo_comment, sizeof(struct CMD_SHOW_COMMENT) );
          g_free(rezo_comment);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_comment_supervision_thread ( struct CLIENT *client )
  { struct CMD_SHOW_COMMENT *rezo_comment;
    struct CMD_ENREG nbr;
    struct COMMENTDB *comment;
    struct DB *Db_watchdog;
    SQLHSTMT hquery;
    Db_watchdog = client->Db_watchdog;

    hquery = Recuperer_commentDB( Config.log, Db_watchdog, client->num_supervision );
    if (!hquery) { Client_mode( client, ENVOI_PASSERELLE_SUPERVISION );         /* Si pas de comments ... */
                   Unref_client( client );                            /* Déréférence la structure cliente */
                   pthread_exit ( NULL );
                 }

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading comments") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { comment = Recuperer_commentDB_suite( Config.log, Db_watchdog, hquery );
       if (!comment)                                                                        /* Terminé ?? */
        { Client_mode( client, ENVOI_PASSERELLE_SUPERVISION );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_COMMENT_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit( NULL );
        }
       rezo_comment = Preparer_envoi_comment( comment );
       g_free(comment);
       if (rezo_comment)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Info_c( Config.log, DEBUG_INFO, "THR Envoyer_comment_supervision: comment LIB", rezo_comment->libelle );
          Info_n( Config.log, DEBUG_INFO, "THR Envoyer_comment_supervision: comment ID ", rezo_comment->id );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_COMMENT,
                         (gchar *)rezo_comment, sizeof(struct CMD_SHOW_COMMENT) );
          g_free(rezo_comment);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
