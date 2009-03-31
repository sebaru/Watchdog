/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_passerelles.c        Envoi des passerelles aux clients              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 22 mai 2005 17:45:31 CEST */
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
/* Preparer_envoi_passerelle: convertit une structure PASSERELLEDB en structure CMD_SHOW_PASSERELLE       */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_PASSERELLE *Preparer_envoi_passerelle ( struct PASSERELLEDB *pass )
  { struct CMD_SHOW_PASSERELLE *rezo_pass;

    rezo_pass = (struct CMD_SHOW_PASSERELLE *)g_malloc0( sizeof(struct CMD_SHOW_PASSERELLE) );
    if (!rezo_pass) { return(NULL); }

    rezo_pass->id = pass->id;
    rezo_pass->syn_id = pass->syn_id;
    rezo_pass->syn_cible_id = pass->syn_cible_id;
    rezo_pass->bit_controle = pass->bit_controle;                                           /* Ixxx, Cxxx */
    rezo_pass->bit_controle_1 = pass->bit_controle_1;                                       /* Ixxx, Cxxx */
    rezo_pass->bit_controle_2 = pass->bit_controle_2;                                       /* Ixxx, Cxxx */
    rezo_pass->position_x = pass->position_x;                                /* en abscisses et ordonnées */
    rezo_pass->position_y = pass->position_y;
    memcpy( &rezo_pass->libelle, pass->libelle, sizeof(rezo_pass->libelle) );
    return( rezo_pass );
  }
/**********************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                         */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_passerelle_atelier ( struct CLIENT *client, struct CMD_ID_PASSERELLE *rezo_pass )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
    Info( Config.log, DEBUG_INFO, "MSRV: demande d'effacement pass" );
    retour = Retirer_passerelleDB( Config.log, Db_watchdog, rezo_pass );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_PASS_OK,
                     (gchar *)rezo_pass, sizeof(struct CMD_ID_PASSERELLE) );
       Info( Config.log, DEBUG_INFO, "MSRV: effacement pass OK" );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to delete pass %s:\n%s"), rezo_pass->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Info( Config.log, DEBUG_INFO, "MSRV: effacement pass NOK" );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                               */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_passerelle_atelier ( struct CLIENT *client, struct CMD_ADD_PASSERELLE *rezo_pass )
  { struct PASSERELLEDB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    Info( Config.log, DEBUG_INFO, "MSRV: demande d'ajout passerelle" );
    id = Ajouter_passerelleDB ( Config.log, Db_watchdog, rezo_pass );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to add pass %s:\n%s"), rezo_pass->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Info( Config.log, DEBUG_INFO, "MSRV: ajout pass NOK" );
     }
    else { result = Rechercher_passerelleDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate pass %s:\n%s"), rezo_pass->libelle, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
              Info( Config.log, DEBUG_INFO, "MSRV: ajout pass NOK (2)" );
            }
           else
            { struct CMD_SHOW_PASSERELLE *pass;
              pass = Preparer_envoi_passerelle( result );
              g_free(result);
              if (!pass)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             _("Not enough memory") );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
                 Info( Config.log, DEBUG_INFO, "MSRV: ajout pass NOK (3)" );
               }
              else { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_PASS_OK,
                                   (gchar *)pass, sizeof(struct CMD_SHOW_PASSERELLE) );
                     g_free(pass);
                     Info( Config.log, DEBUG_INFO, "MSRV: ajout pass OK" );
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_passerelle_atelier ( struct CLIENT *client, struct CMD_EDIT_PASSERELLE *rezo_pass )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
Info( Config.log, DEBUG_INFO, "Debut valider_editer_passerelle_atelier" );

    retour = Modifier_passerelleDB ( Config.log, Db_watchdog, rezo_pass );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to save pass %s:\n%s"), rezo_pass->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
Info( Config.log, DEBUG_INFO, "fin valider_editer_passerelle_atelier" );
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_passerelle_atelier_thread ( struct CLIENT *client )
  { struct CMD_SHOW_PASSERELLE *rezo_pass;
    struct CMD_ENREG nbr;
    struct PASSERELLEDB *pass;
    struct DB *Db_watchdog;
    SQLHSTMT hquery;
    Db_watchdog = client->Db_watchdog;

    hquery = Recuperer_passerelleDB( Config.log, Db_watchdog, client->syn.id );
    if (!hquery) { Client_mode( client, ENVOI_PALETTE_ATELIER );                /* Si pas de comments ... */
                   Unref_client( client );                            /* Déréférence la structure cliente */
                   pthread_exit ( NULL );
                 }

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading gateways") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { pass = Recuperer_passerelleDB_suite( Config.log, Db_watchdog, hquery );
       if (!pass)
        { Client_mode( client, ENVOI_PALETTE_ATELIER );               /* Si pas de comments ... */
          Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       rezo_pass = Preparer_envoi_passerelle( pass );
       g_free(pass);
       if (rezo_pass)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
          Info_c( Config.log, DEBUG_INFO, "THR Envoyer_passerelle_atelier: pass LIB", rezo_pass->libelle );
          Info_n( Config.log, DEBUG_INFO, "THR Envoyer_passerelle_atelier: pass ID ", rezo_pass->id );
          Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS,
                         (gchar *)rezo_pass, sizeof(struct CMD_SHOW_PASSERELLE) );
          g_free(rezo_pass);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_passerelle_supervision_thread ( struct CLIENT *client )
  { struct CMD_SHOW_PASSERELLE *rezo_pass;
    struct CMD_ENREG nbr;
    struct PASSERELLEDB *pass;
    struct DB *Db_watchdog;
    SQLHSTMT hquery;
    Db_watchdog = client->Db_watchdog;

    hquery = Recuperer_passerelleDB( Config.log, Db_watchdog, client->num_supervision );
    if (!hquery) { Client_mode( client, ENVOI_PALETTE_SUPERVISION );            /* Si pas de comments ... */
                   Unref_client( client );                            /* Déréférence la structure cliente */
                   pthread_exit ( NULL );
                 }

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading gateways") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { pass = Recuperer_passerelleDB_suite( Config.log, Db_watchdog, hquery );
       if (!pass)                                                                           /* Terminé ?? */
        { Client_mode( client, ENVOI_PALETTE_SUPERVISION );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PASS_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit( NULL );
        }

       if ( ! g_list_find(client->bit_init_syn, GINT_TO_POINTER(pass->bit_controle_1) )
          )
        { client->bit_init_syn = g_list_append( client->bit_init_syn, GINT_TO_POINTER(pass->bit_controle_1) );
          Info_n( Config.log, DEBUG_INFO , "  liste des bit_init_syn pass", pass->bit_controle_1 );
        }

       if ( ! g_list_find(client->bit_init_syn, GINT_TO_POINTER(pass->bit_controle_2) )
          )
        { client->bit_init_syn = g_list_append( client->bit_init_syn, GINT_TO_POINTER(pass->bit_controle_2) );
          Info_n( Config.log, DEBUG_INFO , "  liste des bit_init_syn pass", pass->bit_controle_2 );
        }

       rezo_pass = Preparer_envoi_passerelle( pass );
       g_free(pass);
       if (rezo_pass)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Info_c( Config.log, DEBUG_INFO, "THR Envoyer_pass_supervision: pass LIB", rezo_pass->libelle );
          Info_n( Config.log, DEBUG_INFO, "THR Envoyer_pass_supervision: pass ID ", rezo_pass->id );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PASS,
                         (gchar *)rezo_pass, sizeof(struct CMD_SHOW_PASSERELLE) );
          g_free(rezo_pass);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
