/******************************************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_motifs.c        Envoi des motifs à l'atelier et supervision                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          dim 22 mai 2005 17:25:01 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * envoi_synoptique_motifs.c
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
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

/****************************************************** Prototypes de fonctions ***********************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
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
     { guint bit_controle;
       bit_controle = GPOINTER_TO_INT( liste_bit_init->data );

       if (bit_controle<NBR_BIT_CONTROLE)                                                          /* Verification des bornes */
        { if ( ! g_slist_find(client->Liste_bit_syns, GINT_TO_POINTER(bit_controle) ) )     /* Ajout dans la liste recurrente */
           { client->Liste_bit_syns = g_slist_prepend( client->Liste_bit_syns, GINT_TO_POINTER(bit_controle) );
             Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                      "Envoyer_bit_init_motif: ajout du bit_syn %03d dans la liste d'envoi recurrent",
                       bit_controle );
           }

          init_etat.num    = bit_controle;                          /* Initialisation de la structure avant envoi au client ! */
          init_etat.etat   = Partage->i[ bit_controle ].etat;
          init_etat.rouge  = Partage->i[ bit_controle ].rouge;
          init_etat.vert   = Partage->i[ bit_controle ].vert;
          init_etat.bleu   = Partage->i[ bit_controle ].bleu;
          init_etat.cligno = Partage->i[ bit_controle ].cligno;
          Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_MOTIF,
                        (gchar *)&init_etat, sizeof(struct CMD_ETAT_BIT_CTRL) );
         }
      liste_bit_init = g_slist_remove (liste_bit_init, liste_bit_init->data);
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
          g_free(motif);
        }

       if ( tag == TAG_SUPERVISION && motif && (! g_slist_find(liste_bit_init, GINT_TO_POINTER(motif->bit_controle) ) ) &&
            motif->type_gestion != 0 /* TYPE_INERTE */
          )
        { liste_bit_init = g_slist_prepend( liste_bit_init, GINT_TO_POINTER(motif->bit_controle) );
          Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                   "Envoyer_motif_tag: liste des bit_init_syn adding bit i %d", motif->bit_controle );
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
