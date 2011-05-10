/**********************************************************************************************************/
/* Watchdogd/distrib.c        Distribution des messages DLS aux clients                                   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       jeu 22 jan 2009 20:01:50 CET */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * distrib_MSGxxx.c
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

 #include <glib.h>
 #include <sys/time.h>
 #include <bonobo/bonobo-i18n.h>
 #include <string.h>
 #include <unistd.h>
 #include <time.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"

/**********************************************************************************************************/
/* Gerer_message_repeat: Gestion de la répétition des messages                                            */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_message_repeat ( struct DB *Db_watchdog )
  { struct CMD_TYPE_MESSAGE *msg;
    GList *liste;
    gint num;

    pthread_mutex_lock( &Partage->com_msrv.synchro );
    liste = Partage->com_msrv.liste_msg_repeat;
    while (liste)
     { num = GPOINTER_TO_INT(liste->data);                               /* Recuperation du numero de msg */

       if (Partage->g[num].next_repeat <= Partage->top)
        { msg = Rechercher_messageDB( Config.log, Db_watchdog, num );
          if (msg->sms)     Envoyer_sms   ( msg      );
          if (msg->bit_voc) Ajouter_audio ( msg->num );
          g_free(msg);
          Partage->g[num].next_repeat = Partage->top + msg->time_repeat*600;                /* En minutes */
        }
       liste = liste->next;
     }
    pthread_mutex_unlock( &Partage->com_msrv.synchro );
  }
/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Gerer_arrive_MSGxxx_dls_on ( struct DB *Db_watchdog, gint num )
  { struct CMD_TYPE_HISTO *new_histo;
    struct CMD_TYPE_MESSAGE *msg;
    struct HISTODB histo;

    msg = Rechercher_messageDB( Config.log, Db_watchdog, num );
    if (!msg)
     { Info_n( Config.log, DEBUG_INFO,
               "MSRV: Gerer_arrive_message_dls_on: Message non trouvé", num );
       return;                                        /* On n'a pas trouvé le message, alors on s'en va ! */
     }
    else if (!msg->enable)                               /* Distribution du message aux sous serveurs */
     { Info_n( Config.log, DEBUG_INFO,
               "MSRV: Gerer_arrive_message_dls_on: Message inhibe", num );
       return;
     }

/***************************************** Envoi de SMS/AUDIO le cas echeant ******************************/
    if (msg->sms)     Envoyer_sms   ( msg      );
    if (msg->bit_voc) Ajouter_audio ( msg->num );
    if (msg->time_repeat) 
     { pthread_mutex_lock( &Partage->com_msrv.synchro );
       Partage->com_msrv.liste_msg_repeat = g_list_append ( Partage->com_msrv.liste_msg_repeat,
                                                            GINT_TO_POINTER(msg->num) );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       Partage->g[num].next_repeat = Partage->top + msg->time_repeat*600;                   /* En minutes */
     }
/***************************** Création de la structure interne de stockage *******************************/   
    new_histo = (struct CMD_TYPE_HISTO *) g_malloc0( sizeof(struct CMD_TYPE_HISTO) );
    if (new_histo)
     { struct timeval tv;
       guint i;
       gettimeofday( &tv, NULL );
       new_histo->date_create_sec  = tv.tv_sec;
       new_histo->date_create_usec = tv.tv_usec;
       new_histo->type             = msg->type;
       new_histo->num_syn          = msg->num_syn;
       new_histo->id               = msg->num;
       memcpy( &new_histo->libelle, msg->libelle, sizeof(msg->libelle) );
       memcpy( &new_histo->objet,   msg->objet,   sizeof(msg->objet  ) );

       for (i=0; i<Config.max_serveur; i++)
        { struct CMD_TYPE_HISTO *histo_ssrv;
          if (Partage->Sous_serveur[i].Thread_run == TRUE)
           { histo_ssrv = (struct CMD_TYPE_HISTO *)g_malloc0( sizeof( struct CMD_TYPE_HISTO ) );
             if (histo_ssrv)
              { memcpy ( histo_ssrv, new_histo, sizeof(struct CMD_TYPE_HISTO) );               /* Recopie */
                pthread_mutex_lock( &Partage->Sous_serveur[i].synchro );
                Partage->Sous_serveur[i].new_histo = g_list_append ( Partage->Sous_serveur[i].new_histo,
                                                                     histo_ssrv );
                pthread_mutex_unlock( &Partage->Sous_serveur[i].synchro );
              }
             else
              { Info( Config.log, DEBUG_INFO, "MSRV: Gerer_arrive_message_dls_on: not enough memory" ); }
           }
        }

       memcpy( &histo.msg, msg, sizeof(struct CMD_TYPE_MESSAGE) );                    /* Ajout dans la DB */
       memset( &histo.nom_ack, 0, sizeof(histo.nom_ack) );
       histo.date_create_sec  = new_histo->date_create_sec;
       histo.date_create_usec = new_histo->date_create_usec;
       histo.date_fixe = 0;

       Ajouter_histoDB( Config.log, Db_watchdog, &histo );                         /* Si ajout dans DB OK */
       g_free (new_histo);
     }
    else
     { Info_n( Config.log, DEBUG_INFO,
               "MSRV: Gerer_arrive_message_dls_on: Probleme d'allocation mémoire", num );
     }
    g_free( msg );                                                 /* On a plus besoin de cette reference */
  }
