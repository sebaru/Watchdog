/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_histo.c        Envoi de l'ihstorique à la connexion client                    */
/* Projet WatchDog version 2.0       Gestion d'habitat                       sam 13 mar 2004 18:41:14 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_histo.c
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
 #include <string.h>
 #include <pthread.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Proto_acquitter_histo: le client demande l'acquittement d'un histo                                     */
/* Entrée: le client demandeur et le histo en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_acquitter_histo ( struct CLIENT *client, struct CMD_TYPE_HISTO *rezo_histo )
  { gboolean retour;

    if (!rezo_histo) return;
    time( (time_t *)&rezo_histo->date_fixe );
    g_snprintf( rezo_histo->nom_ack, sizeof(rezo_histo->nom_ack), "%s", client->util->nom );

    retour = Modifier_histo_msgsDB ( rezo_histo );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to ack histo %d", rezo_histo->msg.num );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { Envoi_client( client, TAG_HISTO, SSTAG_SERVEUR_ACK_HISTO,
                        (gchar *)rezo_histo, sizeof(struct CMD_TYPE_HISTO) );
         }
  }
/**********************************************************************************************************/
/* Envoyer_histos: Envoi des histos au client GID_USERS                                                   */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_histo_thread ( struct CLIENT *client )
  { struct CMD_TYPE_HISTO *histo;
    struct CMD_ENREG nbr;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiHISTO", 0, 0, 0 );

    if ( ! Recuperer_histo_msgsDB_alive( &db ) )                                 /* Si pas de histos (??) */
     { Client_mode( client, VALIDE );         /* Le client est maintenant valide aux yeux du sous-serveur */
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d histo", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { histo = Recuperer_histo_msgsDB_suite( &db );
       if (!histo)
        { Envoi_client ( client, TAG_HISTO, SSTAG_SERVEUR_ADDPROGRESS_HISTO_FIN, NULL, 0 );
          Client_mode( client, VALIDE );      /* Le client est maintenant valide aux yeux du sous-serveur */
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit( NULL );
        }

       Envoi_client ( client, TAG_HISTO, SSTAG_SERVEUR_ADDPROGRESS_HISTO,
                     (gchar *)histo, sizeof(struct CMD_TYPE_HISTO) );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
