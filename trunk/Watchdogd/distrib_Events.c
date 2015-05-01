/******************************************************************************************************************************/
/* Watchdogd/distrib_Events.c        Distribution des changements d'etats motif                                               */
/* Projet WatchDog version 2.0       Gestion d'habitat                                        sam. 24 janv. 2015 13:53:26 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * distrib_Events.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sebastien LEFEVRE
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

 #include <string.h>
 #include <unistd.h>
 #include <time.h>

/****************************************************** Prototypes de fonctions ***********************************************/
 #include "watchdogd.h"

 static GSList *Liste_clients_Events = NULL;

/******************************************************************************************************************************/
/* Envoyer_Event_msrv: ajoute un evenement dans la liste a traiter                                                            */
/* Entr�e : l'evenement en question                                                                                           */
/* sortie : N�ant                                                                                                             */
/******************************************************************************************************************************/
 void Envoyer_Event_msrv( struct CMD_TYPE_MSRV_EVENT *event )
  { pthread_mutex_lock( &Partage->com_msrv.synchro );
    Partage->com_msrv.liste_Event = g_slist_append ( Partage->com_msrv.liste_Event, event );
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
  }
/******************************************************************************************************************************/
/* Send_Event_String: Creation d'un nouvel evenement String                                                                   */
/* Entr�e : la source de l'evenement (thread et son type)                                                                     */
/* Sortie : n�ant                                                                                                             */
/******************************************************************************************************************************/
 void Send_Event ( gchar *instance, gchar *thread, gchar *objet, gfloat val_float )
  { struct CMD_TYPE_MSRV_EVENT *event;

    event = (struct CMD_TYPE_MSRV_EVENT *)g_try_malloc0( sizeof( struct CMD_TYPE_MSRV_EVENT ) );
    if(!event) return;
    g_snprintf( event->instance, sizeof(event->instance), "%s", instance );
    g_snprintf( event->thread, sizeof(event->thread), "%s", thread );
    g_snprintf( event->objet, sizeof(event->objet), "%s", objet );
    event->val_float = val_float;
    Envoyer_Event_msrv ( event );
  }
/******************************************************************************************************************************/
/* Abonner_distribution_EANA: Abonnement d'un thread aux diffusions d'une entree ANA                                          */
/* Entr�e : une fonction permettant de gerer l'arriv�e d'un histo                                                             */
/* Sortie : N�ant                                                                                                             */
/******************************************************************************************************************************/
 void Abonner_distribution_events ( void (*Gerer_event) (struct CMD_TYPE_MSRV_EVENT *event) )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro );
    Liste_clients_Events = g_slist_prepend( Liste_clients_Events, Gerer_event );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/******************************************************************************************************************************/
