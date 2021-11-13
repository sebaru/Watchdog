/******************************************************************************************************************************/
/* Watchdogd/Serveur/admin_serveur.c        Gestion des responses Admin IMSG au serveur watchdog                             */
/* Projet WatchDog version 2.0       Gestion d'habitat                                      sam. 28 juil. 2012 16:35:09 CEST  */
/* Auteur: LEFEVRE Sebastien                                                                                                  */
/******************************************************************************************************************************/
/*
 * admin_serveur.c
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
 
 #include "watchdogd.h"
 #include "Sous_serveur.h"

/******************************************************************************************************************************/
/* Admin_ssrv_msgs : Envoi un message à un user ou tous                                                                       */
/* Entrée: La response d'admin le message et le nom du user ('all' si tous)                                                  */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_ssrv_msgs( gchar *response, gchar *msg, gchar *name )
  { struct CMD_GTK_MESSAGE erreur;
    struct CLIENT *client;
    gchar chaine[128];
    GSList *liste;

    g_snprintf( erreur.message, sizeof(erreur.message), "AdminMSG : %s", msg);

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    liste = Cfg_ssrv.Clients;
    while ( liste )                              /* Parcours de la liste des ssrv (et donc de clients) */
     { client = (struct CLIENT *)liste->data;
       if ( (! strcmp ( client->util->nom, name ) ) || (! strcmp ( name, "all" )) )
        { Envoi_client( client, TAG_GTK_MESSAGE, SSTAG_SERVEUR_WARNING,
                       (gchar *)&erreur, sizeof(struct CMD_GTK_MESSAGE) );
          g_snprintf( chaine, sizeof(chaine), " | - SSRV%06d - %s@%s", client->ssrv_id,
                      (client->util ? client->util->nom : "unknown"), client->machine );
          response = Admin_write ( response, chaine );
        }
       liste = g_slist_next(liste);
     }
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_ssrv_list : Liste m'ensemble des response actives                                                                   */
/* Entrée: La response d'admin                                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_ssrv_list( gchar *response )
  { struct CLIENT *client;
    gchar chaine[256];
    GSList *liste;
    g_snprintf( chaine, sizeof(chaine), " -- Connected Users List --" );
    response = Admin_write ( response, chaine );

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    liste = Cfg_ssrv.Clients;
    while ( liste )                                                     /* Parcours de la liste des ssrv (et donc de clients) */
     { gchar date[32];
       client = (struct CLIENT *)liste->data;
       strftime( date, sizeof(date), "%F %T", localtime(&client->date_connexion) );
       g_snprintf( chaine, sizeof(chaine),
                   " | SSRV%06d - v%s - AccessLevel %02d - mode %02d (%s) ref %d defaut %02d date %s - %s@%s",
                   client->ssrv_id, client->ident.version, (client->util ? client->util->access_level : -1),
                   client->mode, Mode_vers_string(client->mode), client->struct_used, client->defaut, date,
                  (client->util ? client->util->nom : "unknown"), client->machine
                 );
       response = Admin_write ( response, chaine );
       liste = g_slist_next(liste);
     }
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_ssrv_kill : Termine l'ensemble des responses d'un utilisateur                                                       */
/* Entrée: La response d'admin et le nom du client a kicker                                                                  */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_ssrv_kill_name( gchar *response, gchar *name )
  { struct CLIENT *client;
    gchar chaine[128];
    GSList *liste;

    g_snprintf( chaine, sizeof(chaine), " -- Starting killing Sessions for %s --", name );
    response = Admin_write ( response, chaine );

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    liste = Cfg_ssrv.Clients;
    while ( liste )                                                     /* Parcours de la liste des ssrv (et donc de clients) */
     { client = (struct CLIENT *)liste->data;
       if ( ! strcmp ( client->util->nom, name ) )
        { Client_mode ( client, DECONNECTE );
          g_snprintf( chaine, sizeof(chaine), " | - Killed session : SSRV%06d - %s@%s", client->ssrv_id,
                      (client->util ? client->util->nom : "unknown"), client->machine );
          response = Admin_write ( response, chaine );
        }
       liste = g_slist_next(liste);
     }
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_ssrv_killid : Termine la session dont l'id est en parametre                                                          */
/* Entrée: La response d'admin et l'id de la response a kicker                                                              */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_ssrv_kill_id( gchar *response, gint id )
  { struct CLIENT *client;
    gchar chaine[128];
    GSList *liste;

    g_snprintf( chaine, sizeof(chaine), " -- Starting killing Session for id %d --", id );
    response = Admin_write ( response, chaine );

    pthread_mutex_lock( &Cfg_ssrv.lib->synchro );
    liste = Cfg_ssrv.Clients;
    while ( liste )                                                     /* Parcours de la liste des ssrv (et donc de clients) */
     { client = (struct CLIENT *)liste->data;
       if ( client->ssrv_id == id )
        { Client_mode ( client, DECONNECTE );
          g_snprintf( chaine, sizeof(chaine), " | - Killed session : SSRV%06d - %s@%s", client->ssrv_id,
                      (client->util ? client->util->nom : "unknown"), client->machine );
          response = Admin_write ( response, chaine );
        }
       liste = g_slist_next(liste);
     }
    pthread_mutex_unlock( &Cfg_ssrv.lib->synchro );
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_ssrv_status : Affiche le statut du thread                                                                            */
/* Entrée: Le response d'admin                                                                                               */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 static gchar *Admin_ssrv_status( gchar *response )
  { gchar chaine[256];
    if (!Cfg_ssrv.Socket_ecoute)
     { g_snprintf( chaine, sizeof(chaine), " | - NOT listening port %d", Cfg_ssrv.port );
       response = Admin_write ( response, chaine );
     }
    else
     { g_snprintf( chaine, sizeof(chaine),
                   " | - Listening port %d, socket %d\n", Cfg_ssrv.port, Cfg_ssrv.Socket_ecoute );
       response = Admin_write ( response, chaine );
       g_snprintf( chaine, sizeof(chaine),
                   " | - X509 common name = %s\n", Nom_certif ( Cfg_ssrv.ssrv_certif ) );
       response = Admin_write ( response, chaine );
       g_snprintf( chaine, sizeof(chaine),
                   " | - X509 issuer name = %s\n", Nom_certif_signataire ( Cfg_ssrv.ssrv_certif ) );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/******************************************************************************************************************************/
/* Admin_command : Appeller par le thread admin pour traiter une commande                                                     */
/* Entrée: Le response d'admin, la ligne a traiter                                                                           */
/* Sortie: néant                                                                                                              */
/******************************************************************************************************************************/
 gchar *Admin_command( gchar *response, gchar *ligne )
  { gchar commande[128], chaine[128];

    sscanf ( ligne, "%s", commande );                                                    /* Découpage de la ligne de commande */

    if ( ! strcmp ( commande, "list" ) )
     { response = Admin_ssrv_list ( response );
     } else
    if ( ! strcmp ( commande, "status" ) )
     { response = Admin_ssrv_status ( response );
     } else
    if ( ! strcmp ( commande, "msgs" ) )
     { response = Admin_ssrv_msgs ( response, ligne + 5, "all" );
     } else
    if ( ! strcmp ( commande, "msg" ) )
     { gchar name[80];
       sscanf ( ligne, "%s %s", commande, name );                                        /* Découpage de la ligne de commande */
       response = Admin_ssrv_msgs ( response, ligne + 5 + strlen(name), name );
     } else
    if ( ! strcmp ( commande, "kill" ) )
     { gchar name[80];
       sscanf ( ligne, "%s %s", commande, name );                                        /* Découpage de la ligne de commande */
       response = Admin_ssrv_kill_name ( response, name );
     } else
    if ( ! strcmp ( commande, "killid" ) )
     { gint id;
       sscanf ( ligne, "%s %d", commande, &id );                                         /* Découpage de la ligne de commande */
       response = Admin_ssrv_kill_id ( response, id );
     } else
    if ( ! strcmp ( commande, "dbcfg" ) )                     /* Appelle de la fonction dédiée à la gestion des parametres DB */
     { gboolean retour;
       response =  Admin_dbcfg_thread ( response, NOM_THREAD, ligne+6 );                        /* Si changement de parametre */
       retour = Ssrv_Lire_config();
       g_snprintf( chaine, sizeof(chaine), " Reloading Thread Parameters from Database -> %s",
                   (retour ? "Success" : "Failed") );
       response = Admin_write ( response, chaine );
     } else
    if ( ! strcmp ( commande, "help" ) )
     { response = Admin_write ( response, "  -- Watchdog ADMIN -- Help du mode 'SSRV'" );
       response = Admin_write ( response, "  dbcfg...              - Manage Threads Parameters in Database" );
       response = Admin_write ( response, "  list                  - Listes les sous serveurs" );
       response = Admin_write ( response, "  msgs $message         - Send $message to all connected client" );
       response = Admin_write ( response, "  msg $user $message    - Send $message to $user (can be 'all')" );
       response = Admin_write ( response, "  killid $id            - Kill session with ID $id" );
       response = Admin_write ( response, "  kill $name            - Kill all sessions for user $name" );
       response = Admin_write ( response, "  status                - Show status of the Thread SSRV" );
       response = Admin_write ( response, "  help                  - This help\n" );
     }
    else
     { gchar chaine[128];
       g_snprintf( chaine, sizeof(chaine), " Unknown command : %s", ligne );
       response = Admin_write ( response, chaine );
     }
    return(response);
  }
/*----------------------------------------------------------------------------------------------------------------------------*/
