/**********************************************************************************************************/
/* Watchdogd/Enocean/Enocean.c  Gestion des capteurs ENOCEAN Watchdog 2.0                                 */
/* Projet WatchDog version 2.0       Gestion d'habitat                     dim. 28 déc. 2014 15:46:44 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Enocean.c
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
 #include "Enocean.h"

 unsigned char ENOCEAN_CRC8TABLE[256] =
  { 0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15,
    0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65,
    0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    0xe0, 0xe7, 0xee, 0xe9, 0xfc, 0xfb, 0xf2, 0xf5,
    0xd8, 0xdf, 0xd6, 0xd1, 0xc4, 0xc3, 0xca, 0xcd,
    0x90, 0x97, 0x9e, 0x99, 0x8c, 0x8b, 0x82, 0x85,
    0xa8, 0xaf, 0xa6, 0xa1, 0xb4, 0xb3, 0xba, 0xbd,
    0xc7, 0xc0, 0xc9, 0xce, 0xdb, 0xdc, 0xd5, 0xd2,
    0xff, 0xf8, 0xf1, 0xf6, 0xe3, 0xe4, 0xed, 0xea,
    0xb7, 0xb0, 0xb9, 0xbe, 0xab, 0xac, 0xa5, 0xa2,
    0x8f, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9d, 0x9a,
    0x27, 0x20, 0x29, 0x2e, 0x3b, 0x3c, 0x35, 0x32,
    0x1f, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0d, 0x0a,
    0x57, 0x50, 0x59, 0x5e, 0x4b, 0x4c, 0x45, 0x42,
    0x6f, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7d, 0x7a,
    0x89, 0x8e, 0x87, 0x80, 0x95, 0x92, 0x9b, 0x9c,
    0xb1, 0xb6, 0xbf, 0xb8, 0xad, 0xaa, 0xa3, 0xa4,
    0xf9, 0xfe, 0xf7, 0xf0, 0xe5, 0xe2, 0xeb, 0xec,
    0xc1, 0xc6, 0xcf, 0xc8, 0xdd, 0xda, 0xd3, 0xd4,
    0x69, 0x6e, 0x67, 0x60, 0x75, 0x72, 0x7b, 0x7c,
    0x51, 0x56, 0x5f, 0x58, 0x4d, 0x4a, 0x43, 0x44,
    0x19, 0x1e, 0x17, 0x10, 0x05, 0x02, 0x0b, 0x0c,
    0x21, 0x26, 0x2f, 0x28, 0x3d, 0x3a, 0x33, 0x34,
    0x4e, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5c, 0x5b,
    0x76, 0x71, 0x78, 0x7f, 0x6A, 0x6d, 0x64, 0x63,
    0x3e, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2c, 0x2b,
    0x06, 0x01, 0x08, 0x0f, 0x1a, 0x1d, 0x14, 0x13,
    0xae, 0xa9, 0xa0, 0xa7, 0xb2, 0xb5, 0xbc, 0xbb,
    0x96, 0x91, 0x98, 0x9f, 0x8a, 0x8D, 0x84, 0x83,
    0xde, 0xd9, 0xd0, 0xd7, 0xc2, 0xc5, 0xcc, 0xcb,
    0xe6, 0xe1, 0xe8, 0xef, 0xfa, 0xfd, 0xf4, 0xf3
  };

/**********************************************************************************************************/
/* Enocean_Lire_config : Lit la config Watchdog et rempli la structure mémoire                             */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 gboolean Enocean_Lire_config ( void )
  { gchar *nom, *valeur;
    struct DB *db;

    Cfg_enocean.lib->Thread_debug = FALSE;                                  /* Settings default parameters */
    Cfg_enocean.enable            = FALSE; 
    g_snprintf( Cfg_enocean.port, sizeof(Cfg_enocean.port), "%s", DEFAUT_PORT_ENOCEAN );

    if ( ! Recuperer_configDB( &db, NOM_THREAD ) )                      /* Connexion a la base de données */
     { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                "Enocean_Lire_config: Database connexion failed. Using Default Parameters" );
       return(FALSE);
     }

    while (Recuperer_configDB_suite( &db, &nom, &valeur ) )       /* Récupération d'une config dans la DB */
     { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,                     /* Print Config */
                "Enocean_Lire_config: '%s' = %s", nom, valeur );
            if ( ! g_ascii_strcasecmp ( nom, "port" ) )
        { g_snprintf( Cfg_enocean.port, sizeof(Cfg_enocean.port), "%s", valeur ); }
       else if ( ! g_ascii_strcasecmp ( nom, "enable" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_enocean.enable = TRUE;  }
       else if ( ! g_ascii_strcasecmp ( nom, "debug" ) )
        { if ( ! g_ascii_strcasecmp( valeur, "true" ) ) Cfg_enocean.lib->Thread_debug = TRUE;  }
       else
        { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_NOTICE,
                   "Enocean_Lire_config: Unknown Parameter '%s'(='%s') in Database", nom, valeur );
        }
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Retirer_enoceanDB: Elimination d'un module enocean                                                     */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_enoceanDB ( gint id )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db) return(FALSE);

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MODULE_ENOCEAN, id );

    retour = Lancer_requete_SQL ( db, requete );               /* Execution de la requete SQL */
    Libere_DB_SQL( &db );
    Cfg_enocean.reload = TRUE;
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_enoceanDB: Ajout ou edition d'un enocean                                                       */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure enocean                       */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_enoceanDB ( struct ENOCEANDB *enocean )
  { gchar requete[2048];
    gboolean retour;
    gchar *libelle;
    struct DB *db;
    gint last_id;


    libelle = Normaliser_chaine ( enocean->libelle );         /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_ERR,
                 "Ajouter_enoceanDB: Normalisation libelle impossible" );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s(instance_id,type,sstype,id1,id2,id3,id4,housecode,unitcode,libelle,e_min,ea_min,a_min) "
                " VALUES ('%s','%d','%d','%d','%d','%d','%d','%d','%d','%s','%d','%d','%d')",
                NOM_TABLE_MODULE_ENOCEAN, Config.instance_id, enocean->type, enocean->sous_type,
                enocean->id1, enocean->id2, enocean->id3, enocean->id4, enocean->housecode, enocean->unitcode, 
                libelle, enocean->e_min, enocean->ea_min, enocean->a_min
              );
    g_free(libelle);

    db = Init_DB_SQL();       
    if (!db) { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_CRIT,
                         "Ajouter_enoceanDB: Erreur connexion Database" );
               return(FALSE);
             }

    retour = Lancer_requete_SQL ( db, requete );               /* Execution de la requete SQL */
    if (retour == FALSE)  { Libere_DB_SQL( &db );
                            return(-1);
                          }
    last_id = Recuperer_last_ID_SQL ( db );
    Libere_DB_SQL( &db );
    Cfg_enocean.reload = TRUE;
    return( last_id );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_enoceanDB: Recupération de la liste des ids des enoceans                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static gboolean Recuperer_enoceanDB ( struct LOG *log, struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,type,sstype,id1,id2,id3,id4,housecode,unitcode,libelle,e_min,ea_min,a_min"
                " FROM %s WHERE instance_id = '%s' ORDER BY type,sstype",
                NOM_TABLE_MODULE_ENOCEAN, Config.instance_id );/* Ne selectionne que le instance_id spécifique */

    return ( Lancer_requete_SQL ( db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_enoceanDB: Recupération de la liste des ids des enoceans                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct ENOCEANDB *Recuperer_enoceanDB_suite( struct LOG *log, struct DB *db )
  { struct ENOCEANDB *enocean;

    Recuperer_ligne_SQL(db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL (db);
       return(NULL);
     }

    enocean = (struct ENOCEANDB *)g_try_malloc0( sizeof(struct ENOCEANDB) );
    if (!enocean) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_ERR,
                           "Recuperer_enoceanDB_suite: Erreur allocation mémoire" );
    else
     { g_snprintf( enocean->libelle, sizeof(enocean->libelle), "%s", db->row[9] );
       enocean->id                = atoi(db->row[0]);
       enocean->type              = atoi(db->row[1]);
       enocean->sous_type         = atoi(db->row[2]);
       enocean->id1               = atoi(db->row[3]);
       enocean->id2               = atoi(db->row[4]);
       enocean->id3               = atoi(db->row[5]);
       enocean->id4               = atoi(db->row[6]);
       enocean->housecode         = atoi(db->row[7]);
       enocean->unitcode          = atoi(db->row[8]);
       enocean->e_min             = atoi(db->row[10]);
       enocean->ea_min            = atoi(db->row[11]);
       enocean->a_min             = atoi(db->row[12]);
     }
    return(enocean);
  }
/**********************************************************************************************************/
/* Modifier_enoceanDB: Modification d'un enocean Watchdog                                                   */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: FALSE si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Modifier_enoceanDB( struct ENOCEANDB *enocean )
  { gchar requete[2048];
    gboolean retour;
    gchar *libelle;
    struct DB *db;

    db = Init_DB_SQL();       
    if (!db) { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_CRIT,
                         "Modifier_enoceanDB: Erreur connexion Database" );
               return(FALSE);
             }

    libelle = Normaliser_chaine ( enocean->libelle );         /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_ERR,
                 "Modifier_enoceanDB: Normalisation libelle impossible" );
       Libere_DB_SQL( &db );
       return(FALSE);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "type='%d',sstype='%d',id1='%d',id2='%d',id3='%d',id4='%d',housecode='%d',unitcode='%d',"
                "libelle='%s',e_min='%d',ea_min='%d',a_min='%d' "
                " WHERE id=%d",
                NOM_TABLE_MODULE_ENOCEAN,
                enocean->type, enocean->sous_type,
                enocean->id1, enocean->id2, enocean->id3, enocean->id4, enocean->housecode, enocean->unitcode, 
                libelle, enocean->e_min, enocean->ea_min, enocean->a_min,
                enocean->id );
    g_free(libelle);

    retour = Lancer_requete_SQL ( db, requete );               /* Execution de la requete SQL */
    Libere_DB_SQL( &db );
    Cfg_enocean.reload = TRUE;
    return( retour );
  }
