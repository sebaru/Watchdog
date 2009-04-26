/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_motifs.c        Envoi des motifs à l'atelier et supervision         */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 22 mai 2005 17:25:01 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique_motifs.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - 
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
/* Preparer_envoi_motif: convertit une structure MOTIFDB en structure CMD_SHOW_MOTIF                      */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_MOTIF *Preparer_envoi_motif ( struct MOTIFDB *motif )
  { struct CMD_SHOW_MOTIF *rezo_motif;

    rezo_motif = (struct CMD_SHOW_MOTIF *)g_malloc0( sizeof(struct CMD_SHOW_MOTIF) );
    if (!rezo_motif) { return(NULL); }

    rezo_motif->id = motif->id;
    rezo_motif->icone_id = motif->icone_id;                                 /* Correspond au fichier .gif */
    rezo_motif->syn_id = motif->syn_id;
    rezo_motif->gid = motif->gid;                                /* Nom du groupe d'appartenance du motif */
    rezo_motif->bit_controle = motif->bit_controle;                                         /* Ixxx, Cxxx */
    rezo_motif->bit_clic = motif->bit_clic;   /* Bit à activer quand on clic avec le bouton gauche souris */
    rezo_motif->bit_clic2 = motif->bit_clic2; /* Bit à activer quand on clic avec le bouton gauche souris */
    rezo_motif->position_x = motif->position_x;                              /* en abscisses et ordonnées */
    rezo_motif->position_y = motif->position_y;
    rezo_motif->largeur = motif->largeur;                          /* Taille de l'image sur le synoptique */
    rezo_motif->hauteur = motif->hauteur;
    rezo_motif->angle = motif->angle;
    rezo_motif->type_dialog = motif->type_dialog;/* Type de la boite de dialogue pour le clic de commande */
    rezo_motif->rouge0 = motif->rouge0;
    rezo_motif->vert0 = motif->vert0;
    rezo_motif->bleu0 = motif->bleu0;
    rezo_motif->type_gestion = motif->type_gestion;                        /* Statique/dynamique/cyclique */
    memcpy( &rezo_motif->libelle, motif->libelle, sizeof(rezo_motif->libelle) );
    return( rezo_motif );
  }
/**********************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                         */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_motif_atelier ( struct CLIENT *client, struct CMD_ID_MOTIF *rezo_motif )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_motifDB( Config.log, Db_watchdog, rezo_motif );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_MOTIF_OK,
                     (gchar *)rezo_motif, sizeof(struct CMD_ID_MOTIF) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete motif %s", rezo_motif->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_motif_atelier: Ajout d'un motif dans un synoptique                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_motif_atelier ( struct CLIENT *client, struct CMD_ADD_MOTIF *rezo_motif )
  { struct MOTIFDB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    rezo_motif->gid = GID_TOUTLEMONDE;               /* Par défaut, tout le monde peut acceder a ce motif */
    id = Ajouter_motifDB ( Config.log, Db_watchdog, rezo_motif );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add motif %s", rezo_motif->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_motifDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate motif %s", rezo_motif->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_SHOW_MOTIF *motif;
              motif = Preparer_envoi_motif( result );
              g_free(result);
              if (!motif)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_MOTIF_OK,
                                   (gchar *)motif, sizeof(struct CMD_SHOW_MOTIF) );
                     g_free(motif);
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_motif_atelier ( struct CLIENT *client, struct CMD_EDIT_MOTIF *rezo_motif )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
Info( Config.log, DEBUG_INFO, "Debut valider_editer_motif_atelier" );

    retour = Modifier_motifDB ( Config.log, Db_watchdog, rezo_motif );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save motif %s", rezo_motif->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
Info( Config.log, DEBUG_INFO, "fin valider_editer_motif_atelier" );
  }

/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_motif_atelier_thread ( struct CLIENT *client )
  { struct CMD_SHOW_MOTIF *rezo_motif;
    struct CMD_ENREG nbr;
    struct MOTIFDB *motif;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiMotif", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_motifDB( Config.log, db, client->syn.id ) )
     { Client_mode( client, ENVOI_COMMENT_ATELIER );
       Libere_DB_SQL( Config.log, &db );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d motifs", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { motif = Recuperer_motifDB_suite( Config.log, db );
       if (!motif)
        { Libere_DB_SQL( Config.log, &db );
          Client_mode( client, ENVOI_COMMENT_ATELIER );
          Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL ); 
        }

       rezo_motif = Preparer_envoi_motif( motif );
       g_free(motif);
       if (rezo_motif)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Info_c( Config.log, DEBUG_INFO, "THR Envoyer_motif_atelier: motif LIB", rezo_motif->libelle );
          Info_n( Config.log, DEBUG_INFO, "THR Envoyer_motif_atelier: motif ID ", rezo_motif->id );
          Info_n( Config.log, DEBUG_INFO, "THR Envoyer_motif_atelier: motif larg ", rezo_motif->largeur );
          Info_n( Config.log, DEBUG_INFO, "THR Envoyer_motif_atelier: motif haut ", rezo_motif->hauteur );

          Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF,
                         (gchar *)rezo_motif, sizeof(struct CMD_SHOW_MOTIF) );
          g_free(rezo_motif);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_motif_supervision_thread ( struct CLIENT *client )
  { struct CMD_SHOW_MOTIF *rezo_motif;
    struct CMD_ENREG nbr;
    struct MOTIFDB *motif;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiMotif", 0, 0, 0 );

    printf("1 - Recherche supervision %d\n", client->num_supervision);
    if (client->bit_init_syn)
     { g_list_free( client->bit_init_syn );
       client->bit_init_syn = NULL;
     }
    printf("2 - Recherche supervision %d\n", client->num_supervision);

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_motifDB( Config.log, db, client->num_supervision ) )
     { Client_mode( client, ENVOI_COMMENT_SUPERVISION );
       Libere_DB_SQL( Config.log, &db );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d motifs", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { motif = Recuperer_motifDB_suite( Config.log, db );
       if (!motif)                                                                          /* Terminé ?? */
        { Libere_DB_SQL( Config.log, &db );
          Client_mode( client, ENVOI_COMMENT_SUPERVISION );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_MOTIF_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit( NULL );
        }
       if ( ! g_list_find(client->bit_init_syn, GINT_TO_POINTER(motif->bit_controle) ) &&
            motif->type_gestion != 0 /* TYPE_INERTE */
          )
        { client->bit_init_syn = g_list_append( client->bit_init_syn, GINT_TO_POINTER(motif->bit_controle) );
          Info_n( Config.log, DEBUG_INFO , "  liste des bit_init_syn ", motif->bit_controle );
        }
       rezo_motif = Preparer_envoi_motif( motif );
       g_free(motif);
       if (rezo_motif)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
          Info_c( Config.log, DEBUG_INFO, "THR Envoyer_motif_supervision: motif LIB", rezo_motif->libelle );
          Info_n( Config.log, DEBUG_INFO, "THR Envoyer_motif_supervision: motif ID ", rezo_motif->id );

          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_MOTIF,
                         (gchar *)rezo_motif, sizeof(struct CMD_SHOW_MOTIF) );
          g_free(rezo_motif);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
