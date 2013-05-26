/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_comments.c        Envoi des comments aux clients                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 11 avr 2009 10:50:24 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique_comments.c
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
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                         */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_comment_atelier ( struct CLIENT *client, struct CMD_TYPE_COMMENT *rezo_comment )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
    retour = Retirer_commentDB( Config.log, Db_watchdog, rezo_comment );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_COMMENT_OK,
                     (gchar *)rezo_comment, sizeof(struct CMD_TYPE_COMMENT) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete comment %s", rezo_comment->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                               */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_comment_atelier ( struct CLIENT *client, struct CMD_TYPE_COMMENT *rezo_comment )
  { struct CMD_TYPE_COMMENT *result;
    struct DB *Db_watchdog;
    gint id;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_commentDB ( Config.log, Db_watchdog, rezo_comment );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add comment %s", rezo_comment->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_commentDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate comment %s", rezo_comment->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_COMMENT_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_COMMENT) );
              g_free(result);
            }
         }
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_comment_atelier ( struct CLIENT *client, struct CMD_TYPE_COMMENT *rezo_comment )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
    retour = Modifier_commentDB ( Config.log, Db_watchdog, rezo_comment );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save comment %s", rezo_comment->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_comment_atelier_thread ( struct CLIENT *client )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_COMMENT *comment;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiComment", 0, 0, 0 );

    db = Init_DB_SQL();       
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_commentDB( Config.log, db, client->syn.id ) )
     { Client_mode( client, ENVOI_PASSERELLE_ATELIER );
       Libere_DB_SQL( &db );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );                                                    /* Si pas de histos (??) */
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d comments", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }
    for( ; ; )
     { comment = Recuperer_commentDB_suite( Config.log, db );
       if (!comment)
        { Libere_DB_SQL( &db );
          Client_mode( client, ENVOI_PASSERELLE_ATELIER );
          Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        } 
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoyer_comment_atelier: envoi comment %d (%s) to client %d",
                 comment->id, comment->libelle, client->machine );
       Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_COMMENT,
                      (gchar *)comment, sizeof(struct CMD_TYPE_COMMENT) );
       g_free(comment);
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_comment_supervision_thread ( struct CLIENT *client )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_COMMENT *comment;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiComment", 0, 0, 0 );

    db = Init_DB_SQL();       
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_commentDB( Config.log, db, client->num_supervision ) )
     { Client_mode( client, ENVOI_PASSERELLE_SUPERVISION );                     /* Si pas de comments ... */
       Libere_DB_SQL( &db );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d comments", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { comment = Recuperer_commentDB_suite( Config.log, db );
       if (!comment)                                                                        /* Terminé ?? */
        { Libere_DB_SQL( &db );
          Client_mode( client, ENVOI_PASSERELLE_SUPERVISION );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_COMMENT_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit( NULL );
        }

       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoyer_comment_supervision: envoi comment %d (%s) to client %s",
                 comment->id, comment->libelle, client->machine );
       Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_COMMENT,
                      (gchar *)comment, sizeof(struct CMD_TYPE_COMMENT) );
       g_free(comment);
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
