/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_histo_courbe.c        Configuration des histo courbes de Watchdog v2.0         */
/* Projet WatchDog version 2.0       Gestion d'habitat                       dim 18 nov 2007 13:34:53 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_histo_courbe.c
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
 #include <sys/prctl.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "Reseaux.h"
 #include "watchdogd.h"
/**********************************************************************************************************/
/* Proto_effacer_entree: Retrait du entree en parametre                                                   */
/* Entrée: le client demandeur et le entree en question                                                   */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_histo_courbe ( struct CLIENT *client, struct CMD_TYPE_COURBE *rezo_courbe )
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
    struct CMD_TYPE_COURBE rezo_courbe;
    struct DB *db;

    prctl(PR_SET_NAME, "W-HISTOCourbe", 0, 0, 0 );

    memcpy ( &rezo_courbe, &client->courbe, sizeof( rezo_courbe ) );
    client->courbe.id=-1;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Info( Config.log, DEBUG_DB, "Proto_ajouter_histo_courbe_thread: Unable to open database (dsn)" );
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */


printf("New histo courbe: type %d num %d\n", rezo_courbe.type, rezo_courbe.id );

    Envoi_client ( client, TAG_HISTO_COURBE, SSTAG_SERVEUR_ADD_HISTO_COURBE_OK,  /* Envoi préparation au client */
                   (gchar *)&rezo_courbe, sizeof(struct CMD_TYPE_COURBE) );

    if (client->histo_courbe.date_first > client->histo_courbe.date_last)
     { client->histo_courbe.date_last = (guint) time(NULL);
       client->histo_courbe.date_first = client->histo_courbe.date_last - 3600;
     }

    Recuperer_archDB ( Config.log, db, rezo_courbe.type, rezo_courbe.id,
                       client->histo_courbe.date_first,
                       client->histo_courbe.date_last );
               
    envoi_courbe.slot_id = rezo_courbe.slot_id;             /* Valeurs par defaut si pas d'enregistrement */
    envoi_courbe.type    = rezo_courbe.type;

    arch = Recuperer_archDB_suite( Config.log, db );                        /* On prend le premier enreg. */

    if (arch) { envoi_courbe.date    = arch->date_sec;                          /* Si enreg, on le pousse */
                envoi_courbe.val_int = arch->valeur;
              }                                      /* Si pas d'enreg, l'EA n'a pas bougé sur la période */
    else      { envoi_courbe.date    = client->histo_courbe.date_first;
                switch (rezo_courbe.type)
                 { case MNEMO_ENTREE_ANA : envoi_courbe.val_int = Partage->ea[rezo_courbe.id].val_int; break;
                   case MNEMO_ENTREE     : envoi_courbe.val_int = E(rezo_courbe.id);  break;
                   case MNEMO_SORTIE     : envoi_courbe.val_int = A(rezo_courbe.id);  break;
                   default : envoi_courbe.val_int = 0; break;
                 }
              }                              
    Envoi_client( client, TAG_HISTO_COURBE, SSTAG_SERVEUR_APPEND_HISTO_COURBE,
                  (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );

    if (arch)                              /* Si on a traité un enreg, on va traiter les autres en boucle */
     { g_free(arch);                                             /* Libération de l'enregistrement d'init */
       for( ; ; )
        { arch = Recuperer_archDB_suite( Config.log, db );                          /* On prend un enreg. */

          if (!arch) break;
          envoi_courbe.date    = arch->date_sec;
          envoi_courbe.val_int = arch->valeur;
          Envoi_client( client, TAG_HISTO_COURBE, SSTAG_SERVEUR_APPEND_HISTO_COURBE,
                       (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
          g_free(arch);
        }
     }
    Libere_DB_SQL( Config.log, &db );
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
