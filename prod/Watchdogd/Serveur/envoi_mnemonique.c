/******************************************************************************************************************************/
/* Watchdogd/Serveur/envoi_mnemonique.c        Configuration des mnemoniques DLS de Watchdog v2.0                             */
/* Projet WatchDog version 3.0       Gestion d'habitat                                        mar. 04 janv. 2011 12:49:37 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * envoi_mnemonique.c
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

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
 extern struct SSRV_CONFIG Cfg_ssrv;
/******************************************************************************************************************************/
/* Proto_editer_mnemonique: Le client desire editer un mnemo                                                                  */
/* Entrée: le client demandeur et le mnemo en question                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_editer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMO_BASE *rezo_mnemonique )
  { struct CMD_TYPE_MNEMO_FULL *result = NULL;

    result = Rechercher_mnemo_fullDB ( rezo_mnemonique->id );
    if (result)
     { Envoi_client( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_EDIT_MNEMONIQUE_OK,
                    (gchar *)result, sizeof(struct CMD_TYPE_MNEMO_FULL) );
       g_free(result);                                                                                  /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to locate mnemo %s", rezo_mnemonique->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Proto_valider_editer_mnemonique: Le client valide l'edition d'un mnemo                                                     */
/* Entrée: le client demandeur et le mnemo en question                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 static void Proto_ajouter_editer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMO_FULL *rezo_mnemonique,
                                               gboolean ajout )
  { struct CMD_TYPE_MNEMO_BASE *result;
    gboolean retour = FALSE;
    gint id = 0;

    if (ajout)
     { id = Ajouter_mnemo_fullDB ( rezo_mnemonique );
       if (id == -1)
        { struct CMD_GTK_MESSAGE erreur;
          g_snprintf( erreur.message, sizeof(erreur.message),
                      "Unable to add mnemo %s", rezo_mnemonique->mnemo_base.libelle);
          Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                        (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
          return;
        }
     }
    else
     { id = rezo_mnemonique->mnemo_base.id;
       retour = Modifier_mnemo_fullDB ( rezo_mnemonique );
       if (retour==FALSE)
        { struct CMD_GTK_MESSAGE erreur;
          g_snprintf( erreur.message, sizeof(erreur.message),
                      "Unable to edit mnemo %s", rezo_mnemonique->mnemo_base.libelle);
          Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                        (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
          return;
        }
     }

    result = Rechercher_mnemo_baseDB( id );
    if (!result)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                  "Unable to locate mnemo %s", rezo_mnemonique->mnemo_base.libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                    (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       return;
     }

    if (ajout)
     { Envoi_client( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_ADD_MNEMONIQUE_OK,
                    (gchar *)result, sizeof(struct CMD_TYPE_MNEMO_BASE) );
     }
    else
     { Envoi_client( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_VALIDE_EDIT_MNEMONIQUE_OK,
                    (gchar *)result, sizeof(struct CMD_TYPE_MNEMO_BASE) );
     }

    switch( result->type )
     { case MNEMO_ENTREE_ANA :  Charger_analogInput (); break;                                 /* Update de la running config */
     }
    g_free(result);
  }
/******************************************************************************************************************************/
/* Proto_valider_editer_mnemonique: Le client valide l'edition d'un mnemo                                                     */
/* Entrée: le client demandeur et le mnemo en question                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_valider_editer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMO_FULL *rezo_mnemonique )
  { Proto_ajouter_editer_mnemonique ( client, rezo_mnemonique, FALSE ); }
/******************************************************************************************************************************/
/* Proto_ajouter_mnemonique: Un client nous demande d'ajouter un mnemo Watchdog                                               */
/* Entrée: le mnemo à créer                                                                                                   */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_ajouter_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMO_FULL *rezo_mnemonique )
  { Proto_ajouter_editer_mnemonique ( client, rezo_mnemonique, TRUE ); }
