/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_histo_courbe.c        Configuration des histo courbes de Watchdog v2.0         */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 nov 2007 13:34:53 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_histo_courbe.c
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
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Proto_effacer_entree: Retrait du entree en parametre                                                   */
/* Entrée: le client demandeur et le entree en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_histo_courbe ( struct CLIENT *client, struct CMD_ID_COURBE *rezo_courbe )
  { /*struct COURBE *courbe;*/

  }
/**********************************************************************************************************/
/* Proto_ajouter_entree: Un client nous demande d'ajouter un entree Watchdog                              */
/* Entrée: le entree à créer                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_histo_courbe_thread ( struct CLIENT *client )
  { struct CMD_APPEND_COURBE envoi_courbe;
    struct ARCHDB *arch;
    struct CMD_ID_COURBE rezo_courbe;
    struct DB *Db_watchdog;
    SQLHSTMT hquery;
    guint i, delta_x;
/*    Db_watchdog = client->Db_watchdog;*/

    memcpy ( &rezo_courbe, &client->courbe, sizeof( rezo_courbe ) );
    client->courbe.id=-1;

    Db_watchdog = ConnexionDB( Config.log, Config.db_name,
                               Config.db_admin_username, Config.db_password );
    if (!Db_watchdog)
     { Info_c( Config.log, DEBUG_DB,
               _("SSRV: Proto_ajouter_histo_courbe_thread: Unable to open database (dsn)"), Config.db_name );
       Unref_client( client );                                           /* Déréférence la structure cliente */
       pthread_exit(NULL);
     }
 

printf("New histo courbe: type %d num %d\n", rezo_courbe.type, rezo_courbe.id );

    Envoi_client ( client, TAG_HISTO_COURBE, SSTAG_SERVEUR_ADD_HISTO_COURBE_OK,  /* Envoi préparation au client */
                   (gchar *)&rezo_courbe, sizeof(struct CMD_ID_COURBE) );

    if (client->histo_courbe.date_first > client->histo_courbe.date_last)
     { client->histo_courbe.date_last = (guint) time(NULL);
       client->histo_courbe.date_first = client->histo_courbe.date_last - 3600;
     }

    switch( rezo_courbe.type )
     { case MNEMO_ENTREE_ANA:
       case MNEMO_ENTREE:

            hquery = Recuperer_archDB ( Config.log, Db_watchdog, rezo_courbe.type, rezo_courbe.id,
                                        client->histo_courbe.date_first,
                                        client->histo_courbe.date_last );
               
            envoi_courbe.slot_id = rezo_courbe.slot_id;     /* Valeurs par defaut si pas d'enregistrement */
            envoi_courbe.type    = rezo_courbe.type;
            envoi_courbe.date    = 0;
            envoi_courbe.val     = (rezo_courbe.type == MNEMO_ENTREE ? E(rezo_courbe.id) : 0);

            delta_x = (client->histo_courbe.date_last - client->histo_courbe.date_first) /
                      TAILLEBUF_HISTO_EANA;

            if (!hquery) break;
            for( i=0; i<TAILLEBUF_HISTO_EANA; )
             { arch = Recuperer_archDB_suite( Config.log, Db_watchdog, hquery );    /* On prend un enreg. */

               if (!arch) break;

               if (i==0)
                { envoi_courbe.date = arch->date_sec;
                  envoi_courbe.val  = arch->valeur;
                  g_free(arch);
                  Envoi_client( client, TAG_HISTO_COURBE, SSTAG_SERVEUR_APPEND_HISTO_COURBE,
                                (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                  i++;                                     /* nous avons envoyé un enregistrement de plus */
                }
               else
                {
encore:
                  if (envoi_courbe.date + delta_x < arch->date_sec)
                   { envoi_courbe.date = envoi_courbe.date + delta_x;
         /*printf("Envoi %d arch %d val %d  i %d\n", envoi_courbe.date, arch->date_sec, envoi_courbe.val, i );  */
                     Envoi_client( client, TAG_HISTO_COURBE, SSTAG_SERVEUR_APPEND_HISTO_COURBE,
                                   (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                     i++;                                  /* nous avons envoyé un enregistrement de plus */
                     goto encore;
                   }
                  else
                   { if (envoi_courbe.date + delta_x > arch->date_sec)
                      { envoi_courbe.val  = arch->valeur;/* Si plus d'un eregistrement dans les meme 5 sec */
        /* printf("Skip  %d arch %d val %d  i %d\n", envoi_courbe.date, arch->date_sec, envoi_courbe.val, i );*/
                        g_free(arch);
                      }
                     else
                       { envoi_courbe.date = arch->date_sec;
                         envoi_courbe.val  = arch->valeur;
                         g_free(arch);
                         Envoi_client( client, TAG_HISTO_COURBE, SSTAG_SERVEUR_APPEND_HISTO_COURBE,
                                       (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                         i++;                              /* nous avons envoyé un enregistrement de plus */
                       }
                   }
                }
             }

            if (i==TAILLEBUF_HISTO_EANA)     /* si tout a été envoyé, il faut verifier la base de données */
             { EndQueryDB( Config.log, Db_watchdog, hquery ); }

            while (i<TAILLEBUF_HISTO_EANA)                          /* A-t'on envoyé le nombre souhaité ? */
             { envoi_courbe.date = envoi_courbe.date + delta_x;
         /*printf("Termine   %d val %d  i %d\n",     envoi_courbe.date, envoi_courbe.val, i );*/
               Envoi_client( client, TAG_HISTO_COURBE, SSTAG_SERVEUR_APPEND_HISTO_COURBE,
                             (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
               i++;
             }
            break;
       default : printf("Error\n");
     }

    DeconnexionDB( Config.log, &Db_watchdog );                                          /* Deconnexion DB */
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
