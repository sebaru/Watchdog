/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_motifs.c        Envoi des motifs à l'atelier et supervision         */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 22 mai 2005 17:25:01 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

 #include "Reseaux.h"
 #include "Synoptiques_DB.h"
 #include "Erreur.h"
 #include "Config.h"
 #include "Client.h"

 #include "watchdogd.h"
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

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
                   _("Unable to delete motif %s:\n%s"), rezo_motif->libelle, Db_watchdog->last_err);
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

    id = Ajouter_motifDB ( Config.log, Db_watchdog, rezo_motif );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to add motif %s:\n%s"), rezo_motif->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_motifDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate motif %s:\n%s"), rezo_motif->libelle, Db_watchdog->last_err);
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
                             _("Not enough memory") );
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
                   _("Unable to save motif %s:\n%s"), rezo_motif->libelle, Db_watchdog->last_err);
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
    struct DB *Db_watchdog;
    SQLHSTMT hquery;
    Db_watchdog = client->Db_watchdog;

    hquery = Recuperer_motifDB( Config.log, Db_watchdog, client->syn.id );
    if (!hquery) { Client_mode( client, ENVOI_COMMENT_ATELIER );
                   Unref_client( client );                            /* Déréférence la structure cliente */
                   pthread_exit ( NULL ); }                                      /* Si pas de histos (??) */

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading synoptique") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { motif = Recuperer_motifDB_suite( Config.log, Db_watchdog, hquery );
       if (!motif)
        { Client_mode( client, ENVOI_COMMENT_ATELIER );
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
    struct DB *Db_watchdog;
    SQLHSTMT hquery;
    Db_watchdog = client->Db_watchdog;

    printf("1 - Recherche supervision %d\n", client->num_supervision);
    if (client->bit_init_syn)
     { g_list_free( client->bit_init_syn );
       client->bit_init_syn = NULL;
     }
    printf("2 - Recherche supervision %d\n", client->num_supervision);

    hquery = Recuperer_motifDB( Config.log, Db_watchdog, client->num_supervision );
    if (!hquery) { Client_mode( client, ENVOI_COMMENT_SUPERVISION );              /* Si pas de motifs ... */
                   Unref_client( client );                            /* Déréférence la structure cliente */
                   pthread_exit ( NULL );
                 }

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading synoptique") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { motif = Recuperer_motifDB_suite( Config.log, Db_watchdog, hquery );
       if (!motif)                                                                          /* Terminé ?? */
        { Client_mode( client, ENVOI_COMMENT_SUPERVISION );
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
