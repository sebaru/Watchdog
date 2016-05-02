/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_icone.c        Configuration des icones de Watchdog v2.0                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 01 nov 2003 13:13:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_icone.c
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
 #include <sys/prctl.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

 #include <sys/stat.h>
 #include <sys/types.h>
 #include <fcntl.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Preparer_envoi_icone: convertit une structure MSG en structure CMD_TYPE_ICONE                          */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_TYPE_ICONE *Preparer_envoi_icone ( struct ICONEDB *icone )
  { struct CMD_TYPE_ICONE *rezo_icone;

    rezo_icone = (struct CMD_TYPE_ICONE *)g_try_malloc0( sizeof(struct CMD_TYPE_ICONE) );
    if (!rezo_icone) { return(NULL); }

    rezo_icone->id         = icone->id;
    rezo_icone->id_classe  = icone->id_classe;
    memcpy( &rezo_icone->libelle, icone->libelle, sizeof(rezo_icone->libelle) );
    return( rezo_icone );
  }
/**********************************************************************************************************/
/* Proto_editer_icone: Le client desire editer un icone                                                   */
/* Entrée: le client demandeur et le icone en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_icone ( struct CLIENT *client, struct CMD_TYPE_ICONE *rezo_icone )
  { struct CMD_TYPE_ICONE edit_icone;
    struct ICONEDB *icone;

    icone = Rechercher_iconeDB( rezo_icone->id );

    if (icone)
     { edit_icone.id         = icone->id;                                   /* Recopie des info editables */
       edit_icone.id_classe  = icone->id_classe;
       memcpy( &edit_icone.libelle, icone->libelle, sizeof(edit_icone.libelle) );

       Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_EDIT_ICONE_OK,
                  (gchar *)&edit_icone, sizeof(struct CMD_TYPE_ICONE) );
       g_free(icone);                                                               /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate icone %s", rezo_icone->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_icone: Le client valide l'edition d'un icone                                      */
/* Entrée: le client demandeur et le icone en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_icone ( struct CLIENT *client, struct CMD_TYPE_ICONE *rezo_icone )
  { struct ICONEDB *result;
    gboolean retour;

    retour = Modifier_iconeDB ( rezo_icone );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit icone %s", rezo_icone->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_iconeDB( rezo_icone->id );
           if (result) 
            { struct CMD_TYPE_ICONE *icone;
              icone = Preparer_envoi_icone ( result );
              g_free(result);
              if (!icone)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_VALIDE_EDIT_ICONE_OK,
                                   (gchar *)icone, sizeof(struct CMD_TYPE_ICONE) );
                     g_free(icone);
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate icone %s", rezo_icone->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_icone: Retrait du icone en parametre                                                     */
/* Entrée: le client demandeur et le icone en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_icone ( struct CLIENT *client, struct CMD_TYPE_ICONE *rezo_icone )
  { gchar nom_fichier[80];
    gboolean retour;

    retour = Retirer_iconeDB( rezo_icone );
printf("Proto_effacer_icone: id=%d retour = %d\n", rezo_icone->id, retour );

    if (retour)
     { Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_DEL_ICONE_OK,
                     (gchar *)rezo_icone, sizeof(struct CMD_TYPE_ICONE) );
       g_snprintf( nom_fichier, sizeof(nom_fichier), "Gif/%d.gif", rezo_icone->id );
       unlink(nom_fichier);
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete icone %s", rezo_icone->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_icone: Un client nous demande d'ajouter un icone Watchdog                                */
/* Entrée: le icone à créer                                                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_icone ( struct CLIENT *client, struct CMD_TYPE_ICONE *rezo_icone )
  { struct ICONEDB *result;
    gint id;

    id = Ajouter_iconeDB ( rezo_icone );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add icone %s", rezo_icone->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_iconeDB( id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate icone %s", rezo_icone->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_TYPE_ICONE *icone;
              icone = Preparer_envoi_icone ( result );
              g_free(result);
              if (!icone)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { rezo_icone->id = id;
                     Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                              "Proto_ajouter_icone: Envoi demande file %s", rezo_icone->libelle );
                     Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_ADD_ICONE_WANT_FILE,
                                   (gchar *)rezo_icone, sizeof(struct CMD_TYPE_ICONE) );
                     Envoi_client( client, TAG_ICONE, SSTAG_SERVEUR_ADD_ICONE_OK,
                                   (gchar *)icone, sizeof(struct CMD_TYPE_ICONE) );
                     g_free(icone);
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Proto_ajouter_icone_deb_file: Debut de reception du fichier gif associé à l'icone                      */
/* Entrée: le icone à créer                                                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_icone_deb_file( struct CLIENT *client, struct CMD_TYPE_ICONE *icone )
  { gchar nom_fichier[80];
    g_snprintf( nom_fichier, sizeof(nom_fichier), "Gif/%s", icone->nom_fichier );
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Proto_ajouter_icone_deb_file: Rapatriement du fichier %s starting.", icone->nom_fichier );
    unlink( nom_fichier );
  }
/**********************************************************************************************************/
/* Proto_ajouter_icone_file: Reception du fichier gif associé à l'icone                                   */
/* Entrée: le icone à créer                                                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_icone_file( struct CLIENT *client, struct CMD_TYPE_ICONE *icone,
                                gint taille, gchar *buffer )
  { gchar nom_fichier[80];
    gint id;
    g_snprintf( nom_fichier, sizeof(nom_fichier), "Gif/%s", icone->nom_fichier );
    id = open( nom_fichier, O_WRONLY | O_CREAT | O_APPEND, S_IWUSR | S_IRUSR );
    if (id<0) return;
    write( id, buffer, taille );
    close(id);
  }
/**********************************************************************************************************/
/* Proto_ajouter_icone_fin_file: Fin de reception du fichier gif associé à l'icone                        */
/* Entrée: le icone à créer                                                                               */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_icone_fin_file( struct CLIENT *client, struct CMD_TYPE_ICONE *icone )
  { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "Proto_ajouter_icone_fin_file: Reception GIF OK %s", icone->libelle );
    Icone_set_data_version();
  }
