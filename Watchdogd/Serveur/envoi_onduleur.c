/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_onduleur.c        Configuration des onduleurs de Watchdog v2.0                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 13 sept. 2009 11:24:00 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_onduleur.c
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
/* Proto_editer_onduleur: Le client desire editer un onduleur                                             */
/* Entrée: le client demandeur et le onduleur en question                                                 */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_onduleur ( struct CLIENT *client, struct CMD_TYPE_ONDULEUR *rezo_onduleur )
  { struct CMD_TYPE_ONDULEUR *onduleur;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    onduleur = Rechercher_onduleurDB( Config.log, Db_watchdog, rezo_onduleur->id );

    if (onduleur)
     { Envoi_client( client, TAG_ONDULEUR, SSTAG_SERVEUR_EDIT_ONDULEUR_OK,
                     (gchar *)onduleur, sizeof(struct CMD_TYPE_ONDULEUR) );
       g_free(onduleur);                                                            /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate onduleur %s", rezo_onduleur->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_onduleur: Le client valide l'edition d'un onduleur                                */
/* Entrée: le client demandeur et le onduleur en question                                                 */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_onduleur ( struct CLIENT *client, struct CMD_TYPE_ONDULEUR *rezo_onduleur )
  { struct CMD_TYPE_ONDULEUR *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_onduleurDB ( Config.log, Db_watchdog, rezo_onduleur );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit onduleur %s", rezo_onduleur->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_onduleurDB( Config.log, Db_watchdog, rezo_onduleur->id );
         { if (!result)
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate onduleur %s", rezo_onduleur->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else { Envoi_client( client, TAG_ONDULEUR, SSTAG_SERVEUR_VALIDE_EDIT_ONDULEUR_OK,
                                (gchar *)result, sizeof(struct CMD_TYPE_ONDULEUR) );
                  g_free(result);
                  Partage->com_onduleur.Thread_reload = TRUE;
                }
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_onduleur: Retrait du onduleur en parametre                                               */
/* Entrée: le client demandeur et le onduleur en question                                                 */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_onduleur ( struct CLIENT *client, struct CMD_TYPE_ONDULEUR *rezo_onduleur )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_onduleurDB( Config.log, Db_watchdog, rezo_onduleur );

    if (retour)
     { Envoi_client( client, TAG_ONDULEUR, SSTAG_SERVEUR_DEL_ONDULEUR_OK,
                     (gchar *)rezo_onduleur, sizeof(struct CMD_TYPE_ONDULEUR) );
       while (Partage->com_onduleur.admin_del) sched_yield();
       Partage->com_onduleur.admin_del = rezo_onduleur->id;                   /* Envoi au thread onduleur */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete onduleur %s", rezo_onduleur->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_onduleur: Un client nous demande d'ajouter un onduleur Watchdog                          */
/* Entrée: le onduleur à créer                                                                            */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_onduleur ( struct CLIENT *client, struct CMD_TYPE_ONDULEUR *rezo_onduleur )
  { struct CMD_TYPE_ONDULEUR *onduleur;
    struct DB *Db_watchdog;
    gint id;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_onduleurDB ( Config.log, Db_watchdog, rezo_onduleur );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add onduleur %s", rezo_onduleur->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { onduleur = Rechercher_onduleurDB( Config.log, Db_watchdog, id );
           if (!onduleur) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate onduleur %s", rezo_onduleur->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_ONDULEUR, SSTAG_SERVEUR_ADD_ONDULEUR_OK,
                            (gchar *)onduleur, sizeof(struct CMD_TYPE_ONDULEUR) );
              while (Partage->com_onduleur.admin_add) sched_yield();
              Partage->com_onduleur.admin_add = onduleur->id;                 /* Envoi au thread onduleur */
              g_free(onduleur);
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_onduleurs: Envoi des onduleurs au client GID_ONDULEUR                                          */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void *Envoyer_onduleurs_thread_tag ( struct CLIENT *client, guint tag, guint sstag, guint sstag_fin )
  { struct CMD_TYPE_ONDULEUR *onduleur;
    struct CMD_ENREG nbr;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiOND.", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_onduleurDB( Config.log, db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Libere_DB_SQL( Config.log, &db );
       pthread_exit( NULL );
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d onduleurs", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { onduleur = Recuperer_onduleurDB_suite( Config.log, db );
       if (!onduleur)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
       Envoi_client ( client, tag, sstag, (gchar *)onduleur, sizeof(struct CMD_TYPE_ONDULEUR) );
       g_free(onduleur);
     }
  }
/**********************************************************************************************************/
/* Envoyer_onduleurs: Envoi des onduleurs au client GID_ONDULEUR                                          */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_onduleurs_thread ( struct CLIENT *client )
  { Envoyer_onduleurs_thread_tag( client, TAG_ONDULEUR, SSTAG_SERVEUR_ADDPROGRESS_ONDULEUR,
                                                    SSTAG_SERVEUR_ADDPROGRESS_ONDULEUR_FIN );
    return(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
