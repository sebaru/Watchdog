/**********************************************************************************************************/
/* liaisonrs232.c        Test d'interfacage Microcontroleur/PC via la RS 232                              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 29 avr 2004 17:41:10 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <stdio.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <termios.h>
 #include <unistd.h>
 #include <string.h>
 #include <stdlib.h>

 #define ID_ESCLAVE    10
     
 #define FCT_IDENT      0x01
 #define FCT_ENTRE_TOR  0x02
 #define FCT_ENTRE_ANA  0x03
 #define FCT_SORTIE_TOR 0x04
 #define DEBUG_PRENDRE_LIGNE  0xF0
 #define DEBUG_LIBERE_LIGNE 0xF1
 #define FCT_PING       0xFF
 
 #define TAILLE_ENTETE 6
 #define TAILLE_DONNEES 24
 struct TRAME_RS232
  { unsigned char dest;
    unsigned char source;
    unsigned char fonction;
    unsigned char taille;
    unsigned char donnees[TAILLE_DONNEES];
    unsigned char crc16_h;
    unsigned char crc16_l;
  } trame;
 struct TRAME_RS232_IDENT
  { unsigned char id;
    unsigned char version_major;
    unsigned char version_minor;
    unsigned char nbr_entre_ana;
    unsigned char nbr_entre_tor;
    unsigned char nbr_entre_choc;
    unsigned char nbr_sortie_tor;
    unsigned char timer_th0;
    unsigned char timer_tl0;
    unsigned char timer_t3;
    unsigned char nbr_int_can;
    unsigned char bank2_r2;
    unsigned char bank2_r3;
  };
 struct timeval tv_avant, tv_apres;
 char *mem, car;
 int cpt;

 struct TRAME_RS232 Trame_want_ident=
  { ID_ESCLAVE, 0xFF, FCT_IDENT, 0x00, 
	  { 1, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4 },
    0xFF, 0xFF
  };
 
 struct TRAME_RS232 Trame_want_entre_tor=
  { ID_ESCLAVE, 0xFF, FCT_ENTRE_TOR, 0x03,
	  { 0, 0, 0 ,0, 0, 6, 7, 8, 9, 10, 1, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4 },
    0xFF, 0xFF
  };

 struct TRAME_RS232 Trame_want_entre_ana=
  { ID_ESCLAVE, 0xFF, FCT_ENTRE_ANA, 0x00,
	  { 1, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4 },
    0xFF, 0xFF
  };

 struct TRAME_RS232 Trame_send_sortie_tor_0=
  { ID_ESCLAVE, 0xFF, FCT_SORTIE_TOR, 0x0A,
	  { 0x00, 0, 0 ,0, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4 },
    0xFF, 0xFF
  };

 struct TRAME_RS232 Trame_send_sortie_tor_1=
  { ID_ESCLAVE, 0xFF, FCT_SORTIE_TOR, 0x03,
	  { 0xAA, 0xAA, 0xA0 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4 },
    0xFF, 0xFF
  };
 struct TRAME_RS232 Trame_send_sortie_tor_2=
  { ID_ESCLAVE, 0xFF, FCT_SORTIE_TOR, 0x03,
	  { 0x55, 0x55, 0x50 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4 },
    0xFF, 0xFF
  };
 struct TRAME_RS232 Trame_send_sortie_tor_3=
  { ID_ESCLAVE, 0xFF, FCT_SORTIE_TOR, 0x0A,
	  { 0xFF, 0xFF, 0xFF ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4 },
    0xFF, 0xFF
  };
 struct TRAME_RS232 Trame_libere_ligne=
  { ID_ESCLAVE, 0xFF, DEBUG_LIBERE_LIGNE, 0x00,
	  { 0xC0, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4 },
    0xFF, 0xFF
  };
 struct TRAME_RS232 Trame_prendre_ligne=
  { ID_ESCLAVE, 0xFF, DEBUG_PRENDRE_LIGNE, 0x00,
	  { 0xC0, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4, 5, 6, 7, 8, 9, 10, 1, 2, 3 ,4 },
    0xFF, 0xFF
  };
/**********************************************************************************************************/
/* Convertir un numero de fonction dans son equivalent lisible                                            */
/**********************************************************************************************************/
 char *Fct_vers_string( int fct )
  { switch( fct )
     { case 0x01: return("FCT_IDENT");
       case 0x02: return("FCT_ENTRE_TOR");
       case 0x03: return("FCT_ENTRE_ANA");
       case 0x04: return("FCT_SORTIE_TOR");
       case 0xF0: return("DEBUG_PRENDRE_LIGNE");
       case 0xF1: return("DEBUG_LIBERE_LIGNE");
       case 0xFF: return("FCT_PING");
       default: return("Inconnu");
     }
  }
