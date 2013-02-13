/**********************************************************************************************************/
/* Watchdogd/Rfxcom/Rfxcom.c  Gestion des capteurs RFXCOM Watchdog 2.0                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 27 mai 2012 12:52:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Rfxcom.c
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
 #include "Rfxcom.h"

/**********************************************************************************************************/
/* Rfxcom_Lire_config : Lit la config Watchdog et rempli la structure m�moire                             */
/* Entr�e: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: N�ant                                                                                          */
/**********************************************************************************************************/
 static void Rfxcom_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Rfxcom_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_rfxcom.lib->Thread_debug = g_key_file_get_boolean ( gkf, "RFXCOM", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */

    chaine = g_key_file_get_string ( gkf, "RFXCOM", "port", NULL );
    if (!chaine)
     { Info_new ( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_ERR,
                  "Rfxcom_Lire_config: port is missing. Using default." );
       g_snprintf( Cfg_rfxcom.port, sizeof(Cfg_rfxcom.port), "/dev/watchdog_RFXCOM" );
     }
    else
     { g_snprintf( Cfg_rfxcom.port, sizeof(Cfg_rfxcom.port), "%s", chaine );
       g_free(chaine);
     }

    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Rfxcom_Liberer_config : Libere la m�moire allouer pr�c�demment pour lire la config rfxcom              */
/* Entr�e: n�ant                                                                                          */
/* Sortie: N�ant                                                                                          */
/**********************************************************************************************************/
 static void Rfxcom_Liberer_config ( void )
  { 
  }
/**********************************************************************************************************/
/* Retirer_rfxcomDB: Elimination d'un module rfxcom                                                       */
/* Entr�e: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_rfxcomDB ( gint id )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MODULE_RFXCOM, id );

    retour = Lancer_requete_SQL ( Config.log, db, requete );               /* Execution de la requete SQL */
    Libere_DB_SQL( Config.log, &db );
    Cfg_rfxcom.reload = TRUE;
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_rfxcomDB: Ajout ou edition d'un rfxcom                                                         */
/* Entr�e: un log et une database, un flag d'ajout/edition, et la structure rfxcom                        */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_rfxcomDB ( struct RFXCOMDB *rfxcom )
  { gchar requete[2048];
    gboolean retour;
    gchar *libelle;
    struct DB *db;
    gint last_id;

    db = Init_DB_SQL( Config.log );
    if (!db) { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_CRIT,
                         "Ajouter_rfxcomDB: Erreur connexion Database" );
               return(FALSE);
             }

    libelle = Normaliser_chaine ( Config.log, rfxcom->libelle );         /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_ERR,
                 "Ajouter_rfxcomDB: Normalisation libelle impossible" );
       Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s(type,sstype,id1,id2,id3,id4,housecode,unitcode,libelle,e_min,ea_min,a_min) "
                " VALUES ('%d','%d','%d','%d','%d','%d','%d','%d','%s','%d','%d','%d')",
                NOM_TABLE_MODULE_RFXCOM, rfxcom->type, rfxcom->sous_type,
                rfxcom->id1, rfxcom->id2, rfxcom->id3, rfxcom->id4, rfxcom->housecode, rfxcom->unitcode, 
                libelle, rfxcom->e_min, rfxcom->ea_min, rfxcom->a_min
              );
    g_free(libelle);

    retour = Lancer_requete_SQL ( Config.log, db, requete );               /* Execution de la requete SQL */
    if (retour == FALSE)  { Libere_DB_SQL( Config.log, &db );
                            return(-1);
                          }
    last_id = Recuperer_last_ID_SQL( Config.log, db );
    Libere_DB_SQL( Config.log, &db );
    Cfg_rfxcom.reload = TRUE;
    return( last_id );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_rfxcomDB: Recup�ration de la liste des ids des rfxcoms                              */
