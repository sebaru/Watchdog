/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique.c        Configuration des synoptiques de Watchdog v2.0             */
/* Projet WatchDog version 2.0       Gestion d'habitat                      dim 22 mai 2005 18:03:34 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_synoptique.c
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
 #include <bonobo/bonobo-i18n.h>
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

 #include "Reseaux.h"
 #include "Synoptiques_DB.h"
 #include "Erreur.h"
 #include "Config.h"
 #include "Client.h"

 #include "watchdogd.h"
 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
/******************************************** Prototypes de fonctions *************************************/
 #include "proto_srv.h"

/**********************************************************************************************************/
/* Preparer_envoi_synoptique: convertit une structure MSG en structure CMD_SHOW_SYNOPTIQUE                */
/* Entrée: un client et un utilisateur                                                                    */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 static struct CMD_SHOW_SYNOPTIQUE *Preparer_envoi_synoptique ( struct SYNOPTIQUEDB *syn )
  { struct CMD_SHOW_SYNOPTIQUE *rezo_syn;

    rezo_syn = (struct CMD_SHOW_SYNOPTIQUE *)g_malloc0( sizeof(struct CMD_SHOW_SYNOPTIQUE) );
    if (!rezo_syn) { return(NULL); }

    rezo_syn->id         = syn->id;
    rezo_syn->groupe     = syn->groupe;
    memcpy( &rezo_syn->libelle, syn->libelle, sizeof(rezo_syn->libelle) );
    memcpy( &rezo_syn->mnemo,   syn->mnemo,   sizeof(rezo_syn->mnemo) );
    return( rezo_syn );
  }
/**********************************************************************************************************/
/* Proto_editer_syn: Le client desire editer un syn                                                       */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_editer_synoptique ( struct CLIENT *client, struct CMD_ID_SYNOPTIQUE *rezo_syn )
  { struct CMD_EDIT_SYNOPTIQUE edit_syn;
    struct SYNOPTIQUEDB *syn;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    syn = Rechercher_synoptiqueDB( Config.log, Db_watchdog, rezo_syn->id );

    if (syn)
     { edit_syn.id         = syn->id;                                       /* Recopie des info editables */
       edit_syn.groupe     = syn->groupe;
       memcpy( &edit_syn.libelle, syn->libelle, sizeof(edit_syn.libelle) );
       memcpy( &edit_syn.mnemo,   syn->mnemo,   sizeof(edit_syn.mnemo) );

       Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_EDIT_SYNOPTIQUE_OK,
                  (gchar *)&edit_syn, sizeof(struct CMD_EDIT_SYNOPTIQUE) );
       g_free(syn);                                                                 /* liberation mémoire */
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to locate synoptique %s:\n%s"), rezo_syn->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_valider_editer_syn: Le client valide l'edition d'un syn                                          */
/* Entrée: le client demandeur et le syn en question                                                      */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_valider_editer_synoptique ( struct CLIENT *client, struct CMD_EDIT_SYNOPTIQUE *rezo_syn )
  { struct SYNOPTIQUEDB *result;
    gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Modifier_synoptiqueDB ( Config.log, Db_watchdog, rezo_syn );
    if (retour==FALSE)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to edit synoptique %s:\n%s"), rezo_syn->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_synoptiqueDB( Config.log, Db_watchdog, rezo_syn->id );
           if (result) 
            { struct CMD_SHOW_SYNOPTIQUE *syn;
              syn = Preparer_envoi_synoptique ( result );
              g_free(result);
              if (!syn)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             _("Not enough memory") );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_VALIDE_EDIT_SYNOPTIQUE_OK,
                                   (gchar *)syn, sizeof(struct CMD_SHOW_SYNOPTIQUE) );
                     g_free(syn);
                   }
            }
           else
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate synoptique %s:\n%s"), rezo_syn->libelle, Db_watchdog->last_err);
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
 void Proto_effacer_synoptique ( struct CLIENT *client, struct CMD_ID_SYNOPTIQUE *rezo_syn )
  { gboolean retour;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    retour = Retirer_synoptiqueDB( Config.log, Db_watchdog, rezo_syn );

    if (retour)
     { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_DEL_SYNOPTIQUE_OK,
                     (gchar *)rezo_syn, sizeof(struct CMD_ID_SYNOPTIQUE) );
     }
    else
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to delete synoptique %s:\n%s"), rezo_syn->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
  }
