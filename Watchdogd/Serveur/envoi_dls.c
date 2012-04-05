/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_dls.c        Configuration du DLS de Watchdog v2.0                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 08 mar 2009 14:28:35 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_dls.c
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
 #include <string.h>
 #include <sys/stat.h>
 #include <sys/types.h>
 #include <fcntl.h>
 #include <unistd.h>
 #include <sys/file.h>                                            /* Gestion des verrous sur les fichiers */
 #include <sys/wait.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"

 #ifndef REP_INCLUDE_GLIB
 #define REP_INCLUDE_GLIB  "/usr/include/glib-2.0"
 #endif

/**********************************************************************************************************/
/* Proto_effacer_fichier_dls: Suppression du code d'un plugin avant reception du nouveau code client      */
/* Entrée: le client demandeur et l'id du fichier plugin                                                  */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_fichier_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_SOURCE_DLS *edit_dls )
  { gchar chaine[80];
    gint id_fichier;
    g_snprintf( chaine, sizeof(chaine), "%d.dls.new", edit_dls->id );
    unlink ( chaine );
    id_fichier = open( chaine, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    close(id_fichier);
    Info_c( Config.log, DEBUG_DLS, "Remise à zero", chaine );
  }
/**********************************************************************************************************/
/* Proto_effacer_plugin_dls: Destruction du plugin en parametre                                           */
/* Entrée: le client demandeur et le groupe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    pthread_mutex_lock( &Partage->com_dls.synchro );
    Partage->com_dls.liste_plugin_reset = g_list_append ( Partage->com_dls.liste_plugin_reset,
                                                          GINT_TO_POINTER(rezo_dls->id) );
    pthread_mutex_unlock( &Partage->com_dls.synchro );

    retour = Retirer_plugin_dlsDB( Config.log, Db_watchdog, rezo_dls );
    if (retour)
     { Envoi_client( client, TAG_DLS, SSTAG_SERVEUR_DEL_PLUGIN_DLS_OK,
                     (gchar *)rezo_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete plugin %s", rezo_dls->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_editer_msg: Le client desire editer un msg                                                       */
/* Entrée: le client demandeur et le msg en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls )
  { struct CMD_TYPE_PLUGIN_DLS *dls;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    dls = Rechercher_plugin_dlsDB( Config.log, Db_watchdog, rezo_dls->id );

    if (dls)
     { Envoi_client( client, TAG_DLS, SSTAG_SERVEUR_EDIT_PLUGIN_DLS_OK,
                  (gchar *)dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
       g_free(dls);                                                                 /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate plugin DLS %s", rezo_dls->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_msg: Le client valide l'edition d'un msg                                          */
/* Entrée: le client demandeur et le msg en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls )
  { struct CMD_TYPE_PLUGIN_DLS *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_plugin_dlsDB ( Config.log, Db_watchdog, rezo_dls );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit plugin DLS %s", rezo_dls->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_plugin_dlsDB( Config.log, Db_watchdog, rezo_dls->id );
           if (result) 
            { pthread_mutex_lock( &Partage->com_dls.synchro );
              Partage->com_dls.liste_plugin_reset =
                              g_list_append ( Partage->com_dls.liste_plugin_reset,
                                              GINT_TO_POINTER(result->id) );
              pthread_mutex_unlock( &Partage->com_dls.synchro );
                      
              Envoi_client( client, TAG_DLS, SSTAG_SERVEUR_VALIDE_EDIT_PLUGIN_DLS_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
              g_free(result);
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate plugin DLS %s", rezo_dls->nom);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Proto_editer_source_dls: Edition d'un programme DLS                                                    */
/* Entrée: le client demandeur et le groupe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_source_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls )
  { gchar chaine[80];

    client->transfert.buffer = g_malloc0( Config.taille_bloc_reseau );
    if (!client->transfert.buffer)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message), "Not enough memory" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    g_snprintf( chaine, sizeof(chaine), "%d.dls", rezo_dls->id );
    client->transfert.fd = open( chaine, O_RDONLY );
    if (client->transfert.fd < 0)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message), "Unable to open %s", chaine );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       g_free(client->transfert.buffer);
       return;
     }
    client->transfert.index = 0;
    lockf( client->transfert.fd, F_LOCK, 0 );                                  /* Verrouillage du fichier */
    ((struct CMD_TYPE_SOURCE_DLS *)client->transfert.buffer)->id = rezo_dls->id;
    Envoi_client( client, TAG_DLS, SSTAG_SERVEUR_EDIT_SOURCE_DLS_OK,
                  (gchar *)rezo_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
    Client_mode( client, ENVOI_SOURCE_DLS );
  }
/**********************************************************************************************************/
/* Proto_valider_source_dls: Le client nous envoie un prg DLS qu'il nous faudra compiler ensuite          */
/* Entrée: le client demandeur et le groupe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_source_dls( struct CLIENT *client, struct CMD_TYPE_SOURCE_DLS *edit_dls,
                                gchar *buffer )
  { gchar chaine[80];
    if (!client->id_creation_plugin_dls)
     { gint id_fichier;
       g_snprintf( chaine, sizeof(chaine), "%d.dls.new", edit_dls->id );
       id_fichier = open( chaine, O_WRONLY | O_APPEND, S_IRUSR | S_IWUSR );
       if (id_fichier<0 || lockf( id_fichier, F_TLOCK, 0 ) )
        { Info_n( Config.log, DEBUG_DLS, "Proto_valider_source_dls: append impossible", edit_dls->id );
          return;
        }
       lockf( id_fichier, F_LOCK, 0 );
       client->id_creation_plugin_dls = id_fichier;
     }
    
    write( client->id_creation_plugin_dls, buffer, edit_dls->taille );
  }
/**********************************************************************************************************/
/* Proto_compiler_source_dls: Compilation de la source DLS                                                */
/* Entrée: le client demandeur et le groupe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Compiler_source_dls( struct CLIENT *client, gint id )
  { struct CMD_GTK_MESSAGE erreur;
    gint index_buffer_erreur;
    gint retour;

    Info_n( Config.log, DEBUG_DLS, "THRCompil: Compiler_source_dls: Compilation module DLS", id );
    retour = Traduire_DLS( Config.log, (client ? TRUE : FALSE), id );
    Info_n( Config.log, DEBUG_DLS, "THRCompil: Compiler_source_dls: fin traduction", retour );

    memset ( &erreur, 0, sizeof(struct CMD_GTK_MESSAGE) );                     /* zeroing de la structure */
    if (retour == TRAD_DLS_ERROR_FILE && client)                  /* Retour de la traduction D.L.S vers C */
     { Info_n( Config.log, DEBUG_DLS,
               "THRCompil: Compiler_source_dls: envoi erreur file Traduction D.L.S", id );
       g_snprintf( erreur.message, sizeof(erreur.message), "Unable to open file for compilation ID %d", id );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR, (gchar *)&erreur, sizeof(erreur) );
     }

    index_buffer_erreur = 0;            /* On commence par remplir le début du buffer re remonté d'erreur */
    if ( (retour == TRAD_DLS_ERROR || retour == TRAD_DLS_WARNING) && client)
     { gint id_fichier;
       gchar log[20];

       Info_n( Config.log, DEBUG_DLS,
               "THRCompil: Compiler_source_dls: envoi erreur/warning Traduction D.L.S", id );
       g_snprintf( log, sizeof(log), "%d.log", id );

       id_fichier = open( log, O_RDONLY, 0 );
       if (id_fichier<0)
        { g_snprintf( erreur.message, sizeof(erreur.message), "Impossible d'ouvrir le\nfichier de log de compilation" );
          Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR, (gchar *)&erreur, sizeof(erreur) );
        }
       else { int nbr_car;
              nbr_car = 0; 
              while ( (nbr_car = read (id_fichier, erreur.message + index_buffer_erreur,
                                       sizeof(erreur.message)-1-index_buffer_erreur )) > 0 )
               { index_buffer_erreur+=nbr_car; }
              close(id_fichier);
            }
     }

    if (retour == TRAD_DLS_WARNING || retour == TRAD_DLS_OK)
     { gint pidgcc;
       pidgcc = fork();
       if (pidgcc<0)
        { struct CMD_GTK_MESSAGE erreur;
          Info_n( Config.log, DEBUG_DLS,
                  "THRCompilFils: Compiler_source_dls: envoi erreur Fork GCC", id );
          if (client)
           { g_snprintf( erreur.message, sizeof(erreur.message), "Gcc fork failed !" );
             Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(erreur) );
           }
        }
       else if (!pidgcc)
        { struct CMD_GTK_MESSAGE erreur;
          gchar source[80], cible[80];
          g_snprintf( source, sizeof(source), "%d.c", id );
          g_snprintf( cible,  sizeof(cible),  "libdls%d.so", id );
          Info( Config.log, DEBUG_DLS, "THRCompilFils: Proto_compiler_source_dls: GCC start !" );
          execlp( "gcc", "gcc", "-I", REP_INCLUDE_GLIB, "-shared", "-o3",
                  "-Wall", "-ldls", source, "-fPIC", "-o", cible, NULL );
          Info( Config.log, DEBUG_DLS, "THRCompilFils: Proto_compiler_source_dls: lancement GCC failed" );
          if (client)
           { g_snprintf( erreur.message, sizeof(erreur.message), "Lancement compilateur failed !" );
             Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_WARNING,
                            (gchar *)&erreur, sizeof(erreur) );
           }
          _exit(0);
        }

       Info_n( Config.log, DEBUG_DLS,
               "THRCompil: Proto_compiler_source_dls: Waiting for gcc to finish pid", pidgcc );
       wait4(pidgcc, NULL, 0, NULL );
       Info_n( Config.log, DEBUG_DLS,
               "THRCompil: Proto_compiler_source_dls: gcc is down, OK", pidgcc );


       pthread_mutex_lock( &Partage->com_dls.synchro );             /* Demande le reset du plugin à D.L.S */
       Partage->com_dls.liste_plugin_reset = g_list_append ( Partage->com_dls.liste_plugin_reset,
                                                             GINT_TO_POINTER(id) );
       pthread_mutex_unlock( &Partage->com_dls.synchro );

       if (client)
        { if (retour == TRAD_DLS_WARNING)
           { g_snprintf( erreur.message + index_buffer_erreur, sizeof(erreur.message) - index_buffer_erreur,
                         "\n -> Compilation OK with Warnings\nReset plugin OK" );
           }
          else
           { g_snprintf( erreur.message + index_buffer_erreur, sizeof(erreur.message) - index_buffer_erreur,
                         "\n -> Compilation OK\nReset plugin OK" );
           }
        }
     }
    if (client) Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_INFO,           /* Envoi du résultat */
                              (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );

    Info_n( Config.log, DEBUG_DLS, "THRCompil: Compiler_source_dls: terminé", id );
  }