/* Entr�e: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static gboolean Recuperer_rfxcomDB ( struct LOG *log, struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,type,sstype,id1,id2,id3,id4,housecode,unitcode,libelle,e_min,ea_min,a_min"
                " FROM %s ORDER BY type,sstype", NOM_TABLE_MODULE_RFXCOM );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_rfxcomDB: Recup�ration de la liste des ids des rfxcoms                              */
/* Entr�e: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct RFXCOMDB *Recuperer_rfxcomDB_suite( struct LOG *log, struct DB *db )
  { struct RFXCOMDB *rfxcom;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    rfxcom = (struct RFXCOMDB *)g_try_malloc0( sizeof(struct RFXCOMDB) );
    if (!rfxcom) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_ERR,
                           "Recuperer_rfxcomDB_suite: Erreur allocation m�moire" );
    else
     { g_snprintf( rfxcom->libelle, sizeof(rfxcom->libelle), "%s", db->row[9] );
       rfxcom->id                = atoi(db->row[0]);
       rfxcom->type              = atoi(db->row[1]);
       rfxcom->sous_type         = atoi(db->row[2]);
       rfxcom->id1               = atoi(db->row[3]);
       rfxcom->id2               = atoi(db->row[4]);
       rfxcom->id3               = atoi(db->row[5]);
       rfxcom->id4               = atoi(db->row[6]);
       rfxcom->housecode         = atoi(db->row[7]);
       rfxcom->unitcode          = atoi(db->row[8]);
       rfxcom->e_min             = atoi(db->row[10]);
       rfxcom->ea_min            = atoi(db->row[11]);
       rfxcom->a_min             = atoi(db->row[12]);
     }
    return(rfxcom);
  }
/**********************************************************************************************************/
/* Modifier_rfxcomDB: Modification d'un rfxcom Watchdog                                                   */
/* Entr�es: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: FALSE si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Modifier_rfxcomDB( struct RFXCOMDB *rfxcom )
  { gchar requete[2048];
    gboolean retour;
    gchar *libelle;
    struct DB *db;

    db = Init_DB_SQL( Config.log );
    if (!db) { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_CRIT,
                         "Modifier_rfxcomDB: Erreur connexion Database" );
               return(FALSE);
             }

    libelle = Normaliser_chaine ( Config.log, rfxcom->libelle );         /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_ERR,
                 "Modifier_rfxcomDB: Normalisation libelle impossible" );
       Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "type='%d',sstype='%d',id1='%d',id2='%d',id3='%d',id4='%d',housecode='%d',unitcode='%d',"
                "libelle='%s',e_min='%d',ea_min='%d',a_min='%d' "
                " WHERE id=%d",
                NOM_TABLE_MODULE_RFXCOM,
                rfxcom->type, rfxcom->sous_type,
                rfxcom->id1, rfxcom->id2, rfxcom->id3, rfxcom->id4, rfxcom->housecode, rfxcom->unitcode, 
                libelle, rfxcom->e_min, rfxcom->ea_min, rfxcom->a_min,
                rfxcom->id );
    g_free(libelle);

    retour = Lancer_requete_SQL ( Config.log, db, requete );               /* Execution de la requete SQL */
    Libere_DB_SQL( Config.log, &db );
    Cfg_rfxcom.reload = TRUE;
    return( retour );
  }
/**********************************************************************************************************/
/* Charger_tous_rfxcom: Requete la DB pour charger les modules et les bornes rfxcom                       */
/* Entr�e: rien                                                                                           */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static gboolean Charger_tous_rfxcom ( void  )
  { struct DB *db;

    db = Init_DB_SQL( Config.log );
    if (!db) { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_CRIT,
               "Charger_tous_rfxcom: Erreur connexion Database" );
               return(FALSE);
             }
