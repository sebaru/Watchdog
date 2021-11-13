/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_supervision.c    Gestion du protocole_supervision pour Watchdog            */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 avr 2009 11:17:04 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
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
/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_supervision( struct CLIENT *client )
  { struct CONNEXION *connexion;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util, GID_TOUTLEMONDE ) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied for user..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_SUPERVISION:
             { struct CMD_TYPE_SYNOPTIQUE *syn;
               syn = (struct CMD_TYPE_SYNOPTIQUE *)connexion->donnees;
               printf("Le client desire le synoptique de supervision\n" );

               struct CMD_TYPE_SYNOPTIQUE *syndb;
               syndb = Rechercher_synoptiqueDB ( syn->id );
               if ( ! syndb )
                { struct CMD_GTK_MESSAGE gtkmessage;
                  g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Synoptique inconnu..." );
                  Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                                (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
                }
               else
                { if ( ! Tester_groupe_util( client->util, syndb->access_groupe ) )
                   { struct CMD_GTK_MESSAGE gtkmessage;
                     g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied for this syn..." );
                     Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                                   (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
                   }
                  else
                   { Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_AFFICHE_PAGE_SUP,
                                   (gchar *)syndb, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
                     memcpy( &client->syn, syndb, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
                     Client_mode( client, ENVOI_MOTIF_SUPERVISION );
                   }
                  g_free(syndb);
                }
             }
            break;
       case SSTAG_CLIENT_CHANGE_MOTIF_UNKNOWN:
             { struct CMD_ETAT_BIT_CTRL *etat;
               etat = (struct CMD_ETAT_BIT_CTRL *)connexion->donnees;
               printf("Le client n'a plus besoin du bit %d\n", etat->num );
               client->Liste_bit_syns = g_slist_remove ( client->Liste_bit_syns, GINT_TO_POINTER( etat->num ) );
             }
            break;
       case SSTAG_CLIENT_ACTION_M:
             { struct CMD_ETAT_BIT_CLIC *bit;
               bit = (struct CMD_ETAT_BIT_CLIC *)connexion->donnees;
               Envoyer_commande_dls( bit->num );
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
