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

 #define TEMPS_RETENTE   50               /* Tente de se raccrocher au module banni toutes les 5 secondes */

 static gint fd_rs485;

/**********************************************************************************************************/
/* Retirer_rs485DB: Elimination d'un module rs485                                                         */
/* Entrée: un log et une database                                                                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Retirer_rs485DB ( struct LOG *log, struct DB *db, struct CMD_TYPE_RS485 *rs485 )
  { gchar requete[200];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "DELETE FROM %s WHERE id=%d", NOM_TABLE_MODULE_RS485, rs485->id );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Ajouter_rs485DB: Ajout ou edition d'un rs485                                                           */
/* Entrée: un log et une database, un flag d'ajout/edition, et la structure rs485                         */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gint Ajouter_rs485DB ( struct LOG *log, struct DB *db, struct CMD_TYPE_RS485 *rs485 )
  { gchar requete[2048];
    gchar *libelle;

    libelle = Normaliser_chaine ( log, rs485->libelle );                 /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Ajouter_rs485DB: Normalisation libelle impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),
                "INSERT INTO %s(num,bit_comm,libelle,actif,ea_min,ea_max,e_min,e_max,"
                "s_min,s_max,sa_min,sa_max) "
                " VALUES ('%d','%d','%s','%d','%d','%d','%d','%d','%d','%d','%d','%d')",
                NOM_TABLE_MODULE_RS485, rs485->num, rs485->bit_comm, libelle, rs485->actif,
                rs485->ea_min, rs485->ea_max, rs485->e_min, rs485->e_max,
                rs485->s_min, rs485->s_max, rs485->sa_min, rs485->sa_max
              );
    g_free(libelle);

    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(-1); }
    return( Recuperer_last_ID_SQL( log, db ) );
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_rs485DB: Recupération de la liste des ids des rs485s                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 gboolean Recuperer_rs485DB ( struct LOG *log, struct DB *db )
  { gchar requete[256];

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,num,bit_comm,libelle,actif,ea_min,ea_max,e_min,e_max,"
                "sa_min,sa_max,s_min,s_max"
                " FROM %s ORDER BY num", NOM_TABLE_MODULE_RS485 );

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Recuperer_liste_id_rs485DB: Recupération de la liste des ids des rs485s                                */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_RS485 *Recuperer_rs485DB_suite( struct LOG *log, struct DB *db )
  { struct CMD_TYPE_RS485 *rs485;

    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       return(NULL);
     }

    rs485 = (struct CMD_TYPE_RS485 *)g_malloc0( sizeof(struct CMD_TYPE_RS485) );
    if (!rs485) Info( log, DEBUG_RS485, "Recuperer_rs485DB_suite: Erreur allocation mémoire" );
    else
     { memcpy( &rs485->libelle, db->row[3], sizeof(rs485->libelle) );
       rs485->id                = atoi(db->row[0]);
       rs485->num               = atoi(db->row[1]);
       rs485->bit_comm          = atoi(db->row[2]);
       rs485->actif             = atoi(db->row[4]);
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
/* Rechercher_rs485DB: Recupération du rs485 dont le id est en parametre                                  */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 struct CMD_TYPE_RS485 *Rechercher_rs485DB ( struct LOG *log, struct DB *db, guint id )
  { gchar requete[512];
    struct CMD_TYPE_RS485 *rs485;

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "SELECT id,num,bit_comm,libelle,actif,ea_min,ea_max,e_min,e_max,"
                "sa_min,sa_max,s_min,s_max"
                " FROM %s WHERE id=%d",
                NOM_TABLE_MODULE_RS485, id );
       
    if ( Lancer_requete_SQL ( log, db, requete ) == FALSE )
     { return(NULL); }
       
    Recuperer_ligne_SQL (log, db);                                     /* Chargement d'une ligne resultat */
    if ( ! db->row )
     { Liberer_resultat_SQL ( log, db );
       Info_n( log, DEBUG_DB, "Rechercher_rs485DB: RS485 non trouvé dans la BDD", id );
       return(NULL);
     }
       
    rs485 = g_malloc0( sizeof(struct CMD_TYPE_RS485) );
    if (!rs485)
     { Info( log, DEBUG_RS485, "Rechercher_rs485DB: Mem error" ); }
    else
     { memcpy( &rs485->libelle, db->row[3], sizeof(rs485->libelle) );
       rs485->id                = atoi(db->row[0]);
       rs485->num               = atoi(db->row[1]);
       rs485->bit_comm          = atoi(db->row[2]);
       rs485->actif             = atoi(db->row[4]);
       rs485->ea_min            = atoi(db->row[5]);
       rs485->ea_max            = atoi(db->row[6]);
       rs485->e_min             = atoi(db->row[7]);
       rs485->e_max             = atoi(db->row[8]);
       rs485->sa_min            = atoi(db->row[9]);
       rs485->sa_max            = atoi(db->row[10]);
       rs485->s_min             = atoi(db->row[11]);
       rs485->s_max             = atoi(db->row[12]);
     }

    Liberer_resultat_SQL ( log, db );
    return(rs485);
  }