/********************************************** Chargement des modules ************************************/
    if ( ! Recuperer_rfxcomDB( Config.log, db ) )
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_CRIT,
                "Charger_tous_rfxcom: Erreur de requete SQL" );
       Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    Cfg_rfxcom.Modules_RFXCOM = NULL;
    for ( ; ; )
     { struct MODULE_RFXCOM *module;
       struct RFXCOMDB *rfxcom;

       rfxcom = Recuperer_rfxcomDB_suite( Config.log, db );
       if (!rfxcom) break;

       module = (struct MODULE_RFXCOM *)g_try_malloc0( sizeof(struct MODULE_RFXCOM) );
       if (!module)                                                   /* Si probleme d'allocation m�moire */
        { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_ERR,
                    "Charger_tous_Erreur allocation m�moire struct MODULE_RFXCOM" );
          g_free(rfxcom);
          Libere_DB_SQL( Config.log, &db );
          return(FALSE);
        }
       memcpy( &module->rfxcom, rfxcom, sizeof(struct RFXCOMDB) );
       g_free(rfxcom);
                                                                        /* Ajout dans la liste de travail */
       pthread_mutex_lock ( &Cfg_rfxcom.lib->synchro );
       Cfg_rfxcom.Modules_RFXCOM = g_slist_prepend ( Cfg_rfxcom.Modules_RFXCOM, module );
       pthread_mutex_unlock ( &Cfg_rfxcom.lib->synchro );
       Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_DEBUG,
                 "Charger_tous_rfxcom. Module loaded id = %02d", module->rfxcom.id    );
     }
    pthread_mutex_lock ( &Cfg_rfxcom.lib->synchro );
    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
              "Charger_tous_module : %d RFXCOM Modules found !", g_slist_length( Cfg_rfxcom.Modules_RFXCOM ) );
    pthread_mutex_unlock ( &Cfg_rfxcom.lib->synchro );

    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Decharger_tous_rfxcom : Dechargement de tous les capteurs/senseurs/actionneurs RFXCOM                  */
/* Entr�e: n�ant                                                                                          */
/* Sortie: n�ant                                                                                          */
/**********************************************************************************************************/
 static void Decharger_tous_rfxcom ( void  )
  { struct MODULE_RFXCOM *module;
    pthread_mutex_lock ( &Cfg_rfxcom.lib->synchro );
    while ( Cfg_rfxcom.Modules_RFXCOM )
     { module = (struct MODULE_RFXCOM *)Cfg_rfxcom.Modules_RFXCOM->data;
       Cfg_rfxcom.Modules_RFXCOM = g_slist_remove ( Cfg_rfxcom.Modules_RFXCOM, module );
       g_free(module);
     }
    pthread_mutex_unlock ( &Cfg_rfxcom.lib->synchro );
  }
/**********************************************************************************************************/
/* Init_rfxcom: Initialisation de la ligne RFXCOM                                                         */
/* Sortie: l'identifiant de la connexion                                                                  */
/**********************************************************************************************************/
 static int Init_rfxcom ( void )
  { gchar trame_reset[] = { 0x0D, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    gchar trame_get_status[] = { 0x0D, 00, 00, 01, 02, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    gchar trame_set_proto[] = { 0x0D, 00, 00, 02, 03, 0x53, 00, 0x80, 0x08, 0x22, 00, 00, 00, 00 };
                                                           /* Allow only Oregon & ARC protocols & LaCrosse */
    struct termios oldtio;
    int fd;

    fd = open( Cfg_rfxcom.port, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd<0)
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_ERR,
               "Init_rfxcom: Impossible d'ouvrir le port rfxcom %s, erreur %d", Cfg_rfxcom.port, fd );
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
       Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_NOTICE,
                 "Init_rfxcom: Ouverture port rfxcom okay %s", Cfg_rfxcom.port );
     }
    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_DEBUG, "Init_rfxcom: Sending INIT" );
    write (fd, &trame_reset, sizeof(trame_reset) );
    sleep(2);
    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_DEBUG, "Init_rfxcom: Sending SET PROTO" );
    write (fd, &trame_set_proto, sizeof(trame_set_proto) );
    sleep(2);
    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_DEBUG, "Init_rfxcom: Sending GET STATUS" );
    write (fd, &trame_get_status, sizeof(trame_get_status) );
    return(fd);
  }
