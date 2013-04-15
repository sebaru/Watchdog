/**********************************************************************************************************/
/* Watchdogd/Serveur/hangle_client.c                Comportement d'un sous-hangle_client Watchdog         */
/* Projet WatchDog version 2.0       Gestion d'habitat                    dim. 31 mars 2013 20:07:37 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"

 struct CLIENT *Client;

/**********************************************************************************************************/
/* Envoyer_message: Envoi des message en attente dans la file de message au client                        */
/* Entrée : néant                                                                                         */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static void Envoyer_new_histo_aux_clients ( void )
  { struct CMD_TYPE_HISTO *histo;
    
    if ( Client->Liste_new_histo == NULL ) return;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    histo = (struct CMD_TYPE_HISTO *) Client->Liste_new_histo->data;
    Client->Liste_new_histo = g_slist_remove ( Client->Liste_new_histo, histo );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
       
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Envoyer_new_histo: Histo traite : msg=%d, libelle=%s", histo->id, histo->libelle );

    Envoi_client( Client, TAG_HISTO, SSTAG_SERVEUR_SHOW_HISTO,
                  (gchar *)&histo, sizeof(struct CMD_TYPE_HISTO) );
    g_free(histo);
  }
/**********************************************************************************************************/
/* Envoyer_message: Envoi des message en attente dans la file de message au client                        */
/* Entrée : néant                                                                                         */
/* Sortie : néant                                                                                         */
/**********************************************************************************************************/
 static void Envoyer_del_histo_aux_clients ( void )
  { struct CMD_TYPE_HISTO *histo;
    
    if ( Client->Liste_del_histo == NULL ) return;

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    histo = (struct CMD_TYPE_HISTO *) Client->Liste_del_histo->data;
    Client->Liste_del_histo = g_slist_remove ( Client->Liste_del_histo, histo );
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Envoyer_del_histo: Histo traite : msg=%d, libelle=%s", histo->id, histo->libelle );

    Envoi_client( Client, TAG_HISTO, SSTAG_SERVEUR_DEL_HISTO,
                 (gchar *)&histo, sizeof(struct CMD_TYPE_HISTO) );
    g_free(histo);
  }
