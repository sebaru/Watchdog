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

/**********************************************************************************************************/
/* Retirer_rfxcomDB: Elimination d'un module rfxcom                                                       */
/* Entrée: un log et une database                                                                         */
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
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_rfxcomDB: Ajout ou edition d'un rfxcom                                                         */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure rfxcom                        */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_rfxcomDB ( struct RFXCOMDB *rfxcom )
  { gchar requete[2048];
    gboolean retour;
    gchar *libelle;
    struct DB *db;
    gint last_id;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

    libelle = Normaliser_chaine ( Config.log, rfxcom->libelle );         /* Formatage correct des chaines */
    if (!libelle)
     { Info( Config.log, DEBUG_DB, "Ajouter_rfxcomDB: Normalisation libelle impossible" );
       Libere_DB_SQL( Config.log, &db );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s(type,canal,libelle,e_min,ea_min,a_min) "
                " VALUES ('%d','%d','%s','%d','%d','%d')",
                NOM_TABLE_MODULE_RFXCOM, rfxcom->type, rfxcom->canal, libelle,
                rfxcom->e_min, rfxcom->ea_min, rfxcom->a_min
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
/* Recuperer_liste_id_rfxcomDB: Recupération de la liste des ids des rfxcoms                              */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static gboolean Recuperer_rfxcomDB ( struct LOG *log, struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,type,canal,libelle,e_min,ea_min,a_min"
                " FROM %s ORDER BY type,canal", NOM_TABLE_MODULE_RFXCOM );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_rfxcomDB: Recupération de la liste des ids des rfxcoms                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct RFXCOMDB *Recuperer_rfxcomDB_suite( struct LOG *log, struct DB *db )
  { struct RFXCOMDB *rfxcom;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    rfxcom = (struct RFXCOMDB *)g_malloc0( sizeof(struct RFXCOMDB) );
    if (!rfxcom) Info( log, DEBUG_RFXCOM, "Recuperer_rfxcomDB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &rfxcom->libelle, db->row[3], sizeof(rfxcom->libelle) );
       rfxcom->id                = atoi(db->row[0]);
       rfxcom->type              = atoi(db->row[1]);
       rfxcom->canal             = atoi(db->row[2]);
       rfxcom->e_min             = atoi(db->row[4]);
       rfxcom->ea_min            = atoi(db->row[5]);
       rfxcom->a_min             = atoi(db->row[6]);
     }
    return(rfxcom);
  }
/**********************************************************************************************************/
/* Modifier_rfxcomDB: Modification d'un rfxcom Watchdog                                                   */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_rfxcomDB( struct RFXCOMDB *rfxcom )
  { gchar requete[2048];
    gboolean retour;
    gchar *libelle;
    struct DB *db;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

    libelle = Normaliser_chaine ( Config.log, rfxcom->libelle );         /* Formatage correct des chaines */
    if (!libelle)
     { Info( Config.log, DEBUG_DB, "Modifier_rfxcomDB: Normalisation libelle impossible" );
       Libere_DB_SQL( Config.log, &db );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "type='%d',canal='%d',libelle='%s',e_min='%d',ea_min='%d',a_min='%d' "
                " WHERE id=%d",
                NOM_TABLE_MODULE_RFXCOM,
                rfxcom->type, rfxcom->canal, libelle,
                rfxcom->e_min, rfxcom->ea_min, rfxcom->a_min,
                rfxcom->id );
    g_free(libelle);

    retour = Lancer_requete_SQL ( Config.log, db, requete );               /* Execution de la requete SQL */
    Libere_DB_SQL( Config.log, &db );
                            
    return( retour );
  }
/**********************************************************************************************************/
/* Charger_tous_RFXCOM: Requete la DB pour charger les modules et les bornes rfxcom                       */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_tous_rfxcom ( void  )
  { struct DB *db;
    gint cpt;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

/********************************************** Chargement des modules ************************************/
    if ( ! Recuperer_rfxcomDB( Config.log, db ) )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    Partage->com_rfxcom.Modules_RFXCOM = NULL;
    cpt = 0;
    for ( ; ; )
     { struct MODULE_RFXCOM *module;
       struct RFXCOMDB *rfxcom;

       rfxcom = Recuperer_rfxcomDB_suite( Config.log, db );
       if (!rfxcom) break;

       module = (struct MODULE_RFXCOM *)g_malloc0( sizeof(struct MODULE_RFXCOM) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info( Config.log, DEBUG_RFXCOM,
                "Charger_tous_RFXCOM: Erreur allocation mémoire struct MODULE_RFXCOM" );
          g_free(rfxcom);
          Libere_DB_SQL( Config.log, &db );
          return(FALSE);
        }
       memcpy( &module->rfxcom, rfxcom, sizeof(struct RFXCOMDB) );
       g_free(rfxcom);
       cpt++;                                              /* Nous avons ajouté un module dans la liste ! */
                                                                        /* Ajout dans la liste de travail */
       Partage->com_rfxcom.Modules_RFXCOM = g_list_append ( Partage->com_rfxcom.Modules_RFXCOM, module );
       Info_n( Config.log, DEBUG_RFXCOM, "Charger_tous_RFXCOM:  id    = ", module->rfxcom.id    );
     }
    Info_n( Config.log, DEBUG_RFXCOM, "Charger_tous_RFXCOM: module RFXCOM found  !", cpt );

    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Decharger_un_rfxcom: Dechargement d'un RFXCOM                                                            */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_un_rfxcom ( struct MODULE_RFXCOM *module )
  { if (!module) return;
    Partage->com_rfxcom.Modules_RFXCOM = g_list_remove ( Partage->com_rfxcom.Modules_RFXCOM, module );
    g_free(module);
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_tous_rfxcom ( void  )
  { struct MODULE_RFXCOM *module;
    while ( Partage->com_rfxcom.Modules_RFXCOM )
     { module = (struct MODULE_RFXCOM *)Partage->com_rfxcom.Modules_RFXCOM->data;
       Decharger_un_rfxcom ( module );
     }
  }
/**********************************************************************************************************/
/* Init_rfxcom: Initialisation de la ligne RFXCOM                                                           */
/* Sortie: l'identifiant de la connexion                                                                  */
/**********************************************************************************************************/
 static int Init_rfxcom ( void )
  { gchar trame_reset[] = { 0x0D, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    gchar trame_get_status[] = { 0x0D, 00, 00, 01, 02, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    gchar trame_set_all_proto[] = { 0x0D, 00, 00, 02, 04, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    struct termios oldtio;
    int fd;

    fd = open( Config.port_rfxcom, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd<0)
     { Info_c( Config.log, DEBUG_RFXCOM,
               "RFXCOM: Init_rfxcom: Impossible d'ouvrir le port rfxcom", Config.port_rfxcom );
       Info_n( Config.log, DEBUG_RFXCOM,
               "RFXCOM: Init_rfxcom: Code retour                      ", fd );
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
       Info_c( Config.log, DEBUG_RFXCOM,
               "RFXCOM: Init_rfxcom: Ouverture port rfxcom okay", Config.port_rfxcom);
     }
    Info( Config.log, DEBUG_RFXCOM, "RFXCOM: Init_rfxcom: Sending INIT" );
    write (fd, &trame_reset, sizeof(trame_reset) );
    sleep(2);
    Info( Config.log, DEBUG_RFXCOM, "RFXCOM: Init_rfxcom: Sending SET ALL PROTO" );
    write (fd, &trame_set_all_proto, sizeof(trame_set_all_proto) );
    sleep(2);
    Info( Config.log, DEBUG_RFXCOM, "RFXCOM: Init_rfxcom: Sending GET STATUS" );
    write (fd, &trame_get_status, sizeof(trame_get_status) );
    return(fd);
  }
/**********************************************************************************************************/
/* Chercher_rfxcom: Retrouve un module/capteur dans la liste gérée en fonction des paramètres             */
/* Entrée: les paramètres de critères de recherche                                                        */
/* Sortie: le module, ou NULL si erreur                                                                   */
/**********************************************************************************************************/
 static struct MODULE_RFXCOM *Chercher_rfxcom ( gint type, gint canal )
  { GList *liste_modules;
    liste_modules = Partage->com_rfxcom.Modules_RFXCOM;
    while ( liste_modules )
     { struct MODULE_RFXCOM *module;
       module = (struct MODULE_RFXCOM *)liste_modules->data;

       if (module->rfxcom.type == type && module->rfxcom.canal == canal) return(module);
       liste_modules = liste_modules->next;
     }
    return(NULL);
  }
/**********************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                   */
/* Entrée: la trame a recue                                                                               */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static int Processer_trame( struct TRAME_RFXCOM *trame )
  { 

    Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame     taille: ", trame->taille );
    Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame       type: ", trame->type );
    Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame  sous_type: ", trame->sous_type );
    Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame      seqno: ", trame->seqno );

    if (trame->type == 0x01 && trame->sous_type == 0x00)
     { if (trame->data[0] == 0x52) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status 433MHz receiver only" );   
       if (trame->data[0] == 0x53) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status 433MHz transceiver" );   
       Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame get_status firmware", trame->data[1] );   
       if (trame->data[3] & 0x80) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto RFU" );   
       if (trame->data[3] & 0x40) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto Rollertroll" );   
       if (trame->data[3] & 0x20) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto Proguard" );   
       if (trame->data[3] & 0x10) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto FS20" );   
       if (trame->data[3] & 0x08) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto LaCrosse" );   
       if (trame->data[3] & 0x04) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto Hideki" );   
       if (trame->data[3] & 0x02) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto LightwaveRF" );   
       if (trame->data[3] & 0x01) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto Mertik" );   
       if (trame->data[4] & 0x80) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto Visonic" );   
       if (trame->data[4] & 0x40) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto ATI" );   
       if (trame->data[4] & 0x20) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto OregonScientific" );   
       if (trame->data[4] & 0x10) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto IkeaKoppla" );   
       if (trame->data[4] & 0x08) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto HomeEasy" );   
       if (trame->data[4] & 0x04) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto AC" );   
       if (trame->data[4] & 0x02) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto ARC" );   
       if (trame->data[4] & 0x01) Info( Config.log, DEBUG_RFXCOM,
                                         "RFXCOM: Processer_trame get_status proto X10" );   

     }
    else if (trame->type == 0x52 && trame->sous_type == 0x01)
     { struct MODULE_RFXCOM *module;
       Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame get_status id1", trame->data[0] );   
       Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame get_status id2", trame->data[1] );   
       Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame get_status high", trame->data[2] >> 1 );   
       Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame get_status signe", trame->data[2] & 1);   
       Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame get_status low", trame->data[3] );   
       Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame get_status hum", trame->data[4] );   
       Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame get_status humstatus", trame->data[5] );   
       Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame get_status battery", trame->data[6] >> 4 );   
       Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame get_status rssi", trame->data[6] & 0x0F );   
       module = Chercher_rfxcom( trame->type, trame->data[1] );
       if (module)
        { SEA( module->rfxcom.ea_min,     (trame->data[2] & 1 ? -1.0 : 1.0)*trame->data[3] / 10.0 );     /* Temp */
          SEA( module->rfxcom.ea_min + 1,  trame->data[4] );                                         /* Humidity */
          SEA( module->rfxcom.ea_min + 2,  trame->data[6] >> 4);                                      /* Battery */
          SEA( module->rfxcom.ea_min + 3,  trame->data[6] & 0x0F );                                      /* RSSI */

          module->date_last_view = Partage->top;
        }
       else Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame No matching module for packet received", trame->type );
     }
    else Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Processer_trame unkown packet type", trame->type );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Rfxcom                                                          */
/**********************************************************************************************************/
 void Run_rfxcom ( void )
  { struct TRAME_RFXCOM Trame;
    gint retval, nbr_oct_lu;
    struct timeval tv;
    fd_set fdselect;
    gint fd_rfxcom;

    prctl(PR_SET_NAME, "W-RFXCOM", 0, 0, 0 );
    Info( Config.log, DEBUG_RFXCOM, "RFXCOM: demarrage" );

    fd_rfxcom = Init_rfxcom();
    if (fd_rfxcom<0)                                                        /* On valide l'acces aux ports */
     { Info( Config.log, DEBUG_RFXCOM, "RFXCOM: Acces RFXCOM impossible, terminé");
       Partage->com_rfxcom.TID = 0;                        /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     }
    else { Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Acces RFXCOM FD", fd_rfxcom ); }

    Partage->com_rfxcom.Thread_run    = TRUE;                                        /* Le thread tourne ! */

    nbr_oct_lu = 0;
    while(Partage->com_rfxcom.Thread_run == TRUE)                         /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (Partage->com_rfxcom.Thread_reload == TRUE)
        { Info( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: Reloading conf" );
          Decharger_tous_rfxcom();
          Charger_tous_rfxcom();
          Partage->com_rfxcom.Thread_reload = FALSE;
        }

       if (Partage->com_rfxcom.Thread_sigusr1 == TRUE)
        { Info( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: SIGUSR1" );
          Partage->com_rfxcom.Thread_sigusr1 = FALSE;
        }

       FD_ZERO(&fdselect);                                         /* Reception sur la ligne serie RFXCOM */
       FD_SET(fd_rfxcom, &fdselect );
       tv.tv_sec = 1;
       tv.tv_usec= 0;
       retval = select(fd_rfxcom+1, &fdselect, NULL, NULL, &tv );               /* Attente d'un caractere */
       if (retval>=0 && FD_ISSET(fd_rfxcom, &fdselect) )
        { int bute, cpt;

          if (nbr_oct_lu<TAILLE_ENTETE_RFXCOM)
           { bute = TAILLE_ENTETE_RFXCOM; } else { bute = sizeof(Trame); }

          cpt = read( fd_rfxcom, (unsigned char *)&Trame + nbr_oct_lu, bute-nbr_oct_lu );
          if (cpt>0)
           { nbr_oct_lu = nbr_oct_lu + cpt;

             if (nbr_oct_lu >= TAILLE_ENTETE_RFXCOM + Trame.taille)                   /* traitement trame */
              { nbr_oct_lu = 0;
                if (Trame.taille > 0) Processer_trame( &Trame );
                memset (&Trame, 0, sizeof(struct TRAME_RFXCOM) );
              }
           }
        }
     }                                                                     /* Fin du while partage->arret */

    close(fd_rfxcom);
    Info_n( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: Down", pthread_self() );
    Partage->com_rfxcom.TID = 0;                           /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