/**********************************************************************************************************/
/* Charger_tous_enocean: Requete la DB pour charger les modules et les bornes enocean                       */
/* Entrée: rien                                                                                           */
/* Sortie: FALSE si erreur                                                                                */
/**********************************************************************************************************/
 static gboolean Charger_tous_enocean ( void  )
  { struct DB *db;

    db = Init_DB_SQL();       
    if (!db) { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_CRIT,
               "Charger_tous_enocean: Erreur connexion Database" );
               return(FALSE);
             }
/********************************************** Chargement des modules ************************************/
    if ( ! Recuperer_enoceanDB( Config.log, db ) )
     { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_CRIT,
                "Charger_tous_enocean: Erreur de requete SQL" );
       Libere_DB_SQL( &db );
       return(FALSE);
     }

    Cfg_enocean.Modules_ENOCEAN = NULL;
    for ( ; ; )
     { struct MODULE_ENOCEAN *module;
       struct ENOCEANDB *enocean;

       enocean = Recuperer_enoceanDB_suite( Config.log, db );
       if (!enocean) break;

       module = (struct MODULE_ENOCEAN *)g_try_malloc0( sizeof(struct MODULE_ENOCEAN) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_ERR,
                    "Charger_tous_Erreur allocation mémoire struct MODULE_ENOCEAN" );
          g_free(enocean);
          Libere_DB_SQL( &db );
          return(FALSE);
        }
       memcpy( &module->enocean, enocean, sizeof(struct ENOCEANDB) );
       g_free(enocean);
                                                                        /* Ajout dans la liste de travail */
       pthread_mutex_lock ( &Cfg_enocean.lib->synchro );
       Cfg_enocean.Modules_ENOCEAN = g_slist_prepend ( Cfg_enocean.Modules_ENOCEAN, module );
       pthread_mutex_unlock ( &Cfg_enocean.lib->synchro );
       Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
                 "Charger_tous_enocean. Module loaded id = %02d", module->enocean.id    );
     }
    pthread_mutex_lock ( &Cfg_enocean.lib->synchro );
    Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
              "Charger_tous_module : %d ENOCEAN Modules found !", g_slist_length( Cfg_enocean.Modules_ENOCEAN ) );
    pthread_mutex_unlock ( &Cfg_enocean.lib->synchro );

    Libere_DB_SQL( &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Decharger_tous_enocean : Dechargement de tous les capteurs/senseurs/actionneurs ENOCEAN                  */
/* Entrée: néant                                                                                          */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Decharger_tous_enocean ( void  )
  { struct MODULE_ENOCEAN *module;
    pthread_mutex_lock ( &Cfg_enocean.lib->synchro );
    while ( Cfg_enocean.Modules_ENOCEAN )
     { module = (struct MODULE_ENOCEAN *)Cfg_enocean.Modules_ENOCEAN->data;
       Cfg_enocean.Modules_ENOCEAN = g_slist_remove ( Cfg_enocean.Modules_ENOCEAN, module );
       g_free(module);
     }
    pthread_mutex_unlock ( &Cfg_enocean.lib->synchro );
  }
/**********************************************************************************************************/
/* Init_enocean: Initialisation de la ligne ENOCEAN                                                         */
/* Sortie: l'identifiant de la connexion                                                                  */
/**********************************************************************************************************/
 static int Init_enocean ( void )
  { struct termios oldtio;
    int fd;

    fd = open( Cfg_enocean.port, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd<0)
     { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_ERR,
               "Init_enocean: Impossible d'ouvrir le port enocean %s, erreur %d (%s)",
                Cfg_enocean.port, fd, strerror(errno) );
       return(-1);
     }
    else
     { memset(&oldtio, 0, sizeof(oldtio) );
       oldtio.c_cflag = B57600 | CS8 | CREAD | CLOCAL;
       oldtio.c_oflag = 0;
       oldtio.c_iflag = 0;
       oldtio.c_lflag = 0;
       oldtio.c_cc[VTIME]    = 0;
       oldtio.c_cc[VMIN]     = 0;
       tcsetattr(fd, TCSANOW, &oldtio);
       tcflush(fd, TCIOFLUSH);
       Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_NOTICE,
                 "Init_enocean: Ouverture port enocean okay %s", Cfg_enocean.port );
     }
    return(fd);
  }
