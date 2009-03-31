/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_entreeana.c    Gestion du protocole_entreeana pour Watchdog                */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include "Erreur.h"
 #include "Reseaux.h"
 #include "Client.h"
 #include "EntreeANA_DB.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_entreeana( gint Id_serveur, struct CLIENT *client )
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
     { case SSTAG_CLIENT_WANT_PAGE_ENTREEANA:
             { Client_mode( client, ENVOI_ENTREEANA );
             }
            break;
       case SSTAG_CLIENT_EDIT_ENTREEANA:
             { struct CMD_ID_ENTREEANA *entree;
               entree = (struct CMD_ID_ENTREEANA *)connexion->donnees;
               Proto_editer_entreeANA( client, entree );
             }
            break;
       case SSTAG_CLIENT_ADD_ENTREEANA:
             { struct CMD_ADD_ENTREEANA *entree;
               entree = (struct CMD_ADD_ENTREEANA *)connexion->donnees;
               Proto_ajouter_entreeANA( client, entree );
             }
            break;
       case SSTAG_CLIENT_DEL_ENTREEANA:
             { struct CMD_ID_ENTREEANA *entree;
               entree = (struct CMD_ID_ENTREEANA *)connexion->donnees;
               Proto_effacer_entreeANA( client, entree );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_ENTREEANA:
             { struct CMD_EDIT_ENTREEANA *entree;
               entree = (struct CMD_EDIT_ENTREEANA *)connexion->donnees;
               Proto_valider_editer_entreeANA( client, entree );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
