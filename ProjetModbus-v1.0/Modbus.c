/**********************************************************************************************************/
/* Watchdogd/Modbus/Modbus.c  Gestion des modules MODBUS Watchdgo 2.0                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 21 aoû 2005 17:09:19 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

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
 #include <netinet/in.h>
 #include <netdb.h>


 #define MBUS_ENTRE_TOR  0x01
 #define MBUS_ENTRE_ANA  0x04
 #define MBUS_SORTIE_TOR 0x05
 #define MBUS_SORTIE_ANA 0x06
 
 #define NBR_ID_MODBUS            16
 #define NBR_ID_MODBUS_BORNE       8

 #define MODBUS_PORT_TCP    502                           /* Port de connexion TCP pour accès aux modules */
 #define MODBUS_RETRY       10                       /* 10 secondes entre chaque retry si pb de connexion */

 enum
  { BORNE_INPUT_TOR,
    BORNE_OUTPUT_TOR,
    BORNE_INPUT_ANA,
    BORNE_OUTPUT_ANA
  };

 struct MODULE_MODBUS_BORNE
  { gboolean actif;
    gint type;
    guint16 adresse;
    guint16 min;
    guint16 nbr;
  };

 struct MODULE_MODBUS
  { guint16 transaction_id;
    gboolean actif;
    gchar ip[32];                                                         /* Adresses IP du module MODBUS */
    gint connexion;                                                                 /* FD de connexion IP */
    gboolean request;                                                       /* Requete envoyé au module ? */
    struct MODULE_MODBUS_BORNE borne[NBR_ID_MODBUS_BORNE];
  };

 struct TRAME_MODBUS_REQUETE                                             /* Definition d'une trame MODBUS */
  { guint16 transaction_id;
    guint16 proto_id; /* -> 0 = MOBUS */
    guint16 taille; /* taille, en comptant le unit_id */
    guint8 unit_id; /* 0xFF */
    guint8 fct;
    guint16 adresse;
    guint16 nbr;
  };


#define TAILLE_ENTETE_MODBUS   6 /* Nombre d'octet avant d'etre sur d'avoir la taille trame */
 struct TRAME_MODBUS_REPONSE                                             /* Definition d'une trame MODBUS */
  { guint16 transaction_id;
    guint16 proto_id; /* -> 0 = MOBUS */
    guint16 taille; /* taille, en comptant le unit_id */
    guint8 unit_id; /* 0xFF */
    guint8 fct;
    guint8 nbr;
    guint8 data[16];
  } trame;

 #define TEMPS_RETENTE   5                /* Tente de se raccrocher au module banni toutes les 5 secondes */

/* static struct COMM_MODBUS          
  { gboolean started;
    gint connexion;
    guint transaction_id;
    guint borne_en_cours;
    time_t date_retente;
    gboolean request;
    gchar buffer[256];
  } Comm_MODBUS [ NBR_ID_MODBUS ];*/

 gint fd_module;

/**********************************************************************************************************/
/* Envoyer_trame: envoie d'une trame MODBUS sur la ligne                                                  */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Envoyer_trame_input_TOR( void )
  { struct TRAME_MODBUS_REQUETE Trame_input_borne1;
    Trame_input_borne1.transaction_id++;
    Trame_input_borne1.proto_id = 0;
    Trame_input_borne1.taille   = htons( 0x06 );
    Trame_input_borne1.unit_id  = 0;
    Trame_input_borne1.fct      = MBUS_ENTRE_TOR;
    Trame_input_borne1.adresse  = htons( 0 );
    Trame_input_borne1.nbr      = htons( 8 );

{ int cpt;
                for (cpt=0; cpt<sizeof (struct TRAME_MODBUS_REQUETE); cpt++)
		    { printf("%02X \n", (unsigned char)*((unsigned char *)&Trame_input_borne1 + cpt) );
		    }
}

    write( fd_module, &Trame_input_borne1, sizeof (struct TRAME_MODBUS_REQUETE) );/* Ecriture de l'entete */
  }
/**********************************************************************************************************/
/* Envoyer_trame: envoie d'une trame MODBUS sur la ligne                                                  */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Envoyer_trame_input_ANA( void )
  { struct TRAME_MODBUS_REQUETE Trame_input_borne1;
    Trame_input_borne1.transaction_id++;
    Trame_input_borne1.proto_id = 0;
    Trame_input_borne1.taille   = htons( 0x06 );
    Trame_input_borne1.unit_id  = 0;
    Trame_input_borne1.fct      = MBUS_ENTRE_ANA;
    Trame_input_borne1.adresse  = htons( 0 );
    Trame_input_borne1.nbr      = htons( 8 );
    write( fd_module, &Trame_input_borne1, sizeof (struct TRAME_MODBUS_REQUETE) );/* Ecriture de l'entete */
  }
