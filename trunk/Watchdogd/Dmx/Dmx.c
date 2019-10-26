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

/******************************************************************************************************************************/
/* Dmx_Lire_config : Lit la config Watchdog et rempli la structure mémoire                                                    */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                                       */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static gboolean Dmx_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_dmx.lib->Thread_debug = FALSE;                                                         /* Settings default parameters */
    Cfg_dmx.enable            = FALSE;
    g_snprintf( Cfg_dmx.tech_id, sizeof(Cfg_dmx.tech_id), "DMX01" );
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
/* Dmx_send_status_to_master: Envoie le bit de comm au master selon le status du port DMX                                     */
/* Entrée: le status du DMX                                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dmx_send_status_to_master ( gboolean status )
  { JsonBuilder *builder;
    gchar *result;
    gsize taille;
    builder = Json_create ();
    json_builder_begin_object ( builder );
    Json_add_string ( builder, "tech_id",  Cfg_dmx.tech_id );
    Json_add_string ( builder, "acronyme", "COMM" );
    Json_add_bool   ( builder, "etat", status );
    json_builder_end_object ( builder );
    result = Json_get_buf ( builder, &taille );
    Send_zmq_with_tag ( Cfg_dmx.zmq_to_master, NULL, NOM_THREAD, "*", "msrv", "SET_BOOL", result, taille );
    g_free(result);
    Cfg_dmx.comm_status = status;
  }
/******************************************************************************************************************************/
/* Dmx_do_mapping : mappe les entrees/sorties Wago avec la zone de mémoire interne dynamique                                  */
/* Entrée : la structure referencant le module                                                                                */
/* Sortie : rien                                                                                                              */
/******************************************************************************************************************************/
 static void Dmx_do_mapping ( void )
  { gchar critere[80];
    struct DB *db;
    gint cpt;

/******************************* Recherche des event text EA a raccrocher aux bits internes ***********************************/
    cpt = 0;
    g_snprintf( critere, sizeof(critere),"%s:AO%%", Cfg_dmx.tech_id );
    if ( ! Recuperer_mnemos_AO_by_text ( &db, NOM_THREAD, critere ) )
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Error searching Database for '%s'", __func__, critere ); }
    else while ( Recuperer_mnemos_AO_suite( &db ) )
     { gchar *tech_id = db->row[0], *acro = db->row[1], *map_text = db->row[2], *libelle = db->row[3];
       gchar *min = db->row[4], *max = db->row[5], *type=db->row[5], *valeur = db->row[6];
       gchar debut[80];
       gint num;
       Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_INFO, "%s: Match found '%s' '%s:%s' - %s", __func__,
                 map_text, tech_id, acro, libelle );
       if ( sscanf ( map_text, "%[^:]:AO%d", debut, &num ) == 2 )                            /* Découpage de la ligne ev_text */
        { if (num<=24)
           { g_snprintf( Cfg_dmx.Canal[num].tech_id, sizeof(Cfg_dmx.Canal[num].tech_id), "%s", tech_id );
             g_snprintf( Cfg_dmx.Canal[num].acronyme, sizeof(Cfg_dmx.Canal[num].acronyme), "%s", acro );
             Cfg_dmx.Canal[num].min  = atof(min);
             Cfg_dmx.Canal[num].max  = atof(max);
             Cfg_dmx.Canal[num].type = atoi(type);
             Cfg_dmx.Canal[num].val_avant_ech = atof(valeur);
             Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_INFO, "%s: AO '%s:%s' loaded", __func__, tech_id, acro );
           }
          else Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_WARNING, "%s: map '%s': num %d out of range '%d'", __func__,
                         map_text, num, 24 );
        }
       else Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: event '%s': Sscanf Error", __func__, map_text );
     }
    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_INFO, "%s: %d AO loaded", __func__, cpt );

    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: DMX '%s' : mapping done", __func__, Cfg_dmx.tech_id );
  }
/******************************************************************************************************************************/
/* Dmx_init: Initialisation de la ligne DMX                                                                                   */
/* Entrée: Néant.                                                                                                             */
/* Sortie: -1 si erreur, sinon, le FileDescriptor associé                                                                     */
/******************************************************************************************************************************/
 static void Dmx_init ( void )
  { Cfg_dmx.fd = open( Cfg_dmx.device, O_WRONLY | O_NOCTTY /*| O_NONBLOCK*/ );
    if (Cfg_dmx.fd<0)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Impossible d'ouvrir le device '%s', retour=%d (%s)", __func__,
                 Cfg_dmx.device, Cfg_dmx.fd, strerror(errno) );
       return;
     }
    Cfg_dmx.Canal = g_try_malloc0( sizeof(struct DLS_AO) * 24 );                                  /* 24 Canaux pour commencer */
    if (!Cfg_dmx.Canal)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Memory Error for Canal", __func__ );
       close(Cfg_dmx.fd); Cfg_dmx.fd=-1;
       return;
     }
    Cfg_dmx.taille_trame_dmx = sizeof(struct TRAME_DMX_PRE) + 24 + sizeof(struct TRAME_DMX_POST);
    Cfg_dmx.Trame_dmx = g_try_malloc0( Cfg_dmx.taille_trame_dmx );      /* 24 Canaux */
    if (!Cfg_dmx.Trame_dmx)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Memory Error for Trame_DMX", __func__ );
       g_free(Cfg_dmx.Canal); Cfg_dmx.Canal = NULL;
       close(Cfg_dmx.fd);     Cfg_dmx.fd=-1;
       return;
     }
    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Ouverture port dmx okay %s", __func__, Cfg_dmx.device );
    Dmx_send_status_to_master( TRUE );
    Dmx_do_mapping();
  }