/**********************************************************************************************************/
/* Envoyer_icones: Envoi des icones au client GID_ICONE                                                   */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoyer_icones_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_TYPE_ICONE *rezo_icone;
    struct CMD_ENREG nbr;
    struct ICONEDB *icone;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiICO", 0, 0, 0 );

    if ( ! Recuperer_iconeDB( &db, client->classe_icone ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       return;
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d  icones", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { icone = Recuperer_iconeDB_suite( &db );
       if (!icone)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          return;
        }

       rezo_icone = Preparer_envoi_icone( icone );
       g_free(icone);
       if (rezo_icone)
        { Envoi_client ( client, tag, sstag,
                         (gchar *)rezo_icone, sizeof(struct CMD_TYPE_ICONE) );
          g_free(rezo_icone);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_icones: Envoi des icones au client GID_ICONE                                                   */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_icones_thread ( struct CLIENT *client )
  { Envoyer_icones_tag( client, TAG_ICONE, SSTAG_SERVEUR_ADDPROGRESS_ICONE,
                                           SSTAG_SERVEUR_ADDPROGRESS_ICONE_FIN );
    pthread_exit ( NULL );
  }
/**********************************************************************************************************/
/* Envoyer_icones: Envoi des icones au client GID_ICONE                                                   */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_icones_pour_atelier_thread ( struct CLIENT *client )
  { Envoyer_icones_tag( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ICONE_FOR_ATELIER,
                                             SSTAG_SERVEUR_ADDPROGRESS_ICONE_FOR_ATELIER_FIN );
    pthread_exit ( NULL );
  }
/**********************************************************************************************************/
/* Envoyer_donnees: Transmet les données pour le fonctionnement correct du client distant                 */
/* Entrée: structure cliente distante                                                                     */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Send_synchro_directory( struct CLIENT *client, gint icone_version )
  { gchar *filename;
    gint taille, fd;
    void *buffer;

    buffer = g_try_malloc0( client->connexion->taille_bloc + sizeof(struct CMD_FICHIER) );
    if (!buffer)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                "Send_synchro_file: Buffer Memory Alloc Error" );
       Client_mode ( client, DECONNECTE );
       return;
     }
    else
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                "Send_synchro_file: %d files to send", g_slist_length( client->Liste_file ) );
     }

    while (client->Liste_file)
     { filename = (gchar *)(client->Liste_file->data);
       client->Liste_file = g_slist_remove ( client->Liste_file, filename );

       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Send_synchro_file: Start sending %s", filename );

       g_snprintf( ((struct CMD_FICHIER *)buffer)->nom, sizeof( ((struct CMD_FICHIER *)buffer)->nom ),
                   "%s", filename );
       fd = open( filename, O_RDONLY );
       if (fd<0)
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                   "Send_synchro_file: Sending failed for %s (%s)", filename, strerror(errno) );
        }
       else
        { Envoi_client( client, TAG_FICHIER, SSTAG_SERVEUR_DEL_FICHIER,
                        buffer, sizeof(struct CMD_FICHIER) );

          while( (taille = read( fd, buffer + sizeof(struct CMD_FICHIER),/* On n'est pas a fin de fichier */
                                 client->connexion->taille_bloc - sizeof(struct CMD_FICHIER) ) ) > 0 )
           { Envoi_client( client, TAG_FICHIER, SSTAG_SERVEUR_APPEND_FICHIER, buffer,
                           taille + sizeof(struct CMD_FICHIER) );
           }
          close(fd);                                           /* Oui, on le ferme et on passe au suivant */
        }
       g_free(filename);
     }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Send_synchro_file: Send successfull" );
    Envoi_client( client, TAG_FICHIER, SSTAG_SERVEUR_VERSION,
                 (gchar *)&icone_version, sizeof( icone_version ) );
  }
