/**********************************************************************************************************/
/* ihx2eprom.c        Programmation de la RAM/EPROM du microcontroleur                                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                     mar 08 jun 2004 12:38:06 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <sys/io.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <stdio.h>
 
 #define BASE_PORT      0x318
 #define FICHIER_CONF   "conf.asm"
/**********************************************************************************************************/
/* lecture_octet: Conversion de 2 digits du fichier en une valeur numérique sur un octet                  */
/* Entrée: l'id du fichier en cours de lecture                                                            */
/* Sortie: l'octet lu                                                                                     */
/**********************************************************************************************************/
 char lecture_octet ( int id )
  { char pF, pf, o;
    read( id, &pF, sizeof(char) );
    read( id, &pf, sizeof(char) );
    if (pF>='0' && pF<='9') pF-='0'; else pF+=(10-'A');                           /* Recalibrage effectif */
    if (pf>='0' && pf<='9') pf-='0'; else pf+=(10-'A');
    o = (pF<<4) + pf;
    return(o);
  }
/**********************************************************************************************************/
/* Main: Gravure d'un .ihx dans la RAM/ROM du microcontroleur                                             */
/* Entrée: argc, argv pour gnome/gtk                                                                      */
/* Sortie: 0                                                                                              */
/**********************************************************************************************************/
 int main ( int argc, char *argv[] )
  { int longueur;                                         /* Longueur (en octets) de l'enregistrement IHX */
    int cpt_general;                            /* Compteur general d'octets ecrits dans le fichier cible */
    int type;                                                             /* Type de l'enregistrement IHX */
    int crc8;                                                   /* Valeur du crc8 de l'enregistrement IHX */
    int adresse;                                                        /* Adresse de stockage de l'octet */
    int octet;                                                         /* Valeur à envoyer dans la EEPROM */
    int cpt;                                                                         /* Un compteur banal */
    int id_fichier;
    char *nom_fichier;
    int ram;                                         /* True si la cible est une ram, false si c une prom */
    int id_esclave, nbr_entre_ana, nbr_entre_tor, nbr_entre_choc, nbr_sortie_tor, nbr_sortie_ana;
    int pid;
    char chaine[80];

    if (argc!=8)
     { printf("Usage: Ihx2eprom id_esclave inANA inTOR inCHOC outTOR outANA [ram|rom]\n");
       exit(0);
     }

    id_esclave     = atoi(argv[1]);
    nbr_entre_ana  = atoi(argv[2]);
    nbr_entre_tor  = atoi(argv[3]);
    nbr_entre_choc = atoi(argv[4]);
    nbr_sortie_tor = atoi(argv[5]);
    nbr_sortie_ana = atoi(argv[6]);

    printf("Creation du fichier de config avec:\n");
    printf("ID esclave %d type carte %d/%d/%d/%d/%d\n", id_esclave,
           nbr_entre_ana, nbr_entre_tor, nbr_entre_choc, nbr_sortie_tor, nbr_sortie_ana );

    unlink(FICHIER_CONF);
    id_fichier = open( FICHIER_CONF, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR );
    snprintf( chaine, sizeof(chaine), "ID_ESCLAVE = %d\n", id_esclave );
    write( id_fichier, chaine, strlen(chaine) );
    snprintf( chaine, sizeof(chaine), "NBR_ENTRE_ANA = %d\n", nbr_entre_ana );
    write( id_fichier, chaine, strlen(chaine) );
    snprintf( chaine, sizeof(chaine), "NBR_ENTRE_TOR = %d\n", nbr_entre_tor );
    write( id_fichier, chaine, strlen(chaine) );
    snprintf( chaine, sizeof(chaine), "NBR_ENTRE_CHOC = %d\n", nbr_entre_choc );
    write( id_fichier, chaine, strlen(chaine) );
    snprintf( chaine, sizeof(chaine), "NBR_SORTIE_TOR = %d\n", nbr_sortie_tor );
    write( id_fichier, chaine, strlen(chaine) );
    snprintf( chaine, sizeof(chaine), "NBR_SORTIE_ANA = %d\n", nbr_sortie_ana );
    write( id_fichier, chaine, strlen(chaine) );
    close(id_fichier);

    printf("Compilation des sources\n");
    pid = fork();
    if (pid==-1) { printf("For failed ! exit !\n"); exit(-1); }
    if (!pid)                                                                 /* Nous sommes dans le fils */
     { char *arguments[] = { "/usr/bin/make", NULL };
       printf("Fils: execution de la commande de compilation\n");
       execve( "/usr/bin/make", arguments, NULL );
       perror("erreur");
       printf("Fils: execution failed !\n");
       exit(0);
     }
    waitpid( pid, NULL, 0 );                                             /* Attente de fin de compilation */
    printf("Oki, le compilation semble reussie...\n");
           
    nom_fichier = "init.ihx";
    ram = !strcmp(argv[7], "ram");
 
    printf("Demande d'acces aux ports...\n");

    if (ioperm( BASE_PORT, 4, 1 ))                                 /* On prend les droits pour la gravure */
     { printf("Acces au port impossible\n"); exit(-1); } 
    
    printf("MicroControleur en mode RESET\n");                           /* On stoppe le microscontroleur */
    outb ( 0, BASE_PORT );

    id_fichier = open( nom_fichier, O_RDONLY );                              /* Ouverture du fichier .ihx */
    if (id_fichier<0)
     { printf("Ouverture fichier %s impossible\n", argv[1] ); }
    else
     { for( ; ; )
        { unsigned char o1, o2, o3, o4;                                      /* Les octets du fichier IHX */
          read( id_fichier, &o1, sizeof(char) );
          if (o1!=':')
           { printf("Erreur de synchro fichier %c %d\n", o1, o1);
             exit(0);
           }

          o1 = lecture_octet( id_fichier );
          longueur = o1;
          printf("Longueur enregistrement: %d\n", longueur );

          o1 = lecture_octet( id_fichier );
          o2 = lecture_octet( id_fichier );
          adresse = (o1<<8) + o2;
          printf("Adresse de stockage: %d\n", adresse );

          o1 = lecture_octet( id_fichier );
          type = o1;
          printf("Type = %d\n", type );
          if (type==1) { printf("Parsing terminé\n");        break; }
       
          for (cpt=0; cpt<longueur; cpt++)
           { o1 = lecture_octet( id_fichier );
            
             octet = o1;
             printf("Adresse %04X <- %02X\n", adresse, octet );

             outb( adresse & 0xFF, BASE_PORT + 1 );                                  /* Envoie sur la RAM */
             outb( (adresse>>8) & 0xFF, BASE_PORT + 2 );
	     outb( octet, BASE_PORT + 3 );                                         /* Mis dans la bascule */
	     if (type==0) outb( 2, BASE_PORT );                                   /* Gravure ROM */
             if (!ram) { usleep(60000); }                     /* Si eprom, on attend la gravure effective */
	     outb( 0, BASE_PORT );                                                  /* Fin de gravure ROM */

	     adresse++;
           }

          o1 = lecture_octet( id_fichier );
          crc8 = o1;
          printf("Crc8 = %X (swappé)\n", crc8 );  
          read( id_fichier, &o1, sizeof(char) );                                        /* Retour chariot */
        }
       close(id_fichier);                                                     /* fermeture de fichier ihx */
     }
    printf("MicroControleur en mode AUTONOME\n");
    outb ( 1, BASE_PORT );
    
    ioperm( BASE_PORT, 4, 0 );                                                 /* Arret de la requisition */
    exit(0);
  }
/*--------------------------------------------------------------------------------------------------------*/
