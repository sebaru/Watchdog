/******************************************************************************************************************************/
/* Watchdogd/Serveur/envoi_dls.c        Configuration du DLS de Watchdog v2.0                                                 */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           dim 08 mar 2009 14:28:35 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * envoi_dls.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
 #include <sys/file.h>                                                                /* Gestion des verrous sur les fichiers */
 #include <sys/wait.h>

/****************************************************** Prototypes de fonctions ***********************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
 extern struct SSRV_CONFIG Cfg_ssrv;
/******************************************************************************************************************************/
/* Proto_effacer_fichier_dls: Suppression du code d'un plugin avant reception du nouveau code client                          */
/* Entrée: le client demandeur et l'id du fichier plugin                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_effacer_source_dls ( struct CLIENT *client, struct CMD_TYPE_SOURCE_DLS *edit_dls )
  { gchar chaine[80];
    g_snprintf( chaine, sizeof(chaine), "Dls/%s.dls.new", edit_dls->tech_id );
    unlink ( chaine );
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG, "Proto_effacer_fichier_plugin_dls : Zeroing... %s", chaine );
    client->Source_DLS_new = NULL;                             /* On prépare les tampons a la reception du nouveau source DLS */
    client->taille_Source_DLS_new = 0;
  }
/******************************************************************************************************************************/
/* Proto_effacer_plugin_dls: Destruction du plugin en parametre                                                               */
/* Entrée: le client demandeur et le groupe en question                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_effacer_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls )
  { gboolean retour;

    retour = Retirer_plugin_dlsDB( rezo_dls );
    if (retour)
     { Envoi_client( client, TAG_DLS, SSTAG_SERVEUR_DEL_PLUGIN_DLS_OK,
                     (gchar *)rezo_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
       /*Decharger_plugin_by_id ( rezo_dls->id );*/
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete plugin %s", rezo_dls->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Proto_editer_msg: Le client desire editer un msg                                                                           */
/* Entrée: le client demandeur et le msg en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_editer_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls )
  { struct CMD_TYPE_PLUGIN_DLS *dls;

    dls = Rechercher_plugin_dlsDB( rezo_dls->tech_id );

    if (dls)
     { Envoi_client( client, TAG_DLS, SSTAG_SERVEUR_EDIT_PLUGIN_DLS_OK,
                  (gchar *)dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
       g_free(dls);                                                                                     /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate plugin DLS %s", rezo_dls->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Proto_valider_editer_msg: Le client valide l'edition d'un msg                                                              */
