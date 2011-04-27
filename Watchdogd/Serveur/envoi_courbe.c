/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_courbe.c        Configuration des courbes de Watchdog v2.0                     */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 12 jun 2005 18:01:40 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_courbe.c
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
 void Proto_effacer_courbe ( struct CLIENT *client, struct CMD_TYPE_COURBE *rezo_courbe )
  { struct COURBE *courbe;
    GList *liste_courbe;
    courbe = NULL;
    liste_courbe = client->courbes;                                          /* Recherche de la structure */
    while (liste_courbe)
     { courbe = (struct COURBE *)liste_courbe->data;
       if (courbe->slot_id == rezo_courbe->slot_id)
        { client->courbes = g_list_remove( client->courbes, courbe );
          return;
        }
       liste_courbe = liste_courbe -> next;
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_entree: Un client nous demande d'ajouter un entree Watchdog                              */
/* Entrée: le entree à créer                                                                              */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_courbe_thread ( struct CLIENT *client )
  { struct CMD_APPEND_COURBE envoi_courbe;
    struct ARCHDB *arch;
    struct CMD_TYPE_COURBE rezo_courbe;
    struct COURBE *courbe;
    struct DB *db;
    GList *liste_courbe;
    time_t date;
    guint i;

    prctl(PR_SET_NAME, "W-EnvoiCOURBE", 0, 0, 0 );

    if ( g_list_length( client->courbes ) > 18 )
     { struct CMD_GTK_MESSAGE erreur;
       Info( Config.log, DEBUG_INFO, "Proto_ajouter_courbe_thread: trop de courbe pour le client" );
       g_snprintf( erreur.message, sizeof(erreur.message), "Nombre de courbe maximum atteint" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit(NULL);
     }

    memcpy ( &rezo_courbe, &client->courbe, sizeof( rezo_courbe ) );
    client->courbe.id=-1;

    courbe = NULL;
    liste_courbe = client->courbes;                         /* Recherche d'une structure deja initialisée */
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
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit(NULL);
     }

    courbe = (struct COURBE *)g_malloc0( sizeof( struct COURBE ) );
    if (!courbe)
     { struct CMD_GTK_MESSAGE erreur;
       Info( Config.log, DEBUG_COURBE, "Proto_ajouter_courbe_thread: Pb d'allocation memoire" );
       g_snprintf( erreur.message, sizeof(erreur.message), "Pb d'allocation memoire" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit(NULL);
     }

printf("New courbe: type %d num %d\n", rezo_courbe.type, rezo_courbe.id );
    courbe->id      = rezo_courbe.id;
    courbe->slot_id = rezo_courbe.slot_id;
    courbe->type    = rezo_courbe.type;

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       Info( Config.log, DEBUG_DB, "Proto_ajouter_courbe_thread: Unable to open database (dsn)" );
       g_free(courbe);
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */
       
    Envoi_client ( client, TAG_COURBE, SSTAG_SERVEUR_ADD_COURBE_OK,        /* Envoi préparation au client */
                   (gchar *)&rezo_courbe, sizeof(struct CMD_TYPE_COURBE) );

    date = time(NULL);                                                    /* On recupere la date actuelle */

    Recuperer_archDB ( Config.log, db, courbe->type, courbe->id, (date - 3*3600), date );
               
    envoi_courbe.slot_id = courbe->slot_id;                 /* Valeurs par defaut si pas d'enregistrement */
    envoi_courbe.type    = courbe->type;

    arch = Recuperer_archDB_suite( Config.log, db );                        /* On prend le premier enreg. */

    if (arch) { envoi_courbe.date    = arch->date_sec;                          /* Si enreg, on le pousse */
                envoi_courbe.val_int = arch->valeur;
              }                                      /* Si pas d'enreg, l'EA n'a pas bougé sur la période */
    else      { envoi_courbe.date    = date - 3*3600;
                switch (courbe->type)
                 { case MNEMO_ENTREE_ANA : envoi_courbe.val_int = Partage->ea[courbe->id].val_int; break;
                   case MNEMO_ENTREE     : envoi_courbe.val_int = E(courbe->id);  break;
                   case MNEMO_SORTIE     : envoi_courbe.val_int = A(courbe->id);  break;
                   default : envoi_courbe.val_int = 0; break;
                 }
              }                              
    Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                  (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );

    if (arch)                              /* Si on a traité un enreg, on va traiter les autres en boucle */
     { g_free(arch);                                             /* Libération de l'enregistrement d'init */
       for( ; ; )
        { arch = Recuperer_archDB_suite( Config.log, db );                          /* On prend un enreg. */

          if (!arch) break;
          envoi_courbe.date    = arch->date_sec;
          envoi_courbe.val_int = arch->valeur;
          Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                       (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
          g_free(arch);
        }
     }
    client->courbes = g_list_append ( client->courbes, courbe );

    Libere_DB_SQL( Config.log, &db );
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
