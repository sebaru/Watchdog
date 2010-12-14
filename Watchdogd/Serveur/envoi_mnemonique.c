/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_mnemonique.c        Configuration des mnemoniques DLS de Watchdog v2.0         */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 05 déc 2004 14:50:01 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_mnemonique.c
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
/* Proto_editer_entree: Le client desire editer un entree                                                 */
/* Entrée: le client demandeur et le entree en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_option_entreeANA ( struct CLIENT *client, struct CMD_TYPE_MNEMONIQUE *rezo_mnemo )
  { struct CMD_TYPE_OPTION_BIT_INTERNE option;
    struct CMD_TYPE_ENTREEANA *entree;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    entree = Rechercher_entreeANADB( Config.log, Db_watchdog, rezo_mnemo->id );

    if (entree)
     { option.type = MNEMO_ENTREE_ANA;
       memcpy( &option.eana, entree, sizeof( struct CMD_TYPE_ENTREEANA ) );
       Envoi_client( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_EDIT_OPTION_BIT_INTERNE_OK,
                     (gchar *)&option, sizeof(struct CMD_TYPE_OPTION_BIT_INTERNE) );
       g_free(entree);                                                              /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate entree %s", rezo_mnemo->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Preparer_envoi_mnemonique: convertit une structure MNEMONIQUE en structure CMD_TYPE_MNEMONIQUE         */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_TYPE_MNEMONIQUE *Preparer_envoi_mnemonique ( struct MNEMONIQUEDB *mnemo )
  { struct CMD_TYPE_MNEMONIQUE *rezo_mnemonique;

    rezo_mnemonique = (struct CMD_TYPE_MNEMONIQUE *)g_malloc0( sizeof(struct CMD_TYPE_MNEMONIQUE) );
    if (!rezo_mnemonique) { return(NULL); }

    rezo_mnemonique->id         = mnemo->id;
    rezo_mnemonique->type       = mnemo->type;
    rezo_mnemonique->num        = mnemo->num;
    memcpy( &rezo_mnemonique->libelle, mnemo->libelle, sizeof(rezo_mnemonique->libelle) );
    memcpy( &rezo_mnemonique->acronyme, mnemo->acronyme, sizeof(rezo_mnemonique->acronyme) );
    memcpy( &rezo_mnemonique->objet, mnemo->objet, sizeof(rezo_mnemonique->objet) );
   return( rezo_mnemonique );
  }
/**********************************************************************************************************/
/* Proto_editer_mnemonique: Le client desire editer un mnemo                                              */
/* Entrée: le client demandeur et le mnemo en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMONIQUE *rezo_mnemonique )
  { struct CMD_TYPE_MNEMONIQUE edit_mnemonique;
    struct MNEMONIQUEDB *mnemo;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    mnemo = Rechercher_mnemoDB( Config.log, Db_watchdog, rezo_mnemonique->id );

    if (mnemo)
     { edit_mnemonique.id         = mnemo->id;                              /* Recopie des info editables */
       edit_mnemonique.type       = mnemo->type;
       edit_mnemonique.num        = mnemo->num;
       memcpy( &edit_mnemonique.libelle, mnemo->libelle, sizeof(edit_mnemonique.libelle) );
       memcpy( &edit_mnemonique.acronyme, mnemo->acronyme, sizeof(edit_mnemonique.acronyme) );
       memcpy( &edit_mnemonique.objet, mnemo->objet, sizeof(edit_mnemonique.objet) );

       Envoi_client( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_EDIT_MNEMONIQUE_OK,
                  (gchar *)&edit_mnemonique, sizeof(struct CMD_TYPE_MNEMONIQUE) );
       g_free(mnemo);                                                               /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate mnemo %s", rezo_mnemonique->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_mnemonique: Le client valide l'edition d'un mnemo                                 */
/* Entrée: le client demandeur et le mnemo en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMONIQUE *rezo_mnemonique )
  { struct MNEMONIQUEDB *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_mnemoDB ( Config.log, Db_watchdog, rezo_mnemonique );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit mnemo %s", rezo_mnemonique->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_mnemoDB( Config.log, Db_watchdog, rezo_mnemonique->id );
           if (result) 
            { struct CMD_TYPE_MNEMONIQUE *mnemo;
              mnemo = Preparer_envoi_mnemonique ( result );
              g_free(result);
              if (!mnemo)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_VALIDE_EDIT_MNEMONIQUE_OK,
                                   (gchar *)mnemo, sizeof(struct CMD_TYPE_MNEMONIQUE) );
                     g_free(mnemo);
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate mnemo %s", rezo_mnemonique->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_mnemonique: Retrait du mnemo en parametre                                                */
/* Entrée: le client demandeur et le mnemo en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMONIQUE *rezo_mnemonique )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_mnemoDB( Config.log, Db_watchdog, rezo_mnemonique );

    if (retour)
     { Envoi_client( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_DEL_MNEMONIQUE_OK,
                     (gchar *)rezo_mnemonique, sizeof(struct CMD_TYPE_MNEMONIQUE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete mnemo %s", rezo_mnemonique->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_mnemonique: Un client nous demande d'ajouter un mnemo Watchdog                           */
