/**********************************************************************************************************/
/* Client/protocole_dls.c    Gestion du protocole_dls pour la connexion au serveur Watchdog               */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include "Erreur.h"
 #include "Reseaux.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_dls ( struct CONNEXION *connexion )
  { static GList *Arrivee_dls = NULL;
               
    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_ADD_PLUGIN_DLS_OK:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Proto_afficher_un_plugin_dls( dls );
             }
            break;
       case SSTAG_SERVEUR_DEL_PLUGIN_DLS_OK:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Proto_cacher_un_plugin_dls( dls );
             }
            break;
       case SSTAG_SERVEUR_EDIT_PLUGIN_DLS_OK:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Menu_ajouter_editer_plugin_dls( dls );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_PLUGIN_DLS_OK:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Proto_rafraichir_un_plugin_dls( dls );
             }
            break;
       case SSTAG_SERVEUR_EDIT_SOURCE_DLS_OK:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               dls = (struct CMD_TYPE_PLUGIN_DLS *)connexion->donnees;
               Creer_page_source_dls( dls );
             }
            break;
       case SSTAG_SERVEUR_SOURCE_DLS:
             { struct CMD_EDIT_SOURCE_DLS *dls;
               gchar *buffer;
               dls = (struct CMD_EDIT_SOURCE_DLS *)connexion->donnees;
               buffer = (gchar *)dls + sizeof(struct CMD_EDIT_SOURCE_DLS);
               Proto_append_source_dls( dls, buffer );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_PLUGIN_DLS:
             { struct CMD_TYPE_PLUGIN_DLS *dls;
               Set_progress_plusun();

               dls = (struct CMD_TYPE_PLUGIN_DLS *)g_malloc0( sizeof( struct CMD_TYPE_PLUGIN_DLS ) );
               if (!dls) return; 
               memcpy( dls, connexion->donnees, sizeof(struct CMD_TYPE_PLUGIN_DLS ) );
               printf("One plugin receive %s\n", dls->nom );
               Arrivee_dls = g_list_append( Arrivee_dls, dls );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_PLUGIN_DLS_FIN:
             { g_list_foreach( Arrivee_dls, (GFunc)Proto_afficher_un_plugin_dls, NULL );
               g_list_foreach( Arrivee_dls, (GFunc)g_free, NULL );
               g_list_free( Arrivee_dls );
               Arrivee_dls = NULL;
               Chercher_page_notebook( TYPE_PAGE_PLUGIN_DLS, 0, TRUE );
             }
            break;
       case SSTAG_SERVEUR_TYPE_NUM_MNEMO:
             { struct CMD_TYPE_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               Proto_afficher_mnemo_dls( mnemo );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
