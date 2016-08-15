/******************************************************************************************************************************/
/* Watchdogd/Serveur/envoi_synoptique_capteur.c     Envoi des capteurs aux clients                                            */
/* Projet WatchDog version 2.0       Gestion d'habitat                                          dim 22 mai 2005 17:35:28 CEST */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
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
 #include <sys/time.h>
 #include <string.h>
 #include <unistd.h>

/**************************************************** Prototypes de fonctions *************************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/******************************************************************************************************************************/
/* Proto_effacer_capteur_atelier: Retrait du capteur en parametre                                                             */
/* Entr�e: le client demandeur et le capteur en question                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
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
/******************************************************************************************************************************/
/* Proto_ajouter_capteur_atelier: Ajout d'un capteur dans un synoptique                                                       */
/* Entr�e: le client demandeur et le capteur en question                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
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
/******************************************************************************************************************************/
/* Proto_valider_editer_capteur_atelier: Le client desire editer un capteur                                                   */
/* Entr�e: le client demandeur et le capteur en question                                                                      */
/* Sortie: Niet                                                                                                               */
/******************************************************************************************************************************/
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
/******************************************************************************************************************************/
/* Chercher_bit_capteurs: Renvoie 0 si l'element en argument est dans la liste                                                */
/* Entr�e: L'element                                                                                                          */
/* Sortie: 0 si present, 1 sinon                                                                                              */
/******************************************************************************************************************************/
 static gint Chercher_bit_capteurs ( struct CAPTEUR *element, struct CAPTEUR *cherche )
  { if (element->bit_controle == cherche->bit_controle &&
        element->type == cherche->type)
         return 0;
    else return 1;
  }
/******************************************************************************************************************************/
/* Envoyer_capteur_tag: Envoi des capteur au client en parametre                                                              */
/* Entr�e: Le client, le tag reseau et sous-tag                                                                               */
/* Sortie: n�ant                                                                                                              */
/******************************************************************************************************************************/
 void Envoyer_capteur_tag ( struct CLIENT *client, gint tag, gint sstag, gint sstag_fin )
  { struct CMD_TYPE_CAPTEUR *capteur;
    struct CMD_ENREG nbr;
    struct DB *db;

    if ( ! Recuperer_capteurDB( &db, client->syn_to_send->id ) )
     { return; }                                                                                   /* Si pas de capteurs (??) */

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d capteurs", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    while ( (capteur = Recuperer_capteurDB_suite( &db )) != NULL )                   /* Pour tous les capteurs de la database */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoyer_capteur_tag: capteur %d (%s) to client %s",
                 capteur->id, capteur->libelle, client->machine );

       Envoi_client ( client, tag, sstag,                                                       /* Envoi du capteur au client */
                      (gchar *)capteur, sizeof(struct CMD_TYPE_CAPTEUR) );

       if (tag == TAG_SUPERVISION)                                          /* Si mode supervision on envoit la valeur d'init */
        { struct CMD_ETAT_BIT_CAPTEUR *init_capteur;
          struct CAPTEUR *capteur_new;
		  capteur_new = (struct CAPTEUR *) g_try_malloc0 ( sizeof(struct CAPTEUR) );
		  if (!capteur_new)
           { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                      "Envoyer_capteur_tag: Memory Error for %d (%s)", capteur->id, capteur->libelle );
           }
          else
           { capteur_new->type         = capteur->type;
			 capteur_new->bit_controle = capteur->bit_controle;
			 
             init_capteur = Formater_capteur(capteur_new);                                 /* Formatage de la chaine associ�e */
             if (init_capteur)                                                            /* envoi la valeur d'init au client */
              { Envoi_client( client, TAG_SUPERVISION, SSTAG_SERVEUR_SUPERVISION_CHANGE_CAPTEUR,
                              (gchar *)init_capteur, sizeof(struct CMD_ETAT_BIT_CAPTEUR) );
                g_free(init_capteur);                                                                 /* On libere la m�moire */
              }
             else { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_ERR,
                             "Envoyer_capteur_tag: Formater_capteur failed for %d (%s)", capteur->id, capteur->libelle );
                  }

             if ( ! g_slist_find_custom(client->Liste_bit_capteurs, capteur_new, (GCompareFunc) Chercher_bit_capteurs) )
              { client->Liste_bit_capteurs = g_slist_prepend( client->Liste_bit_capteurs, capteur_new ); }
             else g_free( capteur_new );                          /* si deja dans la liste, plus besoin de cette zone m�moire */
	      }
        }
       g_free(capteur);
     }
    Envoi_client ( client, tag, sstag_fin, NULL, 0 );
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
