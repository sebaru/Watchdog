/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_utilisateur.c    Gestion du protocole_utilisateur pour Watchdog            */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include "Erreur.h"
 #include "Reseaux.h"
 #include "Client.h"
 #include "Utilisateur_DB.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_utilisateur( gint Id_serveur, struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util->id, client->util->gids, GID_USERS) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Non Autorisé..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_EDIT_GROUPE:
             { struct CMD_ID_GROUPE *groupe;
               groupe = (struct CMD_ID_GROUPE *)connexion->donnees;
               Proto_editer_groupe( client, groupe );
             }
            break;
       case SSTAG_CLIENT_WANT_PAGE_GROUPE:
             { Client_mode( client, ENVOI_GROUPE );
             }
            break;
       case SSTAG_CLIENT_ADD_GROUPE:
             { struct CMD_ADD_GROUPE *groupe;
               groupe = (struct CMD_ADD_GROUPE *)connexion->donnees;
               Proto_ajouter_groupe( client, groupe );
             }
            break;
       case SSTAG_CLIENT_DEL_GROUPE:
             { struct CMD_ID_GROUPE *groupe;
               groupe = (struct CMD_ID_GROUPE *)connexion->donnees;
               Proto_effacer_groupe( client, groupe );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_GROUPE:
             { struct CMD_EDIT_GROUPE *groupe;
               groupe = (struct CMD_EDIT_GROUPE *)connexion->donnees;
               Proto_valider_editer_groupe( client, groupe );
             }
            break;

       case SSTAG_CLIENT_WANT_PAGE_UTIL:
             { Client_mode( client, ENVOI_UTIL );
             }
            break;
       case SSTAG_CLIENT_WANT_GROUPE_FOR_UTIL:
             { Client_mode( client, ENVOI_GROUPE_FOR_UTIL );
             }
            break;
       case SSTAG_CLIENT_EDIT_UTIL:
             { struct CMD_ID_UTILISATEUR *util;
               util = (struct CMD_ID_UTILISATEUR *)connexion->donnees;
               Proto_editer_utilisateur( client, util );
             }
            break;
       case SSTAG_CLIENT_ADD_UTIL:
             { struct CMD_ADD_UTILISATEUR *util;
               util = (struct CMD_ADD_UTILISATEUR *)connexion->donnees;
               Proto_ajouter_utilisateur( client, util );
             }
            break;
       case SSTAG_CLIENT_DEL_UTIL:
             { struct CMD_ID_UTILISATEUR *util;
               util = (struct CMD_ID_UTILISATEUR *)connexion->donnees;
               Proto_effacer_utilisateur( client, util );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_UTIL:
             { struct CMD_EDIT_UTILISATEUR *util;
               util = (struct CMD_EDIT_UTILISATEUR *)connexion->donnees;
               Proto_valider_editer_utilisateur( client, util );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
