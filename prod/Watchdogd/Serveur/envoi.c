/******************************************************************************************************************************/
/* Watchdogd/envoi.c        Procedures d'envoi de données au(x) client(s) connecté(s)                                         */
/* Projet WatchDog version 3.0       Gestion d'habitat                                           ven 04 mar 2005 10:16:04 CET */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * envoi.c
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

 #include <string.h>
 #include <errno.h>
 #include <sys/stat.h>
 #include <fcntl.h>
 #include <sys/unistd.h>

 #define DEFAUT_MAX 3
/****************************************************** Prototypes de fonctions ***********************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
 extern struct SSRV_CONFIG Cfg_ssrv;
/******************************************************************************************************************************/
/* Envoi_client: Envoi le buffer au client id                                                                                 */
/* Entrée: structure identifiant le client, et le buffer à envoyer                                                            */
/* Sortie: code d'erreur                                                                                                      */
/******************************************************************************************************************************/
 gint Envoi_client( struct CLIENT *client, gint tag, gint ss_tag, gchar *buffer, gint taille )
  { gint retour;

    if ( !client ) return(0);
    if ( client->mode >= DECONNECTE )
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
                "Envoi_client : SSRV%06d, envoi interdit to %d", client->ssrv_id, client->connexion->socket);
       return(0);
     }

    if ( Attendre_envoi_disponible( client->connexion ) )
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                "Envoi_client: SSRV%06d, Deconnexion client sur défaut d'attente envoi disponible", client->ssrv_id );
       Client_mode ( client, DECONNECTE );
       return(0);
     }
                                                                         /* Attente de la possibilité d'envoyer sur le reseau */

    retour = Envoyer_reseau( client->connexion, tag, ss_tag, buffer, taille );
    if (retour)
     { client->defaut++;
       Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,
                "Envoi_client: SSRV%06d, Failed sending to id=%d (%s), error %d",
                client->ssrv_id, client->connexion->socket, client->machine, retour);

       if (client->defaut>=DEFAUT_MAX)
        { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                   "Envoi_client: SSRV%06d, Deconnexion client sur défaut", client->ssrv_id );
          Client_mode ( client, DECONNECTE );
        }
       else switch(retour)
        { case EPIPE:
          case ECONNRESET: Client_mode ( client, DECONNECTE );                              /* Connection resettée par le clt */
                           Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
                                    "SSRV%06d, decision: deconnexion client", client->ssrv_id );
                           break;
        }
     }
    else client->defaut=0;                                                                        /* Ok, pas de defaut client */
    return(retour);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