/**********************************************************************************************************/
/* Proto_ajouter_syn: Un client nous demande d'ajouter un syn Watchdog                                    */
/* Entrée: le syn à créer                                                                                 */
/* Sortie: Niet                                                                                           */
/**********************************************************************************************************/
 void Proto_ajouter_synoptique ( struct CLIENT *client, struct CMD_ADD_SYNOPTIQUE *rezo_syn )
  { struct SYNOPTIQUEDB *result;
    gint id;
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;

    id = Ajouter_synoptiqueDB ( Config.log, Db_watchdog, rezo_syn );
    if (id == -1)
     { struct CMD_GTK_MESSAGE erreur;
       g_snprintf( erreur.message, sizeof(erreur.message),
                   _("Unable to add synoptique %s:\n%s"), rezo_syn->libelle, Db_watchdog->last_err);
       Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                     (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
     }
    else { result = Rechercher_synoptiqueDB( Config.log, Db_watchdog, id );
           if (!result) 
            { struct CMD_GTK_MESSAGE erreur;
              g_snprintf( erreur.message, sizeof(erreur.message),
                          _("Unable to locate synoptique %s:\n%s"), rezo_syn->libelle, Db_watchdog->last_err);
              Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                            (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
            }
           else
            { struct CMD_SHOW_SYNOPTIQUE *syn;
              syn = Preparer_envoi_synoptique ( result );
              g_free(result);
              if (!syn)
               { struct CMD_GTK_MESSAGE erreur;
                 g_snprintf( erreur.message, sizeof(erreur.message),
                             _("Not enough memory") );
                 Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_ERREUR,
                               (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
               }
              else { Envoi_client( client, TAG_ATELIER, SSTAG_SERVEUR_ADD_SYNOPTIQUE_OK,
                                   (gchar *)syn, sizeof(struct CMD_SHOW_SYNOPTIQUE) );
                     g_free(syn);
                   }
            }
         }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void *Envoyer_synoptiques_tag ( struct CLIENT *client, int tag, gint sstag, gint sstag_fin )
  { struct CMD_SHOW_SYNOPTIQUE *rezo_syn;
    struct CMD_ENREG nbr;
    struct SYNOPTIQUEDB *syn;
    SQLHSTMT hquery;                                                   /* Requete SQL en cours d'emission */
    struct DB *Db_watchdog;
    Db_watchdog = client->Db_watchdog;
 
    hquery = Recuperer_synoptiqueDB( Config.log, Db_watchdog );
    if (!hquery) return(NULL);

    SQLRowCount( hquery, (SQLINTEGER *)&nbr.num );
    g_snprintf( nbr.comment, sizeof(nbr.comment), _("Loading synoptiques") );
    Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                   (gchar *)&nbr, sizeof(struct CMD_ENREG) );

    for( ; ; )
     { syn = Recuperer_synoptiqueDB_suite( Config.log, Db_watchdog, hquery );
       if (!syn)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          return(NULL);
        }

       rezo_syn = Preparer_envoi_synoptique( syn );
       g_free(syn);
       if (rezo_syn)
        { while (Attendre_envoi_disponible( Config.log, client->connexion )) sched_yield();
                                                     /* Attente de la possibilité d'envoyer sur le reseau */

          Envoi_client ( client, tag, sstag,
                         (gchar *)rezo_syn, sizeof(struct CMD_SHOW_SYNOPTIQUE) );
          g_free(rezo_syn);
        }
     }
  }
/**********************************************************************************************************/
/* Envoyer_syns: Envoi des syns au client GID_SYNOPTIQUE                                                  */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_synoptiques_thread ( struct CLIENT *client )
  { Envoyer_synoptiques_tag( client, TAG_ATELIER, SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE,
                                                  SSTAG_SERVEUR_ADDPROGRESS_SYNOPTIQUE_FIN );
    Unref_client( client );                                           /* Déréférence la structure cliente */
    pthread_exit( NULL );
    return(NULL);
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
    return(NULL);
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
    Client_mode ( client, ENVOI_PALETTE_FOR_ATELIER_PALETTE );
    pthread_exit( NULL );
    return(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
