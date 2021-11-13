/******************************************************************************************************************************/
/* Watchdogd/Dmx/Dmx.c  Gestion des modules MODBUS Watchdog 2.0                                                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                                    22.10.2019 23:42:08 */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * Dmx.c
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

 #include <stdio.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <errno.h>
 #include <sys/prctl.h>
 #include <sys/ioctl.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <semaphore.h>

 #include "watchdogd.h"                                                                             /* Pour la struct PARTAGE */
 #include "Dmx.h"
 struct DMX_CONFIG Cfg_dmx;
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
    Send_zmq_with_tag ( Cfg_dmx.zmq_to_master, NULL, NOM_THREAD, "*", "msrv", "SET_DI", result, taille );
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
       gchar *min = db->row[4], *max = db->row[5], *type=db->row[6], *valeur = db->row[7];
       gchar debut[80];
       gint num;
       Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_DEBUG, "%s: Match found '%s' '%s:%s' - %s", __func__,
                 map_text, tech_id, acro, libelle );
       if ( sscanf ( map_text, "%[^:]:AO%d", debut, &num ) == 2 )                            /* Découpage de la ligne ev_text */
        { if (1<=num && num<=DMX_CHANNEL)
           { g_snprintf( Cfg_dmx.Canal[num-1].tech_id, sizeof(Cfg_dmx.Canal[num-1].tech_id), "%s", tech_id );
             g_snprintf( Cfg_dmx.Canal[num-1].acronyme, sizeof(Cfg_dmx.Canal[num-1].acronyme), "%s", acro );
             Cfg_dmx.Canal[num-1].min  = atof(min);
             Cfg_dmx.Canal[num-1].max  = atof(max);
             Cfg_dmx.Canal[num-1].type = atoi(type);
             Cfg_dmx.Canal[num-1].val_avant_ech = 0.0;                 /*atof(valeur); a l'init, on considère le canal à zero */
             Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_INFO,
                       "%s: AO Canal %d : '%s:%s'=%s ('%s') loaded", __func__, num, tech_id, acro, valeur, libelle );
             cpt++;
           }
          else Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_WARNING, "%s: map '%s': num %d out of range '%d'", __func__,
                         map_text, num, DMX_CHANNEL );
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
  {
    Cfg_dmx.fd = open( Cfg_dmx.device, O_RDWR | O_NOCTTY /*| O_NONBLOCK*/ );
    if (Cfg_dmx.fd<0)
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Impossible d'ouvrir le device '%s', retour=%d (%s)", __func__,
                 Cfg_dmx.device, Cfg_dmx.fd, strerror(errno) );
       return;
     }

    Cfg_dmx.taille_trame_dmx = sizeof(struct TRAME_DMX);
    memset ( &Cfg_dmx.Trame_dmx, 0, sizeof(struct TRAME_DMX) );
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
    Dmx_send_status_to_master( FALSE );
  }
/******************************************************************************************************************************/
/* Envoyer_trame_dmx_request: envoie une trame DMX au token USB                                                               */
/* Entrée: L'id de la transmission, et la trame a transmettre                                                                 */
/******************************************************************************************************************************/
 static gboolean Envoyer_trame_dmx_request( void )
  { gint cpt;
    if (Cfg_dmx.comm_status == FALSE) return(FALSE);
    Cfg_dmx.Trame_dmx.start_delimiter = 0x7E;
    Cfg_dmx.Trame_dmx.label      = DMX_Output_Only_Send_DMX_Packet_Request;
    Cfg_dmx.Trame_dmx.length_lsb =  (sizeof(Cfg_dmx.Trame_dmx.channel)+1) & 0xFF;
    Cfg_dmx.Trame_dmx.length_msb =  ((sizeof(Cfg_dmx.Trame_dmx.channel)+1) >> 8) & 0xFF;
    Cfg_dmx.Trame_dmx.start_code = 0; /* Start Code DMX = 0 */
    for (cpt=0; cpt<DMX_CHANNEL; cpt++) { Cfg_dmx.Trame_dmx.channel[cpt] = (guchar)Cfg_dmx.Canal[cpt].val_avant_ech; }
    Cfg_dmx.Trame_dmx.end_delimiter = 0xE7; /* End delimiter */
    if ( write( Cfg_dmx.fd, &Cfg_dmx.Trame_dmx, sizeof(struct TRAME_DMX) ) != sizeof(struct TRAME_DMX) )/* Ecriture de la trame */
     { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Write Trame Error '%s'", __func__, strerror(errno) );
       Dmx_close();
       return(FALSE);
     }
    Cfg_dmx.nbr_request++;
    return(TRUE);
  }
