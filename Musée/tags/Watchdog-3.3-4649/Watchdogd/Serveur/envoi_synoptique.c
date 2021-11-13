/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique.c        Configuration des synoptiques de Watchdog v2.0             */
/* Projet WatchDog version 3.0       Gestion d'habitat                      dim 22 mai 2005 18:03:34 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique.c
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
 #include <sys/prctl.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
 extern struct SSRV_CONFIG Cfg_ssrv;
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_synoptique ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn )
  { struct CMD_TYPE_SYNOPTIQUE *syn;

    syn = Rechercher_synoptiqueDB( rezo_syn->id );

    if (syn)
     { Envoi_client( client, TAG_SYNOPTIQUE, SSTAG_SERVEUR_EDIT_SYNOPTIQUE_OK,
                  (gchar *)syn, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
       g_free(syn);                                                                 /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate synoptique %s", rezo_syn->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_syn: Le client valide l'edition d'un syn                                          */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_synoptique ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn )
  { struct CMD_TYPE_SYNOPTIQUE *result;
    gboolean retour;

    retour = Modifier_synoptiqueDB ( rezo_syn );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to edit synoptique %s", rezo_syn->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_synoptiqueDB( rezo_syn->id );
           if (result)
            { Envoi_client( client, TAG_SYNOPTIQUE, SSTAG_SERVEUR_VALIDE_EDIT_SYNOPTIQUE_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
              g_free(result);
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate synoptique %s", rezo_syn->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
         }
  }
/**********************************************************************************************************/
/* Proto_effacer_syn: Retrait du syn en parametre                                                         */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_effacer_synoptique ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn )
  { gboolean retour;

    retour = Retirer_synoptiqueDB( rezo_syn );

    if (retour)
     { Envoi_client( client, TAG_SYNOPTIQUE, SSTAG_SERVEUR_DEL_SYNOPTIQUE_OK,
                     (gchar *)rezo_syn, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete synoptique %s", rezo_syn->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_syn: Un client nous demande d'ajouter un syn Watchdog                                    */
/* Entrée: le syn à créer                                                                                 */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_synoptique ( struct CLIENT *client, struct CMD_TYPE_SYNOPTIQUE *rezo_syn )
  { struct CMD_TYPE_SYNOPTIQUE *result;
    gint id;

    id = Ajouter_synoptiqueDB ( rezo_syn );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to add synoptique %s", rezo_syn->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_synoptiqueDB( id );
           if (!result)
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          "Unable to locate synoptique %s", rezo_syn->libelle);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { Envoi_client( client, TAG_SYNOPTIQUE, SSTAG_SERVEUR_ADD_SYNOPTIQUE_OK,
                            (gchar *)result, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
              g_free(result);
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoyer_synoptiques_tag ( struct CLIENT *client, int tag, gint sstag, gint sstag_fin )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_SYNOPTIQUE *syn;
    struct DB *db;
    gchar titre[20];
    g_snprintf( titre, sizeof(titre), "W-SYNS-%06d", client->ssrv_id );
    prctl(PR_SET_NAME, titre, 0, 0, 0 );

    if ( ! Recuperer_synoptiqueDB( &db ) )
     { return; }                                                                 /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d synoptiques", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { syn = Recuperer_synoptiqueDB_suite( &db );
       if (!syn)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          return;
        }

       Envoi_client ( client, tag, sstag, (gchar *)syn, sizeof(struct CMD_TYPE_SYNOPTIQUE) );
       g_free(syn);
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_synoptiques_thread ( struct CLIENT *client )
  { Envoyer_synoptiques_tag( client, TAG_SYNOPTIQUE, SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE,
                                                     SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FIN );
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit( NULL );
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_synoptiques_pour_atelier_thread ( struct CLIENT *client )
  { Envoyer_synoptiques_tag( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER,
                                                  SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_FIN );
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit( NULL );
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_synoptiques_pour_atelier_palette_thread ( struct CLIENT *client )
  { Envoyer_synoptiques_tag( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_PALETTE,
                                                  SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FOR_ATELIER_PALETTE_FIN );
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit( NULL );
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_synoptiques_pour_plugin_dls_thread ( struct CLIENT *client )
  { Envoyer_synoptiques_tag( client, TAG_DLS, SSTAG_SERVEUR_ADDPROGRESS_SYN_FOR_PLUGIN_DLS,
                                              SSTAG_SERVEUR_ADDPROGRESS_SYN_FOR_PLUGIN_DLS_FIN );
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit( NULL );
  }
/*--------------------------------------------------------------------------------------------------------*/