/**********************************************************************************************************/
/* Calcul_crc16: renvoie le CRC16 de la trame en parametre                                                */
/* Entrée: la trame a tester                                                                              */
/* Sortie: le crc 16 bits                                                                                 */
/**********************************************************************************************************/
 int Calcul_crc16 (struct TRAME_RS232 *Trame)  
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
/* Envoyer_trame: envoie d'une trame RS232 sur la ligne                                                   */
/* Entrée: L'id de la transmission, et la trame a transmettre                                            */
/**********************************************************************************************************/
 void Envoyer_trame( int fd, char dest, struct TRAME_RS232 *trame )
  { int crc16, cpt;

    trame->dest = dest;
    printf("Envoi trame � %d taille=%d\n", trame->dest, trame->taille );
    crc16 = Calcul_crc16 (trame);
    trame->crc16_h = crc16 >> 8;
    trame->crc16_l = crc16 & 0xFF;
    printf("crc16 calcul� � l'envoi: %04X  %2X %2X  taille %d\n", crc16, trame->crc16_h, trame->crc16_l,
                                                                  trame->taille );
    printf("Envoi de la trame:");
    for (cpt=0; cpt< TAILLE_ENTETE-2+trame->taille; cpt++)
     { printf( "%02X ", *((unsigned char *)trame+cpt) ); }
    printf("\n crc %X %X\n", trame->crc16_h, trame->crc16_l );
    
    write( fd, trame, TAILLE_ENTETE - 2 );                                        /* Ecriture de l'entete */
    if (trame->taille) { write( fd, &trame->donnees, trame->taille ); }          /* On envoie les données */
    write( fd, &trame->crc16_h, sizeof(unsigned char) );
    write( fd, &trame->crc16_l, sizeof(unsigned char) );

    gettimeofday( &tv_avant, NULL );
  }
/**********************************************************************************************************/
/* Init_rs232: Initialisation de la ligne RS232                                                           */
/* Sortie: l'identifiant de la connexion                                                                  */
/**********************************************************************************************************/
 int Init_rs232 ( void )
  { struct termios oldtio;
    int fd;

    fd = open( "/dev/tts/1", O_RDWR | O_NOCTTY | O_NONBLOCK );
    if (fd<0)
     { printf("Impossible d'ouvrir le port rs232\n"); exit(0); }

    memset(&oldtio, 0, sizeof(oldtio) );
    oldtio.c_cflag = B19200 | CS8 | CREAD | CLOCAL;
    oldtio.c_oflag = 0;
    oldtio.c_iflag = 0;
    oldtio.c_lflag = 0;
    oldtio.c_cc[VTIME]    = 0;
    oldtio.c_cc[VMIN]     = 0;
    tcsetattr(fd, TCSANOW, &oldtio);
    tcflush(fd, TCIOFLUSH);
    return(fd);
  }
/**********************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                   */
/* Entrée: la trame a recue                                                                           */
/* Sortie: néant                                                                                      */
/**********************************************************************************************************/
 void Processer_trame( struct TRAME_RS232 *trame )
  { struct TRAME_RS232_IDENT *trame_ident;
    if (trame->dest != 0xFF) return;                               /* Si c'est pas pour nous, on se casse */
    printf("De %02X vers %02X. Taille des donn�es: %d\n", trame->source, trame->dest, trame->taille );
    printf("Fonction: %s\n", Fct_vers_string( trame->fonction ) );
    switch( trame->fonction )
     { case FCT_IDENT: trame_ident = (struct TRAME_RS232_IDENT *)trame->donnees;
                       printf("ID=%d Version %d.%d, nbr ana %d, nbr tor %d (%d choc), sortie %d\n", trame_ident->id,
                              trame_ident->version_major, trame_ident->version_minor,
                              trame_ident->nbr_entre_ana, trame_ident->nbr_entre_tor,
                              trame_ident->nbr_entre_choc, trame_ident->nbr_sortie_tor );
                       printf("Timer 0 = 0x%02X%02X Timer 3 = 0x%02X NBR_INT_CAN=%d   b2_r2=%02X  b2_r3=%02X \n",
                              trame_ident->timer_th0, trame_ident->timer_tl0,
                              trame_ident->timer_t3, trame_ident->nbr_int_can, trame_ident->bank2_r2, trame_ident->bank2_r3 );
                       break;
       case FCT_ENTRE_TOR: printf("on recoit les etats TOR\n");
		           break;
       case FCT_ENTRE_ANA: printf("on recoit les etats ANA\n");
		           break;
     }
  }