/**********************************************************************************************************/
/* Chercher_enocean: Retrouve un module/capteur dans la liste gérée en fonction des paramètres             */
/* Entrée: les paramètres de critères de recherche                                                        */
/* Sortie: le module, ou NULL si erreur                                                                   */
/**********************************************************************************************************/
 static struct MODULE_ENOCEAN *Chercher_enocean ( gint type, gint sous_type,
                                                gboolean check_id1, guchar id1,
                                                gboolean check_id2, guchar id2,
                                                gboolean check_id3, guchar id3,
                                                gboolean check_id4, guchar id4,
                                                gboolean check_housecode, guchar housecode,
                                                gboolean check_unitcode, guchar unitcode
                                              )
  { struct MODULE_ENOCEAN *module;
    GSList *liste_modules;

    module = NULL;
    pthread_mutex_lock ( &Cfg_enocean.lib->synchro );
    liste_modules = Cfg_enocean.Modules_ENOCEAN;
    while ( liste_modules )
     { module = (struct MODULE_ENOCEAN *)liste_modules->data;

       if (module->enocean.type == type && module->enocean.sous_type == sous_type && 
           (check_id1       == FALSE || module->enocean.id1 == id1) &&
           (check_id2       == FALSE || module->enocean.id2 == id2) &&
           (check_id3       == FALSE || module->enocean.id3 == id3) &&
           (check_id4       == FALSE || module->enocean.id4 == id4) &&
           (check_housecode == FALSE || module->enocean.housecode == housecode) &&
           (check_unitcode  == FALSE || module->enocean.unitcode  == unitcode)
          ) break;
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_enocean.lib->synchro );
    if (liste_modules) return(module);
    return(NULL);
  }