/* Entrée: le client demandeur et le msg en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_valider_editer_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls )
  { struct CMD_TYPE_PLUGIN_DLS *result;
    gboolean retour;

    retour = Modifier_plugin_dlsDB ( rezo_dls );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit plugin DLS %s", rezo_dls->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_plugin_dlsDB( rezo_dls->tech_id );
           if (result)
            { Partage->com_dls.Thread_reload = TRUE;
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
/******************************************************************************************************************************/
/* Proto_editer_source_dls: Edition d'un programme DLS                                                                        */
/* Entrée: le client demandeur et le groupe en question                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_editer_source_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls )
  { struct CMD_TYPE_SOURCE_DLS *source_dls;
    gint taille_source, taille_sent, taille_max_write;
    gchar *buffer_all, *Source, *buffer_write;

    if ( Get_source_dls_from_DB ( rezo_dls->tech_id, &Source, &taille_source ) == FALSE )
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                "%s: Cannot get Source DLS for id '%d'", __func__, rezo_dls->id );
       return;
     }

    Envoi_client( client, TAG_DLS, SSTAG_SERVEUR_SOURCE_DLS_START,
                  (gchar *)rezo_dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );

    buffer_all = g_try_malloc0 ( Cfg_ssrv.taille_bloc_reseau );
    if (!buffer_all)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message), "Memory Error" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "%s: Memory alloc error", __func__ );
       g_free(Source);
       return;
     }
                                                                     /* Utilisation du debut du buffer pour stocker structure */
    source_dls = (struct CMD_TYPE_SOURCE_DLS *)buffer_all;
    source_dls->id = rezo_dls->id;
    g_snprintf(source_dls->tech_id, sizeof(source_dls->tech_id), "%s", rezo_dls->tech_id );
    buffer_write = buffer_all + sizeof(struct CMD_TYPE_SOURCE_DLS);                          /* Index data dans le buffer_all */
    taille_max_write = Cfg_ssrv.taille_bloc_reseau - sizeof(struct CMD_TYPE_SOURCE_DLS);
    taille_sent = 0;

    while ( taille_sent < taille_source )                                                   /* On envoie le fichier au client */
     { gint taille;
       if (taille_source - taille_sent > taille_max_write)
            { taille = taille_max_write; }
       else { taille = taille_source - taille_sent; }

       memcpy ( buffer_write, Source + taille_sent, taille);                                    /* Index data dans buffer_all */

       source_dls->taille = taille;
       Envoi_client ( client, TAG_DLS, SSTAG_SERVEUR_SOURCE_DLS,
                      buffer_all, taille + sizeof(struct CMD_TYPE_SOURCE_DLS) );
       taille_sent+=taille;
     }
    Envoi_client ( client, TAG_DLS, SSTAG_SERVEUR_SOURCE_DLS_END, buffer_all, sizeof(struct CMD_TYPE_SOURCE_DLS) );
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE, "%s: Source DLS %05d sent", __func__, rezo_dls->id );
    g_free(buffer_all);
    g_free(Source);
  }
/******************************************************************************************************************************/
/* Proto_valider_source_dls: Le client nous envoie un prg DLS qu'il nous faudra compiler ensuite                              */
/* Entrée: le client demandeur et le source D.L.S a sauvgarder                                                                */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_valider_source_dls( struct CLIENT *client, struct CMD_TYPE_SOURCE_DLS *edit_dls, gchar *buffer )
  { guint new_taille;

    new_taille = client->taille_Source_DLS_new + edit_dls->taille;
    client->Source_DLS_new = g_try_realloc ( client->Source_DLS_new, new_taille );               /* Nouvelle taille de buffer */
    memcpy ( client->Source_DLS_new + client->taille_Source_DLS_new, buffer, edit_dls->taille ); /* Recopie du bout de buffer */
    client->taille_Source_DLS_new = new_taille;
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Proto_valider_source_dls: %d bytes received for plugin '%s'. New length=%d bytes",
              edit_dls->taille, edit_dls->tech_id, client->taille_Source_DLS_new );
  }
