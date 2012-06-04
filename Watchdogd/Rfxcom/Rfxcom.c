/**********************************************************************************************************/
/* Watchdogd/Rfxcom/Rfxcom.c  Gestion des tellivages bit_internes Watchdog 2.0                            */
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
/* Init_rfxcom: Initialisation de la ligne RFXCOM                                                           */
/* Sortie: l'identifiant de la connexion                                                                  */
/**********************************************************************************************************/
 static int Init_rfxcom ( void )
  { gchar trame_reset[] = { 0x0D, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
    gchar trame_get_status[] = { 0x0D, 00, 00, 01, 02, 00, 00, 00, 00, 00, 00, 00, 00, 00 };
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
    sleep(5);
    Info( Config.log, DEBUG_RFXCOM, "RFXCOM: Init_rfxcom: Sending GET STATUS" );
    write (fd, &trame_get_status, sizeof(trame_get_status) );
    return(fd);
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


#ifdef bouh
    switch( trame->fonction )
     { case RFXCOM_FCT_IDENT: printf("bouh\n");
	               trame_ident = (struct TRAME_RFXCOM_IDENT *)trame->donnees;
                       printf("Recu Ident de %d: version %d.%d, nbr ana %d, nbr tor %d (%d choc), sortie %d\n",
                              trame->source, trame_ident->version_major, trame_ident->version_minor,
                              trame_ident->nbr_entre_ana, trame_ident->nbr_entre_tor,
                              trame_ident->nbr_entre_choc, trame_ident->nbr_sortie_tor );
                       break;
       case RFXCOM_FCT_ENTRE_TOR:
             { int e, cpt, nbr_e;
               nbr_e = module->rfxcom.e_max - module->rfxcom.e_min + 1;
               for( cpt = 0; cpt<nbr_e; cpt++)
                { e = ! (trame->donnees[cpt >> 3] & (0x80 >> (cpt & 0x07)));
                  SE( module->rfxcom.e_min + cpt, e );
                }
             }
	    break;
       case RFXCOM_FCT_ENTRE_ANA:
             { int cpt, nbr_ea;
               if (module->rfxcom.ea_min == -1) nbr_ea = 0;
               else nbr_ea = module->rfxcom.ea_max - module->rfxcom.ea_min + 1;
               for( cpt = 0; cpt<nbr_ea; cpt++)
                { gint num_ea, val_int, ajout1, ajout2, ajout3, ajout4, ajout5;

                  val_int =   trame->donnees[cpt] << 2;
                  ajout1  =   trame->donnees[nbr_ea + (cpt >> 2)];
                  ajout2 = 0xC0>>((cpt & 0x03)<<1);
                  ajout3 = (3-(cpt & 0x03))<<1;
                  ajout4 = ajout1  & ajout2;
                  ajout5 = ajout4 >> ajout3;
                  val_int += ajout5;
                  num_ea = module->rfxcom.ea_min + cpt;

                  SEA( num_ea, val_int );
                }
             }
	    break;
       default: printf("Trame non traitée\n"); return(FALSE);
     }
#endif
    return(TRUE);
  }
/**********************************************************************************************************/
/* Main: Fonction principale du thread Rfxcom                                                          */
/**********************************************************************************************************/
 void Run_rfxcom ( void )
  { gint retval, nbr_oct_lu, id_en_cours, attente_reponse;
    struct MODULE_RFXCOM *module;
    struct TRAME_RFXCOM Trame;
    struct timeval tv;
    fd_set fdselect;
    gint fd_rfxcom;
    GList *liste;

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
    attente_reponse = FALSE;

    while(Partage->com_rfxcom.Thread_run == TRUE)                         /* On tourne tant que necessaire */
     { usleep(1);
       sched_yield();

       if (Partage->com_rfxcom.Thread_reload == TRUE)
        { Info( Config.log, DEBUG_RFXCOM, "RFXCOM: Run_rfxcom: Reloading conf" );
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
                Processer_trame( &Trame );
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
