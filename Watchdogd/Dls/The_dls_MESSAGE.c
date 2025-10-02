/******************************************************************************************************************************/
/* Watchdogd/Dls/Ths_dls_MESSAGE.c        Déclaration des fonctions pour la gestion des message                               */
/* Projet Abls-Habitat version 4.5       Gestion d'habitat                                     jeu. 29 déc. 2011 14:55:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_MESSAGE.c
 * This file is part of Abls-Habitat
 *
 * Copyright (C) 1988-2025 - Sebastien LEFEVRE
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
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <stdlib.h>
 #include <unistd.h>
 #include <fcntl.h>
 #include <string.h>

 #include "watchdogd.h"

/******************************************************************************************************************************/
/* Dls_data_MESSAGE_free_one: Libère la mémoire associée à un MESSAGE un plugin                                               */
/* Entrée : le pointeur vers la structure du message                                                                          */
/******************************************************************************************************************************/
 static void Dls_data_MESSAGE_free_one ( struct DLS_MESSAGE *bit )
  { Json_node_unref ( bit->source_node );
    g_free(bit);
  }
/******************************************************************************************************************************/
/* Dls_data_MESSAGE_free_all: Libère la mémoire associée aux messages d'un plugin                                             */
/* Entrée : le pointeur vers le plugin                                                                                        */
/******************************************************************************************************************************/
 void Dls_data_MESSAGE_free_all ( struct DLS_PLUGIN *plugin )
  { if (plugin->Dls_data_MESSAGE) g_slist_free_full ( plugin->Dls_data_MESSAGE, (GDestroyNotify) Dls_data_MESSAGE_free_one );
    plugin->Dls_data_MESSAGE = NULL;
  }
/******************************************************************************************************************************/
/* Dls_data_MESSAGE_create_by_array : Création d'un MESSAGE pour le plugin                                                    */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_MESSAGE_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_MESSAGE *bit = g_try_malloc0 ( sizeof(struct DLS_MESSAGE) );
    if (!bit)
     { Info_new( __func__, Config.log_dls, LOG_ERR, "Memory error for '%s:%s'", tech_id, acronyme );
       return;
     }
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    bit->etat = FALSE;                                    /* A l'init, le message est OFF. Json_get_bool ( element, "etat" ); */
    bit->source_node = json_node_ref ( element );
    bit->last_on = 0;                                            /* A l'init, il n'y a pas de last on (en dixieme de seconde) */
    Json_node_add_string ( bit->source_node, "libelle_src",
                           Json_get_string ( bit->source_node, "libelle" ) );            /* Recopie pour conversion dynamique */
    plugin->Dls_data_MESSAGE = g_slist_prepend ( plugin->Dls_data_MESSAGE, bit );
    Info_new( __func__, Config.log_dls, LOG_INFO,
              "Create bit DLS_MESSAGE '%s:%s'=%d", bit->tech_id, bit->acronyme, bit->etat );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_MESSAGE: Recherche un MESSAGE dans les plugins DLS                                                         */
/* Entrée: le tech_id, l'acronyme                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_MESSAGE *Dls_data_lookup_MESSAGE ( gchar *tech_id, gchar *acronyme )
  { if (!(tech_id && acronyme)) return(NULL);
    GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       if (!strcasecmp( plugin->tech_id, tech_id ))
        { GSList *liste = plugin->Dls_data_MESSAGE;
          while (liste)
           { struct DLS_MESSAGE *bit = liste->data;
             if ( !strcasecmp ( bit->acronyme, acronyme ) ) return(bit);
             liste = g_slist_next(liste);
           }
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Dls_data_set_MESSAGE: Emet le message en parametre                                                                         */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_MESSAGE ( struct DLS_TO_PLUGIN *vars, struct DLS_MESSAGE *msg )
  { if (!msg) return;
    msg->new_etat = TRUE;                                                                  /* Sauvegarde de l'état du message */
    if ( msg->etat == TRUE ) return;                                                            /* Si déjà activé, on return; */
    Info_new( __func__, (Config.log_dls || (vars ? vars->debug : FALSE)), LOG_DEBUG,
              "ligne %04d: Changing DLS_MSG '%s:%s'=TRUE", (vars ? vars->num_ligne : -1), msg->tech_id, msg->acronyme );
    Partage->audit_bit_interne_per_sec++;
  }
/******************************************************************************************************************************/
/* Met à jour le message en parametre                                                                                         */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_MESSAGE_apply ( struct DLS_PLUGIN *plugin )
  { if (!plugin) return;

    GSList *liste = plugin->Dls_data_MESSAGE;
    while ( liste )
     { struct DLS_MESSAGE *msg = liste->data;

       if ( msg->etat != msg->new_etat )                                           /* si changement d'etat lors du run plugin */
        { msg->etat = msg->new_etat;
          gint typologie = Json_get_int ( msg->source_node, "typologie" );
          if ( typologie == MSG_NOTIF && msg->etat == TRUE )   /* Si message de notification apparait, on eteint le précédent */
           { struct DLS_MESSAGE_EVENT *event = g_try_malloc0( sizeof (struct DLS_MESSAGE_EVENT) );
             if (event)
              { event->etat = FALSE;                                                                  /* On eteint le message */
                event->msg  = msg;
                pthread_rwlock_wrlock( &Partage->Liste_msg_synchro );                 /* Ajout dans la liste de msg a traiter */
                Partage->Liste_msg  = g_slist_append( Partage->Liste_msg, event );
                pthread_rwlock_unlock( &Partage->Liste_msg_synchro );                 /* Ajout dans la liste de msg a traiter */
              } else Info_new( __func__, (Config.log_dls || plugin->vars.debug), LOG_ERR,
                               "Memory error for MSG'%s:%s' = 0 (etat)", msg->tech_id, msg->acronyme );
           }

          if ( !(typologie == MSG_NOTIF && msg->etat == FALSE) )                /* Un message de notification ne s'éteint pas */
           { struct DLS_MESSAGE_EVENT *event = g_try_malloc0( sizeof (struct DLS_MESSAGE_EVENT) );        /* sinon, on traite */
             if (!event)
              { Info_new( __func__, (Config.log_dls || plugin->vars.debug), LOG_ERR,
                         "Memory error for MSG'%s:%s' = %d", msg->tech_id, msg->acronyme, msg->etat );
              }
             else
              { event->etat = msg->etat;                                                /* Recopie de l'état dans l'evenement */
                event->msg  = msg;
                pthread_rwlock_wrlock( &Partage->Liste_msg_synchro );                 /* Ajout dans la liste de msg a traiter */
                Partage->Liste_msg  = g_slist_append( Partage->Liste_msg, event );
                pthread_rwlock_unlock( &Partage->Liste_msg_synchro );                 /* Ajout dans la liste de msg a traiter */
              }
           }
        }
       msg->new_etat = FALSE;                                             /* Prepare le prochain calcul avec etat initial à 0 */
       liste = g_slist_next(liste);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
