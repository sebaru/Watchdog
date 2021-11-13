/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_passerelles.c        Envoi des passerelles aux clients              */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 22 mai 2005 17:45:31 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique_passerelles.c
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
 void Proto_effacer_passerelle_atelier ( struct CLIENT *client, struct CMD_TYPE_PASSERELLE *rezo_pass )
  { gboolean retour;
    retour = Retirer_passerelleDB( rezo_pass );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_PASS_OK,
                     (gchar *)rezo_pass, sizeof(struct CMD_TYPE_PASSERELLE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete pass %s", rezo_pass->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                               */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_passerelle_atelier ( struct CLIENT *client, struct CMD_TYPE_PASSERELLE *rezo_pass )
  { struct CMD_TYPE_PASSERELLE *result;
    gint id;
    id = Ajouter_passerelleDB ( rezo_pass );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add pass %s", rezo_pass->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_passerelleDB( id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate pass %s", rezo_pass->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_PASS_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_PASSERELLE) );
              g_free(result);
            }
         }
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_passerelle_atelier ( struct CLIENT *client, struct CMD_TYPE_PASSERELLE *rezo_pass )
  { gboolean retour;
    retour = Modifier_passerelleDB ( rezo_pass );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save pass %s", rezo_pass->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_passerelle_atelier_thread ( struct CLIENT *client )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_PASSERELLE *pass;
    struct DB *db;
    gchar titre[20];
    g_snprintf( titre, sizeof(titre), "W-PASS-%06d", client->ssrv_id );
    prctl(PR_SET_NAME, titre, 0, 0, 0 );

    if ( ! Recuperer_passerelleDB( &db, client->syn.id ) )
     { Client_mode( client, ENVOI_CAPTEUR_ATELIER );                            /* Si pas de comments ... */
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d gateways", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                       (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { pass = Recuperer_passerelleDB_suite( &db );
       if (!pass)
        { Client_mode( client, ENVOI_CAPTEUR_ATELIER );               /* Si pas de comments ... */
          Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit ( NULL );
        }

       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoyer_passerelle_atelier: pass %d (%s) to client %s",
                 pass->id, pass->libelle, client->machine );
       Envoi_client ( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PASS,
                      (gchar *)pass, sizeof(struct CMD_TYPE_PASSERELLE) );
       g_free(pass);
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_passerelle_supervision_thread ( struct CLIENT *client )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_PASSERELLE *pass;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiPass", 0, 0, 0 );

    if ( ! Recuperer_passerelleDB( &db, client->syn.id ) )
     { Client_mode( client, ENVOI_PALETTE_SUPERVISION );                        /* Si pas de comments ... */
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d gateways", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                       (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { pass = Recuperer_passerelleDB_suite( &db );
       if (!pass)                                                                           /* Terminé ?? */
        { Client_mode( client, ENVOI_PALETTE_SUPERVISION );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PASS_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit( NULL );
        }

       if ( ! g_list_find(client->bit_init_syn, GINT_TO_POINTER(pass->bit_controle_1) )
          )
        { client->bit_init_syn = g_list_append( client->bit_init_syn, GINT_TO_POINTER(pass->bit_controle_1) );
          Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                   "liste des bit_init_syn pass %d", pass->bit_controle_1 );
        }

       if ( ! g_list_find(client->bit_init_syn, GINT_TO_POINTER(pass->bit_controle_2) )
          )
        { client->bit_init_syn = g_list_append( client->bit_init_syn, GINT_TO_POINTER(pass->bit_controle_2) );
          Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                   "liste des bit_init_syn pass %d", pass->bit_controle_2 );
        }

       if ( ! g_list_find(client->bit_init_syn, GINT_TO_POINTER(pass->bit_controle_3) )
          )
        { client->bit_init_syn = g_list_append( client->bit_init_syn, GINT_TO_POINTER(pass->bit_controle_3) );
          Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                   "liste des bit_init_syn pass %d", pass->bit_controle_3 );
        }

       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoyer_passerelle_supervision: pass %d (%s) to client %s",
                 pass->id, pass->libelle, client->machine );
       Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PASS,
                      (gchar *)pass, sizeof(struct CMD_TYPE_PASSERELLE) );
       g_free(pass);
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