/**********************************************************************************************************/
/* Envoyer_trame: envoie d'une trame MODBUS sur la ligne                                                  */
/* Entrée: L'id de la transmission, et la trame a transmettre                                             */
/**********************************************************************************************************/
 static void Envoyer_trame( struct TRAME_MODBUS_REQUETE *trame )
  { 
    write( fd_module, trame, sizeof (struct TRAME_MODBUS_REQUETE) );              /* Ecriture de l'entete */
  }
#ifdef bouh
/**********************************************************************************************************/
/* Processer_trame: traitement de la trame recue par un microcontroleur                                   */
/* Entrée: la trame a recue                                                                               */
/* Sortie: néant                                                                                          */
/**********************************************************************************************************/
 static int Processer_trame( struct TRAME_MODBUS *trame )
  { struct TRAME_MODBUS_IDENT *trame_ident;
    if (trame->dest != 0xFF) return(FALSE);                        /* Si c'est pas pour nous, on se casse */
    if ( trame->source >= NBR_ID_MODBUS )
     { Info_n( Config.log, DEBUG_INFO, "Processer_trame: module id not in range", trame->source );
       return(TRUE);
     }
    if (Config.module_modbus[trame->source].id != trame->source)
     { Info_n( Config.log, DEBUG_INFO, "Processer_trame: Module MODBUS unknown", trame->source);
       return(TRUE);
     }

    switch( trame->fonction )
     { case FCT_IDENT: printf("bouh\n");
	               trame_ident = (struct TRAME_MODBUS_IDENT *)trame->donnees;
                       printf("Recu Ident de %d: version %d.%d, nbr ana %d, nbr tor %d (%d choc), sortie %d\n",
                              trame->source, trame_ident->version_major, trame_ident->version_minor,
                              trame_ident->nbr_entre_ana, trame_ident->nbr_entre_tor,
                              trame_ident->nbr_entre_choc, trame_ident->nbr_sortie_tor );
                       break;
       case FCT_ENTRE_TOR:
             { int e, cpt, nbr_e;
               nbr_e = Config.module_modbus[trame->source].e_max - Config.module_modbus[trame->source].e_min + 1;
               for( cpt = 0; cpt<nbr_e; cpt++)
                { e = ! (trame->donnees[cpt >> 3] & (0x80 >> (cpt & 0x07)));
                  SE( Config.module_modbus[trame->source].e_min + cpt, e );
                }
             }
	    break;
       case FCT_ENTRE_ANA:
             { int cpt, nbr_ea;
               if (Config.module_modbus[trame->source].ea_min == -1) nbr_ea = 0;
               else nbr_ea = Config.module_modbus[trame->source].ea_max -
                             Config.module_modbus[trame->source].ea_min + 1;
               for( cpt = 0; cpt<nbr_ea; cpt++)
                { gint num_ea, val_int, ajout1, ajout2, ajout3, ajout4, ajout5;

                  val_int =   trame->donnees[cpt] << 2;
                  ajout1  =   trame->donnees[nbr_ea + (cpt >> 2)];
                  ajout2 = 0xC0>>((cpt & 0x03)<<1);
                  ajout3 = (3-(cpt & 0x03))<<1;
                  ajout4 = ajout1  & ajout2;
                  ajout5 = ajout4 >> ajout3;
                  val_int += ajout5;
                  num_ea = Config.module_modbus[trame->source].ea_min + cpt;
                  Partage->ea[ num_ea ].val_ech =                                   /* Valeur à l'echelle */
                   ((((gdouble)val_int - 1023.0 ) * (Partage->ea[num_ea].max - Partage->ea[num_ea].min)) / 824)
                   + Partage->ea[num_ea].max;

                  if (Partage->ea[ num_ea ].val_ech < Partage->ea[num_ea].min)
                   { Partage->ea[ num_ea ].val_ech = Partage->ea[num_ea].min; }

                  Partage->ea[ num_ea ].val = val_int;
                  Partage->ea[ num_ea ].date = time(NULL);   /* utilisé ?? */
                  /*printf("EA[%d] = %d %f (min %f, max %f) ajout %d %d %d %d %d %d\n",
                         num_ea, val_int, Partage->ea[ num_ea ].val_ech, Partage->ea[num_ea].min,
                         Partage->ea[num_ea].max, trame->donnees[cpt], ajout1, ajout2, ajout4, ajout4, ajout5 );*/

                  Ajouter_arch( MNEMO_ENTREE_ANA, num_ea, val_int );

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
#endif
/**********************************************************************************************************/
/* Connecter: Tentative de connexion au serveur                                                           */
/* Entrée: une nom et un password                                                                         */
/* Sortie: les variables globales sont initialisées, FALSE si pb                                          */
/**********************************************************************************************************/
 static gboolean Connecter_module ( void )
  { struct sockaddr_in src;                                            /* Données locales: pas le serveur */
    struct hostent *host;
    int connexion;

    if ( !(host = gethostbyname( "192.168.0.128" )) )                             /* On veut l'adresse IP */
     { printf ("MODBUS: Connecter_module: DNS_Failed\n" );
       return(FALSE);
     } else printf("DNS OK\n");

    src.sin_family = host->h_addrtype;
    memcpy( (char*)&src.sin_addr, host->h_addr, host->h_length );                 /* On recopie les infos */
    src.sin_port = htons( MODBUS_PORT_TCP );                                /* Port d'attaque des modules */

    if ( (connexion = socket( AF_INET, SOCK_STREAM, 0)) == -1)                          /* Protocol = TCP */
     { printf ("MODBUS: Connecter_module: Socket creation failed\n" );
       return(FALSE);
     } else printf("Socket OK\n");

    if (connect (connexion, (struct sockaddr *)&src, sizeof(src)) == -1)
     { printf( "MODBUS: Connecter_module: connexion refused by module\n" );
       close(connexion);
       return(FALSE);
     } else printf("Connect OK\n");

/*    fcntl( connexion, F_SETFL, SO_KEEPALIVE | SO_REUSEADDR );*/
    fd_module = connexion;                                                            /* Sauvegarde du fd */

    return(TRUE);
  }

/**********************************************************************************************************/
/* Main: Fonction principale du MODBUS                                                                    */
/**********************************************************************************************************/
 int main ( void )
  { gint retval, nbr_oct_lu, cpt;
    fd_set fdselect;
    struct TRAME_MODBUS_REQUETE Trame;
    struct timeval tv_avant, tv_apres;
    struct timeval tv;
    gint car;

    if ( !Connecter_module() ) exit(0);

    nbr_oct_lu = 0;
    trame.transaction_id = 0;
    tv.tv_sec = 0;
    tv.tv_usec= 100;
       
    for ( ; ; )
     { FD_ZERO(&fdselect);
       FD_SET(0, &fdselect);
       FD_SET(fd_module, &fdselect );
       retval = select(fd_module+1, &fdselect, NULL, NULL, &tv );               /* Attente d'un caractere */
       if (retval>=0)
	{ if (FD_ISSET(0, &fdselect))                                            /* Est-ce au clavier ?? */
           { read( 0, &car, 1 );
             switch(car)
	      { case 'i': Envoyer_trame_input_TOR();
                          printf("envoi trame ident\n");
			  break;
                case 'a': Envoyer_trame_input_ANA();
                          printf("envoi trame ident\n");
			  break;
                case 'q': close(fd_module); exit(0);
		case '\n': break;
	        default: break;
	      }
	   }
	  else if (FD_ISSET(fd_module, &fdselect))                       /* Est-ce sur la ligne modbus ?? */
           { int bute;
             if (nbr_oct_lu<TAILLE_ENTETE_MODBUS)
	      { bute = TAILLE_ENTETE_MODBUS; } else { bute = TAILLE_ENTETE_MODBUS + ntohs(trame.taille); }

             cpt = read( fd_module, (unsigned char *)&trame + nbr_oct_lu, bute-nbr_oct_lu );
             printf("lecture de cpt=%d caracteres\n", cpt );
             if (cpt>0)
              { nbr_oct_lu = nbr_oct_lu + cpt;
		printf("nbr_oct_lu = %d  on veut %d \n", nbr_oct_lu, TAILLE_ENTETE_MODBUS + ntohs(trame.taille) );
                for (cpt=0; cpt<nbr_oct_lu; cpt++)
		    { printf("%02X ", (unsigned char)*((unsigned char *)&trame + cpt) );
		    }
		if (nbr_oct_lu >= TAILLE_ENTETE_MODBUS + ntohs(trame.taille))                     /* traitement trame */
		 { int crc_recu;
                   printf("Recu trame brute: ");
		   for (cpt=0; cpt<sizeof(trame); cpt++)
		    { printf("%02X ", (unsigned char)*((unsigned char *)&trame +cpt) );
		    }
		   printf("\n");
		   gettimeofday( &tv_avant, NULL );
                   /*Processer_trame( &trame );*/
                   gettimeofday( &tv_apres, NULL );
                   printf("Temps de traitement: %f\n", tv_apres.tv_sec - tv_avant.tv_sec +
                                                       (tv_apres.tv_usec - tv_avant.tv_usec)/1000000.0 );
                   nbr_oct_lu = 0;
		   memset (&trame, 0, sizeof(struct TRAME_MODBUS_REPONSE) );
		 }
              } else { printf("Erreur ? bute= %d nbr_oct_lu = %d\n", bute, nbr_oct_lu); sleep(1); }
	  }
	} else { printf("retval = %d\n", retval ); }

     }
  }
/*--------------------------------------------------------------------------------------------------------*/
