/**********************************************************************************************************/
/* Client/protocole_connexion.c    Gestion du protocole_connexion pour la connexion au serveur Watchdog   */
/* Projet WatchDog version 2.0       Gestion d'habitat                       mar 07 avr 2009 21:11:25 CEST */
/* Auteur: LEFEVRE Sebastien                                                                              */
/**********************************************************************************************************/
/*
 * protocole_connexion.c
 * This file is part of Watchdog
 *
 * Copyright (C) 2010 - Sébastien Lefevre
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

/******************************************** Prototypes de fonctions *************************************/
 #include "watchdogd.h"
 #include "Satellite.h"

/**********************************************************************************************************/
/* Gerer_protocole: Gestion de la communication entre le serveur et le client                             */
/* Entrée: la connexion avec le serveur                                                                   */
/* Sortie: Kedal                                                                                          */
/**********************************************************************************************************/
 void Satellite_Gerer_protocole_connexion ( struct CONNEXION *connexion )
  { switch ( Reseau_ss_tag ( connexion ) )
     { case SSTAG_SERVEUR_PULSE:
             { /*Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_DEBUG, 
                        "Satellite_Gerer_protocole_connexion : SSTAG_SERVEUR_PULSE" );*/
             }
            break;
       case SSTAG_SERVEUR_CLI_VALIDE:
             { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE, 
                        "Satellite_Gerer_protocole_connexion : SSTAG_SERVEUR_CLI_VALIDE" );
             }
            break;
       case SSTAG_SERVEUR_OFF:
             { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE, 
                        "Satellite_Gerer_protocole_connexion : SSTAG_SERVEUR_OFF" );
               Satellite_Deconnecter();
             }
            break;
       case SSTAG_SERVEUR_REFUSE:
             { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR, 
                        "Satellite_Gerer_protocole_connexion : Connection refused" );
               Satellite_Deconnecter_sale();
             }
            break;
       case SSTAG_SERVEUR_ACCOUNT_DISABLED:
             { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                        "Satellite_Gerer_protocole_connexion : User account is disabled" );
               Satellite_Deconnecter();
             }
            break;
       case SSTAG_SERVEUR_ACCOUNT_EXPIRED:
             { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_WARNING,
                        "Satellite_Gerer_protocole_connexion : User account is expired" );
               Satellite_Deconnecter();
             }
            break;
       case SSTAG_SERVEUR_PWDCHANGED:
             { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR,
                        "Satellite_Gerer_protocole_connexion : User need ChangePasswd. Weird." );
               Satellite_Deconnecter();
             }
            break;
       case SSTAG_SERVEUR_CANNOTCHANGEPWD:
             { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR, 
                        "Satellite_Gerer_protocole_connexion : SSTAG_SERVEUR_CANNOTCHANGEPWD" );
               Satellite_Deconnecter();
             }
            break;
       case SSTAG_SERVEUR_NEEDCHANGEPWD:
             { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_ERR, 
                        "Satellite_Gerer_protocole_connexion : User need ChangePasswd. Weird for a Satellite..." );
               Satellite_Deconnecter();
             }
            break;
       case SSTAG_SERVEUR_AUTORISE:
             { Info_new( Config.log, Cfg_satellite.lib->Thread_debug, LOG_NOTICE, 
                        "Satellite_Gerer_protocole_connexion : Connection granted !" );
               Cfg_satellite.Mode = SAT_CONNECTED;
             }
            break;
     }
  }
/*--------------------------------------------------------------------------------------------------------*/
