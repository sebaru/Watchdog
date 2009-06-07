/************************************************************************************************************/
/* Include/Version.h    Gestion de la version des données clientes/serveur                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                        sam 04 oct 2003 12:47:01 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                */
/************************************************************************************************************/

#ifndef _VERSION_H
 #define _VERSION_H
 #include <glib.h>

 #define FICHIER_VERSION "version.wdg"
 #define TAILLE_VERSION  20

 extern time_t Lire_version_donnees( struct LOG *log );
 extern time_t Changer_version_donnees( struct LOG *log, time_t new_version );
#endif
/*----------------------------------------------------------------------------------------------------------*/
