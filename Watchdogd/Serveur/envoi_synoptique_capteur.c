/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_capteur.c     Envoi des capteurs aux clients                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 22 mai 2005 17:35:28 CEST */
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
/* Preparer_envoi_capteur: convertit une structure CAPTEURDB en structure CMD_SHOW_CAPTEUR                */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_CAPTEUR *Preparer_envoi_capteur ( struct CAPTEURDB *capteur )
  { struct CMD_SHOW_CAPTEUR *rezo_capteur;

    rezo_capteur = (struct CMD_SHOW_CAPTEUR *)g_malloc0( sizeof(struct CMD_SHOW_CAPTEUR) );
    if (!rezo_capteur) { return(NULL); }

    rezo_capteur->id           = capteur->id;
    rezo_capteur->syn_id       = capteur->syn_id;
    rezo_capteur->bit_controle = capteur->bit_controle;
    rezo_capteur->type         = capteur->type;
    rezo_capteur->position_x   = capteur->position_x;                                      /* en abcisses */
    rezo_capteur->position_y   = capteur->position_y;                                     /* en ordonnées */
    memcpy( &rezo_capteur->libelle, capteur->libelle, sizeof(rezo_capteur->libelle) );
    return( rezo_capteur );
  }
/**********************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                         */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_capteur_atelier ( struct CLIENT *client, struct CMD_ID_CAPTEUR *rezo_capteur )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
    Info( Config.log, DEBUG_INFO, "MSRV: demande d'effacement capteur" );
    retour = Retirer_capteurDB( Config.log, Db_watchdog, rezo_capteur );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_CAPTEUR_OK,
                     (gchar *)rezo_capteur, sizeof(struct CMD_ID_CAPTEUR) );
       Info( Config.log, DEBUG_INFO, "MSRV: effacement capteur OK" );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to delete capteur %s:\n%s"), rezo_capteur->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Info( Config.log, DEBUG_INFO, "MSRV: effacement capteur NOK" );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                               */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_capteur_atelier ( struct CLIENT *client, struct CMD_ADD_CAPTEUR *rezo_capteur )
  { struct CAPTEURDB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    Info( Config.log, DEBUG_INFO, "MSRV: demande d'ajout capteur" );
    id = Ajouter_capteurDB ( Config.log, Db_watchdog, rezo_capteur );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to add capteur %s:\n%s"), rezo_capteur->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Info( Config.log, DEBUG_INFO, "MSRV: ajout capteur NOK" );
     }
    else { result = Rechercher_capteurDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate capteur %s:\n%s"), rezo_capteur->libelle, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
              Info( Config.log, DEBUG_INFO, "MSRV: ajout capteur NOK (2)" );
            }
           else
            { struct CMD_SHOW_CAPTEUR *capteur;
              capteur = Preparer_envoi_capteur( result );
              g_free(result);
              if (!capteur)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             _("Not enough memory") );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
                 Info( Config.log, DEBUG_INFO, "MSRV: ajout capteur NOK (3)" );
               }
              else { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_CAPTEUR_OK,
                                   (gchar *)capteur, sizeof(struct CMD_SHOW_CAPTEUR) );
                     g_free(capteur);
                     Info( Config.log, DEBUG_INFO, "MSRV: ajout capteur OK" );
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_capteur_atelier ( struct CLIENT *client, struct CMD_EDIT_CAPTEUR *rezo_capteur )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
Info( Config.log, DEBUG_INFO, "Debut valider_editer_capteur_atelier" );

    retour = Modifier_capteurDB ( Config.log, Db_watchdog, rezo_capteur );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to save capteur %s:\n%s"), rezo_capteur->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
Info( Config.log, DEBUG_INFO, "fin valider_editer_capteur_atelier" );
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_capteur_atelier_thread ( struct CLIENT *client )
  { struct CMD_SHOW_CAPTEUR *rezo_capteur;
    struct CMD_ENREG nbr;
    struct CAPTEURDB *capteur;
    struct DB *Db_watchdog;
    SQLHSTMT hquery;
    Db_watchdog = client->Db_watchdog;

    hquery = Recuperer_capteurDB( Config.log, Db_watchdog, client->syn.id );
    if (!hquery)
     {
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }                                                                           /* Si pas de histos (??) */

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading capteurs") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { capteur = Recuperer_capteurDB_suite( Config.log, Db_watchdog, hquery );
       if (!capteur)
        { Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAPTEUR_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       rezo_capteur = Preparer_envoi_capteur( capteur );
       g_free(capteur);
       if (rezo_capteur)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
          Info_c( Config.log, DEBUG_INFO, "THR Envoyer_capteur_atelier: pass LIB", rezo_capteur->libelle );
          Info_n( Config.log, DEBUG_INFO, "THR Envoyer_capteur_atelier: pass ID ", rezo_capteur->id );
          Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAPTEUR,
                         (gchar *)rezo_capteur, sizeof(struct CMD_SHOW_CAPTEUR) );
          g_free(rezo_capteur);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_capteur_supervision_thread ( struct CLIENT *client )
  { struct CMD_SHOW_CAPTEUR *rezo_capteur;
    struct CMD_ENREG nbr;
    struct CAPTEURDB *capteur;
    struct DB *Db_watchdog;
    SQLHSTMT hquery;
    Db_watchdog = client->Db_watchdog;

    hquery = Recuperer_capteurDB( Config.log, Db_watchdog, client->num_supervision );
    if (!hquery) { Client_mode( client, ENVOI_IXXX_SUPERVISION );               /* Si pas de comments ... */
                   Unref_client( client );                            /* Déréférence la structure cliente */
                   pthread_exit ( NULL );
                 }

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading capteurs") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { struct CAPTEUR *capteur_new;;
       capteur = Recuperer_capteurDB_suite( Config.log, Db_watchdog, hquery );
       if (!capteur)                                                                        /* Terminé ?? */
        { Client_mode( client, ENVOI_IXXX_SUPERVISION );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAPTEUR_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit( NULL );
        }

       capteur_new = (struct CAPTEUR *)g_malloc0( sizeof(struct CAPTEUR) );
       if (!capteur_new) continue;
       capteur_new->type = capteur->type;
       capteur_new->bit_controle = capteur->bit_controle;

       if ( ! g_list_find_custom(client->bit_init_capteur, capteur_new, (GCompareFunc) Chercher_bit_capteurs ) )
        { client->bit_init_capteur = g_list_append( client->bit_init_capteur, capteur_new );
          Info_n( Config.log, DEBUG_INFO , "  liste des bit_init_capteur ", capteur->id );
        }
       else g_free(capteur_new);

       rezo_capteur = Preparer_envoi_capteur( capteur );
       g_free(capteur);
       if (rezo_capteur)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Info_c( Config.log, DEBUG_INFO, "THR Envoyer_capteur_supervision: pass LIB", rezo_capteur->libelle );
          Info_n( Config.log, DEBUG_INFO, "THR Envoyer_capteur_supervision: pass ID ", rezo_capteur->id );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAPTEUR,
                         (gchar *)rezo_capteur, sizeof(struct CMD_SHOW_CAPTEUR) );
          g_free(rezo_capteur);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
