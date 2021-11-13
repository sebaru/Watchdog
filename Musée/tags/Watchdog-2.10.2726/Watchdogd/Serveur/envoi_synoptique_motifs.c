/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_motifs.c        Envoi des motifs à l'atelier et supervision         */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 22 mai 2005 17:25:01 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique_motifs.c
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
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                         */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_motif_atelier ( struct CLIENT *client, struct CMD_TYPE_MOTIF *rezo_motif )
  { gboolean retour;

    retour = Retirer_motifDB( rezo_motif );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_MOTIF_OK,
                     (gchar *)rezo_motif, sizeof(struct CMD_TYPE_MOTIF) );
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
 void Proto_ajouter_motif_atelier ( struct CLIENT *client, struct CMD_TYPE_MOTIF *rezo_motif )
  { struct CMD_TYPE_MOTIF *result;
    gint id;

    rezo_motif->gid = GID_TOUTLEMONDE;               /* Par défaut, tout le monde peut acceder a ce motif */
    id = Ajouter_motifDB ( rezo_motif );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add motif %s", rezo_motif->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_motifDB( id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate motif %s", rezo_motif->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_MOTIF_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_MOTIF) );
              g_free(result);
            }
         }
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_motif_atelier ( struct CLIENT *client, struct CMD_TYPE_MOTIF *rezo_motif )
  { gboolean retour;

    retour = Modifier_motifDB ( rezo_motif );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save motif %s", rezo_motif->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_motif_atelier_thread ( struct CLIENT *client )
  { struct CMD_TYPE_MOTIFS *motifs;
    struct CMD_TYPE_MOTIF *motif;
    struct CMD_ENREG nbr;
    struct DB *db;
    gint max_enreg;                                /* Nombre maximum d'enregistrement dans un bloc reseau */
    gchar titre[20];
    g_snprintf( titre, sizeof(titre), "W-MOTI-%06d", client->ssrv_id );
    prctl(PR_SET_NAME, titre, 0, 0, 0 );

    max_enreg = (Cfg_ssrv.taille_bloc_reseau - sizeof(struct CMD_TYPE_MOTIFS)) / sizeof(struct CMD_TYPE_MOTIF);
    motifs = (struct CMD_TYPE_MOTIFS *)g_try_malloc0( Cfg_ssrv.taille_bloc_reseau );    
    if (!motifs)
     { struct CMD_GTK_MESSAGE erreur;
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                "Envoyer_motif_atelier_thread: Pb d'allocation memoire motifs" );
       g_snprintf( erreur.message, sizeof(erreur.message), "Pb d'allocation memoire" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }

    if ( ! Recuperer_motifDB( &db, client->syn.id ) )
     { Client_mode( client, ENVOI_COMMENT_ATELIER );
       g_free(motifs);
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d motifs", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    motifs->nbr_motifs = 0;                                 /* Valeurs par defaut si pas d'enregistrement */

    do
     { motif = Recuperer_motifDB_suite( &db );                        /* Récupération du motif dans la DB */
       if (motif)                                              /* Si enregegistrement, alors on le pousse */
        { memcpy ( &motifs->motif[motifs->nbr_motifs], motif, sizeof(struct CMD_TYPE_MOTIF) );
          motifs->nbr_motifs++;          /* Nous avons 1 enregistrement de plus dans la structure d'envoi */
          g_free(motif);
        }

       if ( motif && (! g_list_find(client->bit_init_syn, GINT_TO_POINTER(motif->bit_controle) ) ) &&
            motif->type_gestion != 0 /* TYPE_INERTE */
          )
        { client->bit_init_syn = g_list_append( client->bit_init_syn, GINT_TO_POINTER(motif->bit_controle) );
                 Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                          "liste des bit_init_syn %d", motif->bit_controle );
        }

       if ( (motif == NULL) || motifs->nbr_motifs == max_enreg )/* Si depassement de tampon ou plus d'enreg */
        { Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF,
                        (gchar *)motifs,
                         sizeof(struct CMD_TYPE_MOTIFS) + motifs->nbr_motifs * sizeof(struct CMD_TYPE_MOTIF)
                       );
          motifs->nbr_motifs = 0;
        }
     }
    while (motif);                                            /* Tant que l'on a des messages e envoyer ! */
    g_free(motifs);

    Client_mode( client, ENVOI_COMMENT_ATELIER );
    Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_MOTIF_FIN, NULL, 0 );
    Unref_client( client );                                     /* Déréférence la structure cliente */
    pthread_exit ( NULL ); 
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_motif_supervision_thread ( struct CLIENT *client )
  { struct CMD_TYPE_MOTIFS *motifs;
    struct CMD_TYPE_MOTIF *motif;
    struct CMD_ENREG nbr;
    struct DB *db;
    gint max_enreg;                                /* Nombre maximum d'enregistrement dans un bloc reseau */

    prctl(PR_SET_NAME, "W-EnvoiMotif", 0, 0, 0 );

    printf("1 - Recherche supervision %d\n", client->syn.id);
    if (client->bit_init_syn)
     { g_list_free( client->bit_init_syn );
       client->bit_init_syn = NULL;
     }
    printf("2 - Recherche supervision %d\n", client->syn.id);

    max_enreg = (Cfg_ssrv.taille_bloc_reseau - sizeof(struct CMD_TYPE_MOTIFS)) / sizeof(struct CMD_TYPE_MOTIF);
    motifs = (struct CMD_TYPE_MOTIFS *)g_try_malloc0( Cfg_ssrv.taille_bloc_reseau );    
    if (!motifs)
     { struct CMD_GTK_MESSAGE erreur;
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                "Envoyer_motif_supervision_thread: Pb d'allocation memoire motifs" );
       g_snprintf( erreur.message, sizeof(erreur.message), "Pb d'allocation memoire" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Libere_DB_SQL( &db );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }

    if ( ! Recuperer_motifDB( &db, client->syn.id ) )
     { Client_mode( client, ENVOI_COMMENT_SUPERVISION );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d motifs", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    motifs->nbr_motifs = 0;                                 /* Valeurs par defaut si pas d'enregistrement */

    do
     { motif = Recuperer_motifDB_suite( &db );                        /* Récupération du motif dans la DB */
       if (motif)                                              /* Si enregegistrement, alors on le pousse */
        { memcpy ( &motifs->motif[motifs->nbr_motifs], motif, sizeof(struct CMD_TYPE_MOTIF) );
          motifs->nbr_motifs++;          /* Nous avons 1 enregistrement de plus dans la structure d'envoi */
          g_free(motif);
        }

       if ( motif && (! g_list_find(client->bit_init_syn, GINT_TO_POINTER(motif->bit_controle) ) ) &&
            motif->type_gestion != 0 /* TYPE_INERTE */
          )
        { client->bit_init_syn = g_list_append( client->bit_init_syn, GINT_TO_POINTER(motif->bit_controle) );
          Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                   "liste des bit_init_syn %d", motif->bit_controle );
        }

       if ( (motif == NULL) || motifs->nbr_motifs == max_enreg )/* Si depassement de tampon ou plus d'enreg */
        { Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_MOTIF,
                        (gchar *)motifs,
                         sizeof(struct CMD_TYPE_MOTIFS) + motifs->nbr_motifs * sizeof(struct CMD_TYPE_MOTIF)
                       );
          motifs->nbr_motifs = 0;
        }
     }
    while (motif);                                            /* Tant que l'on a des messages e envoyer ! */
    g_free(motifs);

    Client_mode( client, ENVOI_COMMENT_SUPERVISION );
    Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_MOTIF_FIN, NULL, 0 );
    Unref_client( client );                                     /* Déréférence la structure cliente */
    pthread_exit( NULL );
  }
/*--------------------------------------------------------------------------------------------------------*/
