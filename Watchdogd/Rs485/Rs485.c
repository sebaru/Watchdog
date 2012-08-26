/**********************************************************************************************************/
/* Watchdogd/Rs485/Rs485.c  Gestion des modules rs485 Watchdgo 2.0                                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 21 aoû 2005 17:09:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Rs485.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
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
 #include <stdio.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/time.h>
 #include <sys/stat.h>
 #include <sys/prctl.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>

 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
 #include "Rs485.h"

/**********************************************************************************************************/
/* Rs485_Lire_config : Lit la config Watchdog et rempli la structure mémoire                               */
/* Entrée: le pointeur sur la LIBRAIRIE                                                                   */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rs485_Lire_config ( void )
  { gchar *chaine;
    GKeyFile *gkf;

    gkf = g_key_file_new();
    if ( ! g_key_file_load_from_file(gkf, Config.config_file, G_KEY_FILE_NONE, NULL) )
     { Info_new( Config.log, TRUE, LOG_CRIT,
                 "Rs485_Lire_config : unable to load config file %s", Config.config_file );
       return;
     }
                                                                               /* Positionnement du debug */
    Cfg_rs485.lib->Thread_debug = g_key_file_get_boolean ( gkf, "RS485", "debug", NULL ); 
                                                                 /* Recherche des champs de configuration */


    chaine = g_key_file_get_string ( gkf, "RS485", "port", NULL );
    if (!chaine)
     { Info_new ( Config.log, Cfg_rs485.lib->Thread_debug, LOG_ERR,
                  "Rs485_Lire_config: port is missing. Using default." );
       g_snprintf( Cfg_rs485.port, sizeof(Cfg_rs485.port), DEFAUT_PORT_RS485 );
     }
    else
     { g_snprintf( Cfg_rs485.port, sizeof(Cfg_rs485.port), "%s", chaine );
       g_free(chaine);
     }
    g_key_file_free(gkf);
  }
/**********************************************************************************************************/
/* Rs485_Liberer_config : Libere la mémoire allouer précédemment pour lire la config imsg                  */
/* Entrée: néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Rs485_Liberer_config ( void )
  {
  }
/**********************************************************************************************************/
/* Retirer_rs485DB: Elimination d'un module rs485                                                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_rs485DB ( struct RS485DB *rs485 )
  { gchar requete[200];
    gboolean retour;
    struct DB *db;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MODULE_RS485, rs485->id );

    retour = Lancer_requete_SQL ( Config.log, db, requete );               /* Execution de la requete SQL */
    Libere_DB_SQL( Config.log, &db );
    Cfg_rs485.reload = TRUE;                       /* Rechargement des modules RS en mémoire de travaille */
    return(retour);
  }
/**********************************************************************************************************/
/* Ajouter_rs485DB: Ajout ou edition d'un rs485                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure rs485                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_rs485DB ( struct RS485DB *rs485 )
  { gchar requete[2048];
    gboolean retour;
    gchar *libelle;
    struct DB *db;
    gint last_id;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

    libelle = Normaliser_chaine ( Config.log, rs485->libelle );          /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_WARNING,
                 "Ajouter_rs485DB: Normalisation libelle impossible" );
       Libere_DB_SQL( Config.log, &db );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s(num,bit_comm,libelle,enable,ea_min,ea_max,e_min,e_max,"
                "s_min,s_max,sa_min,sa_max) "
                " VALUES ('%d','%d','%s','%d','%d','%d','%d','%d','%d','%d','%d','%d')",
                NOM_TABLE_MODULE_RS485, rs485->num, rs485->bit_comm, libelle, rs485->enable,
                rs485->ea_min, rs485->ea_max, rs485->e_min, rs485->e_max,
                rs485->s_min, rs485->s_max, rs485->sa_min, rs485->sa_max
              );
    g_free(libelle);

    retour = Lancer_requete_SQL ( Config.log, db, requete );               /* Execution de la requete SQL */
    if (retour == FALSE)  { Libere_DB_SQL( Config.log, &db );
                            return(-1);
                          }
    last_id = Recuperer_last_ID_SQL( Config.log, db );
    Libere_DB_SQL( Config.log, &db );
    Cfg_rs485.reload = TRUE;                       /* Rechargement des modules RS en mémoire de travaille */
    return( last_id );
  }