/**********************************************************************************************************/
/* Chercher_enocean: Retrouve un module/capteur dans la liste gérée en fonction des paramètres             */
/* Entrée: les paramètres de critères de recherche                                                        */
/* Sortie: le module, ou NULL si erreur                                                                   */
/**********************************************************************************************************/
 static void Enocean_Envoyer_sortie ( gint num_a )
  { gchar trame_send_AC[] = { 0x0B, 0x11, 00, 01, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    struct MODULE_ENOCEAN *module;
    GSList *liste_modules;

    module = NULL;
    pthread_mutex_lock ( &Cfg_enocean.lib->synchro );
    liste_modules = Cfg_enocean.Modules_ENOCEAN;
    while ( liste_modules )
     { module = (struct MODULE_ENOCEAN *)liste_modules->data;

       if (module->enocean.type == 0x11 && module->enocean.sous_type == 0x00 && 
           module->enocean.a_min == num_a
          )
        { gint cpt;
          Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
              "Enocean_envoyer_sortie: Envoi de A(%03d)=%d au module ids=%02d %02d %02d %02d unit %02d",
               num_a, A(num_a), module->enocean.id1, module->enocean.id2,
               module->enocean.id3, module->enocean.id4, module->enocean.unitcode );
          trame_send_AC[0]  = 0x0B; /* Taille */
          trame_send_AC[1]  = 0x11; /* lightning 2 */
          trame_send_AC[2]  = 0x00; /* AC */
          trame_send_AC[3]  = 0x01; /* Seqnbr */
          trame_send_AC[4]  = module->enocean.id1 << 6;
          trame_send_AC[5]  = module->enocean.id2;
          trame_send_AC[6]  = module->enocean.id3;
          trame_send_AC[7]  = module->enocean.id4;
          trame_send_AC[8]  = module->enocean.unitcode;
          trame_send_AC[9]  = (A(num_a) ? 1 : 0);
          trame_send_AC[10] = 0x0; /* level */
          trame_send_AC[11] = 0x0; /* rssi */
          for ( cpt = 0; cpt < 3 ; cpt++)
           { gint retour;
             retour = write ( Cfg_enocean.fd, &trame_send_AC, trame_send_AC[0] + 1 );
             if (retour == -1)
              { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                         "Enocean_envoyer_sortie: Write Error for A(%03d) : %s", num_a, strerror(errno) );
              }
           }
        }
       liste_modules = liste_modules->next;
     }
    pthread_mutex_unlock ( &Cfg_enocean.lib->synchro );
  }
/**********************************************************************************************************/
/* Enocean_crc_header: Calcul le Header CRC de la trame en parametre                                      */
/* Entrée: la trame recue                                                                                 */
/* Sortie: le CRC sur 8 bits !                                                                            */
/**********************************************************************************************************/
 static unsigned char Enocean_crc_header( struct TRAME_ENOCEAN *trame )
  { unsigned char resultCRC = 0;
    unsigned char *ptr;
    gint i;
    ptr = (unsigned char *)trame;
    for (i = 1; i < ENOCEAN_HEADER_LENGTH - 1; i++)                                    /* Last Byte = CRC */
     { resultCRC = ENOCEAN_CRC8TABLE[ resultCRC ^ ptr[i] ]; }
    return( resultCRC );
  }
