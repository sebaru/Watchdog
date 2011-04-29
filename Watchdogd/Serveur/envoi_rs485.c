/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_rs485.c        Configuration des rs485s de Watchdog v2.0                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 15 août 2010 14:04:34 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_rs485.c
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
/* Proto_editer_rs485: Le client desire editer un rs485                                                   */
/* Entrée: le client demandeur et le rs485 en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_rs485 ( struct CLIENT *client, struct CMD_TYPE_RS485 *rezo_rs485 )
  { struct CMD_TYPE_RS485 *rs485;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    rs485 = Rechercher_rs485DB( Config.log, Db_watchdog, rezo_rs485->id );

    if (rs485)
     { Envoi_client( client, TAG_RS485, SSTAG_SERVEUR_EDIT_RS485_OK,
                     (gchar *)rs485, sizeof(struct CMD_TYPE_RS485) );
       g_free(rs485);                                                              /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate rs485 %s", rezo_rs485->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_rs485: Le client valide l'edition d'un rs485                                    */
/* Entrée: le client demandeur et le rs485 en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_rs485 ( struct CLIENT *client, struct CMD_TYPE_RS485 *rezo_rs485 )
  { struct CMD_TYPE_RS485 *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_rs485DB ( Config.log, Db_watchdog, rezo_rs485 );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit rs485 %s", rezo_rs485->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_rs485DB( Config.log, Db_watchdog, rezo_rs485->id );
         { if (!result)
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate rs485 %s", rezo_rs485->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else { Envoi_client( client, TAG_RS485, SSTAG_SERVEUR_VALIDE_EDIT_RS485_OK,
                                (gchar *)result, sizeof(struct CMD_TYPE_RS485) );
                  Partage->com_rs485.Thread_reload = TRUE;         /* Modification -> Reload module rs485 */
                  g_free(result);
                }
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_rs485: Retrait du rs485 en parametre                                                     */
/* Entrée: le client demandeur et le rs485 en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_rs485 ( struct CLIENT *client, struct CMD_TYPE_RS485 *rezo_rs485 )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_rs485DB( Config.log, Db_watchdog, rezo_rs485 );

    if (retour)
     { Envoi_client( client, TAG_RS485, SSTAG_SERVEUR_DEL_RS485_OK,
                     (gchar *)rezo_rs485, sizeof(struct CMD_TYPE_RS485) );
       while (Partage->com_rs485.admin_del) sched_yield();
       Partage->com_rs485.admin_del = rezo_rs485->id;                            /* Envoi au thread rs485 */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete rs485 %s", rezo_rs485->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_rs485: Un client nous demande d'ajouter un rs485 Watchdog                                */
/* Entrée: le rs485 à créer                                                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_rs485 ( struct CLIENT *client, struct CMD_TYPE_RS485 *rezo_rs485 )
  { struct CMD_TYPE_RS485 *rs485;
    struct DB *Db_watchdog;
    gint id;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_rs485DB ( Config.log, Db_watchdog, rezo_rs485 );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add rs485 %s", rezo_rs485->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { rs485 = Rechercher_rs485DB( Config.log, Db_watchdog, id );
           if (!rs485)
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate rs485 %s", rezo_rs485->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_RS485, SSTAG_SERVEUR_ADD_RS485_OK,
                            (gchar *)rs485, sizeof(struct CMD_TYPE_RS485) );
              while (Partage->com_rs485.admin_add) sched_yield();
              Partage->com_rs485.admin_add = rs485->id;                          /* Envoi au thread rs485 */
              g_free(rs485);
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_rs485s: Envoi des rs485s au client GID_RS485                                                   */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void *Envoyer_rs485_thread_tag ( struct CLIENT *client, guint tag, guint sstag, guint sstag_fin )
  { struct CMD_TYPE_RS485 *rs485;
    struct CMD_ENREG nbr;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiRS485", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_rs485DB( Config.log, db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Libere_DB_SQL( Config.log, &db );
       pthread_exit( NULL );
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d modules rs485", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { rs485 = Recuperer_rs485DB_suite( Config.log, db );
       if (!rs485)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
       Envoi_client ( client, tag, sstag, (gchar *)rs485, sizeof(struct CMD_TYPE_RS485) );
       g_free(rs485);
     }
  }
/**********************************************************************************************************/
/* Envoyer_rs485s: Envoi des rs485s au client GID_RS485                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_rs485_thread ( struct CLIENT *client )
  { Envoyer_rs485_thread_tag( client, TAG_RS485, SSTAG_SERVEUR_ADDPROGRESS_RS485,
                                                 SSTAG_SERVEUR_ADDPROGRESS_RS485_FIN );
    return(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