/**********************************************************************************************************/
/* Modifier_rs485DB: Modification d'un rs485 Watchdog                                                     */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_rs485DB( struct RS485DB *rs485 )
  { gchar requete[2048];
    gboolean retour;
    gchar *libelle;
    struct DB *db;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

    libelle = Normaliser_chaine ( Config.log, rs485->libelle );              /* Formatage correct des chaines */
    if (!libelle)
     { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_ERR,
                 "Modifier_rs485DB: Normalisation libelle impossible" );
       Libere_DB_SQL( Config.log, &db );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "num='%d',bit_comm='%d',libelle='%s',enable='%d',"
                "ea_min='%d',ea_max='%d',e_min='%d',e_max='%d',"
                "sa_min='%d',sa_max='%d',s_min='%d',s_max='%d'"
                " WHERE id=%d",
                NOM_TABLE_MODULE_RS485,
                rs485->num, rs485->bit_comm, libelle, rs485->enable,
                rs485->ea_min, rs485->ea_max, rs485->e_min, rs485->e_max,
                rs485->sa_min, rs485->sa_max, rs485->s_min, rs485->s_max,
                rs485->id );
    g_free(libelle);
    retour = Lancer_requete_SQL ( Config.log, db, requete );               /* Execution de la requete SQL */
    Libere_DB_SQL( Config.log, &db );
    Cfg_rs485.reload = TRUE;                       /* Rechargement des modules RS en mémoire de travaille */
    return( retour );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_rs485DB: Recupération de la liste des ids des rs485s                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static gboolean Recuperer_rs485DB ( struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,num,bit_comm,libelle,enable,ea_min,ea_max,e_min,e_max,"
                "sa_min,sa_max,s_min,s_max"
                " FROM %s ORDER BY num", NOM_TABLE_MODULE_RS485 );

    return ( Lancer_requete_SQL ( Config.log, db, requete ) );             /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_rs485DB: Recupération de la liste des ids des rs485s                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static struct RS485DB *Recuperer_rs485DB_suite( struct DB *db )
  { struct RS485DB *rs485;

    Recuperer_ligne_SQL (Config.log, db);                              /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( Config.log, db );
       return(NULL);
     }

    rs485 = (struct RS485DB *)g_malloc0( sizeof(struct RS485DB) );
    if (!rs485) Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_ERR,
                          "Recuperer_rs485DB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &rs485->libelle, db->row[3], sizeof(rs485->libelle) );
       rs485->id                = atoi(db->row[0]);
       rs485->num               = atoi(db->row[1]);
       rs485->bit_comm          = atoi(db->row[2]);
       rs485->enable            = atoi(db->row[4]);
       rs485->ea_min            = atoi(db->row[5]);
       rs485->ea_max            = atoi(db->row[6]);
       rs485->e_min             = atoi(db->row[7]);
       rs485->e_max             = atoi(db->row[8]);
       rs485->sa_min            = atoi(db->row[9]);
       rs485->sa_max            = atoi(db->row[10]);
       rs485->s_min             = atoi(db->row[11]);
       rs485->s_max             = atoi(db->row[12]);
     }
    return(rs485);
  }
/**********************************************************************************************************/
/* Chercher_module_rs485_by_id: Recherche le module dont l'id est en parametre                            */
/* Entrée: l'id du module                                                                                 */
/* Sortie: le module, ou NULL si non trouvé                                                               */
/**********************************************************************************************************/
 struct MODULE_RS485 *Chercher_module_rs485_by_id ( gint id )
  { struct MODULE_RS485 *module;
    GSList *liste;
    module = NULL;
    pthread_mutex_lock ( &Cfg_rs485.lib->synchro );
    liste = Cfg_rs485.Modules_RS485;
    while ( liste )
     { module = ((struct MODULE_RS485 *)liste->data);
       if (module->rs485.id == id) break;
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Cfg_rs485.lib->synchro );
    if (liste) return(module);
    return(NULL);
  }
