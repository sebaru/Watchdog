/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_scenario.c        Configuration des scenario de Watchdog v2.0                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 03 aoû 2008 16:32:59 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_scenario.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2008 - sebastien
 *
 *  Watchdog
  is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 *  Watchdog
  is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with  Watchdog
 ; if not, write to the Free Software
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
/* Preparer_envoi_scenario: convertit une structure MSG en structure CMD_SHOW_SCENARIO                    */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_SCENARIO *Preparer_envoi_sc ( struct SCENARIO_DB *sc )
  { struct CMD_SHOW_SCENARIO *rezo_sc;

    rezo_sc = (struct CMD_SHOW_SCENARIO *)g_malloc0( sizeof(struct CMD_SHOW_SCENARIO) );
    if (!rezo_sc) { return(NULL); }

    rezo_sc->id        = sc->id;
    rezo_sc->actif     = sc->actif;
    rezo_sc->bit_m     = sc->bit_m;
    rezo_sc->ts_jour   = sc->ts_jour;
    rezo_sc->ts_mois   = sc->ts_mois;
    rezo_sc->lundi     = sc->lundi;
    rezo_sc->mardi     = sc->mardi;
    rezo_sc->mercredi  = sc->mercredi;
    rezo_sc->jeudi     = sc->jeudi;
    rezo_sc->vendredi  = sc->vendredi;
    rezo_sc->samedi    = sc->samedi;
    rezo_sc->dimanche  = sc->dimanche;
    rezo_sc->janvier   = sc->janvier;
    rezo_sc->fevrier   = sc->fevrier;
    rezo_sc->mars      = sc->mars;
    rezo_sc->avril     = sc->avril;
    rezo_sc->mai       = sc->mai;
    rezo_sc->juin      = sc->juin;
    rezo_sc->juillet   = sc->juillet;
    rezo_sc->aout      = sc->aout;
    rezo_sc->septembre = sc->septembre;
    rezo_sc->octobre   = sc->octobre;
    rezo_sc->novembre  = sc->novembre;
    rezo_sc->decembre  = sc->decembre;
    memcpy( &rezo_sc->libelle, sc->libelle, sizeof(rezo_sc->libelle) );
    return( rezo_sc );
  }
/**********************************************************************************************************/
/* Proto_editer_sc: Le client desire editer un sc                                                         */
/* Entrée: le client demandeur et le sc en question                                                       */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_scenario ( struct CLIENT *client, struct CMD_ID_SCENARIO *rezo_sc )
  { struct CMD_EDIT_SCENARIO edit_sc;
    struct SCENARIO_DB *sc;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    sc = Rechercher_scenarioDB( Config.log, Db_watchdog, rezo_sc->id );

    if (sc)
     { edit_sc.id        = sc->id;
       edit_sc.actif     = sc->actif;
       edit_sc.bit_m     = sc->bit_m;
       edit_sc.heure     = sc->heure;
       edit_sc.minute    = sc->minute;
       edit_sc.ts_jour   = sc->ts_jour;
       edit_sc.ts_mois   = sc->ts_mois;
       edit_sc.lundi     = sc->lundi;
       edit_sc.mardi     = sc->mardi;
       edit_sc.mercredi  = sc->mercredi;
       edit_sc.jeudi     = sc->jeudi;
       edit_sc.vendredi  = sc->vendredi;
       edit_sc.samedi    = sc->samedi;
       edit_sc.dimanche  = sc->dimanche;
       edit_sc.janvier   = sc->janvier;
       edit_sc.fevrier   = sc->fevrier;
       edit_sc.mars      = sc->mars;
       edit_sc.avril     = sc->avril;
       edit_sc.mai       = sc->mai;
       edit_sc.juin      = sc->juin;
       edit_sc.juillet   = sc->juillet;
       edit_sc.aout      = sc->aout;
       edit_sc.septembre = sc->septembre;
       edit_sc.octobre   = sc->octobre;
       edit_sc.novembre  = sc->novembre;
       edit_sc.decembre  = sc->decembre;
       memcpy( &edit_sc.libelle, sc->libelle, sizeof(rezo_sc->libelle) );

       Envoi_client( client, TAG_SCENARIO, SSTAG_SERVEUR_EDIT_SCENARIO_OK,
                  (gchar *)&edit_sc, sizeof(struct CMD_EDIT_SCENARIO) );
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
 void Proto_valider_editer_scenario ( struct CLIENT *client, struct CMD_EDIT_SCENARIO *rezo_sc )
  { struct SCENARIO_DB *result;
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
            { struct CMD_SHOW_SCENARIO *sc;
              sc = Preparer_envoi_sc ( result );
              g_free(result);
              if (!sc)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_SCENARIO, SSTAG_SERVEUR_VALIDE_EDIT_SCENARIO_OK,
                                   (gchar *)sc, sizeof(struct CMD_SHOW_SCENARIO) );
                     g_free(sc);
                     Charger_scenario ();
                   }
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
 void Proto_effacer_scenario_tag ( struct CLIENT *client, struct CMD_ID_SCENARIO *rezo_sc,
                                          gint tag, gint sstag )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_scenarioDB( Config.log, Db_watchdog, rezo_sc );

    if (retour)
     { Envoi_client( client, tag, sstag,
                     (gchar *)rezo_sc, sizeof(struct CMD_ID_SCENARIO) );
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
 void Proto_effacer_scenario ( struct CLIENT *client, struct CMD_ID_SCENARIO *rezo_sc )
  { Proto_effacer_scenario_tag (client, rezo_sc, TAG_SCENARIO, SSTAG_SERVEUR_DEL_SCENARIO_OK );
  }
/**********************************************************************************************************/
/* Proto_ajouter_sc: Un client nous demande d'ajouter un sc Watchdog                                      */
/* Entrée: le sc à créer                                                                                  */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_scenario ( struct CLIENT *client, struct CMD_ADD_SCENARIO *rezo_sc )
  { struct SCENARIO_DB *result;
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
            { struct CMD_SHOW_SCENARIO *sc;
              sc = Preparer_envoi_sc ( result );
              g_free(result);
              if (!sc)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_SCENARIO, SSTAG_SERVEUR_ADD_SCENARIO_OK,
                                   (gchar *)sc, sizeof(struct CMD_SHOW_SCENARIO) );
                     g_free(sc);
                     Charger_scenario ();
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_scs: Envoi des scs au client GID_SCENARIO                                                      */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_scenario_thread ( struct CLIENT *client )
  { struct CMD_SHOW_SCENARIO *rezo_sc;
    struct CMD_ENREG nbr;
    struct SCENARIO_DB *sc;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiSUPSCE", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
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

       rezo_sc = Preparer_envoi_sc( sc );
       g_free(sc);
       if (rezo_sc)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
          Envoi_client ( client, TAG_SCENARIO, SSTAG_SERVEUR_ADDPROGRESS_SCENARIO,
                         (gchar *)rezo_sc, sizeof(struct CMD_SHOW_SCENARIO) );
          g_free(rezo_sc);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
