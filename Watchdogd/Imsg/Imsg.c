/**********************************************************************************************************/
/* Watchdogd/Imsg/Imsg.c  Gestion des capteurs IMSG Watchdog 2.0                                          */
/* Projet WatchDog version 2.0       Gestion d'habitat                   sam. 28 juil. 2012 16:37:38 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Imsg.c
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
 #include <sys/time.h>
 #include <sys/prctl.h>
 #include <termios.h>
 #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <unistd.h>

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */

 /*static GList *Modules_IMSG;                                  /* Liste des actionneurs/capteurs IMSG */

#ifdef bouh
/**********************************************************************************************************/
/* Retirer_imsgDB: Elimination d'un module imsg                                                       */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_imsgDB ( gint id )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MODULE_IMSG, id );

    retour = Lancer_requete_SQL ( Config.log, db, requete );               /* Execution de la requete SQL */
    Libere_DB_SQL( Config.log, &db );
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_imsgDB: Ajout ou edition d'un imsg                                                         */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure imsg                        */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_imsgDB ( struct IMSGDB *imsg )
  { gchar requete[2048];
    gboolean retour;
    gchar *libelle;
    struct DB *db;
    gint last_id;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

    libelle = Normaliser_chaine ( Config.log, imsg->libelle );         /* Formatage correct des chaines */
    if (!libelle)
     { Info( Config.log, DEBUG_DB, "Ajouter_imsgDB: Normalisation libelle impossible" );
       Libere_DB_SQL( Config.log, &db );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s(type,canal,libelle,e_min,ea_min,a_min) "
                " VALUES ('%d','%d','%s','%d','%d','%d')",
                NOM_TABLE_MODULE_IMSG, imsg->type, imsg->canal, libelle,
                imsg->e_min, imsg->ea_min, imsg->a_min
              );
    g_free(libelle);

    retour = Lancer_requete_SQL ( Config.log, db, requete );               /* Execution de la requete SQL */
    if (retour == FALSE)  { Libere_DB_SQL( Config.log, &db );
                            return(-1);
                          }
    last_id = Recuperer_last_ID_SQL( Config.log, db );
    Libere_DB_SQL( Config.log, &db );
    return( last_id );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_imsgDB: Recupération de la liste des ids des imsgs                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static gboolean Recuperer_imsgDB ( struct LOG *log, struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,type,canal,libelle,e_min,ea_min,a_min"
                " FROM %s ORDER BY type,canal", NOM_TABLE_MODULE_IMSG );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_imsgDB: Recupération de la liste des ids des imsgs                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct IMSGDB *Recuperer_imsgDB_suite( struct LOG *log, struct DB *db )
  { struct IMSGDB *imsg;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    imsg = (struct IMSGDB *)g_malloc0( sizeof(struct IMSGDB) );
    if (!imsg) Info( log, DEBUG_INFO, "Recuperer_imsgDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &imsg->libelle, db->row[3], sizeof(imsg->libelle) );
       imsg->id                = atoi(db->row[0]);
       imsg->type              = atoi(db->row[1]);
       imsg->canal             = atoi(db->row[2]);
       imsg->e_min             = atoi(db->row[4]);
       imsg->ea_min            = atoi(db->row[5]);
       imsg->a_min             = atoi(db->row[6]);
     }
    return(imsg);
  }
/**********************************************************************************************************/
/* Modifier_imsgDB: Modification d'un imsg Watchdog                                                   */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_imsgDB( struct IMSGDB *imsg )
  { gchar requete[2048];
    gboolean retour;
    gchar *libelle;
    struct DB *db;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

    libelle = Normaliser_chaine ( Config.log, imsg->libelle );         /* Formatage correct des chaines */
    if (!libelle)
     { Info( Config.log, DEBUG_DB, "Modifier_imsgDB: Normalisation libelle impossible" );
       Libere_DB_SQL( Config.log, &db );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "type='%d',canal='%d',libelle='%s',e_min='%d',ea_min='%d',a_min='%d' "
                " WHERE id=%d",
                NOM_TABLE_MODULE_IMSG,
                imsg->type, imsg->canal, libelle,
                imsg->e_min, imsg->ea_min, imsg->a_min,
                imsg->id );
    g_free(libelle);

    retour = Lancer_requete_SQL ( Config.log, db, requete );               /* Execution de la requete SQL */
    Libere_DB_SQL( Config.log, &db );
                            
    return( retour );
  }
/**********************************************************************************************************/
/* Charger_tous_ Requete la DB pour charger les modules et les bornes imsg                       */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_tous_imsg ( void  )
  { struct DB *db;
    gint cpt;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

/********************************************** Chargement des modules ************************************/
    if ( ! Recuperer_imsgDB( Config.log, db ) )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    Modules_IMSG = NULL;
    cpt = 0;
    for ( ; ; )
     { struct MODULE_IMSG *module;
       struct IMSGDB *imsg;

       imsg = Recuperer_imsgDB_suite( Config.log, db );
       if (!imsg) break;

       module = (struct MODULE_IMSG *)g_malloc0( sizeof(struct MODULE_IMSG) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info( Config.log, DEBUG_INFO,
                "Charger_tous_ Erreur allocation mémoire struct MODULE_IMSG" );
          g_free(imsg);
          Libere_DB_SQL( Config.log, &db );
          return(FALSE);
        }
       memcpy( &module->imsg, imsg, sizeof(struct IMSGDB) );
       g_free(imsg);
       cpt++;                                              /* Nous avons ajouté un module dans la liste ! */
                                                                        /* Ajout dans la liste de travail */
       Modules_IMSG = g_list_append ( Modules_IMSG, module );
       Info_n( Config.log, DEBUG_INFO, "Charger_tous_  id    = ", module->imsg.id    );
     }
    Info_n( Config.log, DEBUG_INFO, "Charger_tous_ module IMSG found  !", cpt );

    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Decharger_un_imsg: Dechargement d'un IMSG                                                          */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_un_imsg ( struct MODULE_IMSG *module )
  { if (!module) return;
    Modules_IMSG = g_list_remove ( Modules_IMSG, module );
    g_free(module);
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_tous_imsg ( void  )
  { struct MODULE_IMSG *module;
    while ( Modules_IMSG )
     { module = (struct MODULE_IMSG *)Modules_IMSG->data;
       Decharger_un_imsg ( module );
     }
  }
/**********************************************************************************************************/
/* Init_imsg: Initialisation de la ligne IMSG                                                           */
/* Sortie: l'identifiant de la connexion                                                                  */
/**********************************************************************************************************/
 static int Init_imsg ( void )
  { gchar trame_reset[] = { 0x0D, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    gchar trame_get_status[] = { 0x0D, 00, 00, 01, 02, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    gchar trame_set_all_proto[] = { 0x0D, 00, 00, 02, 04, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    struct termios oldtio;
    int fd;

    fd = open( Config.port_imsg, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd<0)
     { Info_c( Config.log, DEBUG_INFO,
               " Init_imsg: Impossible d'ouvrir le port imsg", Config.port_imsg );
       Info_n( Config.log, DEBUG_INFO,
               " Init_imsg: Code retour                      ", fd );
       return(-1);
     }
    else
     { memset(&oldtio, 0, sizeof(oldtio) );
       oldtio.c_cflag = B38400 | CS8 | CREAD | CLOCAL;
       oldtio.c_oflag = 0;
       oldtio.c_iflag = 0;
       oldtio.c_lflag = 0;
       oldtio.c_cc[VTIME]    = 0;
       oldtio.c_cc[VMIN]     = 0;
       tcsetattr(fd, TCSANOW, &oldtio);
       tcflush(fd, TCIOFLUSH);
       Info_c( Config.log, DEBUG_INFO,
               " Init_imsg: Ouverture port imsg okay", Config.port_imsg);
     }
    Info( Config.log, DEBUG_INFO, " Init_imsg: Sending INIT" );
    write (fd, &trame_reset, sizeof(trame_reset) );
    sleep(2);
    Info( Config.log, DEBUG_INFO, " Init_imsg: Sending SET ALL PROTO" );
    write (fd, &trame_set_all_proto, sizeof(trame_set_all_proto) );
    sleep(2);
    Info( Config.log, DEBUG_INFO, " Init_imsg: Sending GET STATUS" );
    write (fd, &trame_get_status, sizeof(trame_get_status) );
    return(fd);
  }
/**********************************************************************************************************/
/* Chercher_imsg: Retrouve un module/capteur dans la liste gérée en fonction des paramètres             */
/* Entrée: les paramètres de critères de recherche                                                        */
/* Sortie: le module, ou NULL si erreur                                                                   */
/**********************************************************************************************************/
 static struct MODULE_IMSG *Chercher_imsg ( gint type, gint canal )
  { GList *liste_modules;
    liste_modules = Modules_IMSG;
    while ( liste_modules )
     { struct MODULE_IMSG *module;
       module = (struct MODULE_IMSG *)liste_modules->data;

       if (module->imsg.type == type && module->imsg.canal == canal) return(module);
       liste_modules = liste_modules->next;
     }
    return(NULL);
  }
/**********************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                   */
/* Entrée: la trame a recue                                                                               */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static int Processer_trame( struct TRAME_IMSG *trame )
  { 

    Info_n( Config.log, DEBUG_INFO, " Processer_trame     taille: ", trame->taille );
    Info_n( Config.log, DEBUG_INFO, " Processer_trame       type: ", trame->type );
    Info_n( Config.log, DEBUG_INFO, " Processer_trame  sous_type: ", trame->sous_type );
    Info_n( Config.log, DEBUG_INFO, " Processer_trame      seqno: ", trame->seqno );

    if (trame->type == 0x01 && trame->sous_type == 0x00)
     { if (trame->data[0] == 0x52) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status 433MHz receiver only" );   
       if (trame->data[0] == 0x53) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status 433MHz transceiver" );   
       Info_n( Config.log, DEBUG_INFO, " Processer_trame get_status firmware", trame->data[1] );   
       if (trame->data[3] & 0x80) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto RFU" );   
       if (trame->data[3] & 0x40) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto Rollertroll" );   
       if (trame->data[3] & 0x20) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto Proguard" );   
       if (trame->data[3] & 0x10) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto FS20" );   
       if (trame->data[3] & 0x08) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto LaCrosse" );   
       if (trame->data[3] & 0x04) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto Hideki" );   
       if (trame->data[3] & 0x02) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto LightwaveRF" );   
       if (trame->data[3] & 0x01) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto Mertik" );   
       if (trame->data[4] & 0x80) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto Visonic" );   
       if (trame->data[4] & 0x40) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto ATI" );   
       if (trame->data[4] & 0x20) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto OregonScientific" );   
       if (trame->data[4] & 0x10) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto IkeaKoppla" );   
       if (trame->data[4] & 0x08) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto HomeEasy" );   
       if (trame->data[4] & 0x04) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto AC" );   
       if (trame->data[4] & 0x02) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto ARC" );   
       if (trame->data[4] & 0x01) Info( Config.log, DEBUG_INFO,
                                         " Processer_trame get_status proto X10" );   

     }
    else if (trame->type == 0x52 && trame->sous_type == 0x01)
     { struct MODULE_IMSG *module;
       Info_n( Config.log, DEBUG_INFO, " Processer_trame get_status id1", trame->data[0] );   
       Info_n( Config.log, DEBUG_INFO, " Processer_trame get_status id2", trame->data[1] );   
       Info_n( Config.log, DEBUG_INFO, " Processer_trame get_status high", trame->data[2] >> 1 );   
       Info_n( Config.log, DEBUG_INFO, " Processer_trame get_status signe", trame->data[2] & 1);   
       Info_n( Config.log, DEBUG_INFO, " Processer_trame get_status low", trame->data[3] );   
       Info_n( Config.log, DEBUG_INFO, " Processer_trame get_status hum", trame->data[4] );   
       Info_n( Config.log, DEBUG_INFO, " Processer_trame get_status humstatus", trame->data[5] );   
       Info_n( Config.log, DEBUG_INFO, " Processer_trame get_status battery", trame->data[6] >> 4 );   
       Info_n( Config.log, DEBUG_INFO, " Processer_trame get_status rssi", trame->data[6] & 0x0F );   
       module = Chercher_imsg( trame->type, trame->data[1] );
       if (module)
        { SEA( module->imsg.ea_min,     (trame->data[2] & 1 ? -1.0 : 1.0)* ( (trame->data[2] >> 1) + trame->data[3])
                                           / 10.0 );                                                     /* Temp */
          SEA( module->imsg.ea_min + 1,  trame->data[4] );                                         /* Humidity */
          SEA( module->imsg.ea_min + 2,  trame->data[6] >> 4);                                      /* Battery */
          SEA( module->imsg.ea_min + 3,  trame->data[6] & 0x0F );                                      /* RSSI */

          module->date_last_view = Partage->top;
        }
       else Info_n( Config.log, DEBUG_INFO, " Processer_trame No matching module for packet received", trame->type );
     }
    else Info_n( Config.log, DEBUG_INFO, " Processer_trame unknown packet type", trame->type );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Imsg_send: Envoi la commande dans Partage                                                              */
/* Entrée: le file descriptor de la connexion imsg                                                        */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Imsg_send ( gint fd_imsg )
  { gchar trame_send_AC[] = { 0x0B, 0x11, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    trame_send_AC[0]  = 0x0B; /* Taille */
    trame_send_AC[1]  = 0x11; /* lightning 2 */
    trame_send_AC[2]  = 0x00; /* AC */
    trame_send_AC[3]  = 0x01; /* Seqnbr */
/*    trame_send_AC[4]  = liblearn.id1 << 6;
    trame_send_AC[5]  = liblearn.id2;
    trame_send_AC[6]  = liblearn.id3;
    trame_send_AC[7]  = liblearn.id4;
    trame_send_AC[8]  = liblearn.unitcode;
    trame_send_AC[9]  = liblearn.cmd;
    trame_send_AC[10] = liblearn.level;*/
    trame_send_AC[11] = 0x0;
    write ( fd_imsg, &trame_send_AC, trame_send_AC[0] );
  }
#endif
/**********************************************************************************************************/
/* Main: Fonction principale du thread Imsg                                                               */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { /*struct TRAME_IMSG Trame;
    gint retval, nbr_oct_lu;
    struct timeval tv;
    fd_set fdselect;
    gint fd_imsg;*/

    prctl(PR_SET_NAME, "W-IMSG", 0, 0, 0 );
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "Run_thread: Demarrage . . . TID = %d", pthread_self() );
    lib->Thread_run = TRUE;                                                         /* Le thread tourne ! */

    g_snprintf( lib->admin_prompt, sizeof(lib->admin_prompt), "imsg" );
    g_snprintf( lib->admin_help,   sizeof(lib->admin_help),   "Manage Instant Messaging system" );

#ifdef bouh
    fd_imsg = Init_imsg();
    if (fd_imsg<0)                                                       /* On valide l'acces aux ports */
     { Info_n( Config.log, DEBUG_INFO, " Run_imsg: Down", pthread_self() );
       lib->Thread_run = FALSE;                                             /* Le thread ne tourne plus ! */
       lib->TID = 0;                                      /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     }
    else { Info_n( Config.log, DEBUG_INFO, " Acces IMSG FD", fd_imsg ); }

    Charger_tous_imsg();                          /* Chargement de tous les capteurs/actionneurs IMSG */
    nbr_oct_lu = 0;
#endif
    while( lib->Thread_run == TRUE)                                      /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "Run_thread: recu signal SIGUSR1" );
          lib->Thread_sigusr1 = FALSE;
        }

     }                                                                     /* Fin du while partage->arret */

#ifdef bouh
    close(fd_imsg);
#endif
    Info_new( Config.log, lib->Thread_debug, LOG_NOTICE, "Run_thread: Down . . . TID = %d", pthread_self() );
    lib->TID = 0;                                         /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