/**********************************************************************************************************/
/* Charger_tous_RS485: Requete la DB pour charger les modules et les bornes rs485                         */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static gboolean Charger_tous_rs485 ( void  )
  { struct DB *db;
    gint cpt;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

/********************************************** Chargement des modules ************************************/
    if ( ! Recuperer_rs485DB( db ) )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    Cfg_rs485.Modules_RS485 = NULL;
    cpt = 0;
    for ( ; ; )
     { struct MODULE_RS485 *module;
       struct RS485DB *rs485;

       rs485 = Recuperer_rs485DB_suite( db );
       if (!rs485) break;

       module = (struct MODULE_RS485 *)g_malloc0( sizeof(struct MODULE_RS485) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_ERR,
                    "Charger_tous_RS485: Erreur allocation mémoire struct MODULE_RS485" );
          g_free(rs485);
          Libere_DB_SQL( Config.log, &db );
          return(FALSE);
        }
       memcpy( &module->rs485, rs485, sizeof(struct RS485DB) );
       if (module->rs485.enable) module->started = TRUE;          /* Si enable at boot... et bien Start ! */
       g_free(rs485);
                                                                        /* Ajout dans la liste de travail */
       pthread_mutex_lock ( &Cfg_rs485.lib->synchro );
       Cfg_rs485.Modules_RS485 = g_slist_prepend ( Cfg_rs485.Modules_RS485, module );
       pthread_mutex_unlock ( &Cfg_rs485.lib->synchro );

       Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_INFO,
                 "Charger_tous_RS485: id = %d, enable = %d", module->rs485.id, module->rs485.enable );
     }
    pthread_mutex_lock ( &Cfg_rs485.lib->synchro );
    Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_INFO,
              "Charger_tous_RS485: %03d module RS485 found  !", g_slist_length(Cfg_rs485.Modules_RS485) );
    pthread_mutex_unlock ( &Cfg_rs485.lib->synchro );

    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_tous_rs485 ( void  )
  { struct MODULE_RS485 *module;

    pthread_mutex_lock ( &Cfg_rs485.lib->synchro );
    while ( Cfg_rs485.Modules_RS485 )
     { module = (struct MODULE_RS485 *)Cfg_rs485.Modules_RS485->data;
       Cfg_rs485.Modules_RS485 = g_slist_remove ( Cfg_rs485.Modules_RS485, module );
       g_free(module);
     }
    pthread_mutex_unlock ( &Cfg_rs485.lib->synchro );
  }
/**********************************************************************************************************/
/* RS485_is_actif: Renvoi TRUE si au moins un des modules rs est actif                                    */
/* Entrée: rien                                                                                           */
/* Sortie: TRUE/FALSE                                                                                     */
/**********************************************************************************************************/
 static gboolean Rs485_is_actif ( void )
  { GSList *liste;
    liste = Cfg_rs485.Modules_RS485;
    pthread_mutex_lock ( &Cfg_rs485.lib->synchro );
    while ( liste )
     { struct MODULE_RS485 *module;
       module = ((struct MODULE_RS485 *)liste->data);

       if (module->started) break;
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Cfg_rs485.lib->synchro );
    if (liste) return(TRUE);
    return(FALSE);
  }
/**********************************************************************************************************/
/* Deconnecter_rs485: Deconnecte un module RS485 de la liste des modules interrogés                       */
/* Entrée: le module                                                                                      */
/* Sortie: void                                                                                           */
/**********************************************************************************************************/
 static void Deconnecter_rs485 ( struct MODULE_RS485 *module )
  { gint nbr_ea, cpt;
                
    if (!module) return;
    if (module->rs485.ea_min == -1) nbr_ea = 0;
    else nbr_ea = module->rs485.ea_max - module->rs485.ea_min + 1;
    for( cpt = 0; cpt<nbr_ea; cpt++)
     { SEA_range( module->rs485.ea_min + cpt, 0 ); }
    SB(module->rs485.bit_comm, 0);
  }
