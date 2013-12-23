/**********************************************************************************************************/
/* verifier.c        Verification de la gravure EPROM                                                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 13 jun 2004 12:03:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <sys/io.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <sys/types.h>
 #include <stdio.h>
 
 #define BASE_PORT      0x318
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
    char chaine[80];

    nom_fichier = "init.ihx";
 
    printf("Demande d'acces aux ports...\n");

    if (ioperm( BASE_PORT, 4, 1 ))                                 /* On prend les droits pour la gravure */
     { printf("Acces au port impossible\n"); exit(-1); } 
    
    printf("MicroControleur en mode RESET\n");                           /* On stoppe le microscontroleur */
    outb ( 0, BASE_PORT );

    id_fichier = open( nom_fichier, O_RDONLY );                              /* Ouverture du fichier .ihx */
    if (id_fichier<0)
     { printf("Ouverture fichier %s impossible\n", nom_fichier ); }
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

          o1 = lecture_octet( id_fichier );
          o2 = lecture_octet( id_fichier );
          adresse = (o1<<8) + o2;

          o1 = lecture_octet( id_fichier );
          type = o1;
          printf("Type = %d\n", type );
          if (type==1) { printf("Parsing terminé\n");        break; }
       
          for (cpt=0; cpt<longueur; cpt++)
           { o1 = lecture_octet( id_fichier );
	     if (type!=0) continue;
            
             octet = o1;

             outb( adresse & 0xFF, BASE_PORT + 1 );                                  /* Envoie sur la RAM */
             outb( (adresse>>8) & 0xFF, BASE_PORT + 2 );


	     o1 = inb(BASE_PORT+3);                                                       /* Verification */
             printf("Dans ram: adresse %04X <- %02X on voulait %02X\n", adresse, o1, octet );
             if ( o1 != octet ) printf("Attention, erreur de gravure !!\n");

	     adresse++;
           }

          o1 = lecture_octet( id_fichier );
          crc8 = o1;
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
