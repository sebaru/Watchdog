/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_capteur.c     Envoi des capteurs aux clients                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 22 mai 2005 17:35:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique_capteur.c
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
 #include "Reseaux.h"
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                         */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_capteur_atelier ( struct CLIENT *client, struct CMD_TYPE_CAPTEUR *rezo_capteur )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
    Info( Config.log, DEBUG_INFO, "MSRV: demande d'effacement capteur" );
    retour = Retirer_capteurDB( Config.log, Db_watchdog, rezo_capteur );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_CAPTEUR_OK,
                     (gchar *)rezo_capteur, sizeof(struct CMD_TYPE_CAPTEUR) );
       Info( Config.log, DEBUG_INFO, "MSRV: effacement capteur OK" );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete capteur %s", rezo_capteur->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Info( Config.log, DEBUG_INFO, "MSRV: effacement capteur NOK" );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                               */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_capteur_atelier ( struct CLIENT *client, struct CMD_TYPE_CAPTEUR *rezo_capteur )
  { struct CMD_TYPE_CAPTEUR *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    Info( Config.log, DEBUG_INFO, "MSRV: demande d'ajout capteur" );
    id = Ajouter_capteurDB ( Config.log, Db_watchdog, rezo_capteur );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add capteur %s", rezo_capteur->libelle );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Info( Config.log, DEBUG_INFO, "MSRV: ajout capteur NOK" );
     }
    else { result = Rechercher_capteurDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate capteur %s", rezo_capteur->libelle );
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
              Info( Config.log, DEBUG_INFO, "MSRV: ajout capteur NOK (2)" );
            }
           else
            { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_CAPTEUR_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_CAPTEUR) );
              g_free(result);
              Info( Config.log, DEBUG_INFO, "MSRV: ajout capteur OK" );
            }
         }
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_capteur_atelier ( struct CLIENT *client, struct CMD_TYPE_CAPTEUR *rezo_capteur )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
Info( Config.log, DEBUG_INFO, "Debut valider_editer_capteur_atelier" );

    retour = Modifier_capteurDB ( Config.log, Db_watchdog, rezo_capteur );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save capteur %s", rezo_capteur->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
Info( Config.log, DEBUG_INFO, "fin valider_editer_capteur_atelier" );
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_capteur_atelier_thread ( struct CLIENT *client )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_CAPTEUR *capteur;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiCapteur", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_capteurDB( Config.log, db, client->syn.id ) )
     { Libere_DB_SQL( Config.log, &db );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d capteurs", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { capteur = Recuperer_capteurDB_suite( Config.log, db );
       if (!capteur)
        { Libere_DB_SQL( Config.log, &db );
          Client_mode( client, ENVOI_CAMERA_SUP_ATELIER );
          Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAPTEUR_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
       Info_c( Config.log, DEBUG_INFO, "THR Envoyer_capteur_atelier: pass LIB", capteur->libelle );
       Info_n( Config.log, DEBUG_INFO, "THR Envoyer_capteur_atelier: pass ID ", capteur->id );
       Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAPTEUR,
                      (gchar *)capteur, sizeof(struct CMD_TYPE_CAPTEUR) );
       g_free(capteur);
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_capteur_supervision_thread ( struct CLIENT *client )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_CAPTEUR *capteur;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiCapteur", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_capteurDB( Config.log, db, client->num_supervision ) )
     { Libere_DB_SQL( Config.log, &db );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d capteurs", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { struct CAPTEUR *capteur_new;;
       capteur = Recuperer_capteurDB_suite( Config.log, db );
       if (!capteur)                                                                        /* Terminé ?? */
        { Libere_DB_SQL( Config.log, &db );
          Client_mode( client, ENVOI_CAMERA_SUP_SUPERVISION );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAPTEUR_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit( NULL );
        }

       capteur_new = (struct CAPTEUR *)g_malloc0( sizeof(struct CAPTEUR) );
       if (!capteur_new) continue;
       capteur_new->type = capteur->type;
       capteur_new->bit_controle = capteur->bit_controle;

       if ( ! g_list_find_custom(client->bit_init_capteur, capteur_new, (GCompareFunc) Chercher_bit_capteurs ) )
        { client->bit_init_capteur = g_list_append( client->bit_init_capteur, capteur_new );
          Info_n( Config.log, DEBUG_INFO , "  liste des bit_init_capteur ", capteur->id );
        }
       else g_free(capteur_new);

       while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

       Info_c( Config.log, DEBUG_INFO, "THR Envoyer_capteur_supervision: pass LIB", capteur->libelle );
       Info_n( Config.log, DEBUG_INFO, "THR Envoyer_capteur_supervision: pass ID ", capteur->id );
       Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAPTEUR,
                      (gchar *)capteur, sizeof(struct CMD_TYPE_CAPTEUR) );
       g_free(capteur);
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