/**********************************************************************************************************/
/* Calcul_crc16: renvoie le CRC16 de la trame en parametre                                                */
/* Entrée: la trame a tester                                                                              */
/* Sortie: le crc 16 bits                                                                                 */
/**********************************************************************************************************/
 static int Calcul_crc16 (struct TRAME_RS485 *Trame)  
  { unsigned int index_bits;                                    /* nombre de bits a traiter dans un octet */
    unsigned int retenue;                        /* valeur de la retenue éventuelle suite aux calculs CRC */
    unsigned int index_octets;                              /* position des octets formant la trame RS485 */
    unsigned short CRC16;                                      /* Valeur du CRC16 lié à la trame en cours */

    CRC16 = 0xFFFF;                                                              /* initialisation à FFFF */

    for ( index_octets=0; index_octets<TAILLE_ENTETE-2+Trame->taille; index_octets++ )
     {                                                                         /* CRC OUEX octet en cours */
       CRC16 = CRC16 ^ (short)*((unsigned char *)Trame + index_octets);
       for( index_bits = 0; index_bits<8; index_bits++ )
        { retenue = CRC16 & 1;                             /* Récuperer la retenue avant traitement CRC16 */ 
          CRC16 = CRC16 >> 1;                                      /* décalage d'un bit à droite du CRC16 */
          if (retenue == 1)
           { CRC16 = CRC16 ^ 0xA001; }                                         /* CRC16 OUEX A001 en hexa */
        }
     }
    return( (int)CRC16 );
  }
/**********************************************************************************************************/
/* Envoyer_trame: envoie d'une trame RS485 sur la ligne                                                   */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Envoyer_trame( int fd, struct TRAME_RS485 *trame )
  { int crc16;

    crc16 = Calcul_crc16 (trame);
    trame->crc16_h = crc16 >> 8;
    trame->crc16_l = crc16 & 0xFF;
/*    printf("crc16 calculÃ© Ã  l'envoi: %04X  %2X %2X  taille %d\n", crc16, trame->crc16_h, trame->crc16_l,
                                                                  trame->taille );*/

    write( fd, trame, TAILLE_ENTETE - 2 );                                        /* Ecriture de l'entete */
    if (trame->taille) { write( fd, &trame->donnees, trame->taille ); }          /* On envoie les données */
    write( fd, &trame->crc16_h, sizeof(unsigned char) );
    write( fd, &trame->crc16_l, sizeof(unsigned char) );
  }
/**********************************************************************************************************/
/* Envoyer_trame: envoie d'une trame RS485 sur la ligne                                                   */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Envoyer_trame_want_inputANA( struct MODULE_RS485 *module, int fd )
  { static struct TRAME_RS485 Trame_want_entre_ana=
     { 0x00, 0xFF, RS485_FCT_ENTRE_ANA, 0x00,
   	  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
       0xFF, 0xFF
     };
    Trame_want_entre_ana.dest = module->rs485.id;
    Envoyer_trame( fd, &Trame_want_entre_ana );
  }
/**********************************************************************************************************/
/* Envoyer_trame: envoie d'une trame RS485 sur la ligne                                                   */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Envoyer_trame_want_inputTOR( struct MODULE_RS485 *module, int fd )
  { static struct TRAME_RS485 Trame_want_entre_tor=
     { 0x00, 0xFF, RS485_FCT_ENTRE_TOR, 0x00,
   	  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
       0xFF, 0xFF
     };
    Trame_want_entre_tor.dest = module->rs485.id;
    if (module->rs485.s_min != -1 )
     { guchar poids, a, octet, taille;
       gint num_a;
       poids = 0x80;
       taille = 1;
       octet = 0;
       for ( num_a =  module->rs485.s_min;
             num_a <= module->rs485.s_max;
             num_a ++ )
        { a = A(num_a);
          if (a) octet |= poids;
          if (poids == 1)
           { Trame_want_entre_tor.donnees[taille-1] = octet;
             taille++;
             octet = 0;
             poids = 0x80;
           }
          else
           { poids = poids >> 1; }
        }
       Trame_want_entre_tor.donnees[taille-1] = octet;
       Trame_want_entre_tor.taille = taille;
     }
    else
     { Trame_want_entre_tor.taille = 0; }
    Envoyer_trame( fd, &Trame_want_entre_tor );
  }
