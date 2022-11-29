/******************************************************************************************************************************/
/* Watchdogd/Dls/Ths_dls_MESSAGE.c        Déclaration des fonctions pour la gestion des message                               */
/* Projet WatchDog version 3.0       Gestion d'habitat                                         jeu. 29 déc. 2011 14:55:42 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * The_dls_MESSAGE.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010-2020 - Sebastien Lefevre
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
/* Dls_data_MESSAGE_create_by_array : Création d'un MESSAGE pour le plugin                                                    */
/* Entrée : l'acronyme, le tech_id et le pointeur de raccourci                                                                */
/******************************************************************************************************************************/
 void Dls_data_MESSAGE_create_by_array ( JsonArray *array, guint index, JsonNode *element, gpointer user_data )
  { struct DLS_PLUGIN *plugin = user_data;
    gchar *tech_id  = Json_get_string ( element, "tech_id" );
    gchar *acronyme = Json_get_string ( element, "acronyme" );
    struct DLS_MESSAGE *bit = g_try_malloc0 ( sizeof(struct DLS_MESSAGE) );
    if (!bit)
     { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR, "%s: Memory error for '%s:%s'", __func__, tech_id, acronyme );
       return;
     }
    g_snprintf( bit->acronyme, sizeof(bit->acronyme), "%s", acronyme );
    g_snprintf( bit->tech_id,  sizeof(bit->tech_id),  "%s", tech_id );
    bit->etat = Json_get_bool ( element, "etat" );
    plugin->Dls_data_MESSAGE = g_slist_prepend ( plugin->Dls_data_MESSAGE, bit );
  }
/******************************************************************************************************************************/
/* Dls_data_lookup_MESSAGE: Recherche un MESSAGE dans les plugins DLS                                                         */
/* Entrée: le tech_id, l'acronyme                                                                                             */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 struct DLS_MESSAGE *Dls_data_lookup_MESSAGE ( gchar *tech_id, gchar *acronyme )
  { GSList *plugins = Partage->com_dls.Dls_plugins;
    while (plugins)
     { struct DLS_PLUGIN *plugin = plugins->data;
       GSList *liste = plugin->Dls_data_MESSAGE;
       while (liste)
        { struct DLS_MESSAGE *bit = liste->data;
          if ( !strcasecmp ( bit->acronyme, acronyme ) && !strcasecmp( bit->tech_id, tech_id ) ) return(bit);
          liste = g_slist_next(liste);
        }
       plugins = g_slist_next(plugins);
     }
    return(NULL);
  }
/******************************************************************************************************************************/
/* Met à jour le message en parametre                                                                                         */
/* Sortie : Néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_data_set_MESSAGE ( struct DLS_TO_PLUGIN *vars, struct DLS_MESSAGE *msg, gboolean update, gboolean etat )
  { if (!msg) return;
    if ( update )
     { if (etat == FALSE) { msg->etat_update = FALSE; }
       else if (msg->etat == TRUE && msg->etat_update == FALSE)
        { struct DLS_MESSAGE_EVENT *event;
          msg->etat_update = TRUE;
          event = (struct DLS_MESSAGE_EVENT *)g_try_malloc0( sizeof (struct DLS_MESSAGE_EVENT) );
          if (!event)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
                      "%s: malloc Event failed. Memory error for Updating DLS_MSG'%s:%s'", __func__, msg->tech_id, msg->acronyme );
           }
          else
           { event->etat = FALSE;                                                       /* Recopie de l'état dans l'evenement */
             event->msg  = msg;
             pthread_mutex_lock( &Partage->com_msrv.synchro );                        /* Ajout dans la liste de msg a traiter */
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
             pthread_mutex_unlock( &Partage->com_msrv.synchro );
           }
          event = (struct DLS_MESSAGE_EVENT *)g_try_malloc0( sizeof (struct DLS_MESSAGE_EVENT) );
          if (!event)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
                      "%s: malloc Event failed. Memory error for Updating DLS_MSG'%s:%s'", __func__, msg->tech_id, msg->acronyme );
           }
          else
           { event->etat = TRUE;                                                        /* Recopie de l'état dans l'evenement */
             event->msg  = msg;
             pthread_mutex_lock( &Partage->com_msrv.synchro );                        /* Ajout dans la liste de msg a traiter */
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
             pthread_mutex_unlock( &Partage->com_msrv.synchro );
           }
        }
     }
    else if ( msg->etat != etat )
     { msg->etat = etat;
       if (etat) msg->etat_update = TRUE;

       if ( msg->last_change + 10 <= Partage->top ) { msg->changes = 0; }            /* Si pas de change depuis plus de 1 sec */

       if ( msg->changes > 5 && !(Partage->top % 50) )              /* Si persistence d'anomalie on prévient toutes les 5 sec */
        { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_NOTICE, "%s: last_change trop tot for DLS_MSG '%s:%s' !", __func__,
                    msg->tech_id, msg->acronyme );
        }
       else
        { struct DLS_MESSAGE_EVENT *event;
          event = (struct DLS_MESSAGE_EVENT *)g_try_malloc0( sizeof (struct DLS_MESSAGE_EVENT) );
          if (!event)
           { Info_new( Config.log, Partage->com_dls.Thread_debug, LOG_ERR,
                      "%s: malloc Event failed. Memory error for MSG'%s:%s'", __func__, msg->tech_id, msg->acronyme );
           }
          else
           { event->etat = etat;                                                        /* Recopie de l'état dans l'evenement */
             event->msg  = msg;
             pthread_mutex_lock( &Partage->com_msrv.synchro );                        /* Ajout dans la liste de msg a traiter */
             Partage->com_msrv.liste_msg  = g_slist_append( Partage->com_msrv.liste_msg, event );
             pthread_mutex_unlock( &Partage->com_msrv.synchro );
           }

          Info_new( Config.log, (Partage->com_dls.Thread_debug || (vars ? vars->debug : FALSE)), LOG_DEBUG,
                    "%s: ligne %04d: Changing DLS_MSG '%s:%s'=%d", __func__,
                    (vars ? vars->num_ligne : -1), msg->tech_id, msg->acronyme, msg->etat );
          msg->changes++;
          msg->last_change = Partage->top;
          Partage->audit_bit_interne_per_sec++;
        }
     }
  }
/******************************************************************************************************************************/
/* Dls_MESSAGE_to_json : Formate un bit au format JSON                                                                        */
/* Entrées: le JsonNode et le bit                                                                                             */
/* Sortie : néant                                                                                                             */
/******************************************************************************************************************************/
 void Dls_MESSAGE_to_json ( JsonNode *element, struct DLS_MESSAGE *bit )
  { Json_node_add_string ( element, "tech_id",  bit->tech_id );
    Json_node_add_string ( element, "acronyme", bit->acronyme );
    Json_node_add_bool   ( element, "etat",     bit->etat );
  }
/******************************************************************************************************************************/
/* Dls_all_MESSAGE_to_json: Transforme tous les bits message d'un plugin en JSON                                              */
/* Entrée: l'array destination et le plugin                                                                                   */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 void Dls_all_MESSAGE_to_json ( gpointer array, struct DLS_PLUGIN *plugin )
  { JsonArray *RootArray = array;

    GSList *liste = plugin->Dls_data_DI;
    while ( liste )
     { struct DLS_DI *bit = liste->data;
       JsonNode *element = Json_node_create();
       Dls_DI_to_json ( element, bit );
       Json_array_add_element ( RootArray, element );
       liste = g_slist_next(liste);
     }
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
