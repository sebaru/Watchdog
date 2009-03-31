/**********************************************************************************************************/
/* Client/protocole_mnemonique.c    Gestion du protocole_mnemonique pour la connexion au serveur Watchdog */
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
 void Gerer_protocole_mnemonique ( struct CONNEXION *connexion )
  { static GList *Arrivee_mnemonique = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_ADD_MNEMONIQUE_OK:
             { struct CMD_SHOW_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_SHOW_MNEMONIQUE *)connexion->donnees;
               Proto_afficher_un_mnemonique( mnemo );
             }
            break;
       case SSTAG_SERVEUR_DEL_MNEMONIQUE_OK:
             { struct CMD_ID_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_ID_MNEMONIQUE *)connexion->donnees;
               Proto_cacher_un_mnemonique( mnemo );
             }
            break;
       case SSTAG_SERVEUR_EDIT_MNEMONIQUE_OK:
             { struct CMD_EDIT_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_EDIT_MNEMONIQUE *)connexion->donnees;
               printf("recu mnemo edit\n");
               Menu_ajouter_editer_mnemonique( mnemo );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_MNEMONIQUE_OK:
             { struct CMD_SHOW_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_SHOW_MNEMONIQUE *)connexion->donnees;
               Proto_rafraichir_un_mnemonique( mnemo );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE:
             { struct CMD_SHOW_MNEMONIQUE *mnemo;
               Set_progress_plusun();

               mnemo = (struct CMD_SHOW_MNEMONIQUE *)g_malloc0( sizeof( struct CMD_SHOW_MNEMONIQUE ) );
               if (!mnemo) return; 
               memcpy( mnemo, connexion->donnees, sizeof(struct CMD_SHOW_MNEMONIQUE ) );
               Arrivee_mnemonique = g_list_append( Arrivee_mnemonique, mnemo );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE_FIN:
             { g_list_foreach( Arrivee_mnemonique, (GFunc)Proto_afficher_un_mnemonique, NULL );
               g_list_foreach( Arrivee_mnemonique, (GFunc)g_free, NULL );
               g_list_free( Arrivee_mnemonique );
               Arrivee_mnemonique = NULL;
               Chercher_page_notebook( TYPE_PAGE_MNEMONIQUE, 0, TRUE );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
