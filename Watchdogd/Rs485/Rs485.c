/**********************************************************************************************************/
/* Watchdogd/Rs485/Rs485.c  Gestion des modules rs485 Watchdgo 2.0                                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 21 aoû 2005 17:09:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * Rs485.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2007 - Sébastien Lefevre
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
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>
 #include <signal.h>
 #include <semaphore.h>

 #include "Erreur.h"
 #include "Config.h"
 #include "Rs485.h"
 #include "EntreeANA_DB.h"
 #include "ValANA_DB.h"
 #include "watchdogd.h"                                                         /* Pour la struct PARTAGE */
 #include "proto_dls.h"                                                             /* Acces a A(x), E(x) */

 #define TEMPS_RETENTE   5                /* Tente de se raccrocher au module banni toutes les 5 secondes */

 static struct COMM_RS485                                     /* Etat de la connexion avec les modules RS */
  { time_t date_requete;
    time_t date_retente;
    time_t date_ana;
  } Comm_RS485 [ NBR_ID_RS485 ];
 static gint fd_rs485;

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 static struct DB *Db_watchdog;                                                      /* Database Watchdog */
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */

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
    {                                                                          /* CRC OUEX octet en cours */
      CRC16 = CRC16 ^ (short)*((unsigned char *)Trame + index_octets);
      for( index_bits = 0; index_bits<8; index_bits++ )
       { retenue = CRC16 & 1;                              /* Récuperer la retenue avant traitement CRC16 */ 
         CRC16 = CRC16 >> 1;                                       /* décalage d'un bit à droite du CRC16 */
         if (retenue == 1)
          { CRC16 = CRC16 ^ 0xA001; }                                          /* CRC16 OUEX A001 en hexa */
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
 static void Envoyer_trame_want_inputANA( int id_esclave, int fd )
  { static struct TRAME_RS485 Trame_want_entre_ana=
     { 0x00, 0xFF, FCT_ENTRE_ANA, 0x00,
   	  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
       0xFF, 0xFF
     };
    Trame_want_entre_ana.dest = id_esclave;
    Envoyer_trame( fd, &Trame_want_entre_ana );
  }
/**********************************************************************************************************/
/* Envoyer_trame: envoie d'une trame RS485 sur la ligne                                                   */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Envoyer_trame_want_inputTOR( int id_esclave, int fd )
  { static struct TRAME_RS485 Trame_want_entre_tor=
     { 0x00, 0xFF, FCT_ENTRE_TOR, 0x00,
   	  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
       0xFF, 0xFF
     };
    Trame_want_entre_tor.dest = id_esclave;
    if (Config.module_rs485[id_esclave].s_min != -1 )
     { guchar poids, a, octet, taille;
       gint num_a;
       poids = 0x80;
       taille = 1;
       octet = 0;
       for ( num_a =  Config.module_rs485[id_esclave].s_min;
             num_a <= Config.module_rs485[id_esclave].s_max;
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
     { Info_c( Config.log, DEBUG_INFO,
               "RS485: Init_rs485: Impossible d'ouvrir le port rs485", Config.port_RS485 );
       Info_n( Config.log, DEBUG_INFO,
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
       Info_c( Config.log, DEBUG_INFO,
               "RS485: Init_rs485: Ouverture port rs485 okay", Config.port_RS485);
     }
    return(fd);
  }
/**********************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                   */
/* Entrée: la trame a recue                                                                               */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static int Processer_trame( struct TRAME_RS485 *trame )
  { struct TRAME_RS485_IDENT *trame_ident;
    if (trame->dest != 0xFF) return(FALSE);                        /* Si c'est pas pour nous, on se casse */
    if ( trame->source >= NBR_ID_RS485 )
     { Info_n( Config.log, DEBUG_INFO, "Processer_trame: module id not in range", trame->source );
       return(TRUE);
     }
    if (Config.module_rs485[trame->source].id != trame->source)
     { Info_n( Config.log, DEBUG_INFO, "Processer_trame: Module RS485 unknown", trame->source);
       return(TRUE);
     }

    switch( trame->fonction )
     { case FCT_IDENT: printf("bouh\n");
	               trame_ident = (struct TRAME_RS485_IDENT *)trame->donnees;
                       printf("Recu Ident de %d: version %d.%d, nbr ana %d, nbr tor %d (%d choc), sortie %d\n",
                              trame->source, trame_ident->version_major, trame_ident->version_minor,
                              trame_ident->nbr_entre_ana, trame_ident->nbr_entre_tor,
                              trame_ident->nbr_entre_choc, trame_ident->nbr_sortie_tor );
                       break;
       case FCT_ENTRE_TOR:
             { int e, cpt, nbr_e;
               nbr_e = Config.module_rs485[trame->source].e_max - Config.module_rs485[trame->source].e_min + 1;
               for( cpt = 0; cpt<nbr_e; cpt++)
                { e = ! (trame->donnees[cpt >> 3] & (0x80 >> (cpt & 0x07)));
                  SE( Config.module_rs485[trame->source].e_min + cpt, e );
                }
             }
	    break;
       case FCT_ENTRE_ANA:
             { int cpt, nbr_ea;
               if (Config.module_rs485[trame->source].ea_min == -1) nbr_ea = 0;
               else nbr_ea = Config.module_rs485[trame->source].ea_max -
                             Config.module_rs485[trame->source].ea_min + 1;
               for( cpt = 0; cpt<nbr_ea; cpt++)
                { gint num_ea, val_int, ajout1, ajout2, ajout3, ajout4, ajout5;

                  val_int =   trame->donnees[cpt] << 2;
                  ajout1  =   trame->donnees[nbr_ea + (cpt >> 2)];
                  ajout2 = 0xC0>>((cpt & 0x03)<<1);
                  ajout3 = (3-(cpt & 0x03))<<1;
                  ajout4 = ajout1  & ajout2;
                  ajout5 = ajout4 >> ajout3;
                  val_int += ajout5;
                  num_ea = Config.module_rs485[trame->source].ea_min + cpt;

                  if (val_int<=10)
                   { SEA( num_ea, 0, 0 ); }
                  else if (val_int<200)
                   { SEA( num_ea, 0, 1 ); }
                  else
                   { SEA( num_ea, (gint)( ((gdouble)val_int-200.0)*4096.0 / 824.0 ), 1 ); }

 /*                 Partage->ea[ num_ea ].val_ech =                                   /* Valeur à l'echelle */
   /*                ((((gdouble)val_int - 1023.0 ) * (Partage->ea[num_ea].max - Partage->ea[num_ea].min)) / 824)
                   + Partage->ea[num_ea].max;*/

                  /*printf("EA[%d] = %d %f (min %f, max %f) ajout %d %d %d %d %d %d\n",
                         num_ea, val_int, Partage->ea[ num_ea ].val_ech, Partage->ea[num_ea].min,
                         Partage->ea[num_ea].max, trame->donnees[cpt], ajout1, ajout2, ajout4, ajout4, ajout5 );*/

                 /*  Ajouter_arch( MNEMO_ENTREE_ANA, num_ea, val_int ); A deplacer dans archive.c */

                                                                     /* Gestion historique interne Valana */
                  memmove( Partage->ea_histo[ num_ea ], Partage->ea_histo[ num_ea ]+1,
                           TAILLEBUF_HISTO_EANA * sizeof( Partage->ea_histo[ num_ea ][0] ) );
                  Partage->ea_histo[ num_ea ][TAILLEBUF_HISTO_EANA-1] = val_int;
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
  { gint retval, nbr_oct_lu, id_en_cours, attente_reponse, cpt;
    struct TRAME_RS485 Trame;
    struct timeval tv;
    fd_set fdselect;

    Info( Config.log, DEBUG_FORK, "RS485: demarrage" );

    for (cpt=0; cpt<NBR_ID_RS485; cpt++)
     { if( Config.module_rs485[cpt].id == cpt ) break; }                         /* Au moins un module ?? */
    if (cpt==NBR_ID_RS485)
     { Info( Config.log, DEBUG_INFO, "RS485: Run_rs485: No module RS485 found in config -> stop" );
       pthread_exit(GINT_TO_POINTER(-1));
     }

    fd_rs485 = Init_rs485();
    if (fd_rs485<0)                                                        /* On valide l'acces aux ports */
     { Info( Config.log, DEBUG_INFO, "RS485: Acces RS485 impossible, terminé");
       pthread_exit(GINT_TO_POINTER(-1));
     }

    Db_watchdog = ConnexionDB( Config.log, Config.db_name,           /* Connexion en tant que user normal */
                               Config.db_admin_username, Config.db_password );

    if (!Db_watchdog)
     { Info_c( Config.log, DEBUG_DB, "RS485: Run_rs485: Unable to open database (dsn)", Config.db_name );
       close(fd_rs485);
       pthread_exit(GINT_TO_POINTER(-1));
     }

    nbr_oct_lu = 0;
    id_en_cours = 0;
    attente_reponse = FALSE;
    memset (&Comm_RS485, 0, sizeof(Comm_RS485) );

    while(Partage->Arret < FIN)                    /* On tourne tant que le pere est en vie et arret!=fin */
     { time_t date;                                            /* On veut parler au prochain module RS485 */
       date = time(NULL);                                                 /* On recupere l'heure actuelle */

       if (Partage->com_dls_rs.sigusr1)
        { Partage->com_dls_rs.sigusr1 = FALSE;
          Info( Config.log, DEBUG_INFO, "RS485: Run_rs485: SIGUSR1" );
        }

       if ( attente_reponse == FALSE )
        { do
           { id_en_cours++;
             if (id_en_cours>=NBR_ID_RS485)              /* On vient de faire un tour de tous les modules */
              { /*static struct timeval before = { 0, 0 };
                struct timeval now;*/
                id_en_cours = 0;
                /*gettimeofday( &now, NULL );*/               /* On impose 1 tour module = 100 ms minimum */
                /*while( before.tv_sec == now.tv_sec && 
                       now.tv_usec - before.tv_usec < 100000 ) sched_yield();
                before.tv_sec = now.tv_sec;
                before.tv_usec = now.tv_usec;*/
              }
           } while( Config.module_rs485[id_en_cours].id != id_en_cours );

          if ( Comm_RS485[ id_en_cours ].date_retente <= date )                  /* module banni ou non ? */
           { if (Comm_RS485[ id_en_cours ].date_ana > date)                 /* Ana toutes les 10 secondes */
              { Envoyer_trame_want_inputTOR( id_en_cours, fd_rs485 );
              }
             else
              { Envoyer_trame_want_inputANA( id_en_cours, fd_rs485 );
                Comm_RS485[ id_en_cours ].date_ana = date + 2;     /* Prochain update ana dans 2 secondes */
              }

             sched_yield();
             Comm_RS485[ id_en_cours ].date_requete = date;
             attente_reponse = TRUE;
           }
           /*} else Info_n(Config.log, DEBUG_INFO, "Module banni pour le moment", id_en_cours );*/
        }
       else
        { if ( date - Comm_RS485[ id_en_cours ].date_requete > 2 )                 /* Si la comm est niet */
           { Comm_RS485[ id_en_cours ].date_retente = date + TEMPS_RETENTE;
             attente_reponse = FALSE;
             memset (&Trame, 0, sizeof(struct TRAME_RS485) );
             nbr_oct_lu = 0;
             Info_n( Config.log, DEBUG_INFO, "RS485: Run_rs485: module down", id_en_cours );
           }
        }

       FD_ZERO(&fdselect);                                          /* Reception sur la ligne serie RS485 */
       FD_SET(fd_rs485, &fdselect );
       tv.tv_sec = 0;
       tv.tv_usec= 100000;
       retval = select(fd_rs485+1, &fdselect, NULL, NULL, &tv );                /* Attente d'un caractere */
       if (retval>=0 && FD_ISSET(fd_rs485, &fdselect) )
	{ int bute, cpt;
          if (nbr_oct_lu<TAILLE_ENTETE)
	   { bute = TAILLE_ENTETE; } else { bute = sizeof(Trame); }

          cpt = read( fd_rs485, (unsigned char *)&Trame + nbr_oct_lu, bute-nbr_oct_lu );
          if (cpt>0)
           { nbr_oct_lu = nbr_oct_lu + cpt;
	     if (nbr_oct_lu >= TAILLE_ENTETE + Trame.taille)                          /* traitement trame */
              { int crc_recu;
                nbr_oct_lu = 0;
                /*for (cpt=0; cpt<sizeof(Trame); cpt++)
                 { printf("%02X ", (unsigned char)*((unsigned char *)&Trame +cpt) ); }
                printf("\n");*/
                crc_recu =  (*(char *)((unsigned int)&Trame + TAILLE_ENTETE + Trame.taille - 1)) & 0xFF;
                crc_recu +=  ((*(char *)((unsigned int)&Trame + TAILLE_ENTETE + Trame.taille - 2)) & 0xFF)<<8;
                if (crc_recu != Calcul_crc16(&Trame))
                 { Info(Config.log, DEBUG_INFO, "RS485: CRC16 failed !!"); }
                else
                 { pthread_mutex_lock( &Partage->com_dls_rs.synchro );
                   if (Processer_trame( &Trame ))          /* Si la trame est processée, on passe suivant */
                    { attente_reponse = FALSE; }
                   pthread_mutex_unlock( &Partage->com_dls_rs.synchro );
                 }
                memset (&Trame, 0, sizeof(struct TRAME_RS485) );
              }
           }
	}
      /*usleep(1);*/
     }
    close(fd_rs485);
    Info_n( Config.log, DEBUG_FORK, "RS485: Run_rs485: Down", pthread_self() );
    pthread_exit(GINT_TO_POINTER(0));
  }
/*--------------------------------------------------------------------------------------------------------*/