/**********************************************************************************************************/
/* Modifier_rs485DB: Modification d'un rs485 Watchdog                                                     */
/* Entrées: un log, une db et une clef de cryptage, une structure utilisateur.                            */
/* Sortie: -1 si pb, id sinon                                                                             */
/**********************************************************************************************************/
 gboolean Modifier_rs485DB( struct LOG *log, struct DB *db, struct CMD_TYPE_RS485 *rs485 )
  { gchar *libelle;
    gchar requete[2048];

    libelle = Normaliser_chaine ( log, rs485->libelle );              /* Formatage correct des chaines */
    if (!libelle)
     { Info( log, DEBUG_DB, "Modifier_rs485DB: Normalisation libelle impossible" );
       return(-1);
     }

    g_snprintf( requete, sizeof(requete),                                                  /* Requete SQL */
                "UPDATE %s SET "             
                "num='%d',bit_comm='%d',libelle='%s',actif='%d',"
                "ea_min='%d',ea_max='%d',e_min='%d',e_max='%d',"
                "sa_min='%d',sa_max='%d',s_min='%d',s_max='%d'"
                " WHERE id=%d",
                NOM_TABLE_MODULE_RS485,
                rs485->num, rs485->bit_comm, libelle, rs485->actif,
                rs485->ea_min, rs485->ea_max, rs485->e_min, rs485->e_max,
                rs485->sa_min, rs485->sa_max, rs485->s_min, rs485->s_max,
                rs485->id );
    g_free(libelle);

    return ( Lancer_requete_SQL ( log, db, requete ) );                    /* Execution de la requete SQL */
  }