/******************************************************************************************************************************/
/* Dmx_close: Fermeture de la ligne DMX                                                                                       */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dmx_close ( void )
  { if ( Cfg_dmx.fd != -1 )
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Fermeture device '%s' dmx okay", __func__, Cfg_dmx.device );
       close(Cfg_dmx.fd);
       Cfg_dmx.fd = -1;
     }
    if (Cfg_dmx.Canal) { g_free(Cfg_dmx.Canal); Cfg_dmx.Canal = NULL; }
    Dmx_send_status_to_master( FALSE );
  }
/******************************************************************************************************************************/
/* Envoyer_trame_dmx_request: envoie une trame DMX au token USB                                                               */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static gboolean Envoyer_trame_dmx_request( void )
  { Cfg_dmx.Trame_dmx[0] = 0x7E;
    Cfg_dmx.Trame_dmx[1] = DMX_Output_Only_Send_DMX_Packet_Request;
/*  trame_dmx.length_lsb =  sizeof(trame_dmx.data) & 0xFF;
    trame_dmx.length_msb = (sizeof(trame_dmx.data) & 0xFF00) >> 8; */
    Cfg_dmx.Trame_dmx[2] = 24 + 1; /* Spécifiquement pour une taille_data < 256 */
    Cfg_dmx.Trame_dmx[3] = 0;
    Cfg_dmx.Trame_dmx[4] = 0; /* Start Code DMX = 0 */
    Cfg_dmx.Trame_dmx[5] ++; /* Pour test */
    Cfg_dmx.Trame_dmx[29] = 0xE7; /* End delimiter */
    if ( write( Cfg_dmx.fd, Cfg_dmx.Trame_dmx, Cfg_dmx.taille_trame_dmx ) != Cfg_dmx.taille_trame_dmx )/* Ecriture de la trame */
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Write Trame Error '%s'", __func__, strerror(errno) );
       return(FALSE);
     }
    Cfg_dmx.nbr_request++;
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Dmx_updater_DB: Met a jour les valeurs des Entrées/Sorties dans la base de données                                         */
/* Entrée: néant                                                                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static void Dmx_Updater_DB ( void )
  { gchar requete[200];
    struct DB *db;
    gint cpt = 0;

    if (!Cfg_dmx.Canal)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Aucun bit à sauver", __func__ );
       return;
     }

    db = Init_DB_SQL();
    if (!db)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Connexion DB impossible", __func__ );
       return;
     }

    for (cpt=0; cpt<24; cpt++)
     { g_snprintf( requete, sizeof(requete),                                                                   /* Requete SQL */
                   "UPDATE mnemos_AO as m SET valeur='%f' "
                   "WHERE m.tech_id='%s' AND m.acronyme='%s';",
                   Cfg_dmx.Canal[cpt].val_avant_ech, Cfg_dmx.Canal[cpt].tech_id, Cfg_dmx.Canal[cpt].acronyme );
       Lancer_requete_SQL ( db, requete );
     }

    Libere_DB_SQL( &db );
    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: %d AO updated", __func__, cpt );
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du MODBUS                                                                                        */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { prctl(PR_SET_NAME, "W-DMX", 0, 0, 0 );
    memset( &Cfg_dmx, 0, sizeof(Cfg_dmx) );                                         /* Mise a zero de la structure de travail */
    Cfg_dmx.lib = lib;                                             /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_dmx.lib->TID = pthread_self();                                                      /* Sauvegarde du TID pour le pere */

    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );
    Cfg_dmx.lib->Thread_run = TRUE;                                                                     /* Le thread tourne ! */

    g_snprintf( Cfg_dmx.lib->admin_prompt, sizeof(Cfg_dmx.lib->admin_prompt), "dmx" );
    g_snprintf( Cfg_dmx.lib->admin_help,   sizeof(Cfg_dmx.lib->admin_help),   "Manage Dmx system" );

reload:
    Cfg_dmx.zmq_to_master = Connect_zmq ( ZMQ_PUB, "pub-to-master",  "inproc", ZMQUEUE_LOCAL_MASTER, 0 );
    Dmx_Lire_config ();                                                     /* Lecture de la configuration logiciel du thread */


    if (!Cfg_dmx.enable)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE,
                "%s: Thread is not enabled in config. Shutting Down %p", __func__, pthread_self() );
       goto end;
     }

    if (Dls_auto_create_plugin( Cfg_dmx.tech_id, "Gestion du DMX" ) == FALSE)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: %s: DLS Create ERROR\n", Cfg_dmx.tech_id ); }

    Mnemo_auto_create_DI ( Cfg_dmx.tech_id, "COMM", "Statut de la communication avec le token DMX" );

    Dmx_init();
    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { usleep(100000);
       sched_yield();

       if (Cfg_dmx.comm_status == FALSE)
        { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Acces DMX not opened, sleeping 5s before retrying.", __func__);
          sleep(5);
          lib->Thread_reload = TRUE;
          break;
        }
       if (Envoyer_trame_dmx_request()==FALSE)
        { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Write error, sleeping 5s before retrying.", __func__);
          sleep(5);
          lib->Thread_reload = TRUE;
          break;
        }
     }                                                                     /* Fin du while partage->arret */

    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Preparing to stop . . . TID = %p", __func__, pthread_self() );
    Dmx_Updater_DB();
    Dmx_close();
    Close_zmq ( Cfg_dmx.zmq_to_master );

    if (lib->Thread_reload == TRUE && lib->Thread_run == TRUE)
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