/******************************************************************************************************************************/
/* Proto_effacer_mnemonique: Retrait du mnemo en parametre                                                                    */
/* Entrée: le client demandeur et le mnemo en question                                                                        */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
 void Proto_effacer_mnemonique ( struct CLIENT *client, struct CMD_TYPE_MNEMO_BASE *rezo_mnemonique )
  { gboolean retour;

    retour = Retirer_mnemo_baseDB( rezo_mnemonique );

    if (retour)
     { Envoi_client( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_DEL_MNEMONIQUE_OK,
                     (gchar *)rezo_mnemonique, sizeof(struct CMD_TYPE_MNEMO_BASE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   "Unable to delete mnemo %s", rezo_mnemonique->libelle);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/******************************************************************************************************************************/
/* Proto_envoyer_type_num_mnemonique: Recherche et envoi du mnemonique de type et num en parametre                            */
/* Entrée: Le client demandeur et la structure de recherche                                                                   */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void Proto_envoyer_type_num_mnemo_tag( int tag, int ss_tag, struct CLIENT *client,
                                        struct CMD_TYPE_NUM_MNEMONIQUE *critere )
  { struct CMD_TYPE_MNEMO_BASE *mnemo;

    mnemo = Rechercher_mnemo_baseDB_type_num( critere );
    if (mnemo)
     { Envoi_client ( client, tag, ss_tag, (gchar *)mnemo, sizeof(struct CMD_TYPE_MNEMO_BASE) );
       g_free(mnemo);
     }
    else
     { struct CMD_TYPE_MNEMO_BASE inconnu;
       inconnu.id = 0;
       inconnu.type = critere->type;
       inconnu.num = critere->num;
       g_snprintf( inconnu.libelle, sizeof(inconnu.libelle), "Unknown" );
       Envoi_client ( client, tag, ss_tag,
                      (gchar *)&inconnu, sizeof(struct CMD_TYPE_MNEMO_BASE) );
     }
  }
/******************************************************************************************************************************/
/* Envoyer_mnemoniques: Envoi des mnemos au client GID_MNEMONIQUE                                                             */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 static void Envoyer_mnemoniques_tag ( struct CLIENT *client, guint tag, gint sstag, gint sstag_fin )
  { struct CMD_TYPE_MNEMONIQUES *mnemos;
    struct CMD_TYPE_MNEMO_BASE *mnemo;
    struct CMD_ENREG nbr;
    gchar critere[30];
    struct DB *db;
    gint max_enreg;                                                    /* Nombre maximum d'enregistrement dans un bloc reseau */

    prctl(PR_SET_NAME, "W-EnvoiMnemo", 0, 0, 0 );
    if (client->mnemo_dls_id_to_send != -1)
     { g_snprintf( critere, sizeof(critere), "mnemo.dls_id = '%d'", client->mnemo_dls_id_to_send ); }
    else
     { g_snprintf( critere, sizeof(critere), "1=1" ); }

    if ( ! Recuperer_mnemo_baseDB_with_conditions( &db, critere, -1, -1 ) )
     { Unref_client( client );                                                            /* Déréférence la structure cliente */
       return;
     }

    nbr.num = db->nbr_result;
    g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d mnemos", nbr.num );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG, (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    max_enreg = (Cfg_ssrv.taille_bloc_reseau - sizeof(struct CMD_TYPE_MNEMONIQUES)) / sizeof(struct CMD_TYPE_MNEMO_BASE);
    mnemos = (struct CMD_TYPE_MNEMONIQUES *)g_try_malloc0( Cfg_ssrv.taille_bloc_reseau );
    if (!mnemos)
     { struct CMD_GTK_MESSAGE erreur;
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR, "%s: Pb d'allocation memoire mnemos", __func__ );
       g_snprintf( erreur.message, sizeof(erreur.message), "Pb d'allocation memoire" );
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
       Unref_client( client );                                                            /* Déréférence la structure cliente */
       return;
     }
    mnemos->nbr_mnemos = 0;                                                     /* Valeurs par defaut si pas d'enregistrement */

    do
     { mnemo = Recuperer_mnemo_baseDB_suite( &db );                                     /* Récupération d'un mnemo dans la DB */
       if (mnemo)                                                                  /* Si enregegistrement, alors on le pousse */
        { memcpy ( &mnemos->mnemo[mnemos->nbr_mnemos], mnemo, sizeof(struct CMD_TYPE_MNEMO_BASE) );
          mnemos->nbr_mnemos++;                              /* Nous avons 1 enregistrement de plus dans la structure d'envoi */
          g_free(mnemo);
        }

       if ( (mnemo == NULL) || mnemos->nbr_mnemos == max_enreg )                  /* Si depassement de tampon ou plus d'enreg */
        { Envoi_client ( client, tag, sstag, (gchar *)mnemos,
                         sizeof(struct CMD_TYPE_MNEMONIQUES) +
                         mnemos->nbr_mnemos * sizeof(struct CMD_TYPE_MNEMO_BASE) );
          mnemos->nbr_mnemos = 0;
        }
     }
    while (mnemo);
    g_free(mnemos);
    Envoi_client ( client, tag, sstag_fin, NULL, 0 );
    Unref_client( client );                                                               /* Déréférence la structure cliente */
  }
/******************************************************************************************************************************/
/* Envoyer_mnemoniques: Envoi des mnemos au client GID_MNEMONIQUE                                                             */
/* Entrée: Néant                                                                                                              */
/* Sortie: Néant                                                                                                              */
/******************************************************************************************************************************/
 void *Envoyer_mnemoniques_thread ( struct CLIENT *client )
  { if (client->mnemo_dls_id_to_send != -1)
     { Envoyer_mnemoniques_tag( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE,
                                                        SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE_FIN );
     }
    else
     { Envoyer_mnemoniques_tag( client, TAG_MNEMONIQUE, SSTAG_SERVEUR_ADDPROGRESS_MNEMONIQUE,
                                                        SSTAG_SERVEUR_ADDPROGRESS_ALL_MNEMONIQUE_FIN );
     }
    pthread_exit ( NULL );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
