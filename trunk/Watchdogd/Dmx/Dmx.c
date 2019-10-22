/******************************************************************************************************************************/
/* Watchdogd/Dmx/Dmx.c  Gestion des modules MODBUS Watchdog 2.0                                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    22.10.2019 23:42:08 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Dmx.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sebastien Lefevre
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

 #include <stdio.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <errno.h>
 #include <sys/prctl.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <semaphore.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Dmx.h"

 static struct TRAME_DMX_REQUEST trame_dmx;                                                     /* Definition d'une trame DMX */

/******************************************************************************************************************************/
/* Dmx_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                    */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 gboolean Dmx_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_dmx.lib->Thread_debug = FALSE;                                                         /* Settings default parameters */
    Cfg_dmx.enable            = FALSE;
    g_snprintf( Cfg_dmx.tech_id, sizeof(Cfg_dmx.tech_id), "DMX" );
    g_snprintf( Cfg_dmx.device, sizeof(Cfg_dmx.device), "/dev/ttyUSB0" );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                                          /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_WARNING,
                "%s: Database connexion failed. Using Default Parameters", __func__ );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )                           /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_INFO, "%s: '%s' = %s", __func__, nom, valeur );   /* Print Config */
       if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_dmx.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_dmx.lib->Thread_debug = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "tech_id" ) )
        { g_snprintf( Cfg_dmx.tech_id, sizeof(Cfg_dmx.tech_id), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "device" ) )
        { g_snprintf( Cfg_dmx.device, sizeof(Cfg_dmx.device), "%s", valeur ); }
       else
        { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE,
                   "%s: Unknown Parameter '%s'(='%s') in Database", __func__, nom, valeur );
        }
     }
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Init_dmx: Initialisation de la ligne DMX                                                                                   */
/* Entrée: Néant.                                                                                                             */
/* Sortie: -1 si erreur, sinon, le FileDescriptor associé                                                                     */
/******************************************************************************************************************************/
 static void Init_dmx ( void )
  { Cfg_dmx.fd = open( Cfg_dmx.device, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (Cfg_dmx.fd<0)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Impossible d'ouvrir le device '%s', retour=%d (%s)", __func__,
                 Cfg_dmx.device, Cfg_dmx.fd, strerror(errno) );
       return;
     }
    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_INFO, "%s: Ouverture port dmx okay %s", __func__, Cfg_dmx.device );
  }
/**********************************************************************************************************/
/* Fermer_dmx: Fermeture de la ligne DMX                                                              */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Fermer_dmx ( void )
  { if ( Cfg_dmx.fd != -1 )
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_INFO, "%s: Fermeture device '%s' dmx okay", __func__, Cfg_dmx.device );
       close(Cfg_dmx.fd);
       Cfg_dmx.fd = -1;
     }
  }
#ifdef bouh
/******************************************************************************************************************************/
/* Dmx_do_mapping : mappe les entrees/sorties Wago avec la zone de mémoire interne dynamique                               */
/* Entrée : la structure referencant le module                                                                                */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Dmx_do_mapping ( struct MODULE_MODBUS *module )
  { gchar critere[80];
    struct DB *db;

    module->AI = g_try_malloc0( sizeof(gpointer) * module->nbr_entree_ana );
    if (!module->AI)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Memory Error for AI", __func__ );
       return;
     }
    module->DI = g_try_malloc0( sizeof(gpointer) * module->nbr_entree_tor );
    if (!module->DI)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Memory Error for DI", __func__ );
       return;
     }

    module->DO = g_try_malloc0( sizeof(gpointer) * module->nbr_sortie_tor );
    if (!module->DO)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Memory Error for DO", __func__ );
       return;
     }

/******************************* Recherche des event text EA a raccrocher aux bits internes ***********************************/
    g_snprintf( critere, sizeof(critere),"%s:AI%%", module->dmx.tech_id );
    if ( ! Recuperer_mnemos_AI_by_text ( &db, NOM_THREAD, critere ) )
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Error searching Database for '%s'", __func__, critere ); }
    else while ( Recuperer_mnemos_AI_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *map_text = db->row[2], *libelle = db->row[3];
       gchar debut[80];
       gint num;
       Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_INFO, "%s: Match found '%s' '%s:%s' - %s", __func__,
                 map_text, tech_id, acro, libelle );
       if ( sscanf ( map_text, "%[^:]:AI%d", debut, &num ) == 2 )                            /* Découpage de la ligne ev_text */
        { if (num<module->nbr_entree_ana)
           { Dls_data_get_AI ( tech_id, acro, &module->AI[num] );        /* bit déjà existant deja dans la structure DLS DATA */
             if(module->AI[num] == NULL) Dls_data_set_AI ( tech_id, acro, &module->AI[num], 0.0 );       /* Sinon, on le crée */
             Charger_conf_AI ( module->AI[num] );                                    /* Chargement de la conf AI depuis la DB */
           }
          else Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_WARNING, "%s: map '%s': num %d out of range '%d'", __func__,
                         map_text, num, module->nbr_entree_ana );
        }
       else Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: event '%s': Sscanf Error", __func__, map_text );
     }