/* Desabonner_distribution_EANA: Desabonnement d'un thread aux diffusions d'une entree ANA                                    */
/* Entr�e : une fonction permettant de gerer l'arriv�e d'un histo                                                             */
/* Sortie : N�ant                                                                                                             */
/******************************************************************************************************************************/
 void Desabonner_distribution_events ( void (*Gerer_event) (struct CMD_TYPE_MSRV_EVENT *event) )
  { pthread_mutex_lock ( &Partage->com_msrv.synchro );
    Liste_clients_Events = g_slist_remove( Liste_clients_Events, Gerer_event );
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/******************************************************************************************************************************/
/* Envoyer_message_aux_abonnes: Envoi le message en parametre aux abonnes                                                     */
/* Entr�e : le message a envoyer                                                                                              */
/* Sortie : N�ant                                                                                                             */
/******************************************************************************************************************************/
 static void Envoyer_Events_aux_abonnes ( struct CMD_TYPE_MSRV_EVENT *event )
  { struct CMD_TYPE_MSRV_EVENT *dup_event;
	   GSList *liste;

    pthread_mutex_lock ( &Partage->com_msrv.synchro );
    liste = Liste_clients_Events;
    while (liste)                                                              /* Pour chacun des abonnes */
     { void (*Gerer_event) (struct CMD_TYPE_MSRV_EVENT *event);
       dup_event = (struct CMD_TYPE_MSRV_EVENT *)g_try_malloc ( sizeof(struct CMD_TYPE_MSRV_EVENT) );
       if (dup_event)
        { memcpy( dup_event, event, sizeof(struct CMD_TYPE_MSRV_EVENT) );
	      Gerer_event = liste->data;
          Gerer_event ( dup_event );
        }
       liste = liste->next;
     }
    pthread_mutex_unlock ( &Partage->com_msrv.synchro );
  }
/******************************************************************************************************************************/
/* Map_event_to_mnemo: Associe l'event en parametre aux mnemoniques D.L.S                                                     */
/* Entr�e: l'evenement � traiter                                                                                              */
/* Sortie: le mnemo en question, ou NULL si non-trouv� (ou multi trouvailles)                                                 */
/******************************************************************************************************************************/
 static struct CMD_TYPE_MNEMO_BASE *Map_event_to_mnemo( gchar *event )
  { struct CMD_TYPE_MNEMO_BASE *mnemo, *result_mnemo = NULL;
    gint nbr_result;
    struct DB *db;

    if ( ! Recuperer_mnemo_baseDB_by_command_text ( &db, event, TRUE ) )
     { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                 "Map_event_to_mnemo: Error searching Database" );
       return(NULL);
     }
    nbr_result = db->nbr_result;
          
    if ( nbr_result == 0 )                                                                  /* Si pas d'enregistrement trouv� */
     { Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                "Map_event_to_mnemo: No match found for %s", event );
     }

    while ( (mnemo = Recuperer_mnemo_baseDB_suite( &db )) != NULL)
     { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "Map_event_to_mnemo: Match found for %s: Type %d Num %d - %s",
                 event, mnemo->type, mnemo->num, mnemo->libelle );

       if (nbr_result==1) result_mnemo = mnemo;                                    /* Si un seul enregistrement, c'est le bon */
       else g_free(mnemo);                          /* Si trop, on les libere tous dans la mesure ou l'on ne sait que choisir */
     }

    return (result_mnemo);                                           /* A-t'on le seul et unique Mnemo associ� � cet event ?? */
  }
/*******************************************************************************************************************************/
/* Gerer_arrive_Event_string: Gere l'arrive d'un event de type string                                                         */
/* Entr�e : l'evenement a traiter                                                                                             */
/* sortie : N�ant                                                                                                             */
/******************************************************************************************************************************/
 static void Gerer_arrive_Event( struct CMD_TYPE_MSRV_EVENT *event )
  { struct CMD_TYPE_MNEMO_BASE *mnemo;
    gchar request[128];
    g_snprintf( request, sizeof(request), "%s:%s:%s", event->instance, event->thread, event->objet );
    mnemo = Map_event_to_mnemo ( request );
    if (!mnemo)                                      /* Si pas trouv�, cr�ation d'un mnemo 'discovered' ? */
     { struct CMD_TYPE_MNEMO_FULL new_mnemo;
       memset( &new_mnemo, 0, sizeof(new_mnemo) );
       new_mnemo.mnemo_base.type       = MNEMO_ENTREE;
       new_mnemo.mnemo_base.num        = 9999;
       new_mnemo.mnemo_base.num_plugin = 1;
       g_snprintf( new_mnemo.mnemo_base.acronyme,     sizeof(new_mnemo.mnemo_base.acronyme), "Discovered Event" );
       g_snprintf( new_mnemo.mnemo_base.libelle,      sizeof(new_mnemo.mnemo_base.libelle),  "To be filled" );
       g_snprintf( new_mnemo.mnemo_base.command_text, sizeof(new_mnemo.mnemo_base.command_text), "%s", request );

       if ( Ajouter_mnemo_fullDB ( &new_mnemo ) < 0 )                                /* Ajout auto dans la base de mnemonique */
        { Info_new( Config.log, Config.log_msrv, LOG_ERR,
                   "Gerer_arrive_Events_string: Error adding new mnemo in DB for event %s", request );
        }
       return;
     }

    switch ( mnemo->type )
     { case MNEMO_MONOSTABLE:                                                                /* Positionnement du bit interne */
            Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                     "Gerer_arrive_Event: From %s -> Mise a un du bit M%03d", request, mnemo->num );
            Envoyer_commande_dls(mnemo->num);
            break;
       case MNEMO_ENTREE:
            Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                     "Gerer_arrive_Event: From %s -> Mise a un du bit E%03d", request, mnemo->num );
            Envoyer_entree_dls(mnemo->num, 1);
            break;
       case MNEMO_ENTREE_ANA:
            Info_new( Config.log, Config.log_msrv, LOG_NOTICE,
                     "Gerer_arrive_Event: From %s -> Positionnement de EA%03d=%f", request, mnemo->num, event->val_float );
            SEA(mnemo->num, event->val_float);
            break;
       default: Info_new( Config.log, Config.log_msrv, LOG_WARNING,
                         "Gerer_arrive_Event: Cannot handle commande type %d (num=%03d) for event %s",
                          mnemo->type, mnemo->num, request );
                break;
     } 
   g_free(mnemo);                                                                          /* Lib�ration du mn�monique trait� */
  }