/**********************************************************************************************************/
/* Run_hangle_client boucle principale d'un sous-hangle_client Watchdog                                   */
/* Entree: l'id du hangle_client et le pid du pere                                                        */
/* Sortie: un code d'erreur EXIT_xxx                                                                      */
/**********************************************************************************************************/
 void Run_handle_client ( struct CLIENT *client )
  { static gint thread_count = 0;
    time_t version_serveur;
    pthread_t tid;
    gchar nom[16];

    Client = client;                     /* Sauvegarde pour assurer les echanges de messages et de motifs */
    client->ssrv_id = thread_count++;
    g_snprintf(nom, sizeof(nom), "W-SSRV-%06d", client->ssrv_id );
    prctl(PR_SET_NAME, nom, 0, 0, 0 );

    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
              "Run_handle_client: Demarrage . . . TID = %d", pthread_self() );

    while( Cfg_ssrv.lib->Thread_run == TRUE )                            /* On tourne tant que necessaire */
     { usleep(100000);
       sched_yield();

       if (client->mode == DECONNECTE)                                        /* Deconnection des clients */
        { Unref_client( client );
          break;
        }

       switch (client->mode)
        { case ENVOI_INTERNAL:
               Envoi_client( client, TAG_INTERNAL, SSTAG_INTERNAL_PAQUETSIZE,
                             NULL, client->connexion->taille_bloc );
               if (Cfg_ssrv.ssl_crypt)
                { Envoi_client( client, TAG_INTERNAL, SSTAG_INTERNAL_SSLNEEDED, NULL, 0 ); 
                  Client_mode ( client, ATTENTE_CONNEXION_SSL );
                }
               else
                { Client_mode ( client, ATTENTE_IDENT ); }
               Envoi_client( client, TAG_INTERNAL, SSTAG_INTERNAL_END,                /* Tag de fin */
                             NULL, 0 );
               break;                 
          case ATTENTE_CONNEXION_SSL:
               Connecter_ssl ( client );                        /* Tentative de connexion securisée */
               break;
          case ENVOI_AUTORISATION :
                { gint new_mode;
                  new_mode = Tester_autorisation( client );
                  if (new_mode == ENVOI_DONNEES)/* Optimisation si pas necessaire */
                   { version_serveur = Lire_version_donnees( Config.log );
                     if ( version_serveur > client->ident.version_d )
                      {  gint taille;
                         taille = 0;
                         taille += Ajouter_repertoire_liste( client, "Gif", client->ident.version_d );
                         client->transfert.taille = taille;
                      }
                   }
                  Client_mode ( client, new_mode );
                }
               break;
          case ENVOI_DONNEES      :
               if(Envoyer_gif( client )) Client_mode (client, ENVOI_HISTO);
               break;
          case ENVOI_HISTO        :
               Client_mode( client, VALIDE_NON_ROOT );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_histo_thread, client );
               pthread_detach( tid );
               break;
          case VALIDE_NON_ROOT    : /*Client_mode(client, VALIDE); */           /* Etat transitoire */
               break;
          case ENVOI_GROUPE_FOR_UTIL:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_groupes_pour_util_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_GROUPE_FOR_SYNOPTIQUE:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_groupes_pour_synoptique_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_GROUPE_FOR_PROPRIETE_SYNOPTIQUE:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_groupes_pour_propriete_synoptique_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_SOURCE_DLS   :
               if(Envoyer_source_dls( client )) Client_mode(client, VALIDE);
               break;
          case ENVOI_MNEMONIQUE_FOR_COURBE:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_mnemoniques_for_courbe_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_MNEMONIQUE_FOR_HISTO_COURBE:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_mnemoniques_for_histo_courbe_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_MOTIF_ATELIER:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_motif_atelier_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_COMMENT_ATELIER:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_comment_atelier_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_PASSERELLE_ATELIER:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_passerelle_atelier_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_CAPTEUR_ATELIER:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_capteur_atelier_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_CAMERA_SUP_ATELIER:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_camera_sup_atelier_thread, client );
               pthread_detach( tid );
               break;

          case ENVOI_MOTIF_SUPERVISION:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_motif_supervision_thread, client );
               pthread_detach( tid );
               break;   
          case ENVOI_COMMENT_SUPERVISION:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_comment_supervision_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_PASSERELLE_SUPERVISION:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_passerelle_supervision_thread, client );
               pthread_detach( tid );
               break;   
          case ENVOI_PALETTE_SUPERVISION:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_palette_supervision_thread, client );
               pthread_detach( tid );
               break;   
          case ENVOI_CAPTEUR_SUPERVISION:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_capteur_supervision_thread, client );
               pthread_detach( tid );
               break;   
          case ENVOI_CAMERA_SUP_SUPERVISION:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_camera_sup_supervision_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_IXXX_SUPERVISION :
               Client_mode( client, VALIDE );
               Ref_client( client );  /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_bit_init_supervision_thread, client );
               pthread_detach( tid );
               break;   
          case ENVOI_ICONE_FOR_ATELIER:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_icones_pour_atelier_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_CLASSE_FOR_ATELIER:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_classes_pour_atelier_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_SYNOPTIQUE_FOR_ATELIER:
               Client_mode( client, VALIDE );
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_synoptiques_pour_atelier_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_SYNOPTIQUE_FOR_ATELIER_PALETTE:
               Client_mode( client, VALIDE );                             /* Si pas de comments ... */
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_synoptiques_pour_atelier_palette_thread, client );
               pthread_detach( tid );
               break;
          case ENVOI_PALETTE_FOR_ATELIER_PALETTE:
               Client_mode( client, VALIDE );                             /* Si pas de comments ... */
               Ref_client( client );                       /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_palette_atelier_thread, client );
               pthread_detach( tid );
               break;
              }
/****************************************** Envoi des chaines capteurs ************************************/
       if (client->mode == VALIDE && client->bit_capteurs && client->date_next_send_capteur < Partage->top)
        { struct CAPTEUR *capteur;
          GList *liste_capteur;
          client->date_next_send_capteur = Partage->top + TEMPS_UPDATE_CAPTEUR;
          liste_capteur = client->bit_capteurs;
          while (liste_capteur)                                 /* Pour tous les capteurs du client */
           { capteur = (struct CAPTEUR *)liste_capteur->data;
              if (Tester_update_capteur(capteur))             /* Doit-on updater le capteur client ? */
              { struct CMD_ETAT_BIT_CAPTEUR *etat;
                etat = Formater_capteur(capteur);                /* Formatage de la chaine associée */
                if (etat)                                                           /* envoi client */
                 { Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_CAPTEUR,
                                 (gchar *)etat, sizeof(struct CMD_ETAT_BIT_CAPTEUR) );
                   g_free(etat);                                            /* On libere la mémoire */
                 }
                else Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "Not enought memory envoi capteur" );
              }
             liste_capteur = liste_capteur->next;                    /* On passe au capteur suivant */
           }
        }