/**********************************************************************************************************/
/* Enocean_crc_data: Calcul le Data CRC de la trame en parametre                                          */
/* Entrée: la trame recue                                                                                 */
/* Sortie: le CRC sur 8 bits !                                                                            */
/**********************************************************************************************************/
 static unsigned char Enocean_crc_data( struct TRAME_ENOCEAN *trame )
  { unsigned char resultCRC = 0;
    unsigned char *ptr;
    gint i;
    ptr = (unsigned char *)trame;
    for (i = ENOCEAN_HEADER_LENGTH; i < Cfg_enocean.index_bute - 1; i++)               /* Last byte = CRC */
     { resultCRC = ENOCEAN_CRC8TABLE[ resultCRC ^ ptr[i] ]; }
    return( resultCRC );
  }
/**********************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                   */
/* Entrée: la trame a recue                                                                               */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static void Processer_trame( struct TRAME_ENOCEAN *trame )
  { switch (trame->packet_type)
     { case 1:                                                                              /* RADIO_ERP1 */
        { if (trame->data[0] == 0xD2)
           { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
                      "Processer_trame Received RADIO_ERP1-VLD" );
           }
          else if (trame->data[0] == 0xA6)
           { gchar chaine[32];
             gint cpt;
             memset( chaine, 0, sizeof(chaine) );
             for (cpt=0; cpt<trame->data_length_lsb; cpt++)                /* Mise en forme au format HEX */
              { g_snprintf( &chaine[2*cpt], 3, "%02X", trame->data[cpt] ); }
             Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
                      "Processer_trame Received RADIO_ERP1-ADT-%s", chaine );
           }
          else if (trame->data[0] == 0xA5)
           { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
                      "Processer_trame Received RADIO_ERP1-4BS" );
           }
          else if (trame->data[0] == 0xF6)                                                /* RPS Telegram */
           { gchar chaine[32];
             gint cpt;
             memset( chaine, 0, sizeof(chaine) );
             for (cpt=0; cpt<trame->data_length_lsb+trame->optional_data_length; cpt++)
              { g_snprintf( &chaine[2*cpt], 3, "%02X", trame->data[cpt] ); }/* Mise en forme au format HEX */
             Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
                      "Processer_trame Received RADIO_ERP1-RPS-%s", chaine );
           }
          return;
        }
     }
    Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
             "Processer_trame: Unmanaged packet type %0X %0X-%0X-%0X",
              trame->packet_type, trame->data[0], trame->data[1], trame->data[2] );