/**********************************************************************************************************/
/* main: fonction principale                                                                              */
/**********************************************************************************************************/
 int main ( int argc, char *argv[] )
  { int fd_rs232;
    fd_set fdselect;
    struct timeval tv;
    int retval, nbr_oct_lu;
    char dest;
    
    dest = 1;
    fd_rs232 = Init_rs232();
    nbr_oct_lu = 0;

    /*dest = 2;
    for ( ; ; )
     {
       Envoyer_trame( fd_rs232, dest, &Trame_send_sortie_tor_1 );
       sleep(1);
       Envoyer_trame( fd_rs232, dest, &Trame_send_sortie_tor_2 );
       sleep(1);
     }*/
   
    for ( ; ; )
     { FD_ZERO(&fdselect);
       FD_SET(0, &fdselect);
       FD_SET(fd_rs232, &fdselect );
       tv.tv_sec = 0;
       tv.tv_usec= 100;
       retval = select(fd_rs232+1, &fdselect, NULL, NULL, &tv );               /* Attente d'un caractere */
       if (retval>=0)
	{ if (FD_ISSET(0, &fdselect))                                            /* Est-ce au clavier ?? */
           { read( 0, &car, 1 );
             switch(car)
	      { case 'i': Envoyer_trame( fd_rs232, dest, &Trame_want_ident );
                          printf("envoi trame ident\n");
			  break;
		case 'e': Envoyer_trame( fd_rs232, dest, &Trame_want_entre_tor );
			  printf("envoi trame want entre tor\n");
			  break;
		case 'a': Envoyer_trame( fd_rs232, dest, &Trame_want_entre_ana );
			  printf("envoi trame want entre ana\n");
			  break;
		case 'w': Envoyer_trame( fd_rs232, dest, &Trame_send_sortie_tor_0 );
			  printf("envoi trame sortie = 0\n");
			  break;
		case 'x': Envoyer_trame( fd_rs232, dest, &Trame_send_sortie_tor_1 );
			  printf("envoi trame sortie = 1\n");
			  break;
		case 'c': Envoyer_trame( fd_rs232, dest, &Trame_send_sortie_tor_2 );
			  printf("envoi trame sortie = 2\n");
			  break;
		case 'v': Envoyer_trame( fd_rs232, dest, &Trame_send_sortie_tor_3 );
			  printf("envoi trame sortie = 3\n");
			  break;
		case 'l': Envoyer_trame( fd_rs232, dest, &Trame_libere_ligne );
			  printf("envoi trame libere ligne\n");
			  break;
		case 'L': Envoyer_trame( fd_rs232, dest, &Trame_prendre_ligne );
			  printf("envoi trame prise ligne 1\n");
			  break;
		case '\n': break;
	        default: dest = car - '0'; break;
	      }
             printf("Esclave selectionn�: %d\n", dest );
	   }
	  else if (FD_ISSET(fd_rs232, &fdselect))                        /* Est-ce sur la ligne rs232 ?? */
           { int bute;
             if (nbr_oct_lu<TAILLE_ENTETE)
	      { bute = TAILLE_ENTETE; } else { bute = sizeof(trame); }
             cpt = read( fd_rs232, (unsigned char *)&trame + nbr_oct_lu, bute-nbr_oct_lu );
             /*printf("lecture cpt=%d\n", cpt );*/
             if (cpt>0)
              { nbr_oct_lu = nbr_oct_lu + cpt;
		/*printf("nbr_oct_lu = %d  on veut %d \n", nbr_oct_lu, TAILLE_ENTETE + trame.taille );
                for (cpt=0; cpt<nbr_oct_lu; cpt++)
		    { printf("%02X ", (unsigned char)*((unsigned char *)&trame + cpt) );
		    }*/
		if (nbr_oct_lu >= TAILLE_ENTETE + trame.taille)                     /* traitement trame */
		 { int crc_recu;
                   nbr_oct_lu = 0;
                   printf("Recu trame brute: ");
		   for (cpt=0; cpt<sizeof(trame); cpt++)
		    { printf("%02X ", (unsigned char)*((unsigned char *)&trame +cpt) );
		    }
		   printf("\n");
		   crc_recu =  (*(char *)((unsigned int)&trame + TAILLE_ENTETE + trame.taille - 1)) & 0xFF;
		   crc_recu +=  ((*(char *)((unsigned int)&trame + TAILLE_ENTETE + trame.taille - 2)) & 0xFF)<<8;
		   if (crc_recu != Calcul_crc16(&trame))
		    { printf("CRC16 failed !!\n"); }
		   else
		    { gettimeofday( &tv_apres, NULL );
                      Processer_trame( &trame );
                      printf("Temps de traitement: %f\n", tv_apres.tv_sec - tv_avant.tv_sec +
                                                          (tv_apres.tv_usec - tv_avant.tv_usec)/1000000.0 );
		      memset (&trame, 0, sizeof(struct TRAME_RS232) );
                    }
		 }
              } else { printf("Erreur ?\n"); }
	  }
	} else { printf("retval = %d\n", retval ); }
     }
    close(fd_rs232);
  }
/*--------------------------------------------------------------------------------------------------------*/
