/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_courbe.c        Configuration des courbes de Watchdog v2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 12 jun 2005 18:01:40 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

 #include "Reseaux.h"
 #include "Archive_DB.h"
 #include "Erreur.h"
 #include "Config.h"
 #include "Client.h"
 #include "watchdogd.h"

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 extern struct PARTAGE *Partage;                             /* Acc�s aux donn�es partag�es des processes */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Proto_effacer_entree: Retrait du entree en parametre                                                   */
/* Entr�e: le client demandeur et le entree en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_courbe ( struct CLIENT *client, struct CMD_ID_COURBE *rezo_courbe )
  { struct COURBE *courbe;
    GList *liste_courbe;
    courbe = NULL;
    liste_courbe = client->courbes;                                          /* Recherche de la structure */
    while (liste_courbe)
     { courbe = (struct COURBE *)liste_courbe->data;
       if (courbe->slot_id == rezo_courbe->slot_id && courbe->type == rezo_courbe->type)
        { client->courbes = g_list_remove( client->courbes, courbe );
          return;
        }
       liste_courbe = liste_courbe -> next;
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_entree: Un client nous demande d'ajouter un entree Watchdog                              */
/* Entr�e: le entree � cr�er                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_courbe_thread ( struct CLIENT *client )
  { struct CMD_APPEND_COURBE envoi_courbe;
    struct ARCHDB *arch;
    struct CMD_ID_COURBE rezo_courbe;
    struct COURBE *courbe;
    struct DB *Db_watchdog;
    GList *liste_courbe;
    SQLHSTMT hquery;
    time_t date;
    guint i;
   /* Db_watchdog = client->Db_watchdog;*/

    memcpy ( &rezo_courbe, &client->courbe, sizeof( rezo_courbe ) );
    client->courbe.id=-1;

    Db_watchdog = ConnexionDB( Config.log, Config.db_name,
                               Config.db_admin_username, Config.db_password );
    if (!Db_watchdog)
     { Info_c( Config.log, DEBUG_DB,
               _("SSRV: Proto_ajouter_courbe_thread: Unable to open database (dsn)"), Config.db_name );
       Unref_client( client );                                           /* D�r�f�rence la structure cliente */
       pthread_exit(NULL);
     }

    if ( g_list_length( client->courbes ) > 18 )
     { struct CMD_GTK_MESSAGE erreur;
       Info( Config.log, DEBUG_INFO, "Proto_ajouter_courbe_thread: trop de courbe pour le client" );
       g_snprintf( erreur.message, sizeof(erreur.message), "Nombre de courbe maximum atteint" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Unref_client( client );                                        /* D�r�f�rence la structure cliente */
       pthread_exit(NULL);
     }

    courbe = NULL;
    liste_courbe = client->courbes;                         /* Recherche d'une structure deja initialis�e */
    while (liste_courbe)
     { courbe = (struct COURBE *)liste_courbe->data;
       if (courbe->id == rezo_courbe.id && courbe->type == rezo_courbe.type) break;
       liste_courbe = liste_courbe -> next;
     }

    if ( liste_courbe )
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message), "Courbe deja affiche" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Unref_client( client );                                        /* D�r�f�rence la structure cliente */
       pthread_exit(NULL);
     }
       
    courbe = (struct COURBE *)g_malloc0( sizeof( struct COURBE ) );
    if (!courbe)
     { struct CMD_GTK_MESSAGE erreur;
       Info( Config.log, DEBUG_MEM, "Proto_ajouter_courbe_thread: Pb d'allocation memoire" );
       g_snprintf( erreur.message, sizeof(erreur.message), "Pb d'allocation memoire" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Unref_client( client );                                        /* D�r�f�rence la structure cliente */
       pthread_exit(NULL);
     }

printf("New courbe: type %d num %d\n", rezo_courbe.type, rezo_courbe.id );
    courbe->id      = rezo_courbe.id;
    courbe->slot_id = rezo_courbe.slot_id;
    courbe->type    = rezo_courbe.type;

    Envoi_client ( client, TAG_COURBE, SSTAG_SERVEUR_ADD_COURBE_OK,        /* Envoi pr�paration au client */
                   (gchar *)&rezo_courbe, sizeof(struct CMD_ID_COURBE) );

    switch( courbe->type )
     { case MNEMO_ENTREE_ANA:
       case MNEMO_ENTREE:
            date = time(NULL);                                            /* On recupere la date actuelle */

            hquery = Recuperer_archDB ( Config.log, Db_watchdog, courbe->type, courbe->id,
                                        (date - TAILLEBUF_HISTO_EANA*COURBE_TEMPS_TOP), date );
               
            envoi_courbe.slot_id = courbe->slot_id;         /* Valeurs par defaut si pas d'enregistrement */
            envoi_courbe.type    = courbe->type;
            envoi_courbe.date    = (date - TAILLEBUF_HISTO_EANA*COURBE_TEMPS_TOP);
            envoi_courbe.val     = (courbe->type == MNEMO_ENTREE ? E(courbe->id) : 0);

            if (!hquery) break;
            for( i=0; i<TAILLEBUF_HISTO_EANA; )
             { arch = Recuperer_archDB_suite( Config.log, Db_watchdog, hquery );    /* On prend un enreg. */

               if (!arch) break;

               if (i==0)
                { envoi_courbe.date = arch->date_sec;
                  envoi_courbe.val  = arch->valeur;
                  g_free(arch);
                  Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                                (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                  i++;                                     /* nous avons envoy� un enregistrement de plus */
                }
               else
                {
encore:
                  if (envoi_courbe.date + COURBE_TEMPS_TOP < arch->date_sec)
                   { envoi_courbe.date = envoi_courbe.date + COURBE_TEMPS_TOP;
         /*printf("Envoi %d arch %d val %d  i %d\n", envoi_courbe.date, arch->date_sec, envoi_courbe.val, i );  */
                     Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                                   (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                     i++;                                  /* nous avons envoy� un enregistrement de plus */
                     goto encore;
                   }
                  else
                   { if (envoi_courbe.date + COURBE_TEMPS_TOP > arch->date_sec)
                      { envoi_courbe.val  = arch->valeur;/* Si plus d'un eregistrement dans les meme 5 sec */
        /* printf("Skip  %d arch %d val %d  i %d\n", envoi_courbe.date, arch->date_sec, envoi_courbe.val, i );*/
                        g_free(arch);
                      }
                     else
                       { envoi_courbe.date = arch->date_sec;
                         envoi_courbe.val  = arch->valeur;
                         g_free(arch);
                         Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                                       (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                         i++;                              /* nous avons envoy� un enregistrement de plus */
                       }
                   }
                }
             }

            while (i<TAILLEBUF_HISTO_EANA)                          /* A-t'on envoy� le nombre souhait� ? */
             { envoi_courbe.date = envoi_courbe.date + COURBE_TEMPS_TOP;
         /*printf("Termine   %d val %d  i %d\n",     envoi_courbe.date, envoi_courbe.val, i );*/
               Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                             (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
               i++;
             }
            break;
       default : printf("Error\n");
     }

    client->courbes = g_list_append ( client->courbes, courbe );

    Unref_client( client );                                           /* D�r�f�rence la structure cliente */
    pthread_exit(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
