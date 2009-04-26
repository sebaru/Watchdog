/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_palettes.c     Envoi des palettes aux clients                       */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 22 mai 2005 17:35:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique_palettes.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2009 - sebastien
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
 #include "Reseaux.h"
 #include "watchdogd.h"
/**********************************************************************************************************/
/* Preparer_envoi_palette: convertit une structure PALETTEDB en structure CMD_SHOW_PALETTE                */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_PALETTE *Preparer_envoi_palette ( struct PALETTEDB *palette )
  { struct CMD_SHOW_PALETTE *rezo_palette;

    rezo_palette = (struct CMD_SHOW_PALETTE *)g_malloc0( sizeof(struct CMD_SHOW_PALETTE) );
    if (!rezo_palette) { return(NULL); }

    rezo_palette->id = palette->id;
    rezo_palette->syn_id = palette->syn_id;
    rezo_palette->syn_cible_id = palette->syn_cible_id;
    rezo_palette->position = palette->position;                                           /* en ordonnées */
    memcpy( &rezo_palette->libelle, palette->libelle, sizeof(rezo_palette->libelle) );
    return( rezo_palette );
  }
/**********************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                         */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_palette_atelier ( struct CLIENT *client, struct CMD_ID_PALETTE *rezo_palette )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
    Info( Config.log, DEBUG_INFO, "MSRV: demande d'effacement palette" );
    retour = Retirer_paletteDB( Config.log, Db_watchdog, rezo_palette );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_PALETTE_OK,
                     (gchar *)rezo_palette, sizeof(struct CMD_ID_PALETTE) );
       Info( Config.log, DEBUG_INFO, "MSRV: effacement palette OK" );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete palette %s", rezo_palette->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Info( Config.log, DEBUG_INFO, "MSRV: effacement palette NOK" );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_comment_atelier: Ajout d'un commentaire dans un synoptique                               */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_palette_atelier ( struct CLIENT *client, struct CMD_ADD_PALETTE *rezo_palette )
  { struct PALETTEDB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    Info( Config.log, DEBUG_INFO, "MSRV: demande d'ajout palette" );
    id = Ajouter_paletteDB ( Config.log, Db_watchdog, rezo_palette );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add palette %s", rezo_palette->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Info( Config.log, DEBUG_INFO, "MSRV: ajout palette NOK" );
     }
    else { result = Rechercher_paletteDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate palette %s", rezo_palette->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
              Info( Config.log, DEBUG_INFO, "MSRV: ajout palette NOK (2)" );
            }
           else
            { struct CMD_SHOW_PALETTE *palette;
              palette = Preparer_envoi_palette( result );
              g_free(result);
              if (!palette)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             "Not enough memory" );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
                 Info( Config.log, DEBUG_INFO, "MSRV: ajout palette NOK (3)" );
               }
              else { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_PALETTE_OK,
                                   (gchar *)palette, sizeof(struct CMD_SHOW_PALETTE) );
                     g_free(palette);
                     Info( Config.log, DEBUG_INFO, "MSRV: ajout palette OK" );
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_palette_atelier ( struct CLIENT *client, struct CMD_EDIT_PALETTE *rezo_palette )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
Info( Config.log, DEBUG_INFO, "Debut valider_editer_palette_atelier" );

    retour = Modifier_paletteDB ( Config.log, Db_watchdog, rezo_palette );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save palette %s", rezo_palette->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
Info( Config.log, DEBUG_INFO, "fin valider_editer_palette_atelier" );
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoyer_palette_atelier_thread_tag ( struct CLIENT *client, gint sstag, gint sstag_fin )
  { struct CMD_SHOW_PALETTE *rezo_palette;
    struct CMD_ENREG nbr;
    struct PALETTEDB *palette;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiPalette", 0, 0, 0 );

    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { return; }                                                                 /* Si pas de histos (??) */


    if ( ! Recuperer_paletteDB( Config.log, db, client->syn.id ) )
     { Libere_DB_SQL( Config.log, &db );
       return;
     }

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d palettes", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { palette = Recuperer_paletteDB_suite( Config.log, db );
       if (!palette)
        { Libere_DB_SQL( Config.log, &db );
          Envoi_client ( client, TAG_ATELIER, sstag_fin, NULL, 0 );
          return;
        }

       rezo_palette = Preparer_envoi_palette( palette );
       g_free(palette);
       if (rezo_palette)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */
          Info_c( Config.log, DEBUG_INFO, "THR Envoyer_palette_atelier: pass LIB", rezo_palette->libelle );
          Info_n( Config.log, DEBUG_INFO, "THR Envoyer_palette_atelier: pass ID ", rezo_palette->id );
          Envoi_client ( client, TAG_ATELIER, sstag,
                         (gchar *)rezo_palette, sizeof(struct CMD_SHOW_PALETTE) );
          g_free(rezo_palette);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_palette_atelier_thread ( struct CLIENT *client )
  { Envoyer_palette_atelier_thread_tag ( client, SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE,
                                                 SSTAG_SERVEUR_ADDPROGRESS_ATELIER_PALETTE_FIN );
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit ( NULL );
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_palette_supervision_thread ( struct CLIENT *client )
  { struct CMD_SHOW_PALETTE *rezo_palette;
    struct CMD_ENREG nbr;
    struct PALETTEDB *palette;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiMotif", 0, 0, 0 );
    db = Init_DB_SQL( Config.log, Config.db_host,Config.db_database, /* Connexion en tant que user normal */
                      Config.db_username, Config.db_password, Config.db_port );
    if (!db)
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit( NULL );
     }                                                                           /* Si pas de histos (??) */

    if ( ! Recuperer_paletteDB( Config.log, db, client->num_supervision ) )
     { Client_mode( client, ENVOI_CAPTEUR_SUPERVISION );                        /* Si pas de comments ... */
       Libere_DB_SQL( Config.log, &db );
       Unref_client( client );                                        /* Déréférence la structure cliente */
       pthread_exit ( NULL );
     }

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d palettes", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { palette = Recuperer_paletteDB_suite( Config.log, db );
       if (!palette)                                                                        /* Terminé ?? */
        { Libere_DB_SQL( Config.log, &db );
          Client_mode( client, ENVOI_CAPTEUR_SUPERVISION );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PALETTE_FIN, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          pthread_exit( NULL );
        }
       rezo_palette = Preparer_envoi_palette( palette );
       g_free(palette);
       if (rezo_palette)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Info_c( Config.log, DEBUG_INFO, "THR Envoyer_palette_supervision: pass LIB", rezo_palette->libelle );
          Info_n( Config.log, DEBUG_INFO, "THR Envoyer_palette_supervision: pass ID ", rezo_palette->id );
          Envoi_client ( client, TAG_SUPERVISION, SSTAG_SERVEUR_ADDPROGRESS_SUPERVISION_PALETTE,
                         (gchar *)rezo_palette, sizeof(struct CMD_SHOW_PALETTE) );
          g_free(rezo_palette);
        }
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