/**********************************************************************************************************/
/* Proto_compiler_source_dls: Compilation de la source DLS                                                */
/* Entrée: le client demandeur et le groupe en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void *Proto_compiler_source_dls( struct CLIENT *client )
  { close( client->id_creation_plugin_dls );                               /* Fermeture du fichier plugin */
    client->id_creation_plugin_dls = 0;

    Compiler_source_dls ( client, client->dls.id );
    pthread_exit( NULL );
  }
/**********************************************************************************************************/
/* Proto_ajouter_groupe: Un client nous demande d'ajouter un groupe Watchdog                              */
/* Entrée: le groupe à créer                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls )
  { struct CMD_TYPE_PLUGIN_DLS *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_plugin_dlsDB ( Config.log, Db_watchdog, rezo_dls );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add plugin %s", rezo_dls->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_plugin_dlsDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to add plugin %s", rezo_dls->nom);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { gchar chaine[80];
              gint id_fichier;

              g_snprintf(chaine, sizeof(chaine), "%d.dls", result->id );
              id_fichier = open( chaine, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
              if (id_fichier == -1)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Unable to create file %s:\n", chaine );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { g_snprintf(chaine, sizeof(chaine), "/* %d.dls: %s */\n", result->id, result->nom );
                     write(id_fichier, chaine, strlen(chaine) );
                     close(id_fichier); 

                     Envoi_client( client, TAG_DLS, SSTAG_SERVEUR_ADD_PLUGIN_DLS_OK,      /* Tout va bien */
                                   (gchar *)result, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
                     g_free(result);
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_groupes: Envoi des groupes au client GID_USERS                                                 */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void *Envoyer_plugins_dls_thread_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_PLUGIN_DLS *dls;
    struct DB *db;
    
    prctl(PR_SET_NAME, "W-EnvoiDLS", 0, 0, 0 );

    db = Init_DB_SQL( Config.log );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_plugins_dlsDB( Config.log, db ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Libere_DB_SQL( Config.log, &db );
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d plugins", nbr.num );
    Envoi_client ( client, TAG_DLS, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { dls = Recuperer_plugins_dlsDB_suite( Config.log, db );
       if (!dls)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
       Envoi_client ( client, tag, sstag,
                      (gchar *)dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
       g_free(dls);
     }
  }
/**********************************************************************************************************/
/* Envoyer_plugins_dls_thread: Envoi la liste des plugin D.L.S au client                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_plugins_dls_thread ( struct CLIENT *client )
  { Envoyer_plugins_dls_thread_tag ( client, TAG_DLS, SSTAG_SERVEUR_ADDPROGRESS_PLUGIN_DLS,
                                                      SSTAG_SERVEUR_ADDPROGRESS_PLUGIN_DLS_FIN
                                   );
    pthread_exit ( NULL );
  }
/**********************************************************************************************************/
/* Envoyer_plugins_dls_thread: Envoi la liste des plugin D.L.S au client                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_plugins_dls_pour_mnemo_thread ( struct CLIENT *client )
  { Envoyer_plugins_dls_thread_tag ( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_ADDPROGRESS_DLS_FOR_MNEMO,
                                                             SSTAG_SERVEUR_ADDPROGRESS_DLS_FOR_MNEMO_FIN
                                   );
    pthread_exit ( NULL );
  }
/**********************************************************************************************************/
/* Envoyer_source_dls: Envoi d'un programme D.L.S                                                         */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 gboolean Envoyer_source_dls ( struct CLIENT *client )
  { gint taille, nbr_carac, taille_valide, taille_buffer;
    struct CMD_TYPE_SOURCE_DLS *edit_dls;
    gchar *buffer;
    gchar *test;

    if (!client->transfert.buffer) return(TRUE);
    if (client->transfert.fd<0) return(TRUE);
    edit_dls = (struct CMD_TYPE_SOURCE_DLS *)client->transfert.buffer;
    buffer = client->transfert.buffer + sizeof(struct CMD_TYPE_SOURCE_DLS);
    taille_buffer = Config.taille_bloc_reseau - sizeof(struct CMD_TYPE_SOURCE_DLS);

    taille = read ( client->transfert.fd,
                    buffer + client->transfert.index,
                    taille_buffer - client->transfert.index );
    if (taille<=0)                                                         /* Détection de fin de fichier */
     { g_free(client->transfert.buffer);
       client->transfert.buffer = NULL;
       close(client->transfert.fd);
       client->transfert.fd = 0;
       return(TRUE);
     }
    nbr_carac = g_utf8_strlen( buffer, taille );
    test = g_utf8_offset_to_pointer( buffer, nbr_carac );
    taille_valide = test - buffer;
    
    edit_dls->taille = taille_valide;
    Envoi_client ( client, TAG_DLS, SSTAG_SERVEUR_SOURCE_DLS,
                   (gchar *)client->transfert.buffer, taille_valide + sizeof(struct CMD_TYPE_SOURCE_DLS) );
    memcpy( buffer, buffer + taille_valide, taille-taille_valide );
    return(FALSE);
  }
/*--------------------------------------------------------------------------------------------------------*/
