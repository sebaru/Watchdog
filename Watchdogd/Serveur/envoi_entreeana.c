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

 #include "Reseaux.h"
 #include "EntreeANA_DB.h"
 #include "Erreur.h"
 #include "Config.h"
 #include "Client.h"
 #include "watchdogd.h"
 #include "Cst_entreeana.h"

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Preparer_envoi_entree: convertit une structure ENTREEANA_DB en structure CMD_SHOW_ENTREEANA            */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_ENTREEANA *Preparer_envoi_entree ( struct ENTREEANA_DB *entree )
  { struct CMD_SHOW_ENTREEANA *rezo_entree;

    rezo_entree = (struct CMD_SHOW_ENTREEANA *)g_malloc0( sizeof(struct CMD_SHOW_ENTREEANA) );
    if (!rezo_entree) { return(NULL); }

    rezo_entree->id    = entree->id;
    rezo_entree->num   = entree->num;
    rezo_entree->min   = entree->min;
    rezo_entree->max   = entree->max;
    rezo_entree->unite = entree->unite;
    memcpy( &rezo_entree->libelle, entree->libelle, sizeof(rezo_entree->libelle) );
    return( rezo_entree );
  }
/**********************************************************************************************************/
/* Proto_editer_entree: Le client desire editer un entree                                                 */
/* Entrée: le client demandeur et le entree en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_entreeANA ( struct CLIENT *client, struct CMD_ID_ENTREEANA *rezo_entree )
  { struct CMD_EDIT_ENTREEANA edit_entree;
    struct ENTREEANA_DB *entree;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    entree = Rechercher_entreeANADB( Config.log, Db_watchdog, rezo_entree->id );

    if (entree)
     { edit_entree.id    = entree->id;                                      /* Recopie des info editables */
       edit_entree.num   = entree->num;
       edit_entree.min   = entree->min;
       edit_entree.max   = entree->max;
       edit_entree.unite = entree->unite;
       memcpy( &edit_entree.libelle, entree->libelle, sizeof(edit_entree.libelle) );

       Envoi_client( client, TAG_ENTREEANA, SSTAG_SERVEUR_EDIT_ENTREEANA_OK,
                  (gchar *)&edit_entree, sizeof(struct CMD_EDIT_ENTREEANA) );
       g_free(entree);                                                              /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate entree %s:\n%s", rezo_entree->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_entree: Le client valide l'edition d'un entree                                    */
/* Entrée: le client demandeur et le entree en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_entreeANA ( struct CLIENT *client, struct CMD_EDIT_ENTREEANA *rezo_entree )
  { struct ENTREEANA_DB *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_entreeANADB ( Config.log, Db_watchdog, rezo_entree );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit entree %s:\n%s", rezo_entree->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { Partage->ea[rezo_entree->num].min = rezo_entree->min;        /* Mise à jour min et max de l'ea */
           Partage->ea[rezo_entree->num].max = rezo_entree->max;
           
           result = Rechercher_entreeANADB( Config.log, Db_watchdog, rezo_entree->id );
           if (result) 
            { struct CMD_SHOW_ENTREEANA *entree;
              entree = Preparer_envoi_entree ( result );
              g_free(result);
              if (!entree)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_ENTREEANA, SSTAG_SERVEUR_VALIDE_EDIT_ENTREEANA_OK,
                                   (gchar *)entree, sizeof(struct CMD_SHOW_ENTREEANA) );
                     g_free(entree);
                     Charger_eana ( Db_watchdog );
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate entree %s:\n%s", rezo_entree->libelle, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_entree: Retrait du entree en parametre                                                   */
/* Entrée: le client demandeur et le entree en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_entreeANA ( struct CLIENT *client, struct CMD_ID_ENTREEANA *rezo_entree )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_entreeANADB( Config.log, Db_watchdog, rezo_entree );

    if (retour)
     { Envoi_client( client, TAG_ENTREEANA, SSTAG_SERVEUR_DEL_ENTREEANA_OK,
                     (gchar *)rezo_entree, sizeof(struct CMD_ID_ENTREEANA) );
       Charger_eana ( Db_watchdog );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete entree %s:\n%s", rezo_entree->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_entree: Un client nous demande d'ajouter un entree Watchdog                              */
/* Entrée: le entree à créer                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_entreeANA ( struct CLIENT *client, struct CMD_ADD_ENTREEANA *rezo_entree )
  { struct ENTREEANA_DB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_entreeANADB ( Config.log, Db_watchdog, rezo_entree );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add entree %d:\n%s", rezo_entree->num, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_entreeANADB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate entree %d:\n%s", rezo_entree->num, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_SHOW_ENTREEANA *entree;
              entree = Preparer_envoi_entree ( result );
              g_free(result);
              if (!entree)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
                 Charger_eana ( Db_watchdog );
               }
              else { Envoi_client( client, TAG_ENTREEANA, SSTAG_SERVEUR_ADD_ENTREEANA_OK,
                                   (gchar *)entree, sizeof(struct CMD_SHOW_ENTREEANA) );
                     g_free(entree);
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_entreeANA_tag : Envoie les entreANA au client. Attention, c'est un thread !                    */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoyer_entreeANA_tag ( struct CLIENT *client, guint tag, gint sstag, gint sstag_fin )
  { struct CMD_SHOW_ENTREEANA *rezo_entree;
    struct CMD_ENREG nbr;
    struct ENTREEANA_DB *entree;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiANA", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       return;
     }                                                                           /* Si pas de histos (??) */

    if (!Recuperer_entreeANADB( Config.log, db ))
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       return;
     }                                                                           /* Si pas de histos (??) */

    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d entrees", db->nbr_result );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { entree = Recuperer_entreeANADB_suite( Config.log, db );
       if (!entree)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          return;
        }

       rezo_entree = Preparer_envoi_entree( entree );
       g_free(entree);
       if (rezo_entree)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Envoi_client ( client, tag, sstag,
                         (gchar *)rezo_entree, sizeof(struct CMD_SHOW_ENTREEANA) );
          g_free(rezo_entree);
        }
     }
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
