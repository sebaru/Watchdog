/**********************************************************************************************************/
/* Client/protocole_courbe.c    Gestion du protocole_courbe pour la connexion au serveur Watchdog         */
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
 void Gerer_protocole_courbe ( struct CONNEXION *connexion )
  { static GList *Arrivee_eana = NULL;
    static GList *Arrivee_mnemo = NULL;

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_ADD_COURBE_OK:
             { struct CMD_ID_COURBE *courbe;
               courbe = (struct CMD_ID_COURBE *)connexion->donnees;
               printf("proto ajouter courbe !!\n");
               Proto_ajouter_courbe( courbe );
             }
            break;
   /*    case SSTAG_SERVEUR_INIT_COURBE:
             { struct CMD_APPEND_COURBE *courbe, *test;
               gint cpt;
               courbe = (struct CMD_APPEND_COURBE *)connexion->donnees;
               for ( cpt=0; cpt<(connexion->entete.taille_donnees/sizeof(struct CMD_APPEND_COURBE)); cpt++ )
                { test = &courbe[cpt];
                  Proto_append_courbe( courbe );
                }
             }
            break;*/
       case SSTAG_SERVEUR_APPEND_COURBE:
             { struct CMD_APPEND_COURBE *courbe;
               courbe = (struct CMD_APPEND_COURBE *)connexion->donnees;
               Proto_append_courbe( courbe );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_COURBE:
             { struct CMD_SHOW_ENTREEANA *eana;
               Set_progress_plusun();

               eana = (struct CMD_SHOW_ENTREEANA *)g_malloc0( sizeof( struct CMD_SHOW_ENTREEANA ) );
               if (!eana) return; 
               memcpy( eana, connexion->donnees, sizeof(struct CMD_SHOW_ENTREEANA ) );
               Arrivee_eana = g_list_append( Arrivee_eana, eana );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_COURBE_FIN:
             { 
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_COURBE:
             { struct CMD_SHOW_MNEMONIQUE *mnemo;
               Set_progress_plusun();

               mnemo = (struct CMD_SHOW_MNEMONIQUE *)g_malloc0( sizeof( struct CMD_SHOW_MNEMONIQUE ) );
               if (!mnemo) return; 
               memcpy( mnemo, connexion->donnees, sizeof(struct CMD_SHOW_MNEMONIQUE ) );
               Arrivee_mnemo = g_list_append( Arrivee_mnemo, mnemo );
             }
            break;
       case SSTAG_SERVEUR_ADDPROGRESS_MNEMO_FOR_COURBE_FIN:
             { g_list_foreach( Arrivee_eana, (GFunc)Proto_afficher_une_source_EA_for_courbe, NULL );
               g_list_foreach( Arrivee_eana, (GFunc)g_free, NULL );
               g_list_free( Arrivee_eana );
               Arrivee_eana = NULL;

               g_list_foreach( Arrivee_mnemo, (GFunc)Proto_afficher_une_source_for_courbe, NULL );
               g_list_foreach( Arrivee_mnemo, (GFunc)g_free, NULL );
               g_list_free( Arrivee_mnemo );
               Arrivee_mnemo = NULL;
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