/**********************************************************************************************************/
/* Chercher_rfxcom: Retrouve un module/capteur dans la liste g�r�e en fonction des param�tres             */
/* Entr�e: les param�tres de crit�res de recherche                                                        */
/* Sortie: le module, ou NULL si erreur                                                                   */
/**********************************************************************************************************/
 static struct MODULE_RFXCOM *Chercher_rfxcom ( gint type, gint sous_type,
                                                gboolean check_id1, guchar id1,
                                                gboolean check_id2, guchar id2,
                                                gboolean check_id3, guchar id3,
                                                gboolean check_id4, guchar id4,
                                                gboolean check_housecode, guchar housecode,
                                                gboolean check_unitcode, guchar unitcode
                                              )
  { struct MODULE_RFXCOM *module;
    GSList *liste_modules;

    module = NULL;
    pthread_mutex_lock ( &Cfg_rfxcom.lib->synchro );
    liste_modules = Cfg_rfxcom.Modules_RFXCOM;
    while ( liste_modules )
     { module = (struct MODULE_RFXCOM *)liste_modules->data;

       if (module->rfxcom.type == type && module->rfxcom.sous_type == sous_type && 
           (check_id1       == FALSE || module->rfxcom.id1 == id1) &&
           (check_id2       == FALSE || module->rfxcom.id2 == id2) &&
           (check_id3       == FALSE || module->rfxcom.id3 == id3) &&
           (check_id4       == FALSE || module->rfxcom.id4 == id4) &&
           (check_housecode == FALSE || module->rfxcom.housecode == housecode) &&
           (check_unitcode  == FALSE || module->rfxcom.unitcode  == unitcode)
          ) break;
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_rfxcom.lib->synchro );
    if (liste_modules) return(module);
    return(NULL);
  }
/**********************************************************************************************************/
/* Chercher_rfxcom: Retrouve un module/capteur dans la liste g�r�e en fonction des param�tres             */
/* Entr�e: les param�tres de crit�res de recherche                                                        */
/* Sortie: le module, ou NULL si erreur                                                                   */
/**********************************************************************************************************/
 static void Rfxcom_Envoyer_sortie ( gint num_a )
  { gchar trame_send_AC[] = { 0x0B, 0x11, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    struct MODULE_RFXCOM *module;
    GSList *liste_modules;

    module = NULL;
    pthread_mutex_lock ( &Cfg_rfxcom.lib->synchro );
    liste_modules = Cfg_rfxcom.Modules_RFXCOM;
    while ( liste_modules )
     { module = (struct MODULE_RFXCOM *)liste_modules->data;

       if (module->rfxcom.type == 0x11 && module->rfxcom.sous_type == 0x00 && 
           module->rfxcom.a_min == num_a
          )
        { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_DEBUG,
              "Rfxcom_envoyer_sortie: Envoi de A(%03d)=%d au module ids=%02d %02d %02d %02d unit %02d",
               num_a, A(num_a), module->rfxcom.id1, module->rfxcom.id2,
               module->rfxcom.id3, module->rfxcom.id4, module->rfxcom.unitcode );
          trame_send_AC[0]  = 0x0B; /* Taille */
          trame_send_AC[1]  = 0x11; /* lightning 2 */
          trame_send_AC[2]  = 0x00; /* AC */
          trame_send_AC[3]  = 0x01; /* Seqnbr */
          trame_send_AC[4]  = module->rfxcom.id1 << 6;
          trame_send_AC[5]  = module->rfxcom.id2;
          trame_send_AC[6]  = module->rfxcom.id3;
          trame_send_AC[7]  = module->rfxcom.id4;
          trame_send_AC[8]  = module->rfxcom.unitcode;
          trame_send_AC[9]  = (A(num_a) ? 1 : 0);
          trame_send_AC[10] = 0x0; /* level */
          trame_send_AC[11] = 0x0; /* rssi */
          write ( Cfg_rfxcom.fd, &trame_send_AC, trame_send_AC[0] + 1 );
        }
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_rfxcom.lib->synchro );
  }
