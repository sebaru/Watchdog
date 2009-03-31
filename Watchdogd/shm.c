/**********************************************************************************************************/
/* Watchdogd/shm.c        Gestion de la mémoire partagée                                                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      lun 02 jun 2003 14:45:52 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/

 #include <glib.h>
 #include <sys/ipc.h>
 #include <sys/shm.h>

 #include "Erreur.h"
 #include "Config.h"
 #include "watchdogd.h"

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
/**********************************************************************************************************/
/* Shm_init: initialisation de la mémoire partagée                                                        */
/* Entrée: rien                                                                                           */
/* Sortie: un pointeur sur la zone mémoire partagée                                                       */
/**********************************************************************************************************/
 struct PARTAGE *Shm_init ( void )
  { struct PARTAGE *partage;
    gint taille;

    taille = sizeof( struct PARTAGE ) +                                     /* Le jeton, les comms, les I */
             sizeof( struct SOUS_SERVEUR ) * Config.max_serveur;         /* Nombre de connexions par fils */
    Info_n( Config.log, DEBUG_MEM, "Shm_init: size required", taille );

#ifdef bouh
    g_snprintf( nom, sizeof(nom), "%s/%s", Config.home, VERROU_SERVEUR );
    k = ftok( nom, 0);
    if (k==-1)
     { Info( Config.log, DEBUG_MEM, "Shm_init: ftok failed" );
       return(NULL);
     }

    shmid = shmget( k, taille, 0600 );                                  /* Test si le segment existe deja */
    if (shmid==-1)
     { shmid = shmget( k, taille, IPC_CREAT | 0600 );                          /* Creation le cas échéant */
       if (shmid==-1)
        { Info( Config.log, DEBUG_MEM, "Shm_init: Shmget failed" );
          return(NULL);
        }
     }
    Info_n( Config.log, DEBUG_MEM, "Shm_init: shmid", shmid );
    partage = shmat( shmid, NULL, 0 );
    partage->shmid = shmid;
#endif
    partage = g_malloc0( taille );
    partage->Sous_serveur = &partage->ss_serveur;
    return(partage);
  }
/**********************************************************************************************************/
/* Shm_stop: Stoppe l'utilisation de la mémoire partagée                                                  */
/* Entrée: une structure de mémoire partagée                                                              */
/* Sortie: false si probleme                                                                              */
/**********************************************************************************************************/
 gboolean Shm_stop ( struct PARTAGE *partage )
  { gint shmid;
#ifdef bouh
    if (partage)
     { shmid = partage->shmid;
       shmctl( shmid, IPC_RMID, 0 );
       if ( shmdt( (void *) partage ) == -1)
        { Info(Config.log, DEBUG_MEM, "Shm_stop: shmdt failed" );
          return(FALSE);
        }
       Info_n(Config.log, DEBUG_MEM, "Shm_stop: shmid", shmid );
     }
#endif
g_free(Partage);

    return(TRUE);
  }
/*--------------------------------------------------------------------------------------------------------*/
