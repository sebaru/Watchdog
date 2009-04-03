/**********************************************************************************************************/
/* Client/protocole_connexion.c    Gestion du protocole_connexion pour la connexion au serveur Watchdog   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <gnome.h>
 #include "Erreur.h"
 #include "Reseaux.h"

/********************************* Définitions des prototypes programme ***********************************/
 #include "Config_cli.h"
 #include "client.h"
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CLIENT Client_en_cours;                           /* Identifiant de l'utilisateur en cours */
 extern GtkWidget *Barre_status;                                         /* Barre d'etat de l'application */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */
/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_connexion ( struct CONNEXION *connexion )
  { GtkWidget *dialog;
    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_REFUSE:
             { dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client),
                                                 GTK_DIALOG_DESTROY_WITH_PARENT | GTK_DIALOG_MODAL,
                                                 GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
                                                 _("You are not authorized\nto connect") );
               g_signal_connect_swapped( dialog, "response",
                                         G_CALLBACK(gtk_widget_destroy), dialog );
               gtk_widget_show_all(dialog);
               Deconnecter();
             }
            break;
       case SSTAG_SERVEUR_ACCOUNT_DISABLED:
             { dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
                                                 _("Your account is\ndisabled") );
               g_signal_connect_swapped( dialog, "response",
                                         G_CALLBACK(gtk_widget_destroy), dialog );
               gtk_widget_show_all(dialog);
               Deconnecter();
             }
            break;
       case SSTAG_SERVEUR_ACCOUNT_EXPIRED:
             { dialog = gtk_message_dialog_new ( GTK_WINDOW(F_client), GTK_DIALOG_DESTROY_WITH_PARENT,
                                                 GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE,
                                                 _("Your account has\nexpired") );
               g_signal_connect_swapped( dialog, "response",
                                         G_CALLBACK(gtk_widget_destroy), dialog );
               gtk_widget_show_all(dialog);
               Deconnecter();
             }
            break;
       case SSTAG_SERVEUR_CHANGEPASS:
             { struct CMD_UTIL_SETPASSWORD *util;
               util = (struct CMD_UTIL_SETPASSWORD *)connexion->donnees;
               Client_en_cours.id = util->id;
               if (!Changer_password()) Deconnecter();
             }
            break;

       case SSTAG_SERVEUR_AUTORISE:
             { struct REZO_SRV_IDENT *ident;
               gchar chaine[80];

               ident = (struct REZO_SRV_IDENT *)connexion->donnees;
               g_snprintf( chaine, sizeof(chaine), _("Connected to %s@%s:%d  %s"),
                           Client_en_cours.user, Client_en_cours.serveur, Config_cli.port, ident->comment );
               gnome_appbar_push( GNOME_APPBAR(Barre_status), chaine );
               Log( _("Connected") );
               Client_en_cours.mode = CONNECTE;
               Info( Config_cli.log, DEBUG_CONNEXION, "Client en mode CONNECTE" );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
