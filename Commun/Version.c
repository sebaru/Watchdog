/**********************************************************************************************************/
/* Commun/Version.c    Gestion de la version des données clientes/serveur                                 */
/* Projet WatchDog version 1.4       Gestion d'habitat                      sam 04 oct 2003 12:47:09 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <sys/types.h>
 #include <sys/stat.h>
 #include <time.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <stdio.h>
 #include <string.h>

 #include "Erreur.h"
 #include "Version.h"

/**********************************************************************************************************/
/* Action_version_donnees: definition de la version des données clientes                                  */
/* Entrée: le type de changement                                                                          */
/* Sortie: ptr sur la chaine de version                                                                   */
/**********************************************************************************************************/
 static time_t Action_version_donnees( struct LOG *log, time_t new_version )
  { int id_fichier;
    time_t temps;

    if (!new_version)
     { id_fichier = open( FICHIER_VERSION, O_RDONLY );
       if (id_fichier>0)
        { read( id_fichier, &temps, sizeof(temps) );
          close(id_fichier);
        }
       else { Info_c( log, DEBUG_INFO, "Action_version_donnees: open failed", FICHIER_VERSION );
              return(0);
            }
       return(temps);
     }
    else { id_fichier = open( FICHIER_VERSION, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
           if (id_fichier<0)
            { Info_c( log, DEBUG_INFO, "Action_version_donnees: write failed", FICHIER_VERSION );
              return(0);
            }
           write( id_fichier, &new_version, sizeof(new_version) );
           close(id_fichier);
           return(new_version);
         }
  }
/**********************************************************************************************************/
/* Lire_Version_donnees: acquiere le numero de version des données clientes                               */
/* Entrée: néant                                                                                          */
/* Sortie: ptr sur la chaine de version                                                                   */
/**********************************************************************************************************/
 time_t Lire_version_donnees( struct LOG *log )
  { return( Action_version_donnees( log, FALSE ) ); }
/**********************************************************************************************************/
/* Changer_version_donnees: change l'actuelle version des données                                         */
/* Entrée: néant                                                                                          */
/* Sortie: ptr sur la chaine de version                                                                   */
/**********************************************************************************************************/
 time_t Changer_version_donnees( struct LOG *log, time_t new_version )
  { if (!new_version) return( Action_version_donnees( log, time(NULL) ) );
                 else return( Action_version_donnees( log, new_version ) );
  }
/*--------------------------------------------------------------------------------------------------------*/