/******************************************************************************************************************************/
/* Gerer_arrive_Events_dls: Gestion de l'arrive des entrees ANAlogiques depuis DLS                                            */
/* Entr�e/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Gerer_arrive_Events ( void )
  { struct CMD_TYPE_MSRV_EVENT *event;

    if (!Partage->com_msrv.liste_Event) return;                                                  /* Si pas de ea, on se barre */

    pthread_mutex_lock( &Partage->com_msrv.synchro );                                  /* Ajout dans la liste de ea a traiter */
    event = Partage->com_msrv.liste_Event->data;                                              /* Recuperation du numero de ea */
    Partage->com_msrv.liste_Event = g_slist_remove ( Partage->com_msrv.liste_Event, event );
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    Envoyer_Events_aux_abonnes ( event );
    if (Config.instance_is_master == TRUE && strcmp ( event->thread, "MSRV" ) )
     { Gerer_arrive_Event ( event ); }
    g_free(event);
  }
/******************************************************************************************************************************/
/* Gerer_arrive_Axxx_dls: Gestion de l'arrive des sorties depuis DLS                                                          */
/* Entr�e/Sortie: rien                                                                                                        */
/******************************************************************************************************************************/
 void Gerer_arrive_Axxx_dls ( void )
  { struct CMD_TYPE_NUM_MNEMONIQUE critere;
    struct CMD_TYPE_MNEMO_BASE *mnemo;
    gint num, reste;

    if (!Partage->com_msrv.liste_a) return;                                                       /* Si pas de a, on se barre */

    pthread_mutex_lock( &Partage->com_msrv.synchro );                                 /* Ajout dans la liste de msg a traiter */
    num = GPOINTER_TO_INT(Partage->com_msrv.liste_a->data);                                    /* Recuperation du numero de a */
    Partage->com_msrv.liste_a = g_slist_remove ( Partage->com_msrv.liste_a, GINT_TO_POINTER(num) );
    reste = g_slist_length(Partage->com_msrv.liste_a);
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    critere.type = MNEMO_SORTIE;                                           /* Recherche du command_text associ� au mnemonique */
    critere.num  = num;
    mnemo = Rechercher_mnemo_baseDB_type_num ( &critere );
    if (!mnemo)
     { Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
                "Gerer_arrive_Axxx_dls: Mnemo not found for A%03d", num
               );
       return;
     }

    Send_Event ( Config.instance_id, "MSRV", mnemo->command_text, 1.0*A(num) );
    Info_new( Config.log, Config.log_msrv, LOG_DEBUG,
              "Gerer_arrive_Axxx_dls: Recu A(%03d)=%d. Reste a traiter %03d",
              num, A(num), reste
            );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
