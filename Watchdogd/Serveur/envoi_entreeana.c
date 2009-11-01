/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_entree.c        Configuration des entrees de Watchdog v2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 25 sep 2003 14:17:17 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_entreeana.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2007 - Sébastien Lefevre
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
 #include "Cst_entreeana.h"
/**********************************************************************************************************/
/* Proto_editer_entree: Le client desire editer un entree                                                 */
/* Entrée: le client demandeur et le entree en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_entreeANA ( struct CLIENT *client, struct CMD_TYPE_ENTREEANA *rezo_entree )
  { struct CMD_TYPE_ENTREEANA *entree;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

#ifdef bouh
    entree = Rechercher_entreeANADB( Config.log, Db_watchdog, rezo_entree->num );
#endif

    if (entree)
     { Envoi_client( client, TAG_ENTREEANA, SSTAG_SERVEUR_EDIT_ENTREEANA_OK,
                  (gchar *)entree, sizeof(struct CMD_TYPE_ENTREEANA) );
       g_free(entree);                                                              /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
#ifdef bouh
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate entree %s", rezo_entree->libelle);
#endif
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_entree: Le client valide l'edition d'un entree                                    */
/* Entrée: le client demandeur et le entree en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_entreeANA ( struct CLIENT *client, struct CMD_TYPE_ENTREEANA *rezo_entree )
  { struct CMD_TYPE_ENTREEANA *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_entreeANADB ( Config.log, Db_watchdog, rezo_entree );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
#ifdef bouh
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit entree %s", rezo_entree->libelle);
#endif
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { Partage->ea[rezo_entree->num].min = rezo_entree->min;        /* Mise à jour min et max de l'ea */
           Partage->ea[rezo_entree->num].max = rezo_entree->max;
           
           result = Rechercher_entreeANADB( Config.log, Db_watchdog, rezo_entree->num );
           if (result) 
            { Envoi_client( client, TAG_ENTREEANA, SSTAG_SERVEUR_VALIDE_EDIT_ENTREEANA_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_ENTREEANA) );
              g_free(result);
              Charger_eana ();                                             /* Update de la running config */
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
#ifdef bouh
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate entree %s", rezo_entree->libelle);
#endif
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_entreeANA_tag : Envoie les entreANA au client. Attention, c'est un thread !                    */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoyer_entreeANA_tag ( struct CLIENT *client, guint tag, gint sstag, gint sstag_fin )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_ENTREEANA *entree;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiANA", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       return;
     }                                                                           /* Si pas de histos (??) */
#ifdef bouh
    if (!Recuperer_entreeANADB( Config.log, db ))
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Libere_DB_SQL( Config.log, &db );
       return;
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d entrees", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { entree = Recuperer_entreeANADB_suite( Config.log, db );
       if (!entree)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          return;
        }

       while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

       Envoi_client ( client, tag, sstag, (gchar *)entree, sizeof(struct CMD_TYPE_ENTREEANA) );
       g_free(entree);
     }
#endif
  }
/**********************************************************************************************************/
/* Envoyer_classes: Envoi des classes au client GID_CLASSE                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_entreeANA_thread ( struct CLIENT *client )
  { Envoyer_entreeANA_tag( client, TAG_ENTREEANA, SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA,
                                                  SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FIN );
    pthread_exit(NULL);
  }
/**********************************************************************************************************/
/* Envoyer_classes: Envoi des classes au client GID_CLASSE                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_entreeANA_for_courbe_thread ( struct CLIENT *client )
  { Envoyer_entreeANA_tag( client, TAG_COURBE, SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_COURBE,
                                               SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_COURBE_FIN );
    Client_mode ( client, ENVOI_MNEMONIQUE_FOR_COURBE );
    pthread_exit(NULL);
  }
/**********************************************************************************************************/
/* Envoyer_classes: Envoi des classes au client GID_CLASSE                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_entreeANA_for_histo_courbe_thread ( struct CLIENT *client )
  { Envoyer_entreeANA_tag( client, TAG_HISTO_COURBE,
                           SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_HISTO_COURBE,
                           SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_HISTO_COURBE_FIN );
    Client_mode ( client, ENVOI_MNEMONIQUE_FOR_HISTO_COURBE );
    pthread_exit(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
