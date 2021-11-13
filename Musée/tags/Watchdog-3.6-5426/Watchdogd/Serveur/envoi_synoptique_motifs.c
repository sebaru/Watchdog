/******************************************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_motifs.c        Envoi des motifs à l'atelier et supervision                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                          dim 22 mai 2005 17:25:01 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * envoi_synoptique_motifs.c
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
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

/****************************************************** Prototypes de fonctions ***********************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
 extern struct SSRV_CONFIG Cfg_ssrv;
/******************************************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                                             */
/* Entrée: le client demandeur et le syn en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_effacer_motif_atelier ( struct CLIENT *client, struct CMD_TYPE_MOTIF *rezo_motif )
  { gboolean retour;

    retour = Retirer_motifDB( rezo_motif );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_DEL_MOTIF_OK,
                     (gchar *)rezo_motif, sizeof(struct CMD_TYPE_MOTIF) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete motif %s", rezo_motif->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Proto_ajouter_motif_atelier: Ajout d'un motif dans un synoptique                                                           */
/* Entrée: le client demandeur et le syn en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_ajouter_motif_atelier ( struct CLIENT *client, struct CMD_TYPE_MOTIF *rezo_motif )
  { struct CMD_TYPE_MOTIF *result;
    gint id;

    rezo_motif->access_level = ACCESS_LEVEL_ALL;                         /* Par défaut, tout le monde peut acceder a ce motif */
    rezo_motif->position_x = 120;
    rezo_motif->position_y = 50;
    id = Ajouter_motifDB ( rezo_motif );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add motif %s", rezo_motif->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_motifDB( id );
           if (!result)
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate motif %s", rezo_motif->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ATELIER_ADD_MOTIF_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_MOTIF) );
              g_free(result);
            }
         }
  }
/******************************************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                                           */
/* Entrée: le client demandeur et le syn en question                                                                          */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_valider_editer_motif_atelier ( struct CLIENT *client, struct CMD_TYPE_MOTIF *rezo_motif )
  { gboolean retour;

    retour = Modifier_motifDB ( rezo_motif );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to save motif %s", rezo_motif->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Envoyer_bit_init_motif: Envoi le status des bits motifs dans la liste en parametre en client en parametre                  */
/* Entrée: Le client et la liste de bits                                                                                      */
/* Sortie: Néant. La liste est free-ée                                                                                        */
/******************************************************************************************************************************/
 void Envoyer_bit_init_motif ( struct CLIENT *client, GSList *liste_bit_init )
  { struct CMD_ETAT_BIT_CTRL init_etat;
    while(liste_bit_init)                                         /* Envoi de la valeur d'initialisation des bits I au client */
     { struct DLS_VISUEL *visuel = liste_bit_init->data;

       if ( ! g_slist_find(client->Liste_bit_syns, visuel) )                                /* Ajout dans la liste recurrente */
        { client->Liste_bit_syns = g_slist_prepend( client->Liste_bit_syns, visuel );
          Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                   "Envoyer_bit_init_motif: ajout du bit_syn %s:%s dans la liste d'envoi recurrent",
                    visuel->tech_id, visuel->acronyme );
        }

       init_etat.etat   = visuel->mode;
       init_etat.cligno = visuel->cligno;
       g_snprintf( init_etat.color, sizeof(init_etat.color), "%s", visuel->color );
       g_snprintf( init_etat.tech_id, sizeof(init_etat.tech_id), "%s", visuel->tech_id );
       g_snprintf( init_etat.acronyme, sizeof(init_etat.acronyme), "%s", visuel->acronyme );
       Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_MOTIF,
                     (gchar *)&init_etat, sizeof(struct CMD_ETAT_BIT_CTRL) );
       liste_bit_init = g_slist_remove (liste_bit_init, visuel);
     }
  }
/******************************************************************************************************************************/
/* Envoyer_motif_tag: Envoi des syns au client selon les tags retenus                                                         */
/* Entrée: Le client destinataire et les tags de connexion                                                                    */
/* Sortie: La liste des bit d'init syn lié au synoptique                                                                      */
/******************************************************************************************************************************/
 void Envoyer_motif_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { GSList *liste_bit_init = NULL;
    struct CMD_TYPE_MOTIFS *motifs;
    struct CMD_TYPE_MOTIF *motif;
    struct CMD_ENREG nbr;
    gint max_enreg;                                                    /* Nombre maximum d'enregistrement dans un bloc reseau */
    struct DB *db;

    max_enreg = (Cfg_ssrv.taille_bloc_reseau - sizeof(struct CMD_TYPE_MOTIFS)) / sizeof(struct CMD_TYPE_MOTIF);
    motifs = (struct CMD_TYPE_MOTIFS *)g_try_malloc0( Cfg_ssrv.taille_bloc_reseau );
    if (!motifs)
     { struct CMD_GTK_MESSAGE erreur;
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                "Envoyer_motif_tag: Pb d'allocation memoire motifs" );
       g_snprintf( erreur.message, sizeof(erreur.message), "Pb d'allocation memoire" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    if ( ! Recuperer_motifDB( &db, client->syn_to_send->id ) )                                  /* Si pas de motifs a envoyer */
     { g_free(motifs);
       return;
     }

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d motifs", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    motifs->nbr_motifs = 0;                                                     /* Valeurs par defaut si pas d'enregistrement */

    do
     { motif = Recuperer_motifDB_suite( &db );                                            /* Récupération du motif dans la DB */
       if (motif)                                                                  /* Si enregegistrement, alors on le pousse */
        { memcpy ( &motifs->motif[motifs->nbr_motifs], motif, sizeof(struct CMD_TYPE_MOTIF) );
          motifs->nbr_motifs++;                              /* Nous avons 1 enregistrement de plus dans la structure d'envoi */

          if ( tag == TAG_SUPERVISION && motif->type_gestion != 0 /* TYPE_INERTE */ )
           { struct DLS_VISUEL *visuel=NULL;
             Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                      "%s: searching for visuel %s:%s", __func__, motif->tech_id, motif->acronyme );
             Dls_data_get_VISUEL ( motif->tech_id, motif->acronyme, (gpointer)&visuel );
             if ( visuel && (! g_slist_find(liste_bit_init, visuel ) ) )
              { liste_bit_init = g_slist_prepend( liste_bit_init, visuel );
                Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                         "Envoyer_motif_tag: liste des bit_init_syn adding bit %s:%s", visuel->tech_id, visuel->acronyme );
              }
           }
          g_free(motif);
        }

       if ( (motif == NULL) || motifs->nbr_motifs == max_enreg )                  /* Si depassement de tampon ou plus d'enreg */
        { Envoi_client ( client, tag, sstag, (gchar *)motifs,
                         sizeof(struct CMD_TYPE_MOTIFS) + motifs->nbr_motifs * sizeof(struct CMD_TYPE_MOTIF)
                       );
          motifs->nbr_motifs = 0;
        }
     }
    while (motif);                                                                /* Tant que l'on a des messages e envoyer ! */
    g_free(motifs);                                                                      /* Libération du tampon multi-motifs */
    Envoi_client ( client, tag, sstag_fin, NULL, 0 );
    Envoyer_bit_init_motif ( client, liste_bit_init );                                     /* Envoi des bits d'initialisation */
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
