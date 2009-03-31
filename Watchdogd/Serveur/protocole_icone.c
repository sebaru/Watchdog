/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_icone.c    Gestion du protocole_icone pour Watchdog            */
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
 void Gerer_protocole_icone( gint Id_serveur, struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util->id, client->util->gids, GID_ICONE) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Non Autorisé..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_CLASSE:
             { Client_mode( client, ENVOI_CLASSE );
             }
            break;
       case SSTAG_CLIENT_EDIT_CLASSE: 
             { struct CMD_ID_CLASSE *classe;
               classe = (struct CMD_ID_CLASSE *)connexion->donnees;
               Proto_editer_classe( client, classe );
             }
            break;
       case SSTAG_CLIENT_ADD_CLASSE:
             { struct CMD_ADD_CLASSE *classe;
               classe = (struct CMD_ADD_CLASSE *)connexion->donnees;
               Proto_ajouter_classe( client, classe );
             }
            break;
       case SSTAG_CLIENT_DEL_CLASSE:
             { struct CMD_ID_CLASSE *classe;
               classe = (struct CMD_ID_CLASSE *)connexion->donnees;
               Proto_effacer_classe( client, classe );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_CLASSE:
             { struct CMD_EDIT_CLASSE *classe;
               classe = (struct CMD_EDIT_CLASSE *)connexion->donnees;
               Proto_valider_editer_classe( client, classe );
             }
            break;
/********************************* Client en VALIDE, gestion des icones ***********************************/
       case SSTAG_CLIENT_WANT_PAGE_ICONE:
            { Client_mode( client, ENVOI_ICONE );
               client->classe_icone = ((struct CMD_ID_CLASSE *)connexion->donnees)->id;
             }
            break;
       case SSTAG_CLIENT_EDIT_ICONE:
             { struct CMD_ID_ICONE *icone;
               icone = (struct CMD_ID_ICONE *)connexion->donnees;
               Proto_editer_icone( client, icone );
             }
            break;
       case SSTAG_CLIENT_ADD_ICONE:
             { struct CMD_ADD_ICONE *icone;
               icone = (struct CMD_ADD_ICONE *)connexion->donnees;
               Proto_ajouter_icone( client, icone );
             }
            break;
       case SSTAG_CLIENT_ADD_ICONE_DEB_FILE:
             { struct CMD_ADD_ICONE *icone;
               icone = (struct CMD_ADD_ICONE *)connexion->donnees;
               Proto_ajouter_icone_deb_file( client, icone );
             }
            break;
       case SSTAG_CLIENT_ADD_ICONE_FILE:
             { struct CMD_ADD_ICONE *icone;
               icone = (struct CMD_ADD_ICONE *)connexion->donnees;
               Proto_ajouter_icone_file( client, icone,
                                         connexion->entete.taille_donnees - sizeof(struct CMD_ADD_ICONE),
                                         connexion->donnees + sizeof(struct CMD_ADD_ICONE) );
             }
            break;
       case SSTAG_CLIENT_ADD_ICONE_FIN_FILE:
             { struct CMD_ADD_ICONE *icone;
               icone = (struct CMD_ADD_ICONE *)connexion->donnees;
               Proto_ajouter_icone_fin_file( client, icone );
             }
            break;
       case SSTAG_CLIENT_DEL_ICONE:
             { struct CMD_ID_ICONE *icone;
               icone = (struct CMD_ID_ICONE *)connexion->donnees;
               Proto_effacer_icone( client, icone );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_ICONE:
             { struct CMD_EDIT_ICONE *icone;
               icone = (struct CMD_EDIT_ICONE *)connexion->donnees;
               Proto_valider_editer_icone( client, icone );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
