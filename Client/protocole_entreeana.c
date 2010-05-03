/**********************************************************************************************************/
/* Client/protocole_entreeana.c    Gestion du protocole_entreeana pour la connexion au serveur Watchdog   */
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
 void Gerer_protocole_entreeana ( struct CONNEXION *connexion )
  { static GList *Arrivee_eana = NULL;
    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_CREATE_PAGE_ENTREEANA_OK:
             { Creer_page_entreeANA();
             }
            break;
       case SSTAG_SERVEUR_EDIT_ENTREEANA_OK:
             { struct CMD_TYPE_ENTREEANA *eana;
               eana = (struct CMD_TYPE_ENTREEANA *)connexion->donnees;
               Menu_ajouter_editer_entreeANA( eana );
             }
            break;
       case SSTAG_SERVEUR_VALIDE_EDIT_ENTREEANA_OK:
             { struct CMD_TYPE_ENTREEANA *eana;
               eana = (struct CMD_TYPE_ENTREEANA *)connexion->donnees;
               Proto_rafraichir_une_entreeANA( eana );
             }
            break;

       case SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA:
             { struct CMD_TYPE_ENTREEANA *eana;
               Set_progress_plusun();

               eana = (struct CMD_TYPE_ENTREEANA *)g_malloc0( sizeof( struct CMD_TYPE_ENTREEANA ) );
               if (!eana) return; 
               printf("Recu une eana !! \n");
               memcpy( eana, connexion->donnees, sizeof(struct CMD_TYPE_ENTREEANA ) );
               Arrivee_eana = g_list_append( Arrivee_eana, eana );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FIN:
             { g_list_foreach( Arrivee_eana, (GFunc)Proto_afficher_une_entreeANA, NULL );
               g_list_foreach( Arrivee_eana, (GFunc)g_free, NULL );
               g_list_free( Arrivee_eana );
               Arrivee_eana = NULL;
               Chercher_page_notebook( TYPE_PAGE_ENTREEANA, 0, TRUE );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
