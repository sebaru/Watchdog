/**********************************************************************************************************/
/* Watchdogd/Serveur/ident.c        Gestion du logon user sur module Client Watchdog                      */
/* Projet WatchDog version 2.0       Gestion d'habitat                      ven 03 avr 2009 19:46:29 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * ident.c
 * This file is part of <Watchdog>
 *
 * Copyright (C) 2009 - sebastien
 *
 * <Watchdog> is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * <Watchdog> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <Watchdog>; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

 #include <glib.h>
 #include <bonobo/bonobo-i18n.h>
 #include <string.h>
 
 #include "sysconfig.h"
 #include "Reseaux.h"
 #include "Db.h"
 #include "Config.h"
 #include "Client.h"
 #include "Utilisateur_DB.h"
 #include "watchdogd.h"

 extern struct CONFIG Config;            /* Parametre de configuration du serveur via /etc/watchdogd.conf */
 
/********************************* Définitions des prototypes programme ***********************************/
 #include "proto_srv.h"

 extern struct PARTAGE *Partage;                             /* Accès aux données partagées des processes */
/**********************************************************************************************************/
/* Autoriser_client: Autorise le client à se connecter                                                    */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 static void Autoriser_client ( gint Id_serveur, struct CLIENT *client )
  { struct REZO_SRV_IDENT ident;
    g_snprintf( ident.comment, sizeof(ident.comment), "Serveur Watchdog %s", VERSION );
    if ( Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_AUTORISE,
                       (gchar *)&ident, sizeof(struct REZO_SRV_IDENT) ) )
     { return; }
    Raz_login_failed( Config.log, client->Db_watchdog, client->util->id );
  }
/**********************************************************************************************************/
/* Proto_set_password: changement de password                                                             */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 void Proto_set_password ( gint Id_serveur, struct CLIENT *client, struct CMD_UTIL_SETPASSWORD *util )
  { if (util->id != client->util->id) Client_mode ( client, DECONNECTE );
    else { if (Set_password( Config.log, client->Db_watchdog,
                             Config.crypto_key, util ))
            { Autoriser_client( Id_serveur, client );
              Client_mode( client, ENVOI_DONNEES );
            }
           else Client_mode ( client, DECONNECTE );
         }
  }
/**********************************************************************************************************/
/* Tester_autorisation: envoi de l'autorisation ou non au client                                          */
/* Entrée/Sortie: rien                                                                                    */
/**********************************************************************************************************/
 gint Tester_autorisation ( gint Id_serveur, struct CLIENT *client )
  { struct CMD_ID_UTILISATEUR util;
    gchar *clef, *crypt;
    gint id;

    clef = Recuperer_clef( Config.log, client->Db_watchdog,
                           client->ident.nom, &id );
    if (!clef)
     { Info_c( Config.log, DEBUG_CONNEXION,
               _("Tester_autorisation: Unable to retrieve the key of user"), client->ident.nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_REFUSE, NULL, 0 );
       return(DECONNECTE);
     }
          
    client->util = Rechercher_utilisateurDB( Config.log, client->Db_watchdog, id );
    if (!client->util)
     { Info_c( Config.log, DEBUG_CONNEXION,
               _("Tester_autorisation: Unable to retrieve the user"), client->ident.nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_REFUSE, NULL, 0 );
       return(DECONNECTE);
     }
    memcpy( client->util->code, clef, sizeof( client->util->code ) );    
    g_free(clef);
/***************************************** Identification du client ***************************************/
    crypt = Crypter( Config.log, Config.crypto_key, client->ident.password );
    if (!crypt)
     { Info_c( Config.log, DEBUG_CRYPTO,
               _("Tester_autorisation: Password encryption error"), client->util->nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_REFUSE, NULL, 0 );
       return(DECONNECTE);
     }

    if (memcmp( crypt, client->util->code, sizeof( client->util->code ) ))       /* Comparaison des codes */
     { Info_c( Config.log, DEBUG_CONNEXION, 
               _("Tester_autorisation: Password error"), client->util->nom );
       Info_c( Config.log, DEBUG_CONNEXION, 
               _("Tester_autorisation: Password error"), client->util->code );
       Info_c( Config.log, DEBUG_CONNEXION, 
               _("Tester_autorisation: Password error"), crypt );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_REFUSE, NULL, 0 );
       g_free(crypt);
       Ajouter_one_login_failed( Config.log, client->Db_watchdog,  /* Dommage ! */
                                 client->util->id, Config.max_login_failed );
       return(DECONNECTE);
     }
    g_free(crypt);

/********************************************* Compte du client *******************************************/
    if (!client->util->actif)                              /* Est-ce que son compte est toujours actif ?? */
     { Info_c( Config.log, DEBUG_CONNEXION, 
               _("Tester_autorisation: Account disabled"), client->util->nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_ACCOUNT_DISABLED, NULL, 0 );
       return(DECONNECTE);
     }

    if (client->util->expire && client->util->date_expire<time(NULL) )   /* Expiration temporel du compte */
     { Info_c( Config.log, DEBUG_CONNEXION,
               _("Tester_autorisation: Account expired"), client->util->nom );
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_ACCOUNT_EXPIRED, NULL, 0 );
       return(DECONNECTE);
     }

    if (client->util->changepass)                       /* L'utilisateur doit-il changer son mot de passe */
     { Info_c( Config.log, DEBUG_CONNEXION, 
               _("Tester_autorisation: User have to change his password"), client->util->nom );
       util.id = client->util->id;
       Envoi_client( client, TAG_CONNEXION, SSTAG_SERVEUR_CHANGEPASS,
                     (gchar *)&util, sizeof(struct CMD_ID_UTILISATEUR) );
       return(ATTENTE_NEW_PASSWORD);
     }
    Autoriser_client ( Id_serveur, client );
    return( ENVOI_DONNEES );
  }
/*--------------------------------------------------------------------------------------------------------*/
