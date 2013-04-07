/**********************************************************************************************************/
/* Watchdogd/Serveur/ident.c        Gestion du logon user sur module Client Watchdog                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 19:46:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ident.c
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
 #include <string.h>
 
/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Sous_serveur.h"
/**********************************************************************************************************/
/* Autoriser_client: Autorise le client à se connecter                                                    */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Autoriser_client ( struct CLIENT *client )
  { struct REZO_SRV_IDENT ident;
    g_snprintf( ident.comment, sizeof(ident.comment), "Serveur Watchdog %s", VERSION );
    if ( Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_AUTORISE,
                       (gchar *)&ident, sizeof(struct REZO_SRV_IDENT) ) )
     { return; }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "Autoriser_autorisation: RAZ 'login failed' for %d", client->util->id );
    
    Raz_login_failed( Config.log, client->Db_watchdog, client->util->id );
  }
/**********************************************************************************************************/
/* Proto_set_password: changement de password                                                             */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Proto_set_password ( struct CLIENT *client, struct CMD_UTIL_SETPASSWORD *util )
  { if (util->id != client->util->id)
     { Client_mode ( client, DECONNECTE );
       return;
     }

    Set_password( Config.log, client->Db_watchdog, Config.crypto_key, util );
    Client_mode ( client, DECONNECTE );                        /* On deconnecte le client tout de suite ! */
  }
/**********************************************************************************************************/
/* Tester_autorisation: envoi de l'autorisation ou non au client                                          */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 gint Tester_autorisation ( struct CLIENT *client )
  { struct CMD_TYPE_UTILISATEUR util;
    gchar *clef, *crypt;
    gint id;

    clef = Recuperer_clef( Config.log, client->Db_watchdog,
                           client->ident.nom, &id );
    if (!clef)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING, 
               "Tester_autorisation: Unable to retrieve the key of user %s", client->ident.nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_REFUSE, NULL, 0 );
       return(DECONNECTE);
     }
          
    client->util = Rechercher_utilisateurDB( Config.log, client->Db_watchdog, id );
    if (!client->util)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING, 
               "Tester_autorisation: Unable to retrieve the user %s", client->ident.nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_REFUSE, NULL, 0 );
       return(DECONNECTE);
     }
    memcpy( &client->util->code, clef, sizeof( client->util->code ) );    
    g_free(clef);
/***************************************** Identification du client ***************************************/
    crypt = Crypter( Config.log, Config.crypto_key, client->ident.password );
    if (!crypt)
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING, 
               "Tester_autorisation: Password encryption error for user %s", client->util->nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_REFUSE, NULL, 0 );
       return(DECONNECTE);
     }

    if (memcmp( crypt, client->util->code, sizeof( client->util->code ) ))       /* Comparaison des codes */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,  
               "Tester_autorisation: Password error for %s", client->util->nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_REFUSE, NULL, 0 );
       g_free(crypt);
       Ajouter_one_login_failed( Config.log, client->Db_watchdog,  /* Dommage ! */
                                 client->util->id, Config.max_login_failed );
       return(DECONNECTE);
     }
    g_free(crypt);

/********************************************* Compte du client *******************************************/
    if (!client->util->actif)                              /* Est-ce que son compte est toujours actif ?? */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,  
                "Tester_autorisation: Account disabled for %s", client->util->nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_ACCOUNT_DISABLED, NULL, 0 );
       return(DECONNECTE);
     }

    if (client->util->expire && client->util->date_expire<time(NULL) )   /* Expiration temporel du compte */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING, 
                "Tester_autorisation: Account expired for %s", client->util->nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_ACCOUNT_EXPIRED, NULL, 0 );
       return(DECONNECTE);
     }

    if (client->util->changepass)                       /* L'utilisateur doit-il changer son mot de passe */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,  
                "Tester_autorisation: User %s have to change his password", client->util->nom );
       util.id = client->util->id;
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_CHANGEPASS,
                     (gchar *)&util, sizeof(struct CMD_TYPE_UTILISATEUR) );
       return(ATTENTE_NEW_PASSWORD);
     }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Tester_autorisation: Envoi Autorisation for %s", client->util->nom );
    Autoriser_client ( client );
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "Tester_autorisation: Autorisation sent for %s", client->util->nom );
    return( ENVOI_DONNEES );
  }
/*--------------------------------------------------------------------------------------------------------*/
