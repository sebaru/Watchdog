/**********************************************************************************************************/
/* Watchdogd/Serveur/envoi_entreeana.c        Configuration des entrees de Watchdog v2.0                  */
/* Projet WatchDog version 2.0       Gestion d'habitat                      jeu 25 sep 2003 14:17:17 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * envoi_entreeana.c
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
/* Envoyer_entreeANA_tag : Envoie les entreANA au client. Attention, c'est un thread !                    */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 static void Envoyer_entreeANA_tag ( struct CLIENT *client, guint tag, gint sstag, gint sstag_fin )
  { struct CMD_ENREG nbr;
    struct CMD_TYPE_MNEMO_AI *entree;
    struct DB *db;

    prctl(PR_SET_NAME, "W-EnvoiANA", 0, 0, 0 );

    if (!Recuperer_entreeANADB( &db ))
     { Unref_client( client );                                        /* Déréférence la structure cliente */
       return;
     }                                                                           /* Si pas de histos (??) */

    nbr.num = db->nbr_result;
    if (nbr.num)
     { g_snprintf( nbr.comment, sizeof(nbr.comment), "Loading %d entrees", nbr.num );
       Envoi_client ( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_NBR_ENREG,
                      (gchar *)&nbr, sizeof(struct CMD_ENREG) );
     }

    for( ; ; )
     { entree = Recuperer_entreeANADB_suite( &db );
       if (!entree)
        { Envoi_client ( client, tag, sstag_fin, NULL, 0 );
          Unref_client( client );                                     /* Déréférence la structure cliente */
          return;
        }

       Envoi_client ( client, tag, sstag, (gchar *)entree, sizeof(struct CMD_TYPE_MNEMO_AI) );
       g_free(entree);
     }
  }
/**********************************************************************************************************/
/* Envoyer_classes: Envoi des classes au client GID_CLASSE                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_entreeANA_for_courbe_thread ( struct CLIENT *client )
  { Envoyer_entreeANA_tag( client, TAG_COURBE, SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_COURBE,
                                               SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_COURBE_FIN );
    Client_mode ( client, ENVOI_MNEMONIQUE_FOR_COURBE );
    pthread_exit(NULL);
  }
/**********************************************************************************************************/
/* Envoyer_classes: Envoi des classes au client GID_CLASSE                                                */
/* Entrée: Néant                                                                                          */
/* Sortie: Néant                                                                                          */
/**********************************************************************************************************/
 void *Envoyer_entreeANA_for_histo_courbe_thread ( struct CLIENT *client )
  { Envoyer_entreeANA_tag( client, TAG_HISTO_COURBE,
                           SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_HISTO_COURBE,
                           SSTAG_SERVEUR_ADDPROGRESS_ENTREEANA_FOR_HISTO_COURBE_FIN );
    Client_mode ( client, ENVOI_MNEMONIQUE_FOR_HISTO_COURBE );
    pthread_exit(NULL);
  }
/*--------------------------------------------------------------------------------------------------------*/
