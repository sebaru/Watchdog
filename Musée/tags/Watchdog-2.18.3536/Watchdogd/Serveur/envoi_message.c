/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_message.c        Configuration des messages de Watchdog v2.0                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 05 sep 2004 14:00:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_message.c
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
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/file.h>                                            /* Gestion des verrous sur les fichiers */
 #include <sys/time.h>
 #include <pthread.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Proto_editer_msg: Le client desire editer un msg                                                       */
/* Entrée: le client demandeur et le msg en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg )
  { struct CMD_TYPE_MESSAGE *msg;

    msg = Rechercher_messageDB_par_id( rezo_msg->id );

    if (msg)
     { Envoi_client( client, TAG_MESSAGE, SSTAG_SERVEUR_EDIT_MESSAGE_OK,
                  (gchar *)msg, sizeof(struct CMD_TYPE_MESSAGE) );
       g_free(msg);                                                                 /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate message %s", rezo_msg->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_msg: Le client valide l'edition d'un msg                                          */
/* Entrée: le client demandeur et le msg en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg )
  { struct CMD_TYPE_MESSAGE *msg;
    gboolean retour;

    retour = Modifier_messageDB ( rezo_msg );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit message %s", rezo_msg->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { msg = Rechercher_messageDB_par_id( rezo_msg->id );
           if (msg) 
            { Envoi_client( client, TAG_MESSAGE, SSTAG_SERVEUR_VALIDE_EDIT_MESSAGE_OK,
                            (gchar *)msg, sizeof(struct CMD_TYPE_MESSAGE) );
              g_free(msg);
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate message %s", rezo_msg->libelle);
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
 void Proto_effacer_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg )
  { gboolean retour;

    retour = Retirer_messageDB( rezo_msg );

    if (retour)
     { Envoi_client( client, TAG_MESSAGE, SSTAG_SERVEUR_DEL_MESSAGE_OK,
                     (gchar *)rezo_msg, sizeof(struct CMD_TYPE_MESSAGE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete message %s", rezo_msg->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_msg: Un client nous demande d'ajouter un msg Watchdog                                    */
/* Entrée: le msg à créer                                                                                 */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg )
  { struct CMD_TYPE_MESSAGE *msg;
    gint id;

    id = Ajouter_messageDB ( rezo_msg );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add message %s", rezo_msg->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { msg = Rechercher_messageDB_par_id( id );
           if (!msg) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate message %s", rezo_msg->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_MESSAGE, SSTAG_SERVEUR_ADD_MESSAGE_OK,
                            (gchar *)msg, sizeof(struct CMD_TYPE_MESSAGE) );
              g_free(msg);
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_msgs: Envoi des msgs au client GID_MESSAGE                                                     */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_messages_thread ( struct CLIENT *client )
  { struct CMD_TYPE_MESSAGES *msgs;
    struct CMD_TYPE_MESSAGE *msg;
    struct CMD_ENREG nbr;
    struct DB *db;
    gint max_enreg;                                /* Nombre maximum d'enregistrement dans un bloc reseau */

    prctl(PR_SET_NAME, "W-EnvoiMSG", 0, 0, 0 );

    if ( ! Recuperer_messageDB( &db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d messages", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    max_enreg = (Cfg_ssrv.taille_bloc_reseau - sizeof(struct CMD_TYPE_MESSAGES)) / sizeof(struct CMD_TYPE_MESSAGE);
    msgs = (struct CMD_TYPE_MESSAGES *)g_try_malloc0( Cfg_ssrv.taille_bloc_reseau );    
    if (!msgs)
     { struct CMD_GTK_MESSAGE erreur;
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                 "Envoyer_messages_tag: not enought memory" );
       g_snprintf( erreur.message, sizeof(erreur.message), "Pb d'allocation memoire" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }
    msgs->nbr_messages = 0;                                 /* Valeurs par defaut si pas d'enregistrement */

    do
     { msg = Recuperer_messageDB_suite( &db );                    /* Récupération d'un message dans la DB */
       if (msg)                                                /* Si enregegistrement, alors on le pousse */
        { memcpy ( &msgs->msg[msgs->nbr_messages], msg, sizeof(struct CMD_TYPE_MESSAGE) );
          msgs->nbr_messages++;          /* Nous avons 1 enregistrement de plus dans la structure d'envoi */
          g_free(msg);
        }

       if ( (msg == NULL) || msgs->nbr_messages == max_enreg )/* Si depassement de tampon ou plus d'enreg */
        { Envoi_client ( client, TAG_MESSAGE, SSTAG_SERVEUR_ADDPROGRESS_MESSAGE, (gchar *)msgs,
                         sizeof(struct CMD_TYPE_MESSAGES) + msgs->nbr_messages * sizeof(struct CMD_TYPE_MESSAGE) );
          msgs->nbr_messages = 0;
        }
     }
    while (msg);                                              /* Tant que l'on a des messages e envoyer ! */
    g_free(msgs);
    Envoi_client ( client, TAG_MESSAGE, SSTAG_SERVEUR_ADDPROGRESS_MESSAGE_FIN, NULL, 0 );
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit ( NULL );
  }
/*--------------------------------------------------------------------------------------------------------*/
