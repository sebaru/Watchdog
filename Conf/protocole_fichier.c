/**********************************************************************************************************/
/* Client/protocole_fichier.c    Gestion du protocole_message pour la connexion au serveur Watchdog       */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include "Erreur.h"
 #include "Reseaux.h"
 #include "Config_cli.h"
 
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_fichier_connecte ( struct CONNEXION *connexion )
  { 
    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_DEL_FICHIER:
             { struct CMD_FICHIER *cmd_fichier;
               cmd_fichier = (struct CMD_FICHIER *)connexion->donnees;
               unlink( cmd_fichier->nom );
               printf("Fichier %s effacé sur ordre serveur\n", cmd_fichier->nom );
             }
            break;
       case SSTAG_SERVEUR_FICHIER:
             { struct CMD_FICHIER *cmd_fichier;
               guint taille, id;
               cmd_fichier = (struct CMD_FICHIER *)connexion->donnees;
               taille = connexion->entete.taille_donnees - sizeof(struct CMD_FICHIER);
               printf("Recu fichier %s: taille %d\n", cmd_fichier->nom, taille );
               id = open( cmd_fichier->nom, O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR );
               if (id>0)
                { write( id, connexion->donnees + sizeof(struct CMD_FICHIER), taille );
                  close ( id );
                }
               Set_progress_plus( taille );
             }
            break;
       case SSTAG_SERVEUR_VERSION:
             { struct CMD_VERSION *cmd_version;
               cmd_version = (struct CMD_VERSION *)connexion->donnees;
               Changer_version_donnees( Config_cli.log, cmd_version->version );
               Info_n( Config_cli.log, DEBUG_CONNEXION, "Nouvelle version données", cmd_version->version );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
