/******************************************************************************************************************************/
/* Watchdogd/Serveur/protocole_supervision.c    Gestion du protocole_supervision pour Watchdog                                */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          sam 04 avr 2009 11:17:04 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * protocole_supervision.c
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
/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/******************************************************************************************************************************/
/* Proto_Acquitter_synoptique: Acquitte le synoptique si il est en parametre                                                  */
/* Entrée: Appellé indirectement par les fonctions recursives DLS sur l'arbre en cours                                        */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Proto_Acquitter_synoptique ( void *user_data, struct PLUGIN_DLS *plugin )
  { gint syn_id = *(gint *)user_data;
    if (plugin->plugindb.syn_id == syn_id)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG, "%s: Synoptique %d -> plugin %s acquitté", __func__,
                 plugin->plugindb.syn_id, plugin->plugindb.nom );
       plugin->vars.bit_acquit = TRUE;
     }
  }
/******************************************************************************************************************************/
/* Proto_Envoyer_supervision_thread: Envoi du synoptique de supervision demandé par le client                                 */
/* Entrée: Le client destinaire                                                                                               */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void *Proto_Envoyer_supervision_thread ( struct CLIENT *client )
  { gchar titre[20];

    g_snprintf( titre, sizeof(titre), "W-SUPR-%06d", client->ssrv_id );
    prctl(PR_SET_NAME, titre, 0, 0, 0 );

    Envoyer_motif_tag ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_MOTIF,
	                                                SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_MOTIF_FIN );

    Envoyer_palette_tag ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PALETTE,
                                                   SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PALETTE_FIN );

    Envoyer_cadran_tag ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CADRAN,
                                                  SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CADRAN_FIN );

    Envoyer_passerelle_tag ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PASS,
	                                                     SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PASS_FIN );

    Envoyer_comment_tag ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_COMMENT,
                                                   SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_COMMENT_FIN );

    Envoyer_camera_sup_tag ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAMERA_SUP,
                                                      SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAMERA_SUP_FIN );

    g_free(client->syn_to_send);
    client->syn_to_send = NULL;
    Unref_client( client );                                                               /* Déréférence la structure cliente */
    pthread_exit ( NULL );
  }
/******************************************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                                                 */
/* Entrée: la connexion avec le serveur                                                                                       */
/* Sortie: Kedal                                                                                                              */
/******************************************************************************************************************************/
 void Gerer_protocole_supervision( struct CLIENT *client )
  { struct CONNEXION *connexion;
    connexion = client->connexion;

    if ( ! Tester_level_util( client->util, ACCESS_LEVEL_ALL ) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied for user..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_SUPERVISION:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
	              pthread_t tid;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;         /* Récupération du numéro du synoptique désiré */

               if ( client->syn_to_send )
                { struct CMD_GTK_MESSAGE gtkmessage;
                  g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Another Syn is sending." );
                  Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                                (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
                  return;
                }

               client->syn_to_send = Rechercher_synoptiqueDB ( syn->id );
               if ( ! client->syn_to_send )
                { struct CMD_GTK_MESSAGE gtkmessage;
                  g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Synoptique inconnu..." );
                  Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                                (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
                  return;
                }

               if ( ! Tester_level_util( client->util, client->syn_to_send->access_level ) )
                { struct CMD_GTK_MESSAGE gtkmessage;
                  g_snprintf( gtkmessage.message, sizeof(gtkmessage.message),
                             "Permission denied for this syn (Access Level %d < %d)...",
                             client->util->access_level, client->syn_to_send->access_level  );
                  Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                                (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
                  g_free(client->syn_to_send);
                  client->syn_to_send = NULL;
                  return;
                }

               Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_AFFICHE_PAGE_SUP,
                              (gchar *)client->syn_to_send, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
               Ref_client( client, "Send supervision" );
               pthread_create( &tid, NULL, (void *)Proto_Envoyer_supervision_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_CHANGE_MOTIF_UNKNOWN:
             { struct CMD_ETAT_BIT_CTRL *etat;
               etat = (struct CMD_ETAT_BIT_CTRL *)connexion->donnees;
               client->Liste_bit_syns = g_slist_remove ( client->Liste_bit_syns, GINT_TO_POINTER( etat->num ) );
             }
            break;
       case SSTAG_CLIENT_SET_SYN_VARS_UNKNOWN:
             { struct CMD_TYPE_SYN_VARS *syn_vars;
               syn_vars = (struct CMD_TYPE_SYN_VARS *)connexion->donnees;
               client->Liste_pass = g_slist_remove ( client->Liste_pass, GINT_TO_POINTER( syn_vars->syn_id ) );
             }
            break;
       case SSTAG_CLIENT_ACQ_SYN:
             { gint syn_id;
               syn_id = *(gint *)connexion->donnees;
               Dls_foreach ( &syn_id, Proto_Acquitter_synoptique, NULL );
             }
            break;
       case SSTAG_CLIENT_SET_BIT_INTERNE:
             { struct CMD_SET_BIT_INTERNE *bit;
               bit = (struct CMD_SET_BIT_INTERNE *)connexion->donnees;
               switch( bit->type )
                { case MNEMO_MONOSTABLE: Envoyer_commande_dls( bit->num );
                                         break;
                  case MNEMO_REGISTRE:   SR( bit->num, bit->valeur );
                                         break;
                }
             }
            break;
       case SSTAG_CLIENT_WANT_HORLOGES:
             { struct DB *db;
               struct CMD_ENREG nbr;
               struct CMD_TYPE_MNEMO_BASE *mnemo;
               gchar critere[128];
               gint syn_id;
               syn_id = *(gint *)connexion->donnees;                           /* Récupération du numéro du synoptique désiré */
               g_snprintf( critere, sizeof(critere), "type=%d AND syn.id=%d", MNEMO_HORLOGE, syn_id );
               if ( ! Recuperer_mnemo_baseDB_with_conditions( &db, critere, -1, -1 ) )
                { return; }

               nbr.num = db->nbr_result;
               g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d horloges", nbr.num );
               Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

               while ( (mnemo = Recuperer_mnemo_baseDB_suite( &db )) != NULL )          /* Récupération d'un mnemo dans la DB */
                { Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_HORLOGES,
                                 (gchar *)mnemo, sizeof(struct CMD_TYPE_MNEMO_BASE) );
                  g_free(mnemo);
                }
               Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_HORLOGES_FIN, NULL, 0 );
             }
            break;
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