/******************************************************************************************************************************/
/* Proto_compiler_source_dls: Compilation de la source DLS                                                                    */
/* Entrée: le client demandeur et le groupe en question                                                                       */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void *Proto_compiler_source_dls_thread( struct CLIENT *client )
  { struct CMD_TYPE_PLUGIN_DLS *result;
    struct CMD_GTK_MESSAGE erreur;
    /*prctl(PR_SET_NAME, "W-Trad.DLS", 0, 0, 0 );*/
    switch ( Compiler_source_dls ( TRUE, client->dls.tech_id, erreur.message, sizeof(erreur.message) ) )
     { case DLS_COMPIL_ERROR_LOAD_SOURCE:
            g_snprintf( erreur.message, sizeof(erreur.message),
                       "Unable to open file for compilation '%s'", client->dls.tech_id );
            break;
       case DLS_COMPIL_ERROR_LOAD_LOG:
            g_snprintf( erreur.message, sizeof(erreur.message),
                       "Unable to open log file for '%s'", client->dls.tech_id );
            break;
       case DLS_COMPIL_OK_WITH_WARNINGS:
            Envoi_client ( client, TAG_DLS, SSTAG_SERVEUR_WARNING,
                           (gchar *)&erreur, sizeof(erreur) );
            break;
       case DLS_COMPIL_SYNTAX_ERROR:
            Envoi_client ( client, TAG_DLS, SSTAG_SERVEUR_ERREUR,
                           (gchar *)&erreur, sizeof(erreur) );
            break;
       case DLS_COMPIL_ERROR_FORK_GCC:
            g_snprintf( erreur.message, sizeof(erreur.message), "Gcc fork failed !" );
            break;
       case DLS_COMPIL_OK:
            g_snprintf( erreur.message, sizeof(erreur.message),
                      "-- No error --\n-- Reset plugin OK --" );
            break;
       default : g_snprintf( erreur.message, sizeof(erreur.message), "Unknown Error !");
     }
    Envoi_client ( client, TAG_DLS, SSTAG_SERVEUR_DLS_COMPIL_STATUS, (gchar *)&erreur, sizeof(erreur) );

    result = Rechercher_plugin_dlsDB( client->dls.tech_id );                /* Mise a jour de l'onglet "plugin" en temps reel */
    if (result)
     { Envoi_client( client, TAG_DLS, SSTAG_SERVEUR_VALIDE_EDIT_PLUGIN_DLS_OK,
                     (gchar *)result, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
       g_free(result);
     }

    Unref_client ( client );                                                           /* Plus besoin de la structure cliente */
    pthread_exit( NULL );
  }
/******************************************************************************************************************************/
/* Proto_ajouter_groupe: Un client nous demande d'ajouter un groupe Watchdog                                                  */
/* Entrée: le groupe à créer                                                                                                  */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_ajouter_plugin_dls ( struct CLIENT *client, struct CMD_TYPE_PLUGIN_DLS *rezo_dls )
  { struct CMD_TYPE_PLUGIN_DLS *result;
    gint id;

    id = Ajouter_plugin_dlsDB ( rezo_dls );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add plugin %s", rezo_dls->nom);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_plugin_dlsDB( rezo_dls->tech_id );
           if (!result)
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to add plugin %s", rezo_dls->nom);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_DLS, SSTAG_SERVEUR_ADD_PLUGIN_DLS_OK,                          /* Tout va bien */
                            (gchar *)result, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
              Partage->com_dls.Thread_reload = TRUE;
              g_free(result);
            }
         }
  }
/******************************************************************************************************************************/
/* Envoyer_groupes: Envoi des groupes au client GID_USERS                                                                     */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void *Envoyer_plugins_dls_thread_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_PLUGIN_DLS *dls;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiDLS", 0, 0, 0 );

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
              "Envoyer_plugins_dls_thread_tag: Starting (TAG=%d, SSTAG=%d, SSTAGFIN=%d)",
               tag, sstag, sstag_fin );

    if ( ! Recuperer_plugins_dlsDB( &db ) )
     { return(NULL);
     }                                                                                               /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d plugins", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { dls = Recuperer_plugins_dlsDB_suite( &db );
       if (!dls)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                   "Envoyer_plugins_dls_thread_tag: End (TAG=%d, SSTAG=%d, SSTAGFIN=%d)",
                    tag, sstag, sstag_fin );
          return(NULL);
        }

       Envoi_client ( client, tag, sstag,
                      (gchar *)dls, sizeof(struct CMD_TYPE_PLUGIN_DLS) );
       g_free(dls);
     }
  }
/******************************************************************************************************************************/
/* Envoyer_plugins_dls_thread: Envoi la liste des plugin D.L.S au client                                                      */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void *Envoyer_plugins_dls_thread ( struct CLIENT *client )
  { Envoyer_plugins_dls_thread_tag ( client, TAG_DLS, SSTAG_SERVEUR_ADDPROGRESS_PLUGIN_DLS,
                                                      SSTAG_SERVEUR_ADDPROGRESS_PLUGIN_DLS_FIN
                                   );
    Unref_client( client );                                                               /* Déréférence la structure cliente */
    pthread_exit ( NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