/**********************************************************************************************************/
/* Preparer_synchro_directory: Prepare l'envoi groupée de l'ensemble des fichiers de travail aux clients  */
/* Entrée: le client destinataire et le directory a envoyer                                               */
/* Sortie: taille globale                                                                                 */
/**********************************************************************************************************/
 static gint Preparer_synchro_directory ( struct CLIENT *client, gchar *directory )
  { struct dirent *fichier;
    DIR *repertoire;
    int taille;
    gchar full_filename[80];

    taille = 0;
    repertoire = opendir ( directory );
    if (!repertoire)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                "Preparer_synchro_directory: Directory %s unknown", directory );
       return(0);
     }

    while( (fichier = readdir( repertoire )) )
     { struct stat info;
       if (!strncmp( fichier->d_name, ".", 1 )) continue;

       g_snprintf( full_filename, sizeof(full_filename), "%s/%s", directory, fichier->d_name );
       stat( full_filename, &info );
       if ( client->ident.icone_version < info.st_mtime )
        { taille += info.st_size;
          client->Liste_file = g_slist_prepend( client->Liste_file, strdup(full_filename) );
        }
     }
    closedir( repertoire );
    return(taille);
  }
/**********************************************************************************************************/
/* Envoyer_synchro_directory_thread: Balance l'ensemble des fichiers de travail aux clients               */
/* Entrée: la liste en cours, le repertoire                                                               */
/* Sortie: taille globale                                                                                 */
/**********************************************************************************************************/
 void Envoyer_synchro_directory_thread ( struct CLIENT *client )
  { struct CMD_ENREG nbr;
    gint taille, icone_version;

    icone_version = Icone_get_data_version ();
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "Envoyer_synchro_directory_thread: Get local  icone_version = %d", icone_version );
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "Envoyer_synchro_directory_thread: Get remote icone_version = %d", client->ident.icone_version );

    if (client->ident.icone_version >= icone_version && icone_version != -1)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoyer_synchro_directory_thread: client already synchronized" );
     }
    else
     { client->Liste_file = NULL;
       taille = 0;
       taille += Preparer_synchro_directory ( client, "Gif" );
       taille += Preparer_synchro_directory ( client, "Son" );

       nbr.num = taille;
       g_snprintf( nbr.comment, sizeof(nbr.comment), "Synchronizing Files" );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
       Send_synchro_directory ( client, icone_version );
     }
    Unref_client( client ); 
  }
/*--------------------------------------------------------------------------------------------------------*/