/**********************************************************************************************************/
/* Init_rs485: Initialisation de la ligne RS485                                                           */
/* Sortie: l'identifiant de la connexion                                                                  */
/**********************************************************************************************************/
 static int Init_rs485 ( void )
  { struct termios oldtio;
    int fd;

    fd = open( Cfg_rs485.port, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd<0)
     { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_ERR,
               "Init_rs485: Impossible d'ouvrir le port %s, retour=%d", Cfg_rs485.port, fd );
     }
    else
     { memset(&oldtio, 0, sizeof(oldtio) );
       oldtio.c_cflag = B19200 | CS8 | CREAD | CLOCAL;
       oldtio.c_oflag = 0;
       oldtio.c_iflag = 0;
       oldtio.c_lflag = 0;
       oldtio.c_cc[VTIME]    = 0;
       oldtio.c_cc[VMIN]     = 0;
       tcsetattr(fd, TCSANOW, &oldtio);
       tcflush(fd, TCIOFLUSH);
       Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_INFO,
               "Init_rs485: Ouverture port rs485 okay %s", Cfg_rs485.port );
     }
    return(fd);
  }
/**********************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                   */
/* Entrée: la trame a recue                                                                               */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static int Processer_trame( struct MODULE_RS485 *module, struct TRAME_RS485 *trame )
  { struct TRAME_RS485_IDENT *trame_ident;
    if (trame->dest != 0xFF) return(FALSE);                        /* Si c'est pas pour nous, on se casse */

    if (module->rs485.id != trame->source)
     { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_NOTICE,
                 "Processer_trame: Module RS485 unknown source=%d", trame->source);
       return(TRUE);
     }

    switch( trame->fonction )
     { case RS485_FCT_IDENT: printf("bouh\n");
	               trame_ident = (struct TRAME_RS485_IDENT *)trame->donnees;
                       printf("Recu Ident de %d: version %d.%d, nbr ana %d, nbr tor %d (%d choc), sortie %d\n",
                              trame->source, trame_ident->version_major, trame_ident->version_minor,
                              trame_ident->nbr_entre_ana, trame_ident->nbr_entre_tor,
                              trame_ident->nbr_entre_choc, trame_ident->nbr_sortie_tor );
                       break;
       case RS485_FCT_ENTRE_TOR:
             { int e, cpt, nbr_e;
               nbr_e = module->rs485.e_max - module->rs485.e_min + 1;
               for( cpt = 0; cpt<nbr_e; cpt++)
                { e = ! (trame->donnees[cpt >> 3] & (0x80 >> (cpt & 0x07)));
                  SE( module->rs485.e_min + cpt, e );
                }
             }
	    break;
       case RS485_FCT_ENTRE_ANA:
             { int cpt, nbr_ea;
               if (module->rs485.ea_min == -1) nbr_ea = 0;
               else nbr_ea = module->rs485.ea_max - module->rs485.ea_min + 1;
               for( cpt = 0; cpt<nbr_ea; cpt++)
                { gint val_avant_ech, ajout1, ajout2, ajout3, ajout4, ajout5;

                  val_avant_ech =   trame->donnees[cpt] << 2;
                  ajout1        =   trame->donnees[nbr_ea + (cpt >> 2)];
                  ajout2        = 0xC0>>((cpt & 0x03)<<1);
                  ajout3        = (3-(cpt & 0x03))<<1;
                  ajout4        = ajout1  & ajout2;
                  ajout5        = ajout4 >> ajout3;
                  val_avant_ech += ajout5;

                  SEA( module->rs485.ea_min + cpt, 1.0*val_avant_ech );
                }
             }
	    break;
       default: printf("Trame non traitée\n"); return(FALSE);
     }
    return(TRUE);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du RS485                                                                     */
