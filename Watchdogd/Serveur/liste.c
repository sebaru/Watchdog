/**********************************************************************************************************/
/* Watchdogd/Serveur/liste.c        Creation des listes de fichiers pour envoi aux clients                */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mer. 09 déc. 2009 20:51:20 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * liste.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien Lefevre
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
 #include <unistd.h>
 #include <stdlib.h>
 #include <dirent.h>
 #include <string.h>
 #include <stdio.h>
 #include <time.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "Reseaux.h"
 #include "watchdogd.h"
/**********************************************************************************************************/
/* Liberer_liste: Liberation de la liste de fichiers                                                      */
/* Entrée: la liste en cours                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Liberer_liste( struct CLIENT *client )
  { g_list_foreach( client->transfert.fichiers, (GFunc)g_free, NULL );
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
    if ( info.st_mtime <= version_d_client )
     { g_free(nouveau); return(0); }
    nouveau->taille = info.st_size;
    client->transfert.fichiers = g_list_append( client->transfert.fichiers, nouveau );
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
     { Info_new( Config.log, Config.log_all, LOG_WARNING,
                "Ajouter_repertoire_liste: Directory %s unknown", Repertoire );
       return(0);
     }

    while( (fichier = readdir( repertoire )) )
     { if (!strncmp( fichier->d_name, ".", 1 )) continue;
       taille += Ajouter_fichier_liste( client, Repertoire, fichier->d_name, version_d_client );
     }
    closedir( repertoire );
    return(taille);
  }
/*--------------------------------------------------------------------------------------------------------*/
