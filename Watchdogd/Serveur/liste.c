/**********************************************************************************************************/
/* Watchdogd/Serveur/liste.c        Creation des listes de fichiers pour envoi aux clients                */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 oct 2003 13:01:25 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <dirent.h>
 #include <string.h>
 #include <stdio.h>
 #include <time.h>

 #include "Erreur.h"
 #include "Config.h"
 #include "Client.h"

/* #define DEBUG*/               /* Activer pour debugger les fonctions de gestion des listes de fichiers */
 extern struct CONFIG Config;                                          /* Configuration generale watchdog */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Liberer_liste: Liberation de la liste de fichiers                                                      */
/* Entrée: la liste en cours                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Liberer_liste( struct CLIENT *client )
  { GList *liste;

    g_list_foreach( client->transfert.fichiers, (GFunc)g_free, NULL );
    g_list_free( client->transfert.fichiers );
    client->transfert.fichiers = NULL;
  }
/**********************************************************************************************************/
/* Ajouter_fichier_liste: Creation ou ajout de fichiers à la liste                                        */
/* Entrée: la liste en cours, le fichier                                                                  */
/* Sortie: la taille du fichier ajouté à la liste                                                         */
/**********************************************************************************************************/
 static gint Ajouter_fichier_liste( struct CLIENT *client, gchar *repertoire, gchar *fichier,
                                    time_t version_d_client )
  { struct LISTE_FICH *nouveau;
    struct stat info;

    nouveau = g_malloc0( sizeof(struct LISTE_FICH) );
    if (!nouveau) return(0);

    if (repertoire)
         g_snprintf( nouveau->fichier_absolu, sizeof(nouveau->fichier_absolu), "%s/%s", repertoire, fichier );
    else g_snprintf( nouveau->fichier_absolu, sizeof(nouveau->fichier_absolu), "%s", fichier );
    g_snprintf( nouveau->fichier, sizeof(nouveau->fichier), "%s", fichier );
    stat( nouveau->fichier_absolu, &info );
    printf("Ajouter_fichier_liste: date fich %d, date client %d fichier %s fichiers %p\n",
           info.st_mtime, version_d_client, nouveau->fichier, client->transfert.fichiers );
    if ( info.st_mtime <= version_d_client )
     { printf("Sortie liste\n"); g_free(nouveau); return(0); }
printf("One\n");
    nouveau->taille = info.st_size;
printf("Two\n");
    client->transfert.fichiers = g_list_append( client->transfert.fichiers, nouveau );
printf("Three\n");
    printf("%s ajouté !\n", nouveau->fichier );
    return(nouveau->taille);
  }
/**********************************************************************************************************/
/* Ajouter_repertoire_liste: Creation ou ajout de repertoire à la liste                                   */
/* Entrée: la liste en cours, le repertoire                                                               */
/* Sortie: taille globale                                                                                 */
/**********************************************************************************************************/
 gint Ajouter_repertoire_liste( struct CLIENT *client, gchar *Repertoire,
                                time_t version_d_client )
  { struct dirent *fichier;
    DIR *repertoire;
    int taille;

    taille = 0;
    repertoire = opendir ( Repertoire );
    if (!repertoire)
     { Info_c( Config.log, DEBUG_INFO, "Ajouter_repertoire_liste: rep. inconnu", Repertoire );
       return(0);
     }

    while( (fichier = readdir( repertoire )) )
     { if (!fichier)                                              /* Est-on deja a la fin du repertoire?? */
        { closedir( repertoire );
          return(0);
        }

       if (!strncmp( fichier->d_name, ".", 1 )) continue;
       taille += Ajouter_fichier_liste( client, Repertoire, fichier->d_name, version_d_client );
     }
    return(taille);
  }
/*--------------------------------------------------------------------------------------------------------*/