/**********************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                   */
/* Entr�e: la trame a recue                                                                               */
/* Sortie: n�ant                                                                                          */
/**********************************************************************************************************/
 static int Processer_trame( struct TRAME_RFXCOM *trame )
  { 
    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_DEBUG,
              "Processer_trame taille=%d, type=%02d(0x%02x), sous_type=%02d(0x%02X), seqno=%03d",
               trame->taille, trame->type, trame->type, trame->sous_type, trame->sous_type, trame->seqno );

    if (trame->type == 0x01 && trame->sous_type == 0x00)
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                 "Processer_trame get_status Cmd= %d (0x%2X)", trame->data[0], trame->data[0] );
       if (trame->data[1] == 0x52) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                             "Processer_trame get_status 433MHz receiver only" );   
       if (trame->data[1] == 0x53) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                             "Processer_trame get_status 433MHz transceiver" );   
       Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                 "Processer_trame get_status firmware %d (0x%2X)", trame->data[2], trame->data[2] );
       if (trame->data[3] & 0x80) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Unencoded Frame" );   
       if (trame->data[3] & 0x40) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU6" );   
       if (trame->data[3] & 0x20) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU5" );   
       if (trame->data[3] & 0x10) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU4" );   
       if (trame->data[3] & 0x08) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU3" );   
       if (trame->data[3] & 0x04) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto FineOffset/Viking" );   
       if (trame->data[3] & 0x02) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Rubicson" );   
       if (trame->data[3] & 0x01) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto AE" );   
       if (trame->data[4] & 0x80) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto BlindsT1" );   
       if (trame->data[4] & 0x40) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto BlindsT0" );   
       if (trame->data[4] & 0x20) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto ProGuard" );   
       if (trame->data[4] & 0x10) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto FS20" );   
       if (trame->data[4] & 0x08) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto LaCrosse" );   
       if (trame->data[4] & 0x04) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Hideki" );   
       if (trame->data[4] & 0x02) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto LightwaveRF" );   
       if (trame->data[4] & 0x01) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Mertik" );   
       if (trame->data[5] & 0x80) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Visonic" );   
       if (trame->data[5] & 0x40) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto ATI" );   
       if (trame->data[5] & 0x20) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto OregonScientific" );   
       if (trame->data[5] & 0x10) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto MeianTech" );   
       if (trame->data[5] & 0x08) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto HomeEasy/EU" );   
       if (trame->data[5] & 0x04) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto AC" );   
       if (trame->data[5] & 0x02) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto ARC" );   
       if (trame->data[5] & 0x01) Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto X10" );   
     }
    else if (trame->type == 0x02)
     { switch (trame->sous_type)
        { case 0x00: Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                              "Processer_trame : Transceiver message : Error, receiver did not lock" );
                     break;
          case 0x01: switch (trame->data[0])
                      { case 0x00: Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : ACK, transmit OK" );
                                   break;
                        case 0x01: Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : ACK, "
                                            "but transmit started after 3 seconds delay anyway with RF receive data" );
                                   break;
                        case 0x02: Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : NAK, transmitter "
                                            "did not lock on the requested transmit frequency" );
                                   break;
                        case 0x03: Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : NAK, "
                                            "AC address zero in id1-id4 not allowed" );
                                   break;
                        default  : Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : Unknown message..." );
                                   break;
                      }
                     break;
          default :  Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                              "Processer_trame : Transceiver message : unknown packet ssous_type %d", trame->sous_type);
        }
     } 
    else if (trame->type == 0x52 && trame->sous_type == 0x01)
     { struct MODULE_RFXCOM *module;
       Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                 "Processer_trame : get status type=%03d(0x%02X), sous_type=%03d(0x%02X), id1=%03d, id2=%03d, high=%03d, "
                 "signe=%02d, low=%03d, hum=%02d, humstatus=%02d, battery=%02d, rssi=%02d",
                 trame->type, trame->type, trame->sous_type, trame->sous_type, trame->data[0], trame->data[1],
                 trame->data[2] & 0x7F, trame->data[2] & 0x80, trame->data[3], trame->data[4], trame->data[5],
                 trame->data[6] >> 4, trame->data[6] & 0x0F
               );   
       module = Chercher_rfxcom( trame->type, trame->sous_type, TRUE, trame->data[0], TRUE, trame->data[1],
                                 FALSE, 0, FALSE, 0, FALSE, 0, FALSE, 0 );
       if (module)
        { SEA( module->rfxcom.ea_min,     (trame->data[2] & 0x80 ? -1.0 : 1.0)* ( (trame->data[2] & 0x7F) + trame->data[3])
                                           / 10.0 );                                              /* Temp */
          SEA( module->rfxcom.ea_min + 1,  trame->data[4] );                                  /* Humidity */
          SEA( module->rfxcom.ea_min + 2,  trame->data[6] >> 4);                               /* Battery */
          SEA( module->rfxcom.ea_min + 3,  trame->data[6] & 0x0F );                               /* RSSI */

          module->date_last_view = Partage->top;
        }
       else Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                      "Processer_trame: No module found for packet received type=%02d(0x%02X), sous_type=%02d(0x%02X)",
                      trame->type, trame->type, trame->sous_type, trame->sous_type );
     }
    else Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                   "Processer_trame unknown packet type %02d(0x%02X), sous_type=%02d(0x%02X)",
                   trame->type, trame->type, trame->sous_type, trame->sous_type );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Rfxcom_send: Envoi la commande dans Partage                                                            */
