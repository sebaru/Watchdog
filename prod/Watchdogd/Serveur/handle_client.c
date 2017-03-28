/******************************************************************************************************************************/
/* Watchdogd/Serveur/hangle_client.c                Comportement d'un sous-hangle_client Watchdog                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        dim. 31 mars 2013 20:07:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * handle_client.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
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
 #include <unistd.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"

/******************************************************************************************************************************/
/* Envoyer_new_motif_au_client: Parcours la liste des motifs et les envoi                                                     */
/* Entrée : le client a gerer                                                                                                 */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Envoyer_new_motif_au_client ( struct CLIENT *client )
  { struct CMD_ETAT_BIT_CTRL *motif;

    if ( client->Liste_new_motif == NULL ) return;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    motif = (struct CMD_ETAT_BIT_CTRL *) client->Liste_new_motif->data;
    client->Liste_new_motif = g_slist_remove ( client->Liste_new_motif, motif );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
       
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Envoyer_new_motif: Motif traite : I%03d=%d rvbc %d/%d/%d/%d",
              motif->num, motif->etat, motif->rouge, motif->vert, motif->bleu, motif->cligno );

    if ( g_slist_find( client->Liste_bit_syns, GINT_TO_POINTER(motif->num) ) )
     { Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_MOTIF,
                     (gchar *)motif, sizeof(struct CMD_ETAT_BIT_CTRL) );
     }
    g_free(motif);
  }
/******************************************************************************************************************************/
/* Envoyer_new_histo_au_client: Parcours la liste des histo et les envoi                                                      */
/* Entrée : le client a gerer                                                                                                 */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Envoyer_histo_au_client ( struct CLIENT *client )
  { struct CMD_TYPE_HISTO *histo;
    
    if ( client->Liste_histo == NULL ) return;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    histo = (struct CMD_TYPE_HISTO *) client->Liste_histo->data;
    client->Liste_histo = g_slist_remove ( client->Liste_histo, histo );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
       
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Envoyer_histo_au_client: Histo traite : id = %06d, msg=%04d, libelle=%s",
              histo->id, histo->msg.num, histo->msg.libelle );

    Envoi_client( client, TAG_HISTO, (histo->alive ? SSTAG_SERVEUR_SHOW_HISTO : SSTAG_SERVEUR_DEL_HISTO),
                  (gchar *)histo, sizeof(struct CMD_TYPE_HISTO) );
    g_free(histo);
  }
/******************************************************************************************************************************/
/* Envoyer_event_au_client: Parcours la liste des events et les envoi                                                         */
/* Entrée : le client a gerer                                                                                                 */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Envoyer_event_au_client ( struct CLIENT *client )
  { struct CMD_TYPE_MSRV_EVENT *event;
    
    if ( client->Liste_events == NULL ) return;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    event = (struct CMD_TYPE_MSRV_EVENT *) client->Liste_events->data;
    client->Liste_events = g_slist_remove ( client->Liste_events, event );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
       
    if ( Tester_groupe_util( client->util, GID_SATELLITE) )                    /* Il faut etre dans le bon groupe Satellite ! */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoyer_event_au_client: Event traite %s (instance %s, thread %s)",
                 event->objet, event->instance, event->thread );

       Envoi_client( client, TAG_SATELLITE, SSTAG_SSRV_SAT_SET_INTERNAL,
                     (gchar *)event, sizeof(struct CMD_TYPE_MSRV_EVENT) );
     }
    g_free(event);
  }