#ifdef bouh


    if (trame->type == 0x01 && trame->sous_type == 0x00)
     { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                 "Processer_trame get_status Cmd= %d (0x%2X)", trame->data[0], trame->data[0] );
       if (trame->data[1] == 0x52) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                             "Processer_trame get_status 433MHz receiver only" );   
       if (trame->data[1] == 0x53) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                             "Processer_trame get_status 433MHz transceiver" );   
       Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                 "Processer_trame get_status firmware %d (0x%2X)", trame->data[2], trame->data[2] );
       if (trame->data[3] & 0x80) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Unencoded Frame" );   
       if (trame->data[3] & 0x40) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU6" );   
       if (trame->data[3] & 0x20) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU5" );   
       if (trame->data[3] & 0x10) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU4" );   
       if (trame->data[3] & 0x08) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto RFU3" );   
       if (trame->data[3] & 0x04) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto FineOffset/Viking" );   
       if (trame->data[3] & 0x02) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Rubicson" );   
       if (trame->data[3] & 0x01) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto AE" );   
       if (trame->data[4] & 0x80) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto BlindsT1" );   
       if (trame->data[4] & 0x40) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto BlindsT0" );   
       if (trame->data[4] & 0x20) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto ProGuard" );   
       if (trame->data[4] & 0x10) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto FS20" );   
       if (trame->data[4] & 0x08) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto LaCrosse" );   
       if (trame->data[4] & 0x04) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Hideki" );   
       if (trame->data[4] & 0x02) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto LightwaveRF" );   
       if (trame->data[4] & 0x01) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Mertik" );   
       if (trame->data[5] & 0x80) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto Visonic" );   
       if (trame->data[5] & 0x40) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto ATI" );   
       if (trame->data[5] & 0x20) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto OregonScientific" );   
       if (trame->data[5] & 0x10) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto MeianTech" );   
       if (trame->data[5] & 0x08) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto HomeEasy/EU" );   
       if (trame->data[5] & 0x04) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto AC" );   
       if (trame->data[5] & 0x02) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto ARC" );   
       if (trame->data[5] & 0x01) Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame get_status proto X10" );   
     }
    else if (trame->type == 0x02)
     { switch (trame->sous_type)
        { case 0x00: Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                              "Processer_trame : Transceiver message : Error, receiver did not lock" );
                     break;
          case 0x01: switch (trame->data[0])
                      { case 0x00: Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : ACK, transmit OK" );
                                   break;
                        case 0x01: Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : ACK, "
                                            "but transmit started after 3 seconds delay anyway with RF receive data" );
                                   break;
                        case 0x02: Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : NAK, transmitter "
                                            "did not lock on the requested transmit frequency" );
                                   break;
                        case 0x03: Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : NAK, "
                                            "AC address zero in id1-id4 not allowed" );
                                   break;
                        default  : Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                                            "Processer_trame : Transceiver message : Unknown message..." );
                                   break;
                      }
                     break;
          default :  Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                              "Processer_trame : Transceiver message : unknown packet ssous_type %d", trame->sous_type);
        }
     } 
    else if (trame->type == 0x52 && trame->sous_type == 0x01)                                   /* Oregon */
     { struct MODULE_ENOCEAN *module;
       Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                 "Processer_trame : get status type=%03d(0x%02X), sous_type=%03d(0x%02X), id1=%03d, id2=%03d, high=%03d, "
                 "signe=%02d, low=%03d, hum=%02d, humstatus=%02d, battery=%02d, rssi=%02d",
                 trame->type, trame->type, trame->sous_type, trame->sous_type, trame->data[0], trame->data[1],
                 trame->data[2] & 0x7F, trame->data[2] & 0x80, trame->data[3], trame->data[4], trame->data[5],
                 trame->data[6] >> 4, trame->data[6] & 0x0F
               );   
       module = Chercher_enocean( trame->type, trame->sous_type, TRUE, trame->data[0], TRUE, trame->data[1],
                                 FALSE, 0, FALSE, 0, FALSE, 0, FALSE, 0 );
       if (module)
        { SEA( module->enocean.ea_min,     (trame->data[2] & 0x80 ? -1.0 : 1.0)* ( ((trame->data[2] & 0x7F)<<8) + trame->data[3])
                                           / 10.0 );                                              /* Temp */
          SEA( module->enocean.ea_min + 1,  trame->data[4] );                                  /* Humidity */
          SEA( module->enocean.ea_min + 2,  trame->data[6] >> 4);                               /* Battery */
          SEA( module->enocean.ea_min + 3,  trame->data[6] & 0x0F );                               /* RSSI */

          module->date_last_view = Partage->top;
        }
       else Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                      "Processer_trame: No module found for packet received type=%02d(0x%02X), sous_type=%02d(0x%02X)",
                      trame->type, trame->type, trame->sous_type, trame->sous_type );
     }
    else if (trame->type == 0x11 && trame->sous_type == 0x00)                            /* Lighting 2 AC */
     { struct MODULE_ENOCEAN *module;
       Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                 "Processer_trame : get lighting ! type=%03d(0x%02X), sous_type=%03d(0x%02X), id1=%03d, id2=%03d, "
                 "id3=%03d, id4=%03d, unitcode=%03d, cmnd=%03d, level=%03d rssi=%02d",
                 trame->type, trame->type, trame->sous_type, trame->sous_type, trame->data[0] & 0x03, trame->data[1],
                 trame->data[2], trame->data[3], trame->data[4], trame->data[5],
                 trame->data[6], trame->data[7] & 0x0F
               );   
       module = Chercher_enocean( trame->type, trame->sous_type, TRUE, trame->data[0] & 0x03, TRUE, trame->data[1],
                                 TRUE, trame->data[2], TRUE, trame->data[3], FALSE, 0, TRUE, trame->data[4] );
       if (module)
        { SE( module->enocean.e_min, trame->data[5] );
          Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
                    "Processer_trame : Module found (%s), Setting E%03d=%d", module->enocean.libelle, module->enocean.e_min, trame->data[5] );
          module->date_last_view = Partage->top;
        }
       else Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                      "Processer_trame: No module found for packet received type=%02d(0x%02X), sous_type=%02d(0x%02X)",
                      trame->type, trame->type, trame->sous_type, trame->sous_type );
     }
    else Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                   "Processer_trame unknown packet type %02d(0x%02X), sous_type=%02d(0x%02X)",
                   trame->type, trame->type, trame->sous_type, trame->sous_type );
    return(TRUE);
#endif
  }
/**********************************************************************************************************/
/* Enocean_Gerer_sortie: Ajoute une demande d'envoi RF dans la liste des envois ENOCEAN                     */
/* Entrées: le numéro de la sortie                                                                        */
/**********************************************************************************************************/
 void Enocean_Gerer_sortie( gint num_a )                                    /* Num_a est l'id de la sortie */
  { gint taille;

    pthread_mutex_lock( &Cfg_enocean.lib->synchro );              /* Ajout dans la liste de tell a traiter */
    taille = g_slist_length( Cfg_enocean.Liste_sortie );
    pthread_mutex_unlock( &Cfg_enocean.lib->synchro );

    if (taille > 150)
     { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                "Enocean_Gerer_sortie: DROP (taille>150)  id=%d", num_a );
       return;
     }

    pthread_mutex_lock( &Cfg_enocean.lib->synchro );       /* Ajout dans la liste de tell a traiter */
    Cfg_enocean.Liste_sortie = g_slist_prepend( Cfg_enocean.Liste_sortie, GINT_TO_POINTER(num_a) );
    pthread_mutex_unlock( &Cfg_enocean.lib->synchro );
  }
