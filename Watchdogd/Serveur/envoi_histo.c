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
/* Preparer_envoi_histo: convertit une structure HISTO en structure CMD_TYPE_HISTO                        */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_TYPE_HISTO *Preparer_envoi_histo ( struct HISTODB *histo )
  { struct CMD_TYPE_HISTO *rezo_histo;

    rezo_histo = (struct CMD_TYPE_HISTO *)g_try_malloc0( sizeof(struct CMD_TYPE_HISTO) );
    if (!rezo_histo) { return(NULL); }

    rezo_histo->id               = histo->msg.num;
    rezo_histo->type             = histo->msg.type;
    rezo_histo->num_syn          = histo->msg.num_syn;
    rezo_histo->date_create_sec  = histo->date_create_sec;
    rezo_histo->date_create_usec = histo->date_create_usec;
    rezo_histo->date_fixe        = histo->date_fixe;
    memcpy( &rezo_histo->nom_ack, histo->nom_ack, sizeof(rezo_histo->nom_ack) );
    memcpy( &rezo_histo->groupe,  histo->msg.groupe, sizeof(rezo_histo->groupe  ) );
    memcpy( &rezo_histo->page,    histo->msg.page, sizeof(rezo_histo->page      ) );
    memcpy( &rezo_histo->libelle, histo->msg.libelle, sizeof(rezo_histo->libelle) );
    return( rezo_histo );
  }
/**********************************************************************************************************/
/* Proto_acquitter_histo: le client demande l'acquittement d'un histo                                     */
/* Entrée: le client demandeur et le histo en question                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_acquitter_histo ( struct CLIENT *client, struct CMD_TYPE_HISTO *rezo_histo )
  { struct CMD_TYPE_HISTO edit_histo;
    struct HISTODB *result;
    gboolean retour;

    result = Rechercher_histoDB( rezo_histo->id );
    if (!result) return;
    if (result->date_fixe)                                                            /* Deja acquitté ?? */
     { g_free(result);
       return;
     }
    g_free(result);

    edit_histo.id = rezo_histo->id;               /* On renseigne la structure de modification de l'histo */
    time( (time_t *)&edit_histo.date_fixe );
    memcpy( &edit_histo.nom_ack, client->util->nom, sizeof(edit_histo.nom_ack) );

    retour = Modifier_histoDB ( &edit_histo );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to ack histo %d", rezo_histo->id);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_histoDB( rezo_histo->id );
           if (result) 
            { struct CMD_TYPE_HISTO *histo;
              histo = Preparer_envoi_histo ( result );
              g_free(result);
              if (!histo)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_HISTO, SSTAG_SERVEUR_ACK_HISTO,
                                   (gchar *)histo, sizeof(struct CMD_TYPE_HISTO) );
                     g_free(histo);
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate histo %d", rezo_histo->id);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_histos: Envoi des histos au client GID_USERS                                                   */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_histo_thread ( struct CLIENT *client )
  { struct CMD_TYPE_HISTO *rezo_histo;
    struct HISTODB *histo;
    struct CMD_ENREG nbr;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiHISTO", 0, 0, 0 );

    if ( ! Recuperer_histoDB( &db ) )                                            /* Si pas de histos (??) */
     { Client_mode( client, VALIDE );         /* Le client est maintenant valide aux yeux du sous-serveur */
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d histo", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { histo = Recuperer_histoDB_suite( &db );
       if (!histo)
        { Envoi_client ( client, TAG_HISTO, SSTAG_SERVEUR_ADDPROGRESS_HISTO_FIN, NULL, 0 );
          Client_mode( client, VALIDE );      /* Le client est maintenant valide aux yeux du sous-serveur */
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit( NULL );
        }

       rezo_histo = Preparer_envoi_histo( histo );
       g_free(histo);
       if (rezo_histo)
        { Envoi_client ( client, TAG_HISTO, SSTAG_SERVEUR_ADDPROGRESS_HISTO,
                          (gchar *)rezo_histo, sizeof(struct CMD_TYPE_HISTO) );
          g_free(rezo_histo);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