/**********************************************************************************************************/
 void Run_thread ( struct LIBRAIRIE *lib )
  { gint retval, nbr_oct_lu, id_en_cours, attente_reponse;
    struct MODULE_RS485 *module;
    struct TRAME_RS485 Trame;
    struct timeval tv;
    fd_set fdselect;
    GSList *liste;

    prctl(PR_SET_NAME, "W-RS485", 0, 0, 0 );
    memset( &Cfg_rs485, 0, sizeof(Cfg_rs485) );                 /* Mise a zero de la structure de travail */
    Cfg_rs485.lib = lib;                       /* Sauvegarde de la structure pointant sur cette librairie */
    Rs485_Lire_config ();                               /* Lecture de la configuration logiciel du thread */

    Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Demarrage . . . TID = %d", pthread_self() );
    Cfg_rs485.lib->Thread_run = TRUE;                                               /* Le thread tourne ! */

    g_snprintf( Cfg_rs485.lib->admin_prompt, sizeof(Cfg_rs485.lib->admin_prompt), "rs485" );
    g_snprintf( Cfg_rs485.lib->admin_help,   sizeof(Cfg_rs485.lib->admin_help),   "Manage RS485 Modules" );

    Cfg_rs485.fd = Init_rs485();
    if (Cfg_rs485.fd<0)                                                    /* On valide l'acces aux ports */
     { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_CRIT,
                 "Run_thread: Acces RS485 impossible, Thread down");
       Rs485_Liberer_config ();                         /* Lecture de la configuration logiciel du thread */
       lib->Thread_run = FALSE;                                             /* Le thread ne tourne plus ! */
       lib->TID        = 0;                               /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     }
    else { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_INFO, 
                     "Run_thread: Acces RS485 FD=%d", Cfg_rs485.fd );
         }

    Cfg_rs485.Modules_RS485 = NULL;                             /* Initialisation des variables du thread */
    Charger_tous_rs485();                                                 /* Chargement des modules rs485 */

    nbr_oct_lu = 0;
    id_en_cours = 0;
    attente_reponse = FALSE;

    while(lib->Thread_run == TRUE)                                       /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (Cfg_rs485.reload == TRUE)
        { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_NOTICE,
                    "Run_thread: Run_rs485: Reloading conf" );
          Decharger_tous_rs485();
          close(Cfg_rs485.fd);
          Cfg_rs485.fd = Init_rs485();
          if (Cfg_rs485.fd<0)                                              /* On valide l'acces aux ports */
           { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_CRIT,
                       "Run_thread: Restart Acces RS485 impossible, terminé");
             Rs485_Liberer_config ();                   /* Lecture de la configuration logiciel du thread */
             lib->Thread_run = FALSE;                                       /* Le thread ne tourne plus ! */
             lib->TID        = 0;                         /* On indique au master que le thread est mort. */
             pthread_exit(GINT_TO_POINTER(-1));
           }
          Charger_tous_rs485();
          Cfg_rs485.reload = FALSE;
        }

       if (lib->Thread_sigusr1 == TRUE)
        { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_NOTICE,
                    "Run_thread: Run_rs485: SIGUSR1" );
          lib->Thread_sigusr1 = FALSE;
        }

       if (Cfg_rs485.admin_start)
        { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_INFO,
                    "Run_thread: Run_rs485: Starting module" );
          if (Rs485_is_actif() == FALSE)                /* Si aucun module started, on restart la comm RS */
           { Cfg_rs485.fd = Init_rs485();
             if (Cfg_rs485.fd<0)                                           /* On valide l'acces aux ports */
              { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_CRIT,
                          "Run_thread: Restart Acces RS485 impossible, terminé");
                Cfg_rs485.lib->TID = 0;                   /* On indique au master que le thread est mort. */
                pthread_exit(GINT_TO_POINTER(-1));
              }
           }
          module = Chercher_module_rs485_by_id ( Cfg_rs485.admin_start );
          if (module) { module->started = 1; }
          Cfg_rs485.admin_start = 0;
        }

       if (Cfg_rs485.admin_stop)
        { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_INFO,
                    "Run_thread: Run_rs485: Stopping module" );
          module = Chercher_module_rs485_by_id ( Cfg_rs485.admin_stop );
          if (module) module->started = 0;
          Deconnecter_rs485 ( module );
          Cfg_rs485.admin_stop = 0;
          if (Rs485_is_actif() == FALSE)                  /* Si aucun module actif, on restart la comm RS */
           { close(Cfg_rs485.fd);
             Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_NOTICE,
                       "Run_thread: Restart FileDescriptor sur All RS down");
           }
        }

       if (Cfg_rs485.Modules_RS485 == NULL )                    /* Si pas de module référencés, on attend */
        { sleep(2); continue; }

       pthread_mutex_lock ( &Cfg_rs485.lib->synchro );                  /* Car utilisation de la liste chainée */
       liste = Cfg_rs485.Modules_RS485;
       while (liste && lib->Thread_run == TRUE)
        { module = (struct MODULE_RS485 *)liste->data;
          if (module->started != TRUE)                           /* Si le module est stopped, on le zappe */
           { liste = liste->next;
             continue;
           }

          if ( attente_reponse == FALSE )
           { if ( module->date_retente <= Partage->top )                         /* module banni ou non ? */
              { if (module->date_next_get_ana > Partage->top)               /* Ana toutes les 10 secondes */
                 { Envoyer_trame_want_inputTOR( module, Cfg_rs485.fd );
                 }
                else
                 { Envoyer_trame_want_inputANA( module, Cfg_rs485.fd );
                                                                   /* Prochain update ana dans 2 secondes */
                   module->date_next_get_ana = Partage->top + RS485_TEMPS_UPDATE_IO_ANA;
                 }

                usleep(1);
                module->date_requete = Partage->top;
                module->date_retente = 0;
                attente_reponse = TRUE;
              }
           }
          else
           { if ( Partage->top - module->date_requete >= RS485_TEMPS_SEUIL_DOWN )  /* Si la comm est niet */
              { module->date_retente = Partage->top + RS485_TEMPS_RETENTE;
                attente_reponse = FALSE;
                memset (&Trame, 0, sizeof(struct TRAME_RS485) );
                nbr_oct_lu = 0;
                Deconnecter_rs485 ( module );
                Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_WARNING,
                          "Run_thread: Run_rs485: module %03d down", module->rs485.id );
                liste = liste->next;
                continue;
              }
             else { module->date_retente = 0;
                  }
           }

          FD_ZERO(&fdselect);                                       /* Reception sur la ligne serie RS485 */
          FD_SET(Cfg_rs485.fd, &fdselect );
          tv.tv_sec = 1;
          tv.tv_usec= 0;
          retval = select(Cfg_rs485.fd+1, &fdselect, NULL, NULL, &tv );         /* Attente d'un caractere */
          if (retval>=0 && FD_ISSET(Cfg_rs485.fd, &fdselect) )
	   { int bute, cpt;
             if (nbr_oct_lu<TAILLE_ENTETE)
	      { bute = TAILLE_ENTETE; } else { bute = sizeof(Trame); }
 
             cpt = read( Cfg_rs485.fd, (unsigned char *)&Trame + nbr_oct_lu, bute-nbr_oct_lu );
             if (cpt>0)
              { nbr_oct_lu = nbr_oct_lu + cpt;
   	        if (nbr_oct_lu >= TAILLE_ENTETE + Trame.taille)                       /* traitement trame */
                 { int crc_recu;
                   nbr_oct_lu = 0;
#ifdef bouh
                   for (cpt=0; cpt<sizeof(Trame); cpt++)
                     { printf("%02X ",(unsigned char)*((unsigned char *)&Trame +cpt) ); }
                   printf(" entete   = %d nbr_lu = %d\n", TAILLE_ENTETE, nbr_oct_lu );
                   printf(" dest     = %d\n", Trame.dest );
                   printf(" source   = %d\n", Trame.source );
                   printf(" fonction = %d\n", Trame.fonction );
                   printf(" taille   = %d\n", Trame.taille );
#endif
                   crc_recu =   *((unsigned char *)&Trame + TAILLE_ENTETE + Trame.taille - 1) & 0xFF;
                   crc_recu += (*((unsigned char *)&Trame + TAILLE_ENTETE + Trame.taille - 2) & 0xFF)<<8;
                   if (crc_recu != Calcul_crc16(&Trame))
                    { Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_INFO,
                                "Run_thread: CRC16 failed !!");
                    }
                   else
                    { if (Processer_trame( module, &Trame ))/* Si la trame est processée, on passe suivant */
                       { attente_reponse = FALSE;
                         liste = liste->next;
                         SB(module->rs485.bit_comm, 1);
                       }
                    }
                   memset (&Trame, 0, sizeof(struct TRAME_RS485) );
                 }
              }
	   }
        }                                                                           /* Fin du While liste */
       pthread_mutex_unlock ( &Cfg_rs485.lib->synchro );                /* Car utilisation de la liste chainée */
      }                                                                    /* Fin du while partage->arret */
    close(Cfg_rs485.fd);
    Decharger_tous_rs485();
    Info_new( Config.log, Cfg_rs485.lib->Thread_debug, LOG_NOTICE,
              "Run_thread: Down . . . TID = %d", pthread_self() );
    Cfg_rs485.lib->TID = 0;                               /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