/******************************************************************************************************************************/
/* Main: Fonction principale du MODBUS                                                                                        */
/******************************************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct ZMQUEUE *zmq_from_bus;
    prctl(PR_SET_NAME, "W-DMX", 0, 0, 0 );
reload:
    memset( &Cfg_dmx, 0, sizeof(Cfg_dmx) );                                         /* Mise a zero de la structure de travail */
    Cfg_dmx.lib = lib;                                             /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_dmx.lib->TID = pthread_self();                                                      /* Sauvegarde du TID pour le pere */

    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );
    Cfg_dmx.lib->Thread_run = TRUE;                                                                     /* Le thread tourne ! */

    g_snprintf( Cfg_dmx.lib->admin_prompt, sizeof(Cfg_dmx.lib->admin_prompt), "dmx" );
    g_snprintf( Cfg_dmx.lib->admin_help,   sizeof(Cfg_dmx.lib->admin_help),   "Manage Dmx system" );

    zmq_from_bus          = Connect_zmq ( ZMQ_SUB, "listen-to-bus",  "inproc", ZMQUEUE_LOCAL_BUS, 0 );
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
    Envoyer_trame_dmx_request();
    while(lib->Thread_run == TRUE && lib->Thread_reload == FALSE)                            /* On tourne tant que necessaire */
     { struct ZMQ_TARGET *event;
       gchar buffer[256];
       void *payload;
       gint byte;
       usleep(1000);
       sched_yield();

       if (Cfg_dmx.comm_status == FALSE)
        { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Acces DMX not opened, sleeping 5s before retrying.", __func__);
          sleep(5);
          lib->Thread_reload = TRUE;
          break;
        }

       if ( (byte=Recv_zmq_with_tag ( zmq_from_bus, NOM_THREAD, &buffer, sizeof(buffer)-1, &event, &payload )) > 0) /* Reception d'un paquet master ? */
        { JsonObject *object;
          JsonNode *Query;
          buffer[byte] = 0;

          if ( !strcasecmp( event->tag, "SET_AO" ) )
           { gchar *tech_id, *acronyme;
             gdouble valeur;
             gint num;
             Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_DEBUG, "%s: Recu SET_AO from bus: %s", __func__, payload );
             Query = json_from_string ( payload, NULL );

             if (!Query)
              { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: requete non Json", __func__ ); continue; }

             object = json_node_get_object (Query);
             if (!object)
              { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: Object non trouvé", __func__ );
                json_node_unref (Query);
                continue;
              }

             tech_id  = json_object_get_string_member ( object, "tech_id" );
             acronyme = json_object_get_string_member ( object, "acronyme" );
             valeur   = json_object_get_double_member ( object, "valeur" );
             if (!tech_id)
              { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque tech_id", __func__ );
                json_node_unref (Query);
                continue;
              }
             else if (!acronyme)
              { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_ERR, "%s: requete mal formée manque acronyme", __func__ );
                json_node_unref (Query);
                continue;
              }

             for (num=0; num<DMX_CHANNEL; num++)
              { if (!strcasecmp( Cfg_dmx.Canal[num].tech_id, tech_id) &&
                    !strcasecmp( Cfg_dmx.Canal[num].acronyme, acronyme))
                 { Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Setting %s:%s=%f (Canal %d)", __func__,
                             tech_id, acronyme, valeur, num );
                   Cfg_dmx.Canal[num].val_avant_ech = valeur;
                   break;
                 }

              }
             json_node_unref (Query);
             Envoyer_trame_dmx_request();
           }
        }
     }                                                                     /* Fin du while partage->arret */

    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Preparing to stop . . . TID = %p", __func__, pthread_self() );
    Dmx_close();
    Close_zmq ( Cfg_dmx.zmq_to_master );
    Close_zmq ( zmq_from_bus );

end:
    Info_new( Config.log, Cfg_dmx.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    if (lib->Thread_reload == TRUE)
     { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "%s: Reloading", __func__ );
       lib->Thread_reload = FALSE;
       goto reload;
     }
    Cfg_dmx.lib->Thread_run = FALSE;                                                            /* Le thread ne tourne plus ! */
    Cfg_dmx.lib->TID = 0;                                                     /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