/******************************************************************************************************************************/
/* Run_hangle_client boucle principale d'un sous-hangle_client Watchdog                                                       */
/* Entree: l'id du hangle_client et le pid du pere                                                                            */
/* Sortie: un code d'erreur EXIT_xxx                                                                                          */
/******************************************************************************************************************************/
 void Run_handle_client ( struct CLIENT *client )
  { static gint thread_count = 0;
    pthread_t tid;
    gchar nom[16];

    client->ssrv_id = thread_count++;
    g_snprintf(nom, sizeof(nom), "W-SSRV-%06d", client->ssrv_id );
    prctl(PR_SET_NAME, nom, 0, 0, 0 );

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
              "Run_handle_client: Demarrage . . . TID = %p", pthread_self() );


    while( Cfg_ssrv.lib->Thread_run == TRUE )                                                /* On tourne tant que necessaire */
     { usleep(1000);
       sched_yield();

       if (client->mode == DECONNECTE) break;                                                     /* Deconnection des clients */

       switch (client->mode)
        { case ENVOI_INTERNAL:
               Envoi_client( client, TAG_INTERNAL, SSTAG_INTERNAL_PAQUETSIZE,
                             NULL, client->connexion->taille_bloc );
               if (Cfg_ssrv.ssl_needed)
                { Envoi_client( client, TAG_INTERNAL, SSTAG_INTERNAL_SSLNEEDED, NULL, 0 ); 
                  if (Cfg_ssrv.ssl_peer_cert)
                   { Envoi_client( client, TAG_INTERNAL, SSTAG_INTERNAL_SSLNEEDED_WITH_CERT, NULL, 0 );  }
                  Client_mode ( client, ATTENTE_CONNEXION_SSL );
                }
               else
                { Client_mode ( client, WAIT_FOR_IDENT ); }
               Envoi_client( client, TAG_INTERNAL, SSTAG_INTERNAL_END,                                          /* Tag de fin */
                             NULL, 0 );
               break;                 
          case ATTENTE_CONNEXION_SSL:
               Connecter_ssl ( client );                                                  /* Tentative de connexion securisée */
               break;
          case ENVOI_GROUPE_FOR_UTIL:
               Client_mode( client, VALIDE );
               Ref_client( client, "Send groupe util" );
               pthread_create( &tid, NULL, (void *)Envoyer_groupes_pour_util_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_GROUPE_FOR_SYNOPTIQUE:
               Client_mode( client, VALIDE );
               Ref_client( client, "Send groupe synoptique" );
               pthread_create( &tid, NULL, (void *)Envoyer_groupes_pour_synoptique_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_GROUPE_FOR_PROPRIETE_SYNOPTIQUE:
               Client_mode( client, VALIDE );
               Ref_client( client, "Send groupe propriete syn" );
               pthread_create( &tid, NULL, (void *)Envoyer_groupes_pour_propriete_synoptique_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_ICONE_FOR_ATELIER:
               Client_mode( client, VALIDE );
               Ref_client( client, "Send icone atelier" );
               pthread_create( &tid, NULL, (void *)Envoyer_icones_pour_atelier_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_CLASSE_FOR_ATELIER:
               Client_mode( client, VALIDE );
               Ref_client( client, "Send classes atelier" );
               pthread_create( &tid, NULL, (void *)Envoyer_classes_pour_atelier_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_SYNOPTIQUE_FOR_ATELIER:
               Client_mode( client, VALIDE );
               Ref_client( client, "Send synoptique atelier" );
               pthread_create( &tid, NULL, (void *)Envoyer_synoptiques_pour_atelier_thread, client );
               pthread_detach( tid );
               break;
        }
/************************************************* Envoi des chaines cadrans *************************************************/
       if (client->mode == VALIDE && client->Liste_bit_cadrans && client->date_next_send_cadran < Partage->top)
        { struct CADRAN *cadran;
          GSList *liste_cadran;
          client->date_next_send_cadran = Partage->top + TEMPS_UPDATE_CADRAN;
          liste_cadran = client->Liste_bit_cadrans;
          while (liste_cadran)                                                           /* Pour tous les cadrans du client */
           { cadran = (struct CADRAN *)liste_cadran->data;
              if (Tester_update_cadran(cadran))                                      /* Doit-on updater le cadran client ? */
              { struct CMD_ETAT_BIT_CADRAN *etat;
                etat = Formater_cadran(cadran);                                          /* Formatage de la chaine associée */
                if (etat)                                                                                     /* envoi client */
                 { Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_CADRAN,
                                 (gchar *)etat, sizeof(struct CMD_ETAT_BIT_CADRAN) );
                   g_free(etat);                                                                      /* On libere la mémoire */
                 }
                else Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "Not enought memory to send cadran" );
              }
             liste_cadran = liste_cadran->next;                                              /* On passe au cadran suivant */
           }
        }
/********************************************** Envoi des histos et des motifs ************************************************/
       if (client->mode == VALIDE)                                                /* Envoi au suppression des histo au client */
        { Envoyer_histo_au_client (client);
          Envoyer_new_motif_au_client (client);
          Envoyer_event_au_client (client);
        }
/************************************************ Ecoute du client  ***********************************************************/
       if (client->mode >= WAIT_FOR_IDENT) Ecouter_client( client );

       if (Partage->top > client->pulse && client->mode == VALIDE)                                    /* Gestion du KEEPALIVE */
        { Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_PULSE, NULL, 0 );
          client->pulse = Partage->top + TEMPS_PULSE;
        }
     }

/**************************************************** Arret du hangle_client **************************************************/
    Deconnecter(client);
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
              "Run_handle_client: Down . . . TID = %p", pthread_self() );
    pthread_exit( NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/

