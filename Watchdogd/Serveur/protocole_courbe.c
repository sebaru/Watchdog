/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_courbe.c    Gestion du protocole_courbe pour Watchdog                      */
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
 void Gerer_protocole_courbe( gint Id_serveur, struct CLIENT *client )
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
     { case SSTAG_CLIENT_WANT_PAGE_SOURCE_FOR_COURBE:
             { Client_mode( client, ENVOI_ENTREEANA_FOR_COURBE );
             }
            break;
       case SSTAG_CLIENT_ADD_COURBE:
             { while (client->courbe.id != -1) { printf("attends\n"); sched_yield(); }
               memcpy( &client->courbe, connexion->donnees, sizeof(struct CMD_ID_COURBE) );
               pthread_create( &tid, NULL, (void *)Proto_ajouter_courbe_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_DEL_COURBE:
             { struct CMD_ID_COURBE *courbe;
               courbe = (struct CMD_ID_COURBE *)connexion->donnees;
               Proto_effacer_courbe( client, courbe );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
