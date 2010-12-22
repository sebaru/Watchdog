/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_modbus.c        Configuration des modbuss de Watchdog v2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                   dim. 05 sept. 2010 15:36:46 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_modbus.c
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
/* Proto_editer_modbus: Le client desire editer un modbus                                                 */
/* Entrée: le client demandeur et le modbus en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_modbus ( struct CLIENT *client, struct CMD_TYPE_MODBUS *rezo_modbus )
  { struct CMD_TYPE_MODBUS *modbus;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    modbus = Rechercher_modbusDB( Config.log, Db_watchdog, rezo_modbus->id );

    if (modbus)
     { Envoi_client( client, TAG_MODBUS, SSTAG_SERVEUR_EDIT_MODBUS_OK,
                     (gchar *)modbus, sizeof(struct CMD_TYPE_MODBUS) );
       g_free(modbus);                                                              /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate modbus %s", rezo_modbus->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_modbus: Le client valide l'edition d'un modbus                                    */
/* Entrée: le client demandeur et le modbus en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_modbus ( struct CLIENT *client, struct CMD_TYPE_MODBUS *rezo_modbus )
  { struct CMD_TYPE_MODBUS *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_modbusDB ( Config.log, Db_watchdog, rezo_modbus );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit modbus %s", rezo_modbus->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_modbusDB( Config.log, Db_watchdog, rezo_modbus->id );
         { if (!result)
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate modbus %s", rezo_modbus->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else { Envoi_client( client, TAG_MODBUS, SSTAG_SERVEUR_VALIDE_EDIT_MODBUS_OK,
                                (gchar *)result, sizeof(struct CMD_TYPE_MODBUS) );
                  Partage->com_modbus.reload = TRUE;                /* Modification -> Reload module modbus */
                  g_free(result);
                }
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_modbus: Retrait du modbus en parametre                                                   */
/* Entrée: le client demandeur et le modbus en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_modbus ( struct CLIENT *client, struct CMD_TYPE_MODBUS *rezo_modbus )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_modbusDB( Config.log, Db_watchdog, rezo_modbus );

    if (retour)
     { Envoi_client( client, TAG_MODBUS, SSTAG_SERVEUR_DEL_MODBUS_OK,
                     (gchar *)rezo_modbus, sizeof(struct CMD_TYPE_MODBUS) );
       while (Partage->com_modbus.admin_del) sched_yield();
       Partage->com_modbus.admin_del = rezo_modbus->id;                         /* Envoi au thread modbus */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete modbus %s", rezo_modbus->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_modbus: Un client nous demande d'ajouter un modbus Watchdog                              */
/* Entrée: le modbus à créer                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_modbus ( struct CLIENT *client, struct CMD_TYPE_MODBUS *rezo_modbus )
  { struct CMD_TYPE_MODBUS *modbus;
    struct DB *Db_watchdog;
    gint id;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_modbusDB ( Config.log, Db_watchdog, rezo_modbus );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add modbus %s", rezo_modbus->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { modbus = Rechercher_modbusDB( Config.log, Db_watchdog, id );
           if (!modbus)
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate modbus %s", rezo_modbus->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_MODBUS, SSTAG_SERVEUR_ADD_MODBUS_OK,
                            (gchar *)modbus, sizeof(struct CMD_TYPE_MODBUS) );
              while (Partage->com_modbus.admin_add) sched_yield();
              Partage->com_modbus.admin_add = modbus->id;                          /* Envoi au thread modbus */
              g_free(modbus);
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_modbuss: Envoi des modbuss au client GID_MODBUS                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void *Envoyer_modbus_thread_tag ( struct CLIENT *client, guint tag, guint sstag, guint sstag_fin )
  { struct CMD_TYPE_MODBUS *modbus;
    struct CMD_ENREG nbr;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiMODBUS", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_modbusDB( Config.log, db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Libere_DB_SQL( Config.log, &db );
       pthread_exit( NULL );
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d modules modbus", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { modbus = Recuperer_modbusDB_suite( Config.log, db );
       if (!modbus)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
       Envoi_client ( client, tag, sstag, (gchar *)modbus, sizeof(struct CMD_TYPE_MODBUS) );
       g_free(modbus);
     }
  }
/**********************************************************************************************************/
/* Envoyer_modbus_thread: Envoi des modules modbuss au client                                             */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_modbus_thread ( struct CLIENT *client )
  { Envoyer_modbus_thread_tag( client, TAG_MODBUS, SSTAG_SERVEUR_ADDPROGRESS_MODBUS,
                                                   SSTAG_SERVEUR_ADDPROGRESS_MODBUS_FIN );
    return(NULL);
  }
/**********************************************************************************************************/
/* Proto_effacer_borne_modbus: Retrait d'une borne modbus en parametre                                    */
/* Entrée: le client demandeur et le modbus en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_borne_modbus ( struct CLIENT *client, struct CMD_TYPE_BORNE_MODBUS *rezo_borne )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_borne_modbusDB( Config.log, Db_watchdog, rezo_borne );

    if (retour)
     { Envoi_client( client, TAG_MODBUS, SSTAG_SERVEUR_DEL_BORNE_MODBUS_OK,
                     (gchar *)rezo_borne, sizeof(struct CMD_TYPE_BORNE_MODBUS) );
       while (Partage->com_modbus.admin_del) sched_yield();
       Partage->com_modbus.admin_del_borne = rezo_borne->id;                    /* Envoi au thread modbus */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete borne");
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Envoyer_borne_modbus_thread_tag: Envoi des bornes au client, avec des tags en paralmètres              */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void *Envoyer_borne_modbus_thread_tag ( struct CLIENT *client, guint tag, guint sstag, guint sstag_fin )
  { struct CMD_TYPE_BORNE_MODBUS *borne;
    struct CMD_ENREG nbr;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiBORNE ", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_borne_modbusDB( Config.log, db, client->id_modbus_bornes_a_editer ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Libere_DB_SQL( Config.log, &db );
       pthread_exit( NULL );
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d bornes modbus", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { borne = Recuperer_borne_modbusDB_suite( Config.log, db );
       if (!borne)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
       Envoi_client ( client, tag, sstag, (gchar *)borne, sizeof(struct CMD_TYPE_BORNE_MODBUS) );
       g_free(borne);
     }
  }
/**********************************************************************************************************/
/* Envoyer_bornes_modbus_thread: Envoi les bornes modbus au client                                        */
/* Entrée: le client qui a demandé l'edition                                                              */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_borne_modbus_thread ( struct CLIENT *client )
  { Envoyer_borne_modbus_thread_tag( client, TAG_MODBUS, SSTAG_SERVEUR_ADDPROGRESS_BORNE_MODBUS,
                                                         SSTAG_SERVEUR_ADDPROGRESS_BORNE_MODBUS_FIN );
    return(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
