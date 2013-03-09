/**********************************************************************************************************/
/* Watchdogd/Serveur/protocole_mnemonique.c    Gestion du protocole_mnemonique pour Watchdog              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      sam 04 avr 2009 11:13:22 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_mnemonique.c
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
 #include "Reseaux.h"
 #include "watchdogd.h"
/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Gerer_protocole_mnemonique( gint Id_serveur, struct CLIENT *client )
  { struct CONNEXION *connexion;
    pthread_t tid;
    connexion = client->connexion;

    if ( ! Tester_groupe_util( client->util->id, client->util->gids, GID_DLS) )
     { struct CMD_GTK_MESSAGE gtkmessage;
       g_snprintf( gtkmessage.message, sizeof(gtkmessage.message), "Permission denied..." );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&gtkmessage, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_CLIENT_WANT_PAGE_MNEMONIQUE:
             { Envoi_client( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_CREATE_PAGE_MNEMO_OK, NULL, 0 );
               Ref_client( client );                             /* Indique que la structure est utilisée */
               pthread_create( &tid, NULL, (void *)Envoyer_mnemoniques_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_EDIT_MNEMONIQUE:
             { struct CMD_TYPE_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               Proto_editer_mnemonique( client, mnemo );
             }
            break;
       case SSTAG_CLIENT_ADD_MNEMONIQUE:
             { struct CMD_TYPE_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               Proto_ajouter_mnemonique( client, mnemo );
             }
            break;
       case SSTAG_CLIENT_DEL_MNEMONIQUE:
             { struct CMD_TYPE_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               Proto_effacer_mnemonique( client, mnemo );
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_MNEMONIQUE:
             { struct CMD_TYPE_MNEMONIQUE *mnemo;
               mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               Proto_valider_editer_mnemonique( client, mnemo );
             }
            break;
       case SSTAG_CLIENT_WANT_DLS_FOR_MNEMO:
             { pthread_create( &tid, NULL, (void *)Envoyer_plugins_dls_pour_mnemo_thread, client );
               pthread_detach( tid );
             }
            break;
       case SSTAG_CLIENT_EDIT_OPTION_BIT_INTERNE:
             { struct CMD_TYPE_MNEMONIQUE *rezo_mnemo;
               rezo_mnemo = (struct CMD_TYPE_MNEMONIQUE *)connexion->donnees;
               switch ( rezo_mnemo->type )
                { case MNEMO_ENTREE_ANA  : Proto_editer_option_entreeANA( client, rezo_mnemo );
                                           break;
                  case MNEMO_CPT_IMP     : Proto_editer_option_compteur_imp( client, rezo_mnemo );
                                           break;
                  case MNEMO_TEMPO       : Proto_editer_option_tempo( client, rezo_mnemo );
                                           break;
                  default: { struct CMD_GTK_MESSAGE erreur;
                             g_snprintf( erreur.message, sizeof(erreur.message),
                                         "No options for this object %s", rezo_mnemo->libelle);
                             Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                                           (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
                           }
                }
             }
            break;
       case SSTAG_CLIENT_VALIDE_EDIT_OPTION_BIT_INTERNE:
             { struct CMD_TYPE_OPTION_BIT_INTERNE *option;
               option = (struct CMD_TYPE_OPTION_BIT_INTERNE *)connexion->donnees;
               switch(option->type)
                { case MNEMO_ENTREE_ANA  : Proto_valider_editer_option_entreeANA( client, &option->eana );
                                           break;
                  case MNEMO_CPT_IMP     : Proto_valider_editer_option_compteur_imp( client, &option->cpt_imp );
                                           break;
                  case MNEMO_TEMPO       : Proto_valider_editer_option_tempo( client, &option->tempo );
                                           break;
                }
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