/****************************************** Envoi des courbes analogiques *********************************/
       if (client->mode == VALIDE && client->courbes)
        { static gint update;
          struct CMD_TYPE_COURBE *courbe;
          GList *liste_courbe;
          if (update < Partage->top)
           { struct CMD_APPEND_COURBE envoi_courbe;
             update = Partage->top + COURBE_TEMPS_TOP*10;/* Refresh toutes les 5 secondes au client */
             envoi_courbe.date = time(NULL);
             liste_courbe = client->courbes;
             while (liste_courbe)
              { courbe = (struct CMD_TYPE_COURBE *)liste_courbe->data;

                envoi_courbe.slot_id = courbe->slot_id;
                envoi_courbe.type    = courbe->type;
                              
                switch (courbe->type)
                 { case MNEMO_SORTIE:
                        envoi_courbe.val_avant_ech = 1.0*A(courbe->num);
                        Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                                      (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                        break;
                   case MNEMO_ENTREE:
                        envoi_courbe.val_avant_ech = 1.0*E(courbe->num);
                        Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                                      (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                        break;
                   case MNEMO_ENTREE_ANA:
                        envoi_courbe.val_avant_ech = Partage->ea[courbe->num].val_avant_ech;
                        Envoi_client( client, TAG_COURBE, SSTAG_SERVEUR_APPEND_COURBE,
                                      (gchar *)&envoi_courbe, sizeof(struct CMD_APPEND_COURBE) );
                        break;
                   default: printf("type courbe inconnu\n");
                 }
                liste_courbe = liste_courbe->next;
              }
           }
        }


       if (client->mode >= ATTENTE_IDENT) Ecouter_client( client );

       if (Partage->top > client->pulse && client->mode == VALIDE)                /* Gestion du KEEPALIVE */
        { Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_PULSE, NULL, 0 );
          SQL_ping ( Config.log, client->Db_watchdog );/* A remplacer plus tard par des connexions furtives */
          client->pulse = Partage->top + TEMPS_PULSE;
        }
     }

#ifdef bouh
/****************************************** Ecoute des messages histo  ************************************/


       if (Cfg_ssrv.new_motif)
        { GList *liste_clients;
          struct CLIENT *client;
          struct CMD_ETAT_BIT_CTRL *motif;

          pthread_mutex_lock( &Cfg_ssrv.synchro );
          motif = (struct CMD_ETAT_BIT_CTRL *) Cfg_ssrv.new_motif->data;
          Cfg_ssrv.new_motif = g_list_remove ( Cfg_ssrv.new_motif, motif );
          pthread_mutex_unlock( &Cfg_ssrv.synchro );

          liste_clients = Cfg_ssrv.Clients;
          while (liste_clients)
           { client = (struct CLIENT *)liste_clients->data;
             if ( g_list_find( client->bit_syns, GINT_TO_POINTER(motif->num) ) )
              { Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_MOTIF,
                              (gchar *)motif, sizeof(struct CMD_ETAT_BIT_CTRL) );
              }
             liste_clients = liste_clients->next;
           }
          g_free(motif);
        }

#endif
/********************************************* Arret du hangle_client *************************************/

    if (Cfg_ssrv.lib->Thread_run == FALSE)            /* Arret demandé par MSRV. Nous prevenons le client */
     { Deconnecter(client); }

    if (client->Liste_new_histo)                                         /* Si la liste est encore pleine */
     { g_slist_foreach( client->Liste_new_histo, (GFunc) g_free, NULL );
       g_slist_free ( client->Liste_new_histo );
     }

    if (client->Liste_del_histo)                                         /* Si la liste est encore pleine */
     { g_slist_foreach( client->Liste_del_histo, (GFunc) g_free, NULL );
       g_slist_free ( client->Liste_del_histo );
     }

#ifdef bouh
    if (Cfg_ssrv.new_motif)
     { g_list_foreach( Cfg_ssrv.new_motif, (GFunc) g_free, NULL );
       g_list_free ( Cfg_ssrv.new_motif );
       Cfg_ssrv.new_motif = NULL;
     }
#endif
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_NOTICE,
              "Run_handle_client: Down . . . TID = %d", pthread_self() );
    pthread_exit( NULL );
  }
/*--------------------------------------------------------------------------------------------------------*/