/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Gerer_arrive_MSGxxx_dls_off ( struct DB *Db_watchdog, gint num )
  { /* Le virer de la base histoDB */
    struct CMD_TYPE_HISTO *del_histo;
    struct HISTO_HARDDB histo_hard;
    struct HISTODB *histo;
    guint i;

    pthread_mutex_lock( &Partage->com_msrv.synchro );       /* Retrait de la liste des messages en REPEAT */
    Partage->com_msrv.liste_msg_repeat = g_list_remove ( Partage->com_msrv.liste_msg_repeat,
                                                         GINT_TO_POINTER(num) );
    pthread_mutex_unlock( &Partage->com_msrv.synchro );

    histo = Rechercher_histoDB( Config.log, Db_watchdog, num );
    if (!histo) return;

    memcpy( &histo_hard, histo, sizeof(struct HISTODB) );
    time ( &histo_hard.date_fin );
    Ajouter_histo_hardDB( Config.log, Db_watchdog, &histo_hard );
    g_free(histo);

    del_histo = (struct CMD_TYPE_HISTO *) g_malloc0( sizeof(struct CMD_TYPE_HISTO) );
    if (del_histo)
     { del_histo->id = num;

       for (i=0; i<Config.max_serveur; i++)
        { struct CMD_TYPE_HISTO *histo_ssrv;
          if (Partage->Sous_serveur[i].Thread_run == TRUE)
           { histo_ssrv = (struct CMD_TYPE_HISTO *)g_malloc0( sizeof( struct CMD_TYPE_HISTO ) );
             if (histo_ssrv)
              { memcpy ( histo_ssrv, del_histo, sizeof(struct CMD_TYPE_HISTO) );               /* Recopie */
                pthread_mutex_lock( &Partage->Sous_serveur[i].synchro );
                Partage->Sous_serveur[i].del_histo = g_list_append ( Partage->Sous_serveur[i].del_histo,
                                                                     histo_ssrv );
                pthread_mutex_unlock( &Partage->Sous_serveur[i].synchro );
              }
             else
              { Info( Config.log, DEBUG_INFO, "MSRV: Gerer_arrive_message_dls_off: not enough memory" ); }
           }
        }

       Retirer_histoDB( Config.log, Db_watchdog, del_histo );
       g_free (del_histo);
     }
    else
     { Info_n( Config.log, DEBUG_INFO,
               "MSRV: Gerer_arrive_message_dls_off: Probleme d'allocation mémoire", num );
     }
  }
/**********************************************************************************************************/
/* Gerer_arrive_message_dls: Gestion de l'arrive des messages depuis DLS                                  */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Gerer_arrive_MSGxxx_dls ( struct DB *Db_watchdog )
  { gint num;

    if (Partage->com_msrv.liste_msg_off)                        /* Priorité à la disparition des messages */
     { pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = GPOINTER_TO_INT(Partage->com_msrv.liste_msg_off->data);     /* Recuperation du numero de msg */
       Partage->com_msrv.liste_msg_off = g_list_remove ( Partage->com_msrv.liste_msg_off,
                                                         GINT_TO_POINTER(num) );
       Info_n( Config.log, DEBUG_DLS, "MSRV: Gerer_arrive_message_dls: Reste a traiter OFF",
                                      g_list_length(Partage->com_msrv.liste_msg_off) );
       Info_n( Config.log, DEBUG_DLS, "MSRV: Gerer_arrive_message_dls: Disparition msg", num );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       Gerer_arrive_MSGxxx_dls_off( Db_watchdog, num );
     }
    else if (Partage->com_msrv.liste_msg_on)
     { pthread_mutex_lock( &Partage->com_msrv.synchro );          /* Ajout dans la liste de msg a traiter */
       num = GPOINTER_TO_INT(Partage->com_msrv.liste_msg_on->data);      /* Recuperation du numero de msg */
       Partage->com_msrv.liste_msg_on = g_list_remove ( Partage->com_msrv.liste_msg_on,
                                                            GINT_TO_POINTER(num) );
       Info_n( Config.log, DEBUG_DLS, "MSRV: Gerer_arrive_message_dls: Reste a traiter ON",
                                      g_list_length(Partage->com_msrv.liste_msg_on) );
       Info_n( Config.log, DEBUG_DLS, "MSRV: Gerer_arrive_message_dls: Apparition msg", num );
       pthread_mutex_unlock( &Partage->com_msrv.synchro );
       Gerer_arrive_MSGxxx_dls_on( Db_watchdog, num );
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
