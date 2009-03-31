/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_mnemonique.c    Gestion du protocole_mnemonique pour Watchdog              */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include "Erreur.h"
 #include "Reseaux.h"
 #include "Client.h"
 #include "Mnemonique_DB.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_mnemonique( gint Id_serveur, struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util->id, client->util->gids, GID_DLS) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Non Autorisé..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_MNEMONIQUE:
             { Client_mode( client, ENVOI_MNEMONIQUE );
             }
            break;
       case SSTAG_CLIENT_EDIT_MNEMONIQUE:
             { struct CMD_ID_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_ID_MNEMONIQUE *)connexion->donnees;
               Proto_editer_mnemonique( client, mnemo );
             }
            break;
       case SSTAG_CLIENT_ADD_MNEMONIQUE:
             { struct CMD_ADD_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_ADD_MNEMONIQUE *)connexion->donnees;
               Proto_ajouter_mnemonique( client, mnemo );
             }
            break;
       case SSTAG_CLIENT_DEL_MNEMONIQUE:
             { struct CMD_ID_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_ID_MNEMONIQUE *)connexion->donnees;
               Proto_effacer_mnemonique( client, mnemo );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_MNEMONIQUE:
             { struct CMD_EDIT_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_EDIT_MNEMONIQUE *)connexion->donnees;
               Proto_valider_editer_mnemonique( client, mnemo );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