/******************************* Recherche des event text EA a raccrocher aux bits internes ***********************************/
    g_snprintf( critere, sizeof(critere),"%s:DI%%", module->dmx.tech_id );
    if ( ! Recuperer_mnemos_DI_by_text ( &db, NOM_THREAD, critere ) )
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Error searching Database for '%s'", __func__, critere ); }
    else while ( Recuperer_mnemos_DI_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *libelle = db->row[3], *src_text = db->row[2];
       char debut[80];
       gint num;
       Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_INFO, "%s: Match found '%s' '%s:%s' - %s", __func__,
                 src_text, tech_id, acro, libelle );
       if ( sscanf ( src_text, "%[^:]:DI%d", debut, &num ) == 2 )                            /* Découpage de la ligne ev_text */
        { if (num<module->nbr_entree_tor)
           { Dls_data_get_bool ( tech_id, acro, &module->DI[num] );      /* bit déjà existant deja dans la structure DLS DATA */
             if(module->DI[num] == NULL) Dls_data_set_bool ( tech_id, acro, &module->DI[num], FALSE );   /* Sinon, on le crée */
           }
          else Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_WARNING, "%s: map '%s': num %d out of range '%d'", __func__,
                         src_text, num, module->nbr_entree_tor );
        }
       else Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: event '%s': Sscanf Error", __func__, src_text );
     }
/*********************************** Recherche des events DO a raccrocher aux bits internes ***********************************/
    g_snprintf( critere, sizeof(critere),"%s:DO%%", module->dmx.tech_id );
    if ( ! Recuperer_mnemos_DO_by_tag ( &db, NOM_THREAD, critere ) )
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Error searching Database for '%s'", __func__, critere ); }
    else while ( Recuperer_mnemos_DO_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *libelle = db->row[3], *dst_tag = db->row[2];
       char debut[80];
       gint num;
       Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_INFO, "%s: Match found '%s' '%s:%s' - %s", __func__,
                 dst_tag, tech_id, acro, libelle );
       if ( sscanf ( dst_tag, "%[^:]:DO%d", debut, &num ) == 2 )                      /* Découpage de la ligne ev_text */
        { if (num<module->nbr_sortie_tor)
           { Dls_data_get_bool ( tech_id, acro, &module->DO[num] );      /* bit déjà existant deja dans la structure DLS DATA */
             if(module->DO[num] == NULL) Dls_data_set_bool ( tech_id, acro, &module->DO[num], FALSE );   /* Sinon, on le crée */
           }
          else Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_WARNING, "%s: map '%s': num %d out of range '%d'", __func__,
                         dst_tag, num, module->nbr_entree_tor );
        }
       else Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: event '%s': Sscanf Error", __func__, dst_tag );
     }
/******************************* Recherche des event text EA a raccrocher aux bits internes ***********************************/
    Dls_data_set_bool ( module->dmx.tech_id, "COMM", &module->bit_comm, FALSE );

    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Module '%s' : mapping done", __func__,
              module->dmx.description );
  }
#endif
/******************************************************************************************************************************/
/* Envoyer_trame_dmx_request: envoie une trame DMX au token USB                                                               */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static void Envoyer_trame_dmx_request( void )
  { trame_dmx.start_delimiter = 0x7E;
    trame_dmx.label = DMX_Output_Only_Send_DMX_Packet_Request;
/*  trame_dmx.length_lsb =  sizeof(trame_dmx.data) & 0xFF;
    trame_dmx.length_msb = (sizeof(trame_dmx.data) & 0xFF00) >> 8; */
    trame_dmx.length_lsb = sizeof(trame_dmx.data); /* Spécifiquement pour une taille_data < 256 */
    trame_dmx.length_msb = 0;
    trame_dmx.data[0] = 0;          /* Start Code DMX = 0 */
    trame_dmx.end_delimiter = 0xE7; /* End delimiter */
    if ( write( Cfg_dmx.fd, &trame_dmx, sizeof(struct TRAME_DMX_REQUEST) ) == -1)                     /* Ecriture de la trame */
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_WARNING, "%s: Write Trame Error '%s'", __func__, strerror(errno) ); }
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du MODBUS                                                                                        */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { prctl(PR_SET_NAME, "W-DMX", 0, 0, 0 );
    memset( &Cfg_dmx, 0, sizeof(Cfg_dmx) );                                         /* Mise a zero de la structure de travail */
    Cfg_dmx.lib = lib;                                             /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_dmx.lib->TID = pthread_self();                                                      /* Sauvegarde du TID pour le pere */
reload:
    Dmx_Lire_config ();                                                     /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );
    Cfg_dmx.lib->Thread_run = TRUE;                                                                     /* Le thread tourne ! */

    g_snprintf( Cfg_dmx.lib->admin_prompt, sizeof(Cfg_dmx.lib->admin_prompt), "dmx" );
    g_snprintf( Cfg_dmx.lib->admin_help,   sizeof(Cfg_dmx.lib->admin_help),   "Manage Dmx system" );

    if (!Cfg_dmx.enable)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE,
                "%s: Thread is not enabled in config. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }

    Init_dmx();
    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (Cfg_dmx.fd == -1)
        { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Acces DMX failed, sleeping 10s before retrying.", __func__);
          sleep(10);
          lib->Thread_reload = TRUE;
          break;
        }
       Envoyer_trame_dmx_request();
       trame_dmx.data[1]++; /* Pour test */
     }                                                                     /* Fin du while partage->arret */

    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Preparing to stop . . . TID = %p", __func__, pthread_self() );
    Fermer_dmx();
    if (lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
end:
    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    Cfg_dmx.lib->Thread_run = FALSE;                                                            /* Le thread ne tourne plus ! */
    Cfg_dmx.lib->TID = 0;                                                     /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
