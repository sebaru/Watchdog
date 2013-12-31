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
    
    Raz_login_failed( client->util->id );
  }
/**********************************************************************************************************/
/* Proto_set_password: changement de password                                                             */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Proto_set_password ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *util )
  { if (util->id != client->util->id)                                        /* Le client est-il le bon ? */
     { Client_mode ( client, DECONNECTE );
       return;
     }

    Modifier_utilisateurDB_set_password( util );
    Client_mode ( client, DECONNECTE );                        /* On deconnecte le client tout de suite ! */
  }
/**********************************************************************************************************/
/* Tester_autorisation: envoi de l'autorisation ou non au client                                          */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Tester_autorisation ( struct CLIENT *client, struct CMD_TYPE_UTILISATEUR *util )
  { 
/*************************************** Authentification du client ***************************************/
    if (memcmp( util->hash, client->util->hash, sizeof( client->util->hash ) ))  /* Comparaison des codes */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,  
               "Tester_autorisation: Password error for %s", client->util->nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_REFUSE, NULL, 0 );
       Ajouter_one_login_failed( client->util->id, Config.max_login_failed );                /* Dommage ! */
       Client_mode (client, DECONNECTE);
       return;
     }
/********************************************* Compte du client *******************************************/
    if (!client->util->enable)                             /* Est-ce que son compte est toujours actif ?? */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,  
                "Tester_autorisation: Account disabled for %s", client->util->nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_ACCOUNT_DISABLED, NULL, 0 );
       Client_mode (client, DECONNECTE);
       return;
     }

    if (client->util->expire && client->util->date_expire<time(NULL) )   /* Expiration temporel du compte */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING, 
                "Tester_autorisation: Account expired for %s", client->util->nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_ACCOUNT_EXPIRED, NULL, 0 );
       Client_mode (client, DECONNECTE);
       return;
     }

    if (client->util->mustchangepwd)                    /* L'utilisateur doit-il changer son mot de passe */
     { Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_WARNING,  
                "Tester_autorisation: User %s(id=%d) have to change his password",
                 client->util->nom, client->util->id );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_NEEDCHANGEPWD, NULL, 0 );
       Client_mode (client, WAIT_FOR_NEWPWD);
       return;
     }
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_DEBUG,
             "Tester_autorisation: Envoi Autorisation for %s", client->util->nom );
    Autoriser_client ( client );
    Info_new( Config.log, Cfg_ssrv.lib->Thread_debug, LOG_INFO,
             "Tester_autorisation: Autorisation sent for %s", client->util->nom );
    Client_mode (client, ENVOI_HISTO);
  }
/*--------------------------------------------------------------------------------------------------------*/
