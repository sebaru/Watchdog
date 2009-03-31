/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_histo.c    Gestion du protocole_histo pour Watchdog            */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include "Erreur.h"
 #include "Reseaux.h"
 #include "Client.h"

/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_histo( gint Id_serveur, struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util->id, client->util->gids, GID_HISTO) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Non Autorisé..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_REQUETE_HISTO_HARD:
             { memcpy( &client->requete, (struct CMD_REQUETE_HISTO_HARD *)connexion->donnees,
                       sizeof( client->requete ) );
               pthread_create( &tid, NULL, (void *)Proto_envoyer_histo_hard_thread, client );
               pthread_detach( tid );
               Client_mode( client, VALIDE );
             }
            break;
       case SSTAG_CLIENT_ACK_HISTO:
             { struct CMD_ID_HISTO *histo;
               histo = (struct CMD_ID_HISTO *)connexion->donnees;
               Proto_acquitter_histo( client, histo );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
