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
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>
 #include <pthread.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "Reseaux.h"
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Proto_editer_msg: Le client desire editer un msg                                                       */
/* Entrée: le client demandeur et le msg en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_message ( struct CLIENT *client, struct CMD_TYPE_MESSAGE *rezo_msg )
  { struct CMD_TYPE_MESSAGE *msg;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    msg = Rechercher_messageDB_par_id( Config.log, Db_watchdog, rezo_msg->id );

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
    struct DB *Db_watchdog;
    gboolean retour;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_messageDB ( Config.log, Db_watchdog, rezo_msg );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit message %s", rezo_msg->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { msg = Rechercher_messageDB_par_id( Config.log, Db_watchdog, rezo_msg->id );
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
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_messageDB( Config.log, Db_watchdog, rezo_msg );

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
    struct DB *Db_watchdog;
    gint id;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_messageDB ( Config.log, Db_watchdog, rezo_msg );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add message %s", rezo_msg->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { msg = Rechercher_messageDB_par_id( Config.log, Db_watchdog, id );
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
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_MESSAGE *msg;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiMSG", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_messageDB( Config.log, db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Libere_DB_SQL( Config.log, &db );
       pthread_exit( NULL );
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d messages", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { msg = Recuperer_messageDB_suite( Config.log, db );
       if (!msg)
        { Envoi_client ( client, TAG_MESSAGE, SSTAG_SERVEUR_ADDPROGRESS_MESSAGE_FIN, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }
       while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
printf("Envoi message %d %d %s\n", msg->id, msg->num, msg->libelle );
       Envoi_client ( client, TAG_MESSAGE, SSTAG_SERVEUR_ADDPROGRESS_MESSAGE,
                      (gchar *)msg, sizeof(struct CMD_TYPE_MESSAGE) );
       g_free(msg);
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