/* Entr�e: le file descriptor de la connexion rfxcom                                                      */
/* Sortie: n�ant                                                                                          */
/**********************************************************************************************************/
 static void Rfxcom_send ( void )
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
    write ( Cfg_rfxcom.fd, &trame_send_AC, trame_send_AC[0] );
  }
/**********************************************************************************************************/
/* Rfxcom_Gerer_sortie: Ajoute une demande d'envoi RF dans la liste des envois RFXCOM                     */
/* Entr�es: le num�ro de la sortie                                                                        */
/**********************************************************************************************************/
 void Rfxcom_Gerer_sortie( gint num_a )                                    /* Num_a est l'id de la sortie */
  { gint taille;

    pthread_mutex_lock( &Cfg_rfxcom.lib->synchro );              /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_rfxcom.Liste_sortie );
    pthread_mutex_unlock( &Cfg_rfxcom.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_WARNING,
                "Rfxcom_Gerer_sortie: DROP (taille>150)  id=%d", num_a );
       return;
     }

    pthread_mutex_lock( &Cfg_rfxcom.lib->synchro );       /* Ajout dans la liste de tell a traiter */
    Cfg_rfxcom.Liste_sortie = g_slist_prepend( Cfg_rfxcom.Liste_sortie, GINT_TO_POINTER(num_a) );
    pthread_mutex_unlock( &Cfg_rfxcom.lib->synchro );
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Rfxcom                                                             */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct TRAME_RFXCOM Trame;
    gint retval, nbr_oct_lu;
    struct timeval tv;
    fd_set fdselect;

    prctl(PR_SET_NAME, "W-RFXCOM", 0, 0, 0 );
    memset( &Cfg_rfxcom, 0, sizeof(Cfg_rfxcom) );               /* Mise a zero de la structure de travail */
    Cfg_rfxcom.lib = lib;                      /* Sauvegarde de la structure pointant sur cette librairie */
    Rfxcom_Lire_config ();                              /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );
    Cfg_rfxcom.lib->Thread_run = TRUE;                                              /* Le thread tourne ! */

    g_snprintf( lib->admin_prompt, sizeof(lib->admin_prompt), "rfxcom" );
    g_snprintf( lib->admin_help,   sizeof(lib->admin_help),   "Manage RFXCOM sensors" );

    Cfg_rfxcom.fd = Init_rfxcom();
    if (Cfg_rfxcom.fd<0)                                                   /* On valide l'acces aux ports */
     { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_CRIT,
                 "Run_thread: Init RFXCOM failed. exiting..." );
       Rfxcom_Liberer_config ();
       lib->Thread_run = FALSE;                                             /* Le thread ne tourne plus ! */
       lib->TID = 0;                                      /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     }
    else { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,"Acces RFXCOM FD=%d", Cfg_rfxcom.fd ); }

    Abonner_distribution_sortie ( Rfxcom_Gerer_sortie );     /* Desabonnement de la diffusion des sorties */
    Charger_tous_rfxcom();                          /* Chargement de tous les capteurs/actionneurs RFXCOM */
    nbr_oct_lu = 0;
    while( lib->Thread_run == TRUE)                                      /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_NOTICE, "Run_thread: recu signal SIGUSR1" );
          lib->Thread_sigusr1 = FALSE;
        }

       if (Cfg_rfxcom.reload == TRUE)
        { Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_NOTICE, "Run_thread: Reloading in progress" );
          Decharger_tous_rfxcom();
          Charger_tous_rfxcom();
          Cfg_rfxcom.reload = FALSE;
        }

