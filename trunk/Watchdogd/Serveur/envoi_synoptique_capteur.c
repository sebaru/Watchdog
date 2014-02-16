/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_capteur.c     Envoi des capteurs aux clients                        */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 22 mai 2005 17:35:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique_capteur.c
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
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                         */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_capteur_atelier ( struct CLIENT *client, struct CMD_TYPE_CAPTEUR *rezo_capteur )
  { gboolean retour;
    retour = Retirer_capteurDB( rezo_capteur );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_CAPTEUR_OK,
                     (gchar *)rezo_capteur, sizeof(struct CMD_TYPE_CAPTEUR) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete capteur %s", rezo_capteur->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                               */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_capteur_atelier ( struct CLIENT *client, struct CMD_TYPE_CAPTEUR *rezo_capteur )
  { struct CMD_TYPE_CAPTEUR *result;
    gint id;

    id = Ajouter_capteurDB ( rezo_capteur );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add capteur %s", rezo_capteur->libelle );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_capteurDB( id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate capteur %s", rezo_capteur->libelle );
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_CAPTEUR_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_CAPTEUR) );
              g_free(result);
            }
         }
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_capteur_atelier ( struct CLIENT *client, struct CMD_TYPE_CAPTEUR *rezo_capteur )
  { gboolean retour;
    retour = Modifier_capteurDB ( rezo_capteur );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save capteur %s", rezo_capteur->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoyer_capteur_thread_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_CAPTEUR *capteur;
    struct DB *db;
    gchar titre[20];
    g_snprintf( titre, sizeof(titre), "W-CAPT-%06d", client->ssrv_id );
    prctl(PR_SET_NAME, titre, 0, 0, 0 );

    if ( ! Recuperer_capteurDB( &db, client->syn.id ) )
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       return;
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d capteurs", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { capteur = Recuperer_capteurDB_suite( &db );
       if (!capteur)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          return;
        }

       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoyer_capteur_thread_tag: capteur %d (%s) to client %s",
                 capteur->id, capteur->libelle, client->machine );
       if (tag == TAG_SUPERVISION)
        { struct CAPTEUR *capteur_new;
          capteur_new = (struct CAPTEUR *)g_try_malloc0( sizeof(struct CAPTEUR) );
          if (capteur_new)
           { capteur_new->type = capteur->type;
             capteur_new->bit_controle = capteur->bit_controle;

             if ( ! g_list_find_custom(client->bit_init_capteur, capteur_new, (GCompareFunc) Chercher_bit_capteurs ) )
              { client->bit_init_capteur = g_list_append( client->bit_init_capteur, capteur_new );
                Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "liste des bit_init_capteur %d", capteur->id );
              }
             else g_free(capteur_new);
           }
        }

       Envoi_client ( client, tag, sstag,
                      (gchar *)capteur, sizeof(struct CMD_TYPE_CAPTEUR) );
       g_free(capteur);
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_capteur_atelier_thread ( struct CLIENT *client )
  { Envoyer_capteur_thread_tag ( client,  TAG_ATELIER, 
                                          SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAPTEUR,
                                          SSTAG_SERVEUR_ADDPROGRESS_ATELIER_CAPTEUR_FIN );
    Client_mode( client, ENVOI_CAMERA_SUP_ATELIER );
    pthread_exit(EXIT_SUCCESS);
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_capteur_supervision_thread ( struct CLIENT *client )
  { Envoyer_capteur_thread_tag ( client,  TAG_SUPERVISION, 
                                          SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAPTEUR,
                                          SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_CAPTEUR_FIN );
    Client_mode( client, ENVOI_CAMERA_SUP_SUPERVISION );
    pthread_exit(EXIT_SUCCESS);
  }
/*--------------------------------------------------------------------------------------------------------*/