/* Entrée: le mnemo à créer                                                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMONIQUE *rezo_mnemonique )
  { struct MNEMONIQUEDB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_mnemoDB ( Config.log, Db_watchdog, rezo_mnemonique );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add mnemo %s", rezo_mnemonique->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_mnemoDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate mnemo %s", rezo_mnemonique->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_TYPE_MNEMONIQUE *mnemo;
              mnemo = Preparer_envoi_mnemonique ( result );
              g_free(result);
              if (!mnemo)
               { struct CMD_GTK_MESSAGE erreur;
               
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_ADD_MNEMONIQUE_OK,
                                   (gchar *)mnemo, sizeof(struct CMD_TYPE_MNEMONIQUE) );
                     g_free(mnemo);
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Proto_envoyer_type_num_mnemonique: Recherche et envoi du mnemonique de type et num en parametre        */
/* Entrée: Le client demandeur et la structure de recherche                                               */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void Proto_envoyer_type_num_mnemo_tag( int tag, int ss_tag, struct CLIENT *client,
                                        struct CMD_TYPE_NUM_MNEMONIQUE *critere )
  { struct MNEMONIQUEDB *mnemo;
    struct CMD_TYPE_MNEMONIQUE *result;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    mnemo = Rechercher_mnemoDB_type_num( Config.log, Db_watchdog, critere );
    if (mnemo)
     { result = Preparer_envoi_mnemonique( mnemo );
       g_free(mnemo);
       if (result)
        { Envoi_client ( client, tag, ss_tag,
                         (gchar *)result, sizeof(struct CMD_TYPE_MNEMONIQUE) );
          g_free(result);
        }       
     }
    else
     { struct CMD_TYPE_MNEMONIQUE unconnu;
       unconnu.id = 0;
       unconnu.type = critere->type;
       unconnu.num = critere->num;
       g_snprintf( unconnu.objet, sizeof(unconnu.objet), "Unknown" );
       g_snprintf( unconnu.libelle, sizeof(unconnu.libelle), "Unknown" );
       Envoi_client ( client, tag, ss_tag,
                      (gchar *)&unconnu, sizeof(struct CMD_TYPE_MNEMONIQUE) );
     }
  }
/**********************************************************************************************************/
/* Envoyer_mnemoniques: Envoi des mnemos au client GID_MNEMONIQUE                                         */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoyer_mnemoniques_tag ( struct CLIENT *client, guint tag, gint sstag, gint sstag_fin )
  { struct CMD_TYPE_MNEMONIQUE *rezo_mnemonique;
    struct CMD_ENREG nbr;
    struct MNEMONIQUEDB *mnemo;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiMnemo", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       return;
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_mnemoDB( Config.log, db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Libere_DB_SQL( Config.log, &db );
       return;
     }

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d mnemos", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { mnemo = Recuperer_mnemoDB_suite( Config.log, db );
       if (!mnemo)
        { Libere_DB_SQL( Config.log, &db );
          Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          return;
        }

       rezo_mnemonique = Preparer_envoi_mnemonique( mnemo );
       g_free(mnemo);
       if (rezo_mnemonique)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Envoi_client ( client, tag, sstag,
                         (gchar *)rezo_mnemonique, sizeof(struct CMD_TYPE_MNEMONIQUE) );
          g_free(rezo_mnemonique);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_mnemoniques: Envoi des mnemos au client GID_MNEMONIQUE                                         */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoyer_mnemoniques_for_courbe_tag ( struct CLIENT *client, guint tag, gint sstag, gint sstag_fin )
  { struct CMD_TYPE_MNEMONIQUE *rezo_mnemonique;
    struct CMD_ENREG nbr;
    struct MNEMONIQUEDB *mnemo;
    struct DB *db;

    prctl(PR_SET_NAME, "W-MnemoCourbe", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       return;
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_mnemoDB_for_courbe( Config.log, db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Libere_DB_SQL( Config.log, &db );
       return;
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d mnemos", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { mnemo = Recuperer_mnemoDB_for_courbe_suite( Config.log, db );
       if (!mnemo)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          return;
        }

       rezo_mnemonique = Preparer_envoi_mnemonique( mnemo );
       g_free(mnemo);
       if (rezo_mnemonique)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Envoi_client ( client, tag, sstag,
                         (gchar *)rezo_mnemonique, sizeof(struct CMD_TYPE_MNEMONIQUE) );
          g_free(rezo_mnemonique);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_mnemoniques: Envoi des mnemos au client GID_MNEMONIQUE                                         */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_mnemoniques_thread ( struct CLIENT *client )
  { Envoyer_mnemoniques_tag( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE,
                                                     SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE_FIN );   
    pthread_exit ( NULL );
  }
/**********************************************************************************************************/
/* Envoyer_mnemoniques: Envoi des mnemos au client GID_MNEMONIQUE                                         */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_mnemoniques_for_courbe_thread ( struct CLIENT *client )
  { Envoyer_mnemoniques_for_courbe_tag( client, TAG_COURBE, SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_COURBE,
                                                            SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_COURBE_FIN );
    pthread_exit ( NULL );
  }
/**********************************************************************************************************/
/* Envoyer_mnemoniques: Envoi des mnemos au client GID_MNEMONIQUE                                         */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_mnemoniques_for_histo_courbe_thread ( struct CLIENT *client )
  { Envoyer_mnemoniques_for_courbe_tag( client, TAG_HISTO_COURBE,
                                        SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_HISTO_COURBE,
                                        SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_HISTO_COURBE_FIN );
    pthread_exit ( NULL );
  }
/*--------------------------------------------------------------------------------------------------------*/