/**********************************************************************************************************/
/* Charger_tous_RS485: Requete la DB pour charger les modules et les bornes rs485                         */
/* Entrée: rien                                                                                           */
/* Sortie: le nombre de modules trouvé                                                                    */
/**********************************************************************************************************/
 static struct MODULE_RS485 *Chercher_module_by_id ( gint id )
  { GList *liste;
    liste = Partage->com_rs485.Modules_RS485;
    while ( liste )
     { struct MODULE_RS485 *module;
       module = ((struct MODULE_RS485 *)liste->data);
       if (module->rs485.id == id) return(module);
       liste = liste->next;
     }
    return(NULL);
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static gboolean Charger_un_rs485 ( gint id )
  { struct MODULE_RS485 *module;
    struct CMD_TYPE_RS485 *rs485;
    struct DB *db;

    db = Init_DB_SQL( Config.log );
    if (!db) return(FALSE);

    module = (struct MODULE_RS485 *)g_malloc0(sizeof(struct MODULE_RS485));
    if (!module)                                                      /* Si probleme d'allocation mémoire */
     { Info( Config.log, DEBUG_RS485,
             "Charger_un_rs485: Erreur allocation mémoire struct MODULE_RS485" );
       Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    rs485 = Rechercher_rs485DB ( Config.log, db, id );
    Libere_DB_SQL( Config.log, &db );
    if (!rs485)                                                 /* Si probleme d'allocation mémoire */
     { Info( Config.log, DEBUG_RS485,
             "Charger_un_rs485: Erreur allocation mémoire struct CMD_TYPE_RS485" );
       g_free(module);
       return(FALSE);
     }

    memcpy( &module->rs485, rs485, sizeof(struct CMD_TYPE_RS485) );
    g_free(rs485);

    Partage->com_rs485.Modules_RS485 = g_list_append ( Partage->com_rs485.Modules_RS485, module );

    Info_n( Config.log, DEBUG_RS485, "Charger_un_rs485:  id      = ", module->rs485.id    );
    Info_n( Config.log, DEBUG_RS485, "                -  actif   = ", module->rs485.actif );
   return(TRUE);
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
    if ( ! Recuperer_rs485DB( Config.log, db ) )
     { Libere_DB_SQL( Config.log, &db );
       return(FALSE);
     }

    Partage->com_rs485.Modules_RS485 = NULL;
    cpt = 0;
    for ( ; ; )
     { struct MODULE_RS485 *module;
       struct CMD_TYPE_RS485 *rs485;

       rs485 = Recuperer_rs485DB_suite( Config.log, db );
       if (!rs485) break;

       module = (struct MODULE_RS485 *)g_malloc0( sizeof(struct MODULE_RS485) );
       if (!module)                                                   /* Si probleme d'allocation mémoire */
        { Info( Config.log, DEBUG_RS485,
                "Charger_tous_RS485: Erreur allocation mémoire struct MODULE_RS485" );
          g_free(rs485);
          Libere_DB_SQL( Config.log, &db );
          return(FALSE);
        }
       memcpy( &module->rs485, rs485, sizeof(struct CMD_TYPE_RS485) );
       g_free(rs485);
       cpt++;                                              /* Nous avons ajouté un module dans la liste ! */
                                                                        /* Ajout dans la liste de travail */
       Partage->com_rs485.Modules_RS485 = g_list_append ( Partage->com_rs485.Modules_RS485, module );
       Info_n( Config.log, DEBUG_RS485, "Charger_tous_RS485:  id    = ", module->rs485.id    );
       Info_n( Config.log, DEBUG_RS485, "                  -  actif = ", module->rs485.actif );
     }
    Info_n( Config.log, DEBUG_RS485, "Charger_tous_RS485: module RS485 found  !", cpt );

    Libere_DB_SQL( Config.log, &db );
    return(TRUE);
  }
/**********************************************************************************************************/
/* Decharger_un_rs485: Dechargement d'un RS485                                                            */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_un_rs485 ( struct MODULE_RS485 *module )
  { if (!module) return;
    Partage->com_rs485.Modules_RS485 = g_list_remove ( Partage->com_rs485.Modules_RS485, module );
    g_free(module);
  }
/**********************************************************************************************************/
/* Rechercher_msgDB: Recupération du message dont le num est en parametre                                 */
/* Entrée: un log et une database                                                                         */
/* Sortie: une GList                                                                                      */
/**********************************************************************************************************/
 static void Decharger_tous_rs485 ( void  )
  { struct MODULE_RS485 *module;
    while ( Partage->com_rs485.Modules_RS485 )
     { module = (struct MODULE_RS485 *)Partage->com_rs485.Modules_RS485->data;
       Decharger_un_rs485 ( module );
     }
  }
/**********************************************************************************************************/
/* RS485_is_actif: Renvoi TRUE si au moins un des modules rs est actif                                    */
/* Entrée: rien                                                                                           */
/* Sortie: TRUE/FALSE                                                                                     */
/**********************************************************************************************************/
 static gboolean Rs485_is_actif ( void )
  { GList *liste;
    liste = Partage->com_rs485.Modules_RS485;
    while ( liste )
     { struct MODULE_RS485 *module;
       module = ((struct MODULE_RS485 *)liste->data);

       if (module->rs485.actif) return(TRUE);
       liste = liste->next;
     }
    return(FALSE);
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

    fd = open( Config.port_RS485, O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd<0)
     { Info_c( Config.log, DEBUG_RS485,
               "RS485: Init_rs485: Impossible d'ouvrir le port rs485", Config.port_RS485 );
       Info_n( Config.log, DEBUG_RS485,
               "RS485: Init_rs485: Code retour                      ", fd );
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
       Info_c( Config.log, DEBUG_RS485,
               "RS485: Init_rs485: Ouverture port rs485 okay", Config.port_RS485);
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
     { Info_n( Config.log, DEBUG_RS485, "Processer_trame: Module RS485 unknown", trame->source);
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
                { gint num_ea, val_int, ajout1, ajout2, ajout3, ajout4, ajout5;

                  val_int =   trame->donnees[cpt] << 2;
                  ajout1  =   trame->donnees[nbr_ea + (cpt >> 2)];
                  ajout2 = 0xC0>>((cpt & 0x03)<<1);
                  ajout3 = (3-(cpt & 0x03))<<1;
                  ajout4 = ajout1  & ajout2;
                  ajout5 = ajout4 >> ajout3;
                  val_int += ajout5;
                  num_ea = module->rs485.ea_min + cpt;

                  SEA( num_ea, val_int );
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
 void Run_rs485 ( void )
  { gint retval, nbr_oct_lu, id_en_cours, attente_reponse;
    struct MODULE_RS485 *module;
    struct TRAME_RS485 Trame;
    struct timeval tv;
    fd_set fdselect;
    GList *liste;

    prctl(PR_SET_NAME, "W-RS485", 0, 0, 0 );
    Info( Config.log, DEBUG_RS485, "RS485: demarrage" );

    fd_rs485 = Init_rs485();
    if (fd_rs485<0)                                                        /* On valide l'acces aux ports */
     { Info( Config.log, DEBUG_RS485, "RS485: Acces RS485 impossible, terminé");
       Partage->com_rs485.TID = 0;                        /* On indique au master que le thread est mort. */
       pthread_exit(GINT_TO_POINTER(-1));
     }
    else { Info_n( Config.log, DEBUG_RS485, "RS485: Acces RS485 FD", fd_rs485 ); }

    Partage->com_rs485.Modules_RS485 = NULL;                    /* Initialisation des variables du thread */
    Partage->com_rs485.Thread_run    = TRUE;                                        /* Le thread tourne ! */

    Charger_tous_rs485();                                                 /* Chargement des modules rs485 */

    nbr_oct_lu = 0;
    id_en_cours = 0;
    attente_reponse = FALSE;

    while(Partage->com_rs485.Thread_run == TRUE)                         /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (Partage->com_rs485.Thread_reload == TRUE)
        { Info( Config.log, DEBUG_RS485, "RS485: Run_rs485: Reloading conf" );
          Decharger_tous_rs485();
          Charger_tous_rs485();
          Partage->com_rs485.Thread_reload = FALSE;
        }

       if (Partage->com_rs485.Thread_sigusr1 == TRUE)
        { Info( Config.log, DEBUG_RS485, "RS485: Run_rs485: SIGUSR1" );
          Partage->com_rs485.Thread_sigusr1 = FALSE;
        }

       if (Partage->com_rs485.admin_del)
        { Info( Config.log, DEBUG_RS485, "RS485: Run_rs485: Deleting module" );
          module = Chercher_module_by_id ( Partage->com_rs485.admin_del );
          Decharger_un_rs485 ( module );
          Partage->com_rs485.admin_del = 0;
        }

       if (Partage->com_rs485.admin_add)
        { Info( Config.log, DEBUG_RS485, "RS485: Run_rs485: Adding module" );
          Charger_un_rs485 ( Partage->com_rs485.admin_add );
          Partage->com_rs485.admin_add = 0;
        }

       if (Partage->com_rs485.admin_start)
        { Info( Config.log, DEBUG_RS485, "RS485: Run_rs485: Starting module" );
          module = Chercher_module_by_id ( Partage->com_rs485.admin_start );
          if (module) { module->rs485.actif = 1; }
          Partage->com_rs485.admin_start = 0;
        }

       if (Partage->com_rs485.admin_stop)
        { Info( Config.log, DEBUG_RS485, "RS485: Run_rs485: Stopping module" );
          module = Chercher_module_by_id ( Partage->com_rs485.admin_stop );
          if (module) module->rs485.actif = 0;
          Partage->com_rs485.admin_stop = 0;
        }

       if (Partage->com_rs485.Modules_RS485 == NULL )           /* Si pas de module référencés, on attend */
        { sleep(2); continue; }

       if (Rs485_is_actif() == FALSE)                     /* Si aucun module actif, on restart la comm RS */
        { close(fd_rs485);
          fd_rs485 = Init_rs485();
          if (fd_rs485<0)                                                  /* On valide l'acces aux ports */
           { Info( Config.log, DEBUG_RS485, "RS485: Restart Acces RS485 impossible, terminé");
             pthread_exit(GINT_TO_POINTER(-1));
           }
          continue;
        }

       liste = Partage->com_rs485.Modules_RS485;
       while (liste)
        { module = (struct MODULE_RS485 *)liste->data;
          if (module->rs485.actif != TRUE)           /* Le le module est administravely down, on le zappe */
           { SB(module->rs485.bit_comm, 0);
             liste = liste->next;
             continue;
           }

          if ( attente_reponse == FALSE )
           { if ( module->date_retente <= Partage->top )                         /* module banni ou non ? */
              { if (module->date_next_get_ana > Partage->top)               /* Ana toutes les 10 secondes */
                 { Envoyer_trame_want_inputTOR( module, fd_rs485 );
                 }
                else
                 { Envoyer_trame_want_inputANA( module, fd_rs485 );
                   module->date_next_get_ana = Partage->top + RS485_TEMPS_UPDATE_IO_ANA;/* Prochain update ana dans 2 secondes */
                 }

                usleep(1);
                module->date_requete = Partage->top;
                module->date_retente = 0;
                attente_reponse = TRUE;
              }
           }
          else
           { if ( Partage->top - module->date_requete > 20 )                       /* Si la comm est niet */
              { module->date_retente = Partage->top + TEMPS_RETENTE;
                attente_reponse = FALSE;
                memset (&Trame, 0, sizeof(struct TRAME_RS485) );
                nbr_oct_lu = 0;
                Info_n( Config.log, DEBUG_RS485, "RS485: Run_rs485: module down", module->rs485.id );
                SB(module->rs485.bit_comm, 0);
                liste = liste->next;
                continue;
              }
             else { module->date_retente = 0;
                  }
           }

          FD_ZERO(&fdselect);                                       /* Reception sur la ligne serie RS485 */
          FD_SET(fd_rs485, &fdselect );
          tv.tv_sec = 1;
          tv.tv_usec= 0;
          retval = select(fd_rs485+1, &fdselect, NULL, NULL, &tv );             /* Attente d'un caractere */
          if (retval>=0 && FD_ISSET(fd_rs485, &fdselect) )
	   { int bute, cpt;
             if (nbr_oct_lu<TAILLE_ENTETE)
	      { bute = TAILLE_ENTETE; } else { bute = sizeof(Trame); }
 
             cpt = read( fd_rs485, (unsigned char *)&Trame + nbr_oct_lu, bute-nbr_oct_lu );
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
                    { Info(Config.log, DEBUG_RS485, "RS485: CRC16 failed !!"); }
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
      }                                                                    /* Fin du while partage->arret */
    close(fd_rs485);
    Decharger_tous_rs485();
    Info_n( Config.log, DEBUG_RS485, "RS485: Run_rs485: Down", pthread_self() );
    Partage->com_rs485.TID = 0;                           /* On indique au master que le thread est mort. */
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
