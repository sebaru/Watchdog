/******************************************************************************************************************************/
/* Watchdogd/Archive/Archive.c  Gestion des archivages bit_internes Watchdog 2.0                                              */
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
/* Arch_clear_list: efface la liste des archives a prendre en compte                                                          */
/* Entrées: néant                                                                                                             */
/* Sortie : le nombre d'archive detruites                                                                                     */
/******************************************************************************************************************************/
 static gint Arch_Clear_list ( void )
  { JsonNode *arch;
    gint save_nbr;
    pthread_mutex_lock( &Partage->com_arch.synchro );                                                        /* lockage futex */
    save_nbr = Partage->com_arch.taille_arch;
    while ( Partage->com_arch.liste_arch )
     { arch = Partage->com_arch.liste_arch->data;                                                     /* Recuperation du arch */
       Json_node_unref(arch);                                                                           /* Libération mémoire */
       Partage->com_arch.liste_arch = g_slist_remove ( Partage->com_arch.liste_arch, arch );
       Partage->com_arch.taille_arch--;
     }
    pthread_mutex_unlock( &Partage->com_arch.synchro );
    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "%s: Clear %05d archive(s)", __func__, save_nbr );
    return(save_nbr);
 }
/******************************************************************************************************************************/
/* Ajouter_arch: Ajoute une archive dans la base de données                                                                   */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                                    */
/******************************************************************************************************************************/
 static void Ajouter_arch_all( gchar *tech_id, gchar *acronyme, gdouble valeur )
  { struct timeval tv;
    JsonNode *arch = Json_node_create ();
    if (!arch) return;

    gettimeofday( &tv, NULL );                                                                   /* On prend l'heure actuelle */
    Json_node_add_string ( arch, "tech_id",   tech_id );
    Json_node_add_string ( arch, "acronyme",  acronyme );
    Json_node_add_double ( arch, "valeur",    valeur );
    Json_node_add_int    ( arch, "date_sec",  tv.tv_sec );
    Json_node_add_int    ( arch, "date_usec", tv.tv_usec );

    pthread_mutex_lock( &Partage->com_arch.synchro );                                /* Ajout dans la liste de arch a traiter */
    Partage->com_arch.liste_arch = g_slist_append( Partage->com_arch.liste_arch, arch );
    Partage->com_arch.taille_arch++;
    pthread_mutex_unlock( &Partage->com_arch.synchro );
  }
/******************************************************************************************************************************/
/* Ajouter_arch: Ajoute une archive dans la base de données                                                                   */
/* Entrées: le type de bit, le numéro du bit, et sa valeur                                                                    */
/******************************************************************************************************************************/
 void Ajouter_arch( gchar *tech_id, gchar *acronyme, gdouble valeur )
  { static gint last_log = 0;

    if (Config.instance_is_master == FALSE) return;                                  /* Les instances Slave n'archivent pas ! */
    else if (Partage->com_arch.Thread_run == FALSE)                                      /* Si administratively DOWN, on sort */
     { if ( last_log + 600 < Partage->top )
        { Info_new( Config.log, Config.log_arch, LOG_INFO,
                   "%s: Thread is down. Dropping '%s:%s'=%f", __func__, tech_id, acronyme, valeur );
          last_log = Partage->top;
        }
       return;
     }
    Info_new( Config.log, Config.log_arch, LOG_DEBUG, "%s: Add Arch in list: '%s:%s'=%f", __func__, tech_id, acronyme, valeur );
    Ajouter_arch_all( tech_id, acronyme, valeur );
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du thread                                                                                        */
/******************************************************************************************************************************/
 void Run_arch ( void )
  { prctl(PR_SET_NAME, "W-Arch", 0, 0, 0 );

    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "Starting" );

    Partage->com_arch.liste_arch  = NULL;                                                     /* Initialisation des variables */
    Partage->com_arch.Thread_run  = TRUE;                                                               /* Le thread tourne ! */
    Partage->com_arch.taille_arch = 0;
    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );

    while(Partage->com_arch.Thread_run == TRUE)                                              /* On tourne tant que necessaire */
     { if (!Partage->com_arch.liste_arch)                                                     /* Si pas de message, on tourne */
        { sched_yield();
          sleep(2);
          continue;
        }

       Info_new( Config.log, Config.log_arch, LOG_DEBUG, "%s: Traitement de %05d archive(s)", __func__, Partage->com_arch.taille_arch );
       gint top            = Partage->top;
       gint nb_enreg       = 0;                                            /* Au début aucun enregistrement est passé a la DB */
       JsonNode *RootNode  = Json_node_create();
       JsonArray *archives = Json_node_add_array ( RootNode, "archives" );

       pthread_mutex_lock( &Partage->com_arch.synchro );                                                     /* lockage futex */
       GSList *liste = Partage->com_arch.liste_arch;
       while (liste && Partage->com_arch.Thread_run == TRUE && nb_enreg<500)
        { JsonNode *arch = liste->data;                                                               /* Recuperation du arch */
          json_node_ref ( arch );
          Json_array_add_element ( archives, arch );
          nb_enreg++;                        /* Permet de limiter a au plus 500 enregistrements histoire de limiter la famine */
          liste = g_slist_next(liste);
        }
       pthread_mutex_unlock( &Partage->com_arch.synchro );

       Json_node_add_int ( RootNode, "nbr_archives", nb_enreg );
       Info_new( Config.log, Config.log_arch, LOG_DEBUG, "%s: Sending %05d archive(s).", __func__, nb_enreg );

       JsonNode *api_result = Http_Post_to_global_API ( "/run/archive/save", RootNode );
       if (api_result && Json_get_int ( api_result, "api_status" ) == SOUP_STATUS_OK )
        { gint nbr_saved = Json_get_int ( api_result, "nbr_archives_saved" );

          pthread_mutex_lock( &Partage->com_arch.synchro );                                                  /* lockage futex */
          while (nbr_saved)
           { JsonNode *arch = Partage->com_arch.liste_arch->data;                                     /* Recuperation du arch */
             Partage->com_arch.liste_arch = g_slist_remove ( Partage->com_arch.liste_arch, arch );
             Partage->com_arch.taille_arch--;
             Json_node_unref ( arch );
             nbr_saved--;
           }
          pthread_mutex_unlock( &Partage->com_arch.synchro );

          Info_new( Config.log, Config.log_arch, LOG_INFO, "%s: Traitement de %05d archive(s) en %06.1fs. Reste %05d", __func__,
                    Json_get_int ( api_result, "nbr_archives_saved" ), (Partage->top-top)/10.0, Partage->com_arch.taille_arch );
        }
       else
        { Info_new( Config.log, Config.log_arch, LOG_ERR, "%s: API Error. Sleeping 5s before retrying. reste %05d", __func__,
                    Partage->com_arch.taille_arch );
          sleep(5);
        }

       Json_node_unref ( api_result );
       Json_node_unref ( RootNode );
     }

    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "%s: Cleaning Arch List before stop", __func__);
    Arch_Clear_list();                                              /* Suppression des enregistrements restants dans la liste */

    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "%s: Down (%p)", __func__, pthread_self() );
    Partage->com_arch.Thread_run  = FALSE;                                                              /* Le thread tourne ! */
    Partage->com_arch.TID = 0;                                                /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
