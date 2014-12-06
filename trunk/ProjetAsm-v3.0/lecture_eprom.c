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

/**********************************************************************************************************/
/* Main: Gravure d'un .ihx dans la RAM/ROM du microcontroleur                                             */
/* Entrée: argc, argv pour gnome/gtk                                                                      */
/* Sortie: 0                                                                                              */
/**********************************************************************************************************/
 int main ( int argc, char *argv[] )
  { int adresse;                                                        /* Adresse de stockage de l'octet */
    int octet;                                                         /* Valeur à envoyer dans la EEPROM */
    
    if (ioperm( BASE_PORT, 4, 1 ))                                 /* On prend les droits pour la gravure */
     { printf("Acces au port impossible\n"); exit(-1); } 
    
    printf("MicroControleur en mode RESET\n");                           /* On stoppe le microscontroleur */
    outb ( 0, BASE_PORT );


    for(adresse=0;adresse<0x2000;adresse++)
     { outb( adresse & 0xFF, BASE_PORT + 1 );                                        /* Envoie sur la RAM */
       outb( (adresse>>8) & 0xFF, BASE_PORT + 2 );

       octet = inb(BASE_PORT+3);                                                          /* Verification */
       if ( octet != 0xFF ) { printf("Pas vierge\n" ); break; }
     }

    printf("MicroControleur en mode AUTONOME\n");
    outb ( 1, BASE_PORT );
    
    ioperm( BASE_PORT, 4, 0 );                                                 /* Arret de la requisition */
    exit(0);
  }
/*--------------------------------------------------------------------------------------------------------*/
