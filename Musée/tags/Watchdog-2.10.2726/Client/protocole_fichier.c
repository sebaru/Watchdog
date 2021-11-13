/**********************************************************************************************************/
/* Client/protocole_fichier.c    Gestion du protocole_message pour la connexion au serveur Watchdog       */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 21 fév 2006 14:07:22 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_fichier.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
 *
 * Watchdog is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Watchdog is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Watchdog; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

 #include <glib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include "Erreur.h"
 #include "Reseaux.h"
 #include "Config_cli.h"
 
/********************************* Définitions des prototypes programme ***********************************/
 #include "protocli.h"
 #include "client.h"

 extern GtkWidget *F_client;                                                     /* Widget Fenetre Client */
 extern struct CONFIG_CLI Config_cli;                          /* Configuration generale cliente watchdog */

/**********************************************************************************************************/
/* Action_version_donnees: definition de la version des données clientes                                  */
/* Entrée: le type de changement                                                                          */
/* Sortie: ptr sur la chaine de version                                                                   */
/**********************************************************************************************************/
 static gint Action_icone_version( guint new_version )
  { int id_fichier;
    gint temps;

    if (new_version==0)
     { id_fichier = open( FICHIER_VERSION, O_RDONLY );
       if (id_fichier>0)
        { read( id_fichier, &temps, sizeof(temps) );
          close(id_fichier);
          return(temps);
        }
       else
        { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                   "Action_icone_version: open %s failed (%s)", FICHIER_VERSION, strerror(errno) );
          return(0);
        }
     }
    else { id_fichier = open( FICHIER_VERSION, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
           if (id_fichier<0)
            { Info_new( Config_cli.log, Config_cli.log_override, LOG_ERR,
                       "Action_icone_version: write %s failed (%s)", FICHIER_VERSION, strerror(errno) );
              return(0);
            }
           write( id_fichier, &new_version, sizeof(new_version) );
           close(id_fichier);
           Info_new( Config_cli.log, Config_cli.log_override, LOG_DEBUG,
                    "Action_icone_version: write %s sucessfull (version %d)", FICHIER_VERSION, new_version );
           return(new_version);
         }
  }
/**********************************************************************************************************/
/* Lire_Version_donnees: acquiere le numero de version des données clientes                               */
/* Entrée: néant                                                                                          */
/* Sortie: ptr sur la chaine de version                                                                   */
/**********************************************************************************************************/
 gint Get_icone_version( void )
  { return( Action_icone_version( 0 ) ); }
/**********************************************************************************************************/
/* Changer_version_donnees: change l'actuelle version des données                                         */
/* Entrée: néant                                                                                          */
/* Sortie: ptr sur la chaine de version                                                                   */
/**********************************************************************************************************/
 static void Set_icone_version( guint new_version )
  { Action_icone_version( new_version ); }
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
       case SSTAG_SERVEUR_APPEND_FICHIER:
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
             { gint *cmd_version;
               cmd_version = (gint *)connexion->donnees;
               Set_icone_version( *cmd_version );
               Info_new( Config_cli.log, Config_cli.log_override, LOG_INFO,
                        "Nouvelle version données %d", *cmd_version );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
