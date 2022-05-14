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
  { struct ARCHDB *arch;
    gint save_nbr;
    pthread_mutex_lock( &Partage->com_arch.synchro );                                                        /* lockage futex */
    save_nbr = Partage->com_arch.taille_arch;
    while ( Partage->com_arch.liste_arch )
     { arch = Partage->com_arch.liste_arch->data;                                                     /* Recuperation du arch */
       g_free(arch);                                                                                    /* Libération mémoire */
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
    struct ARCHDB *arch;

    arch = (struct ARCHDB *)g_try_malloc0( sizeof(struct ARCHDB) );
    if (!arch) return;

    gettimeofday( &tv, NULL );                                                                   /* On prend l'heure actuelle */
    g_snprintf( arch->tech_id,  sizeof(arch->tech_id),  "%s", tech_id );
    g_snprintf( arch->acronyme, sizeof(arch->acronyme), "%s", acronyme );
    arch->valeur    = valeur;
    arch->date_sec  = tv.tv_sec;
    arch->date_usec = tv.tv_usec;

    pthread_mutex_lock( &Partage->com_arch.synchro );                                /* Ajout dans la liste de arch a traiter */
    Partage->com_arch.liste_arch = g_slist_prepend( Partage->com_arch.liste_arch, arch );
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
    else if (Partage->com_arch.taille_arch > Partage->com_arch.buffer_size)
     { if ( last_log + 600 < Partage->top )
        { Info_new( Config.log, Config.log_arch, LOG_INFO,
                   "%s: DROP arch (taille>%d) '%s:%s'", __func__, Partage->com_arch.buffer_size, tech_id, acronyme );
          last_log = Partage->top;
        }
       return;
     }
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
  { gint top = 0, last_count = 0, nb_enreg = 0;
    static gpointer arch_request_number;
    prctl(PR_SET_NAME, "W-Arch", 0, 0, 0 );

    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "Starting" );

    Partage->com_arch.liste_arch  = NULL;                                                     /* Initialisation des variables */
    Partage->com_arch.Thread_run  = TRUE;                                                               /* Le thread tourne ! */
    Partage->com_arch.taille_arch = 0;
    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );

    Mnemo_auto_create_AI ( FALSE, "SYS", "ARCH_REQUEST_NUMBER", "Nb enregistrements dans le tampon d'archivage", "enreg." );
    Dls_data_set_AI ( "SYS", "ARCH_REQUEST_NUMBER", &arch_request_number, 0.0, TRUE );

reload:
    while(Partage->com_arch.Thread_run == TRUE && Partage->com_arch.Thread_reload == FALSE)  /* On tourne tant que necessaire */
     { struct ARCHDB *arch;

       if ( (Partage->top - last_count) >= 600 )                                                       /* Une fois par minute */
        { Dls_data_set_AI ( "SYS", "ARCH_REQUEST_NUMBER", &arch_request_number, 1.0*Partage->com_arch.taille_arch, TRUE );
          last_count=Partage->top;
        }

       if (!Partage->com_arch.liste_arch)                                                     /* Si pas de message, on tourne */
        { sched_yield();
          sleep(2);
          continue;
        }

       Info_new( Config.log, Config.log_arch, LOG_DEBUG, "%s: Traitement de %05d archive(s)", __func__, Partage->com_arch.taille_arch );
       top = Partage->top;
       nb_enreg = 0;                                                       /* Au début aucun enregistrement est passé a la DB */
       JsonNode *RootNode = Json_node_create();
       JsonArray *archives = Json_node_add_array ( RootNode, "archives" );

       GSList *liste = Partage->com_arch.liste_arch;
       pthread_mutex_lock( &Partage->com_arch.synchro );                                                     /* lockage futex */
       while (liste && Partage->com_arch.Thread_run == TRUE &&
              Partage->com_arch.Thread_reload == FALSE && nb_enreg<1000)
        { arch = liste->data;                                                                         /* Recuperation du arch */
          JsonNode *element = Json_node_create();
          Json_node_add_string ( element, "tech_id",   arch->tech_id );
          Json_node_add_string ( element, "acronyme",  arch->acronyme );
          Json_node_add_int    ( element, "date_sec",  arch->date_sec );
          Json_node_add_int    ( element, "date_usec", arch->date_usec );
          Json_node_add_double ( element, "valeur",    arch->valeur );
          Json_array_add_element ( archives, element );
          nb_enreg++;                       /* Permet de limiter a au plus 1000 enregistrements histoire de limiter la famine */
          liste = g_slist_next(liste);
        }
       pthread_mutex_unlock( &Partage->com_arch.synchro );

       JsonNode *api_result = Http_Post_to_global_API ( "/run/archive", "save", RootNode );
       if (api_result && Json_get_int ( api_result, "api_result" ) == SOUP_STATUS_OK )
        { while ( nb_enreg )
           { arch = Partage->com_arch.liste_arch->data;                                               /* Recuperation du arch */
             Partage->com_arch.liste_arch = g_slist_remove ( Partage->com_arch.liste_arch, arch );
             Partage->com_arch.taille_arch--;
             g_free(arch);
             nb_enreg--;
           }
        } else sleep(5);

       Json_node_unref ( api_result );
       Json_node_unref ( RootNode );
       Info_new( Config.log, Config.log_arch, LOG_INFO, "%s: Traitement de %05d archive(s) en %06.1fs. Reste %05d", __func__,
                 nb_enreg, (Partage->top-top)/10.0, Partage->com_arch.taille_arch );
     }

    if (Partage->com_arch.Thread_reload)                                                          /* On a recu reload ?? */
     { Info_new( Config.log, Config.log_arch, LOG_NOTICE, "%s: RELOAD", __func__ );
       pthread_mutex_lock( &Partage->com_arch.synchro );                                                  /* lockage futex */
       Info_new( Config.log, Config.log_arch, LOG_INFO, "%s: Reste %05d a traiter", __func__,
                 g_slist_length(Partage->com_arch.liste_arch) );
       pthread_mutex_unlock( &Partage->com_arch.synchro );
       Partage->com_arch.Thread_reload = FALSE;
       goto reload;
     }


    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "%s: Cleaning Arch List before stop", __func__);
    Arch_Clear_list();                                              /* Suppression des enregistrements restants dans la liste */

    Info_new( Config.log, Config.log_arch, LOG_NOTICE, "%s: Down (%p)", __func__, pthread_self() );
    Partage->com_arch.Thread_run  = FALSE;                                                              /* Le thread tourne ! */
    Partage->com_arch.TID = 0;                                                /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
