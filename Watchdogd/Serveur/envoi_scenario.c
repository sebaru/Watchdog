/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_scenario.c        Configuration des scenario de Watchdog v2.0                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 03 aoû 2008 16:32:59 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_scenario.c
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
/* Proto_editer_sc: Le client desire editer un sc                                                         */
/* Entrée: le client demandeur et le sc en question                                                       */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_scenario ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc )
  { struct CMD_TYPE_SCENARIO *sc;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    sc = Rechercher_scenarioDB( Config.log, Db_watchdog, rezo_sc->id );
    if (sc)
     { Envoi_client( client, TAG_SCENARIO, SSTAG_SERVEUR_EDIT_SCENARIO_OK,
                  (gchar *)sc, sizeof(struct CMD_TYPE_SCENARIO) );
       g_free(sc);                                                                  /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate scenario %s", rezo_sc->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_sc: Le client valide l'edition d'un sc                                            */
/* Entrée: le client demandeur et le sc en question                                                       */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_scenario ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc )
  { struct CMD_TYPE_SCENARIO *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_scenarioDB ( Config.log, Db_watchdog, rezo_sc );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit scenario %s", rezo_sc->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_scenarioDB( Config.log, Db_watchdog, rezo_sc->id );
           if (result) 
            { Envoi_client( client, TAG_SCENARIO, SSTAG_SERVEUR_VALIDE_EDIT_SCENARIO_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_SCENARIO) );
              g_free(result);
              Charger_scenario ();
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate scenario %s", rezo_sc->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_sc: Retrait du sc en parametre                                                           */
/* Entrée: le client demandeur et le sc en question                                                       */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_scenario_tag ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc,
                                          gint tag, gint sstag )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_scenarioDB( Config.log, Db_watchdog, rezo_sc );

    if (retour)
     { Envoi_client( client, tag, sstag,
                     (gchar *)rezo_sc, sizeof(struct CMD_TYPE_SCENARIO) );
       Charger_scenario ();
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete scenario %s", rezo_sc->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_effacer_sc: Retrait du sc en parametre                                                           */
/* Entrée: le client demandeur et le sc en question                                                       */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_scenario ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc )
  { Proto_effacer_scenario_tag (client, rezo_sc, TAG_SCENARIO, SSTAG_SERVEUR_DEL_SCENARIO_OK );
  }
/**********************************************************************************************************/
/* Proto_ajouter_sc: Un client nous demande d'ajouter un sc Watchdog                                      */
/* Entrée: le sc à créer                                                                                  */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_scenario ( struct CLIENT *client, struct CMD_TYPE_SCENARIO *rezo_sc )
  { struct CMD_TYPE_SCENARIO *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_scenarioDB ( Config.log, Db_watchdog, rezo_sc );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add scenario %s", rezo_sc->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_scenarioDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate scenario %s", rezo_sc->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_SCENARIO, SSTAG_SERVEUR_ADD_SCENARIO_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_SCENARIO) );
              g_free(result);
              Charger_scenario ();
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_scs: Envoi des scs au client GID_SCENARIO                                                      */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_scenario_thread ( struct CLIENT *client )
  { struct CMD_TYPE_SCENARIO *sc;
    struct CMD_ENREG nbr;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiSUPSCE", 0, 0, 0 );

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_scenarioDB( Config.log, db ) )
     { Libere_DB_SQL( Config.log, &db );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d scenarios", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { sc = Recuperer_scenarioDB_suite( Config.log, db );
       if (!sc)
        { Envoi_client ( client, TAG_SCENARIO, SSTAG_SERVEUR_ADDPROGRESS_SCENARIO_FIN, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       Envoi_client ( client, TAG_SCENARIO, SSTAG_SERVEUR_ADDPROGRESS_SCENARIO,
                      (gchar *)sc, sizeof(struct CMD_TYPE_SCENARIO) );
       g_free(sc);
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