/******************************************* Reception trame RFXCOM ***************************************/
       FD_ZERO(&fdselect);                                         /* Reception sur la ligne serie RFXCOM */
       FD_SET(Cfg_rfxcom.fd, &fdselect );
       tv.tv_sec = 1;
       tv.tv_usec= 0;
       retval = select(Cfg_rfxcom.fd+1, &fdselect, NULL, NULL, &tv );               /* Attente d'un caractere */
       if (retval>=0 && FD_ISSET(Cfg_rfxcom.fd, &fdselect) )
        { int bute, cpt;

          if (nbr_oct_lu<TAILLE_ENTETE_RFXCOM)
           { bute = TAILLE_ENTETE_RFXCOM; } else { bute = sizeof(Trame); }

          cpt = read( Cfg_rfxcom.fd, (unsigned char *)&Trame + nbr_oct_lu, bute-nbr_oct_lu );
          if (cpt>0)
           { nbr_oct_lu = nbr_oct_lu + cpt;

             if (nbr_oct_lu >= TAILLE_ENTETE_RFXCOM + Trame.taille)                   /* traitement trame */
              { nbr_oct_lu = 0;
                if (Trame.taille > 0) Processer_trame( &Trame );
                memset (&Trame, 0, sizeof(struct TRAME_RFXCOM) );
              }
           }
        }
/********************************************** Transmission des trames aux sorties ***********************/
       if (Cfg_rfxcom.Liste_sortie)                            /* Si pas de message, on tourne */
        { gint num_a;
          pthread_mutex_lock( &Cfg_rfxcom.lib->synchro );                                /* lockage futex */
          num_a = GPOINTER_TO_INT(Cfg_rfxcom.Liste_sortie->data);               /* Recuperation du numero */
          Cfg_rfxcom.Liste_sortie = g_slist_remove ( Cfg_rfxcom.Liste_sortie, GINT_TO_POINTER(num_a) );
          Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_INFO,
                   "Run_rfxcom: Reste a traiter %d",
                    g_slist_length(Cfg_rfxcom.Liste_sortie) );
          pthread_mutex_unlock( &Cfg_rfxcom.lib->synchro );

          Rfxcom_Envoyer_sortie ( num_a );
        }
     }                                                                     /* Fin du while partage->arret */

    Desabonner_distribution_sortie ( Rfxcom_Gerer_sortie );  /* Desabonnement de la diffusion des sorties */
    Decharger_tous_rfxcom ();
    close(Cfg_rfxcom.fd);                                                 /* Fermeture de la connexion FD */

    Rfxcom_Liberer_config();                                  /* Liberation de la configuration du thread */
    Info_new( Config.log, Cfg_rfxcom.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_rfxcom.lib->TID = 0;                              /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
