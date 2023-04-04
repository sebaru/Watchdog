/******************************************************************************************************************************/
/* Watchdogd/Archive.c  Gestion des archivages des bit_internes Watchdog                                                      */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         mer. 09 mai 2012 12:44:56 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Archive.c
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
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <unistd.h>
 #include <locale.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */

/******************************************************************************************************************************/
/* Ajouter_arch: Ajoute une archive dans la base de données                                                                   */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                                    */
/******************************************************************************************************************************/
 void Ajouter_arch( gchar *tech_id, gchar *acronyme, gdouble valeur )
  { if (Config.instance_is_master == FALSE) return;                                  /* Les instances Slave n'archivent pas ! */
    if (Partage->com_msrv.Thread_run  == FALSE) return;

    Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Add Arch in list: '%s:%s'=%f", tech_id, acronyme, valeur );
    struct timeval tv;
    JsonNode *arch = Json_node_create ();
    if (!arch) return;

    gettimeofday( &tv, NULL );                                                                   /* On prend l'heure actuelle */
    Json_node_add_string ( arch, "tech_id",   tech_id );
    Json_node_add_string ( arch, "acronyme",  acronyme );
    Json_node_add_double ( arch, "valeur",    valeur );
    Json_node_add_int    ( arch, "date_sec",  tv.tv_sec );
    Json_node_add_int    ( arch, "date_usec", tv.tv_usec );

    pthread_mutex_lock( &Partage->archive_liste_sync );                                /* Ajout dans la liste de arch a traiter */
    Partage->archive_liste = g_slist_append( Partage->archive_liste, arch );
    Partage->archive_liste_taille++;
    pthread_mutex_unlock( &Partage->archive_liste_sync );
  }
/******************************************************************************************************************************/
/* Arch_clear_list: efface la liste des archives a prendre en compte                                                          */
/* Entrées: néant                                                                                                             */
/* Sortie : le nombre d'archive detruites                                                                                     */
/******************************************************************************************************************************/
 static void ARCH_Clear ( void )
  { pthread_mutex_lock( &Partage->archive_liste_sync );                                                      /* lockage futex */
    gint save_nbr = Partage->archive_liste_taille;
    g_slist_foreach ( Partage->archive_liste, (GFunc) Json_node_unref, NULL );
    pthread_mutex_unlock( &Partage->archive_liste_sync );
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Clear %05d archive(s)", save_nbr );
 }
/******************************************************************************************************************************/
/* Run_arch_sync: Envoi les archives a l'API                                                                                  */
/* Entrée/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Run_arch_sync ( void )
  { prctl(PR_SET_NAME, "W-ARCHSYNC", 0, 0, 0 );

    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Demarrage . . . TID = %p", pthread_self() );

    gint cpt_1_minute = Partage->top + 600;
    gint max_enreg = ARCHIVE_MAX_ENREG_TO_API;
    while(Partage->com_msrv.Thread_run == TRUE)                                              /* On tourne tant que necessaire */
     { if (cpt_1_minute < Partage->top)                                                      /* Sauvegarde toutes les minutes */
        { Ajouter_arch ( "SYS", "ARCHIVE_LIST_SIZE", 1.0*Partage->archive_liste_taille );
          cpt_1_minute += 600;
        }

       if (!Partage->archive_liste) { sleep(2); continue; }
       Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Begin %05d archive(s)", Partage->archive_liste_taille );
       gint top            = Partage->top;
       gint nb_enreg       = 0;                                            /* Au début aucun enregistrement est passé a la DB */
       JsonNode *RootNode  = Json_node_create();
       JsonArray *archives = Json_node_add_array ( RootNode, "archives" );

       pthread_mutex_lock( &Partage->archive_liste_sync );                                                   /* lockage futex */
       GSList *liste = Partage->archive_liste;
       while (liste && Partage->com_msrv.Thread_run == TRUE && nb_enreg<max_enreg)
        { JsonNode *arch = liste->data;                                                               /* Recuperation du arch */
          json_node_ref ( arch );
          Json_array_add_element ( archives, arch );
          nb_enreg++;                        /* Permet de limiter a au plus 500 enregistrements histoire de limiter la famine */
          liste = g_slist_next(liste);
        }
       pthread_mutex_unlock( &Partage->archive_liste_sync );

       Json_node_add_int ( RootNode, "nbr_archives", nb_enreg );
       Info_new( __func__, Config.log_msrv, LOG_DEBUG, "Sending %05d archive(s).", nb_enreg );

       JsonNode *api_result = Http_Post_to_global_API ( "/run/archive/save", RootNode );
       if (api_result && Json_get_int ( api_result, "api_status" ) == SOUP_STATUS_OK )
        { gint nbr_saved = Json_get_int ( api_result, "nbr_archives_saved" );

          pthread_mutex_lock( &Partage->archive_liste_sync );                                                   /* lockage futex */
          while (nbr_saved)
           { JsonNode *arch = Partage->archive_liste->data;                                              /* Recuperation du arch */
             Partage->archive_liste = g_slist_remove ( Partage->archive_liste, arch );
             Partage->archive_liste_taille--;
             Json_node_unref ( arch );
             nbr_saved--;
           }
          pthread_mutex_unlock( &Partage->archive_liste_sync );

          Info_new( __func__, Config.log_msrv, LOG_INFO, "Traitement de %05d archive(s) en %06.1fs (max %d). Reste %05d",
                    Json_get_int ( api_result, "nbr_archives_saved" ), (Partage->top-top)/10.0, max_enreg, Partage->archive_liste_taille );
          max_enreg = max_enreg + 50;
          if (max_enreg>ARCHIVE_MAX_ENREG_TO_API) max_enreg = ARCHIVE_MAX_ENREG_TO_API;
        }
       else
        { max_enreg = 10;
          Info_new( __func__, Config.log_msrv, LOG_ERR, "API Error when sending %d enregs. Reste %05d. Reduce max_enreg to %d.",
                    nb_enreg, Partage->archive_liste_taille, max_enreg );
        }
       Json_node_unref ( api_result );
       Json_node_unref ( RootNode );
       Dls_data_set_AI ( NULL, Partage->com_dls.sys_nbr_archive_queue, 1.0*Partage->archive_liste_taille, TRUE );
     }

    ARCH_Clear();                                                   /* Suppression des enregistrements restants dans la liste */
    Info_new( __func__, Config.log_msrv, LOG_NOTICE, "Down (%p)", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
