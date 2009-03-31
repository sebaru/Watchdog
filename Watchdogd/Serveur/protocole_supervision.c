/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_supervision.c    Gestion du protocole_supervision pour Watchdog            */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include "Erreur.h"
 #include "Reseaux.h"
 #include "Client.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_supervision( gint Id_serveur, struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util->id, client->util->gids, GID_SYNOPTIQUE) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Non Autorisé..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_SUPERVISION:
             { struct CMD_ID_SYNOPTIQUE *syn;
               syn = (struct CMD_ID_SYNOPTIQUE *)connexion->donnees;
               printf("Le client desire le synoptique de supervision\n" );
               client->num_supervision = syn->id;             /* Sauvegarde du syn voulu pour envoi motif */
               Client_mode( client, ENVOI_MOTIF_SUPERVISION );
             }
            break;
       case SSTAG_CLIENT_CHANGE_MOTIF_UNKNOWN:
             { struct CMD_ETAT_BIT_CTRL *etat;
               etat = (struct CMD_ETAT_BIT_CTRL *)connexion->donnees;
               printf("Le client n'a plus besoin du bit %d\n", etat->num );
               client->bit_syns = g_list_remove ( client->bit_syns, GINT_TO_POINTER( etat->num ) );
             }
            break;
       case SSTAG_CLIENT_ACTION_M:
             { struct CMD_ETAT_BIT_CLIC *bit;
               bit = (struct CMD_ETAT_BIT_CLIC *)connexion->donnees;
               Envoyer_commande_dls( bit->num );
             }
            break;
       case SSTAG_CLIENT_SUP_WANT_SCENARIO:
             { struct CMD_WANT_SCENARIO_MOTIF *sce;
               sce = (struct CMD_WANT_SCENARIO_MOTIF *)connexion->donnees;
               printf("Envoi scenario bitm %d %d\n", sce->bit_clic, sce->bit_clic2 );
               memcpy ( &client->sce, sce, sizeof( struct CMD_WANT_SCENARIO_MOTIF ) );
               Client_mode( client, ENVOI_SCENARIO_SUP );
             }
            break;
       case SSTAG_CLIENT_SUP_EDIT_SCENARIO:
             { struct CMD_ID_SCENARIO *sce;
               sce = (struct CMD_ID_SCENARIO *)connexion->donnees;
               Proto_editer_scenario_sup( client, sce );
             }
            break;
       case SSTAG_CLIENT_SUP_ADD_SCENARIO:
             { struct CMD_ADD_SCENARIO *sce;
               sce = (struct CMD_ADD_SCENARIO *)connexion->donnees;
               Proto_ajouter_scenario_sup( client, sce );
             }
            break;
       case SSTAG_CLIENT_SUP_DEL_SCENARIO:
             { struct CMD_ID_SCENARIO *sce;
               sce = (struct CMD_ID_SCENARIO *)connexion->donnees;
               Proto_effacer_scenario_sup( client, sce );
             }
            break;
       case SSTAG_CLIENT_SUP_VALIDE_EDIT_SCENARIO:
             { struct CMD_EDIT_SCENARIO *sce;
               sce = (struct CMD_EDIT_SCENARIO *)connexion->donnees;
               Proto_valider_editer_scenario_sup( client, sce );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