/**********************************************************************************************************/
/* Enocean_select: Permet d'estimer la disponibilité d'une information reçue à traiter                    */
/* Entrée : Néant                                                                                         */
/* Sortie : 0 - pas d'info, 1 presence d'info, -1, erreur                                                 */
/**********************************************************************************************************/
 static gint Enocean_select ( void )
  { struct timeval tv;
    fd_set fdselect;
    gint retval;
    tv.tv_sec = 0;
    tv.tv_usec= 100000;
    FD_ZERO(&fdselect);                                  /* Reception sur la ligne serie ENOCEAN */
    FD_SET(Cfg_enocean.fd, &fdselect );
    retval = select(Cfg_enocean.fd+1, &fdselect, NULL, NULL, &tv );    /* Attente d'un caractere */
    if (retval==0) return(0);
    if (retval==1 && FD_ISSET(Cfg_enocean.fd, &fdselect) )
     { Cfg_enocean.date_last_view = Partage->top;
       return(1);
     }
    Cfg_enocean.comm_status = ENOCEAN_DISCONNECT;                                /* Disconnect sur erreur */
    Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_ERR,
             "Enocean_select: Error %d (%s)", errno, strerror(errno) );
    return(-1);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Enocean                                                            */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { struct TRAME_ENOCEAN Trame;

    prctl(PR_SET_NAME, "W-ENOCEAN", 0, 0, 0 );
    memset( &Cfg_enocean, 0, sizeof(Cfg_enocean) );               /* Mise a zero de la structure de travail */
    Cfg_enocean.lib = lib;                      /* Sauvegarde de la structure pointant sur cette librairie */
    Cfg_enocean.lib->TID = pthread_self();                               /* Sauvegarde du TID pour le pere */
    Enocean_Lire_config ();                              /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %p", pthread_self() );
    Cfg_enocean.lib->Thread_run = TRUE;                                              /* Le thread tourne ! */

    g_snprintf( lib->admin_prompt, sizeof(lib->admin_prompt), "enocean" );
    g_snprintf( lib->admin_help,   sizeof(lib->admin_help),   "Manage ENOCEAN sensors" );

    if (!Cfg_enocean.enable)
     { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_NOTICE,
                "Run_thread: Thread is not enabled in config. Shutting Down %p",
                 pthread_self() );
       goto end;
     }

