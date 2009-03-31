/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_dls.c    Gestion du protocole_dls pour Watchdog                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include "Erreur.h"
 #include "Reseaux.h"
 #include "Client.h"
 #include "Dls_DB.h"
 #include "Config.h"

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
/********************************* Définitions des prototypes programme ***********************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_dls( gint Id_serveur, struct CLIENT *client )
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
     { case SSTAG_CLIENT_WANT_PAGE_DLS:
             { Client_mode( client, ENVOI_DLS );
             }
            break;
       case SSTAG_CLIENT_ADD_PLUGIN_DLS:
             { struct CMD_ADD_PLUGIN_DLS *dls;
               dls = (struct CMD_ADD_PLUGIN_DLS *)connexion->donnees;
               Proto_ajouter_plugin_dls( client, dls );
             }
            break;
       case SSTAG_CLIENT_EDIT_PLUGIN_DLS:
             { struct CMD_ID_PLUGIN_DLS *dls;
               dls = (struct CMD_ID_PLUGIN_DLS *)connexion->donnees;
               Proto_editer_plugin_dls( client, dls );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_PLUGIN_DLS:
             { struct CMD_EDIT_PLUGIN_DLS *dls;
               dls = (struct CMD_EDIT_PLUGIN_DLS *)connexion->donnees;
               Proto_valider_editer_plugin_dls( client, dls );
             }
            break;
       case SSTAG_CLIENT_DEL_PLUGIN_DLS:
             { struct CMD_ID_PLUGIN_DLS *dls;
               dls = (struct CMD_ID_PLUGIN_DLS *)connexion->donnees;
               Proto_effacer_plugin_dls( client, dls );
             }
            break;
       case SSTAG_CLIENT_EDIT_SOURCE_DLS:
             { struct CMD_ID_PLUGIN_DLS *dls;
               dls = (struct CMD_ID_PLUGIN_DLS *)connexion->donnees;
               Proto_editer_source_dls( client, dls );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS_DEB:
             { struct CMD_EDIT_SOURCE_DLS *edit_dls;
               edit_dls = (struct CMD_EDIT_SOURCE_DLS *)connexion->donnees;
               Proto_effacer_fichier_plugin_dls( client, edit_dls->id );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS:
             { struct CMD_EDIT_SOURCE_DLS *edit_dls;
               edit_dls = (struct CMD_EDIT_SOURCE_DLS *)connexion->donnees;
               Proto_valider_source_dls( client, edit_dls,
                                         (gchar *)edit_dls + sizeof(struct CMD_EDIT_SOURCE_DLS) );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_SOURCE_DLS_FIN:
             { memcpy( &client->dls, (struct CMD_EDIT_SOURCE_DLS *)connexion->donnees,
                       sizeof( client->dls ) );
               Info_n( Config.log, DEBUG_FORK, "SSRV: Protole: Creation pthread compilation DLS", client->dls.id );
               pthread_create( &tid, NULL, (void *)Proto_compiler_source_dls, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_WANT_TYPE_NUM_MNEMO:
             { struct CMD_TYPE_NUM_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_NUM_MNEMONIQUE *)connexion->donnees;
               printf("Le client desire le mnemonique %d %d\n", mnemo->type, mnemo->num );
               Proto_envoyer_type_num_mnemo_tag( TAG_DLS, SSTAG_SERVEUR_TYPE_NUM_MNEMO, client, mnemo );
             }
            break;

     }
  }
/*--------------------------------------------------------------------------------------------------------*/
