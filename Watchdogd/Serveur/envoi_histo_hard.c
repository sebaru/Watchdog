/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_histo_hard.c        Requetes de l'ihstorique à la connexion client             */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 13 mar 2004 18:41:14 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_histo_hard.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - 
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
 #include <string.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "Reseaux.h"
 #include "watchdogd.h"
/**********************************************************************************************************/
/* Preparer_envoi_histo: convertit une structure HISTO_HARD en structure CMD_SHOW_HISTO_HARD              */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_HISTO_HARD *Preparer_envoi_histo ( struct HISTO_HARDDB *histo )
  { struct CMD_SHOW_HISTO_HARD *rezo_histo;

    rezo_histo = (struct CMD_SHOW_HISTO_HARD *)g_malloc0( sizeof(struct CMD_SHOW_HISTO_HARD) );
    if (!rezo_histo) { return(NULL); }
    rezo_histo->num         = histo->histo.msg.num;
    rezo_histo->type        = histo->histo.msg.type;
    rezo_histo->date_create_sec  = histo->histo.date_create_sec;
    rezo_histo->date_create_usec = histo->histo.date_create_usec;
    rezo_histo->date_fixe   = histo->histo.date_fixe;
    rezo_histo->date_fin    = histo->date_fin;
    memcpy( &rezo_histo->nom_ack, histo->histo.nom_ack, sizeof(rezo_histo->nom_ack) );
    memcpy( &rezo_histo->libelle, histo->histo.msg.libelle, sizeof(rezo_histo->libelle) );
    memcpy( &rezo_histo->objet,   histo->histo.msg.objet, sizeof(rezo_histo->objet) );
    return( rezo_histo );
  }
/**********************************************************************************************************/
/* Envoyer_histos: Envoi des histos au client GID_USERS                                                   */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Proto_envoyer_histo_hard_thread ( struct CLIENT *client )
  { struct CMD_SHOW_HISTO_HARD *rezo_histo;
    struct CMD_REQUETE_HISTO_HARD requete;
    struct HISTO_HARDDB *histo;
    struct CMD_ENREG nbr;
    struct DB *db;

    memcpy ( &requete, &client->requete, sizeof( requete ) );

    prctl(PR_SET_NAME, "W-EnvoiHISTOHARD", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Rechercher_histo_hardDB( Config.log, db, &requete ) )
     { Libere_DB_SQL( Config.log, &db );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d histo_hard", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for ( ; ; )
     { histo = Rechercher_histo_hardDB_suite( Config.log, db );
       if (!histo)
        { Envoi_client ( client, TAG_HISTO, SSTAG_SERVEUR_ADDPROGRESS_REQUETE_HISTO_HARD_FIN, NULL, 0 );
          Libere_DB_SQL( Config.log, &db );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       rezo_histo = Preparer_envoi_histo( histo );
       rezo_histo->page_id = requete.page_id;              /* Envoie à la bonne page d'historique cliente */
       g_free(histo);
       if (rezo_histo)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          /*printf("Envoi_histo_hard: num %d, type %d nom_ack %s, objet %s, libelle %s\n",
                  rezo_histo->num,rezo_histo->type,rezo_histo->nom_ack,rezo_histo->objet,rezo_histo->libelle );*/
          Envoi_client ( client, TAG_HISTO, SSTAG_SERVEUR_ADDPROGRESS_REQUETE_HISTO_HARD,
                         (gchar *)rezo_histo, sizeof(struct CMD_SHOW_HISTO_HARD) );
          g_free(rezo_histo);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
