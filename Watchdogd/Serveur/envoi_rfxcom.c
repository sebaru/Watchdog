/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_rfxcom.c        Configuration des rfxcoms de Watchdog v2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                    mer. 13 juin 2012 19:01:06 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_rfxcom.c
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
/* Proto_editer_rfxcom: Le client desire editer un rfxcom                                                   */
/* Entrée: le client demandeur et le rfxcom en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_rfxcom ( struct CLIENT *client, struct CMD_TYPE_RFXCOM *rezo_rfxcom )
  { struct CMD_TYPE_RFXCOM *rfxcom;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    rfxcom = Partage->com_rfxcom.Rechercher_rfxcomDB( Config.log, Db_watchdog, rezo_rfxcom->id );

    if (rfxcom)
     { Envoi_client( client, TAG_RFXCOM, SSTAG_SERVEUR_EDIT_RFXCOM_OK,
                     (gchar *)rfxcom, sizeof(struct CMD_TYPE_RFXCOM) );
       g_free(rfxcom);                                                              /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate rfxcom %s", rezo_rfxcom->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_rfxcom: Le client valide l'edition d'un rfxcom                                    */
/* Entrée: le client demandeur et le rfxcom en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_rfxcom ( struct CLIENT *client, struct CMD_TYPE_RFXCOM *rezo_rfxcom )
  { struct CMD_TYPE_RFXCOM *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Partage->com_rfxcom.Modifier_rfxcomDB ( Config.log, Db_watchdog, rezo_rfxcom );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit rfxcom %s", rezo_rfxcom->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Partage->com_rfxcom.Rechercher_rfxcomDB( Config.log, Db_watchdog, rezo_rfxcom->id );
         { if (!result)
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate rfxcom %s", rezo_rfxcom->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else { Envoi_client( client, TAG_RFXCOM, SSTAG_SERVEUR_VALIDE_EDIT_RFXCOM_OK,
                                (gchar *)result, sizeof(struct CMD_TYPE_RFXCOM) );
                  while (Partage->com_rfxcom.Thread_reload) sched_yield();
                  Partage->com_rfxcom.Thread_reload = TRUE;         /* Modification -> Reload module rfxcom */
                  g_free(result);
                }
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_rfxcom: Retrait du rfxcom en parametre                                                     */
/* Entrée: le client demandeur et le rfxcom en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_rfxcom ( struct CLIENT *client, struct CMD_TYPE_RFXCOM *rezo_rfxcom )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Partage->com_rfxcom.Retirer_rfxcomDB( Config.log, Db_watchdog, rezo_rfxcom );

    if (retour)
     { Envoi_client( client, TAG_RFXCOM, SSTAG_SERVEUR_DEL_RFXCOM_OK,
                     (gchar *)rezo_rfxcom, sizeof(struct CMD_TYPE_RFXCOM) );
       while (Partage->com_rfxcom.Thread_reload) sched_yield();
       Partage->com_rfxcom.Thread_reload = TRUE;                                /* Envoi au thread rfxcom */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete rfxcom %s", rezo_rfxcom->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_rfxcom: Un client nous demande d'ajouter un rfxcom Watchdog                                */
/* Entrée: le rfxcom à créer                                                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_rfxcom ( struct CLIENT *client, struct CMD_TYPE_RFXCOM *rezo_rfxcom )
  { struct CMD_TYPE_RFXCOM *rfxcom;
    struct DB *Db_watchdog;
    gint id;
    Db_watchdog = client->Db_watchdog;

    id = Partage->com_rfxcom.Ajouter_rfxcomDB ( Config.log, Db_watchdog, rezo_rfxcom );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add rfxcom %s", rezo_rfxcom->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { rfxcom = Partage->com_rfxcom.Rechercher_rfxcomDB( Config.log, Db_watchdog, id );
           if (!rfxcom)
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate rfxcom %s", rezo_rfxcom->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_RFXCOM, SSTAG_SERVEUR_ADD_RFXCOM_OK,
                            (gchar *)rfxcom, sizeof(struct CMD_TYPE_RFXCOM) );
              while (Partage->com_rfxcom.Thread_reload) sched_yield();
              Partage->com_rfxcom.Thread_reload = TRUE;                         /* Envoi au thread rfxcom */
              g_free(rfxcom);
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_rfxcoms: Envoi des rfxcoms au client                                                           */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void *Envoyer_rfxcom_thread_tag ( struct CLIENT *client, guint tag, guint sstag, guint sstag_fin )
  { struct CMD_TYPE_RFXCOM *rfxcom;
    struct CMD_ENREG nbr;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiRFXCOM", 0, 0, 0 );

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Partage->com_rfxcom.Recuperer_rfxcomDB( Config.log, db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Libere_DB_SQL( Config.log, &db );
       pthread_exit( NULL );
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d modules rfxcom", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { rfxcom = Partage->com_rfxcom.Recuperer_rfxcomDB_suite( Config.log, db );
       if (!rfxcom)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
       Envoi_client ( client, tag, sstag, (gchar *)rfxcom, sizeof(struct CMD_TYPE_RFXCOM) );
       g_free(rfxcom);
     }
  }
/**********************************************************************************************************/
/* Envoyer_rfxcoms: Envoi des rfxcoms au client GID_RFXCOM                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_rfxcom_thread ( struct CLIENT *client )
  { Envoyer_rfxcom_thread_tag( client, TAG_RFXCOM, SSTAG_SERVEUR_ADDPROGRESS_RFXCOM,
                                                   SSTAG_SERVEUR_ADDPROGRESS_RFXCOM_FIN );
    return(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