/*  Abonner_distribution_sortie ( Enocean_Gerer_sortie );     /* Desabonnement de la diffusion des sorties */
/*  Charger_tous_enocean();                          /* Chargement de tous les capteurs/actionneurs ENOCEAN */
    Cfg_enocean.nbr_oct_lu = 0;
    Cfg_enocean.comm_status = ENOCEAN_CONNECT;
    while( lib->Thread_run == TRUE )                                     /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_NOTICE, "Run_thread: recu signal SIGUSR1" );
          lib->Thread_sigusr1 = FALSE;
        }

       if (Cfg_enocean.reload == TRUE)
        { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_NOTICE, "Run_thread: Reloading in progress" );
          Decharger_tous_enocean();
          Charger_tous_enocean();
          Cfg_enocean.reload = FALSE;
        }

       switch (Cfg_enocean.comm_status)
        { case ENOCEAN_CONNECT:
           { Cfg_enocean.fd = Init_enocean();
             if (Cfg_enocean.fd<0)                                         /* On valide l'acces aux ports */
              { Cfg_enocean.comm_status = ENOCEAN_DISCONNECT; }
             else { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                             "Run_thread: ENOCEAN FileDescriptor = %d opened", Cfg_enocean.fd );
                    Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;
                  }
             break;
           }
          case ENOCEAN_WAIT_FOR_SYNC:
           { guchar sync;
             gint cpt;
             if (Enocean_select()<=0) break;
             cpt = read( Cfg_enocean.fd, &sync, 1 );
             if (cpt>0)
              { if (sync==0x55) Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_HEADER;
                else Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_DEBUG,
                              "Run_thread: Wrong SYNC Byte (%02X). Dropping Frame", sync );
                Cfg_enocean.nbr_oct_lu = 0;
              }
             else Cfg_enocean.comm_status = ENOCEAN_DISCONNECT;                              /* Si erreur */
             break;
           }
          case ENOCEAN_WAIT_FOR_HEADER:
           { gint cpt;
             if (Cfg_enocean.date_last_view + ENOCEAN_TRAME_TIMEOUT <= Partage->top)
              { Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;
                Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                         "Run_thread: Timeout wating for HEADER. Dropping Frame" );
                break;
              }
             if (Enocean_select()<=0) break;
             cpt = read( Cfg_enocean.fd, (unsigned char *)&Trame + Cfg_enocean.nbr_oct_lu,
                         ENOCEAN_HEADER_LENGTH - Cfg_enocean.nbr_oct_lu );
             if (cpt>0)
              { Cfg_enocean.nbr_oct_lu = Cfg_enocean.nbr_oct_lu + cpt;

                if (Cfg_enocean.nbr_oct_lu == ENOCEAN_HEADER_LENGTH)
                 { if (Trame.crc_header != Enocean_crc_header( &Trame ))    /* Vérification du CRC Header */
                    { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                               "Run_thread: Wrong CRC HEADER. Dropping Frame" );
                      Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;
                    }
                   else
                    { Cfg_enocean.index_bute = ENOCEAN_HEADER_LENGTH + (Trame.data_length_msb << 8)
                                             + Trame.data_length_lsb
                                             + Trame.optional_data_length+1; /* On compte le CRC de fin ! */
                      if (Cfg_enocean.index_bute > sizeof(Trame))
                       { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_ERR,
                                  "Run_thread: Trame too long (%d / %d max), can't handle, dropping",
                                   Cfg_enocean.index_bute, sizeof(Trame) );
                         Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;
                       }
                      else Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_DATA;
                    }
                 }
              }
             else Cfg_enocean.comm_status = ENOCEAN_DISCONNECT;                              /* Si erreur */
             break;
           }
          case ENOCEAN_WAIT_FOR_DATA:
           { gint cpt;
             if (Cfg_enocean.date_last_view + ENOCEAN_TRAME_TIMEOUT <= Partage->top)
              { Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;
                Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                         "Run_thread: Timeout wating for DATA. Dropping Frame" );
                break;
              }
             if (Enocean_select()<=0) break;
             cpt = read( Cfg_enocean.fd, (unsigned char *)&Trame + Cfg_enocean.nbr_oct_lu,
                         Cfg_enocean.index_bute - Cfg_enocean.nbr_oct_lu );
             if (cpt>0)
              { Cfg_enocean.nbr_oct_lu = Cfg_enocean.nbr_oct_lu + cpt;

                if (Cfg_enocean.nbr_oct_lu == Cfg_enocean.index_bute)         /* Vérification du CRC Data */
                 { if ( ((unsigned char *)&Trame)[Cfg_enocean.index_bute-1] != Enocean_crc_data( &Trame ) )
                    { Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_WARNING,
                               "Run_thread: Wrong CRC DATA. Dropping Frame" );
                      Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;
                    }
                   else
                    { Processer_trame ( &Trame );                            /* Precessing received trame */
                      Cfg_enocean.comm_status = ENOCEAN_WAIT_FOR_SYNC;            /* and wait for another */
                    }
                 }
              }
             else Cfg_enocean.comm_status = ENOCEAN_DISCONNECT;                              /* Si erreur */
             break;
           }
          case ENOCEAN_DISCONNECT:
           { if (Cfg_enocean.fd)
              { close(Cfg_enocean.fd);
                Cfg_enocean.fd = 0;
              }
             Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_ERR,
                      "Run_thread: ENOCEAN Disconnected. Re-Trying in %s sec...",
                       ENOCEAN_RECONNECT_DELAY );
             Cfg_enocean.date_retry_connect = Partage->top + ENOCEAN_RECONNECT_DELAY;
             Cfg_enocean.comm_status = ENOCEAN_WAIT_BEFORE_RECONNECT;
             break;
           }
          case ENOCEAN_WAIT_BEFORE_RECONNECT:
           { if (Cfg_enocean.date_retry_connect >= Partage->top)
              { Cfg_enocean.comm_status = ENOCEAN_CONNECT;
                Cfg_enocean.date_retry_connect = 0;
              }
             break;
           }
          default: Cfg_enocean.comm_status = ENOCEAN_CONNECT;
        }
/********************************************** Transmission des trames aux sorties ***********************/
#ifdef bouh
       if (Cfg_enocean.Liste_sortie)                            /* Si pas de message, on tourne */
        { gint num_a;
          pthread_mutex_lock( &Cfg_enocean.lib->synchro );                                /* lockage futex */
          num_a = GPOINTER_TO_INT(Cfg_enocean.Liste_sortie->data);               /* Recuperation du numero */
          Cfg_enocean.Liste_sortie = g_slist_remove ( Cfg_enocean.Liste_sortie, GINT_TO_POINTER(num_a) );
          Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_INFO,
                   "Run_enocean: Reste a traiter %d",
                    g_slist_length(Cfg_enocean.Liste_sortie) );
          pthread_mutex_unlock( &Cfg_enocean.lib->synchro );

          Enocean_Envoyer_sortie ( num_a );
        }
#endif
     }                                                                     /* Fin du while partage->arret */

/*  Desabonner_distribution_sortie ( Enocean_Gerer_sortie );  /* Desabonnement de la diffusion des sorties */
/*    Decharger_tous_enocean ();*/
    close(Cfg_enocean.fd);                                                 /* Fermeture de la connexion FD */
end:
    Info_new( Config.log, Cfg_enocean.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Down . . . TID = %p", pthread_self() );
    Cfg_enocean.lib->Thread_run = FALSE;                                     /* Le thread ne tourne plus ! */
    Cfg_enocean.lib->TID = 0;                              /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
