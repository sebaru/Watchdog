/******************************************************************************************************************************/
/* Watchdogd/Serveur/hangle_client.c                Comportement d'un sous-hangle_client Watchdog                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        dim. 31 mars 2013 20:07:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * handle_client.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2019 - Sébastien Lefevre
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
 static void Envoyer_new_motif_au_client ( struct CLIENT *client, gint num_i )
  { struct CMD_ETAT_BIT_CTRL motif;

/**************************************** Création de la structure passée aux clients *****************************************/
    motif.num    = num_i;
    motif.etat   = Partage->i[num_i].etat;
    motif.rouge  = Partage->i[num_i].rouge;
    motif.vert   = Partage->i[num_i].vert;
    motif.bleu   = Partage->i[num_i].bleu;
    motif.cligno = Partage->i[num_i].cligno;

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "%s: Motif traite : I%03d=%d rvbc %d/%d/%d/%d", __func__,
              motif.num, motif.etat, motif.rouge, motif.vert, motif.bleu, motif.cligno );

    if ( g_slist_find( client->Liste_bit_syns, GINT_TO_POINTER(motif.num) ) )    /* Envoi uniquement si le client en a besoin */
     { Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_MOTIF,
                     (gchar *)&motif, sizeof(struct CMD_ETAT_BIT_CTRL) );
     }
  }
/******************************************************************************************************************************/
/* Envoyer_new_histo_au_client: Parcours la liste des histo et les envoi                                                      */
/* Entrée : le client a gerer                                                                                                 */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 static void Envoyer_histo_au_client ( struct CLIENT *client, struct CMD_TYPE_HISTO *histo )
  { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "%s: Histo traite : id = %08d, msg_id=%d, alive = %d, msg=%04d, libelle=%s", __func__,
              histo->id, histo->msg.id, histo->alive, histo->msg.num, histo->msg.libelle );

    Envoi_client( client, TAG_HISTO, (histo->alive ? SSTAG_SERVEUR_SHOW_HISTO : SSTAG_SERVEUR_DEL_HISTO),
                  (gchar *)histo, sizeof(struct CMD_TYPE_HISTO) );
  }
/******************************************************************************************************************************/
/* Run_handle_client: boucle principale d'un handle client Watchdog                                                           */
/* Entree: l'id du hangle_client et le pid du pere                                                                            */
/* Sortie: un code d'erreur EXIT_xxx                                                                                          */
/******************************************************************************************************************************/
 void Run_handle_client ( struct CLIENT *client )
  { static gint thread_count = 0;
    gchar nom[16];
    struct ZMQUEUE *zmq_msg;
    struct ZMQUEUE *zmq_motif;
    struct ZMQUEUE *zmq_from_bus;

    client->ssrv_id = thread_count++;
    g_snprintf(nom, sizeof(nom), "W-SSRV-%03d", client->ssrv_id );
    prctl(PR_SET_NAME, nom, 0, 0, 0 );

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE, "%s: Demarrage . . . TID = %p", __func__, pthread_self() );

    zmq_from_bus = Connect_zmq ( ZMQ_SUB, "listen-to-bus",    "inproc", ZMQUEUE_LOCAL_BUS, 0 );
    zmq_msg      = Connect_zmq ( ZMQ_SUB, "listen-to-msgs",   "inproc", ZMQUEUE_LIVE_MSGS, 0 );
    zmq_motif    = Connect_zmq ( ZMQ_SUB, "listen-to-motifs", "inproc", ZMQUEUE_LIVE_MOTIFS, 0 );

    while( Cfg_ssrv.lib->Thread_run == TRUE )                                                /* On tourne tant que necessaire */
     { usleep(10000);
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
        }
/************************************************* Envoi des chaines cadrans *************************************************/
       if (client->mode == VALIDE && client->Liste_bit_cadrans && client->date_next_send_cadran < Partage->top)
        { struct CMD_ETAT_BIT_CADRAN *cadran;
          GSList *liste_cadran;
          client->date_next_send_cadran = Partage->top + TEMPS_UPDATE_CADRAN;
          liste_cadran = client->Liste_bit_cadrans;
          while (liste_cadran)                                                             /* Pour tous les cadrans du client */
           { cadran = (struct CMD_ETAT_BIT_CADRAN *)liste_cadran->data;
             if (Tester_update_cadran(cadran))                                          /* Doit-on updater le cadran client ? */
              { Formater_cadran(cadran);                                                  /* Formatage de la chaine associée */
                Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_CADRAN,
                              (gchar *)cadran, sizeof(struct CMD_ETAT_BIT_CADRAN) );
              }
             liste_cadran = liste_cadran->next;                                                 /* On passe au cadran suivant */
           }
        }
/********************************************** Envoi des histos et des motifs ************************************************/
       if (client->mode == VALIDE)                                                /* Envoi au suppression des histo au client */
        { struct CMD_TYPE_HISTO histo;
          struct ZMQ_TARGET *event;
          gchar buffer[2048];
          void *payload;
          gint num_i;
          gint byte;

          if ( Recv_zmq ( zmq_msg, &histo, sizeof(histo) ) == sizeof(struct CMD_TYPE_HISTO) )
           { Envoyer_histo_au_client ( client, &histo ); }
          if ( Recv_zmq ( zmq_motif, &num_i, sizeof(gint) ) == sizeof(gint) )
           { Envoyer_new_motif_au_client ( client, num_i ); }

          if ( (byte=Recv_zmq_with_tag( zmq_from_bus, "ssrv", &buffer, sizeof(buffer), &event, &payload )) > 0 )
           { if (!strcmp(event->tag, "SET_SYN_VARS"))
              { struct CMD_TYPE_SYN_VARS *syn_vars;
                syn_vars = (struct CMD_TYPE_SYN_VARS *)payload;
                if ( g_slist_find( client->Liste_pass, GINT_TO_POINTER(syn_vars->syn_id) ) )/* Envoi uniquement si le client en a besoin */
                 { Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_SET_SYN_VARS,
                                 (gchar *)syn_vars, sizeof(struct CMD_TYPE_SYN_VARS) );
                 }
              }
           }
        }
/************************************************ Ecoute du client  ***********************************************************/
       if (client->mode >= WAIT_FOR_IDENT) Ecouter_client( client );

       if (Partage->top > client->pulse && client->mode == VALIDE)                                    /* Gestion du KEEPALIVE */
        { Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_PULSE, NULL, 0 );
          client->pulse = Partage->top + TEMPS_PULSE;
        }
     }
/**************************************************** Arret du handle_client **************************************************/
    Close_zmq ( zmq_msg );
    Close_zmq ( zmq_motif );
    Close_zmq ( zmq_from_bus );
    Deconnecter(client);
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE, "%s: Down . . . TID = %p", __func__, pthread_self() );
    pthread_exit( NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
